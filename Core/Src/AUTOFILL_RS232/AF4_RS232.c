#include "myHeader.h"

#define _AF4_RS232_RETRY_NUM			4	//số lần retry update BYTE0 => DCS
#define _RETRY_CHECK_ERR_CURRENT		4	//nếu lỗi 4 lần liên tục mới khẳng định

#define _TIME_CHECK_ERR_CURRENT			(CLOCK_SECOND/2)
#define _TIME_CHECK_ERR_CURRENT_FASTER	(CLOCK_SECOND/20)

#define GET_WTER_LV(Rx_Epprom1)			(Rx_Epprom1 & 0b00001111) 					//bit 3 2 1 0
#define GET_RGB_CTRL(Rx_Epprom1)		((Rx_Epprom1 & 0b11110000)>>4) 				//bit 6 5 4 3

#define GET_STEAM_TIME(Rx_Epprom2)		(Rx_Epprom2 & 0b00001111) 					//bit 3 2 1 0
#define GET_SET_SAFETTY(Rx_Epprom2)		((Rx_Epprom2 & 0b00010000)>>4) 				//bit 4
#define GET_SET_POWER(Rx_Epprom2)		(Rx_Epprom2 >>5) 							//bit 7 6 5

#define GET_VAL_UPDATE(AF_infor_byte0)	((AF_infor_byte0) & (0b11110000)) 			//chi update khi 4 bit cao thay doi
#define GET_RELAYSTT(AF_infor_byte2)	((AF_infor_byte2) & (0b00111111))			//bo qua stt cua solenoid

/*global variables*/
RS232_Struct_t	mainRS232;
const uint8_t 	RxPowerCtrl[_DEF_POWER_TOTAL_NUMMER+1]={50, _POWER_DIMER_P1, _POWER_DIMER_P2, _POWER_DIMER_P3};//[_POWER_DIMER_MIN - _POWER_DIMER_MAX]

uint8_t 		Old_UpdateVal;
uint8_t			Local_RxPower_compare=1; 	//[1-3]

struct timer 	_timeout_RS232_Wait,		// Han che thoi gian Wait qua lau
				_timeout_delayRelay;		// Het timeout nay moi chac chan relay duoc ON

static struct timer _timer_RetryUpdateDCS;	// neu chu dong goi len DCS ma DCS ko tra loi ve thi retry 3 lan
static struct timer _timer_ActiveCalibSS;	// phai gui lenh "_RS232LL_AfCmd_PrepareConfig" va "_RS232LL_AfCmd_CalibCapSS" moi dc calib

static uint8_t	find_RxPowerInArr(const uint8_t *ArrPower, uint8_t Power_actualy);
static void autoRepDCS_AF4_Byte0(uint8_t *old_value, uint8_t AF4info_Byte0 ,RS232_Struct_t *rs232_result);
static void get_CurrentStt_Byte2(uint8_t allRelaysStt,__IO uint8_t *AF4info_Byte2 ,uint16_t total_Current);
static CheckStatus check_EveryLoads(uint8_t allRelaysStt, uint16_t total_Current);
//==============================================================================================================================================
static uint8_t	find_RxPowerInArr(const uint8_t ArrPower[], uint8_t Pwr_Flash){
	uint8_t i;

	for(i=1;i <= _DEF_POWER_TOTAL_NUMMER;i++){
		if((Pwr_Flash) == ArrPower[i]) return (i);
	}
	return 0;
}

void AF4_RS232_Task(RS232_Struct_t *RS232_tmp, AF_Struct_t *AF_Tmp, STEAMER_Struct_t *Steamer){
	uint8_t get_AF_WterLV,
			get_AF_RGB,
			get_STtime,
			get_STsafe,
			get_ST_Rxpower,
			getSpot,
			CheckRxData;

	uint8_t	val_EppRom1,
			val_EppRom2;

	/*Kiem tra RxBuffer ==================================================================================*/
	if(RS232LL_RxCheck_Cmd_Data(_RS232LL_AddrMe, &RS232_tmp->RxCmd, &RS232_tmp->RxData)){
		//lenh giong nhau thi bo qua , khac nhau thi bat dau lai tu dau
		if(_timeout_RS232_Wait.status == _timer_off){
			RS232_tmp->state = _RS232LL_START;
																										debug_msg("\nAF_RX=%x%x",RS232_tmp->RxCmd,RS232_tmp->RxData);
		}
	}

	/*Kiem tra State ==================================================================================*/
	switch(RS232_tmp->state){
		case _RS232LL_NONE :
			RS232_tmp->Backup_RxCmd = 0;
			RS232_tmp->TxData = 0;
			return;
			break;
		case _RS232LL_START :
			RS232_tmp->Backup_RxCmd = RS232_tmp->RxCmd;
			goto	___label_RS232_CHECKCMD;
			break;

		case _RS232LL_WAIT_GETSTT:
			if(timer_expired(&_timeout_RS232_Wait)){
				timer_stop(&_timeout_RS232_Wait);
				RS232_tmp->state = _RS232LL_NONE;
			}
			return;
			break;

		case _RS232LL_WAIT_FINISHED:
			//TxData da duoc cap nhat sau khi thuc hien xong
			timer_stop(&_timeout_RS232_Wait);
			goto	___label_RS232_REPLY;
			break;
	}
	return;

/*Thuc hien 1 lan roi thoat ra **********************************************************************/
___label_RS232_CHECKCMD:

	RS232_tmp->TxData =	RS232_tmp->RxData;
	RS232_tmp->f_Skip_AutoRep = _FALSE;

	switch(RS232_tmp->Backup_RxCmd){
		//0x20-----------------------------------------------------------------------------
		case _RS232LL_AfCmd_ClickSW:
			if(RS232_tmp->RxData == 0xFF){
				if(AF_Tmp->Section == _ENABLE){
					AF_Tmp->JetState = _AF_CONTROL_DISABLE ;
				}else{
					AF_Tmp->JetState = _AF_CONTROL_ENABLE ;
				}
				RS232_tmp->TxData = _RS232_TX_OK;
			}
			else
				RS232_tmp->TxData = _RS232_TX_FAIL;

			goto ___label_RS232_REPLY;
			break;

		//0x21-----------------------------------------------------------------------------
		case _RS232LL_AfCmd_PrepareConfig:
			RS232_tmp->f_Skip_AutoRep = _TRUE;

			if(RS232_tmp->RxData == 0xFF){
				//all off
				SPOTLIGHT.SpotPwr = 0;
				CtrlRGB_Spot(_CTRLSPOT_EFFECT_FADING, SPOTLIGHT.SpotPwr);
				AF_SectionOFF(AF_Tmp);
				AF_CtrlRGB(&RGB_2020, _AFRGB_OFF, AF_Flash[_IDF_RGB_MASTERCTRL]);
				if(Steamer->activity==_ST_CTRL_ON) STEAMER_Set(Steamer, _ST_CTRL_OFF);
				timer_set(&_timer_ActiveCalibSS, CLOCK_MINUTE);
			}
			else if(RS232_tmp->RxData == 0x00) timer_stop(&_timer_ActiveCalibSS);

			goto ___label_RS232_REPLY;
			break;

		//0x22-----------------------------------------------------------------------------
		case _RS232LL_AfCmd_CalibCapSS:
			if(RS232_tmp->RxData == 0xFF){
				if(_timer_ActiveCalibSS.status == _timer_on) {
					timer_restart(&AF_Tmp->_timer_Check_CapSS);
					AF_CalibAllCapSS();
				}
				goto ___label_RS232_WAIT;
			}
			else RS232_tmp->TxData = _RS232_TX_FAIL;

			goto ___label_RS232_REPLY;
			break;

		//0x23-----------------------------------------------------------------------------
		case _RS232LL_AfCmd_SetEPPROM1:
			//DCS ack
			if(RS232_tmp->RxData == 0x00){
				val_EppRom1 = (AF_Flash[_IDF_RGB_MASTERCTRL]<<4) + AF_Flash[_IDF_WTER_LEVEL];
				RS232_tmp->TxData = val_EppRom1;
				goto ___label_RS232_REPLY;
			}

			//DCS set new value
			get_AF_WterLV = GET_WTER_LV(RS232_tmp->RxData); //1-5
			get_AF_RGB 	  = GET_RGB_CTRL(RS232_tmp->RxData);//0-7

			if(_IS_WATERLEVER(get_AF_WterLV) && _IS_AFCTRLRGB(get_AF_RGB)){
				//neu ko thay doi thi tra ve
				if((AF_Flash[_IDF_WTER_LEVEL] == get_AF_WterLV) &&
						(AF_Flash[_IDF_RGB_MASTERCTRL] == get_AF_RGB)) goto ___label_RS232_REPLY;
                //giá trị mới
				AF_Flash[_IDF_WTER_LEVEL] = get_AF_WterLV;
				AF_Flash[_IDF_RGB_MASTERCTRL] = get_AF_RGB;
				Flash_SetTimer();
				//khi RGB = OFF thi chi luu lai chu ko dieu khien
				if(RGB_2020.EffectColor != _CTRLRGB_EFFECT_OFF) AF_CtrlRGB(&RGB_2020, _AFRGB_ON, AF_Flash[_IDF_RGB_MASTERCTRL]);
			} else RS232_tmp->TxData = _RS232_TX_FAIL;

			goto ___label_RS232_REPLY;
			break;

		//0x24-----------------------------------------------------------------------------
		case _RS232LL_AfCmd_SetEPPROM2:
			//DCS ack
			if(RS232_tmp->RxData == 0x00){
				get_STtime = ST_Flash[_IDF_ST_TIMESERVICE];
				get_STsafe = ST_Flash[_IDF_ST_ENSAFETY];
				get_ST_Rxpower = find_RxPowerInArr(RxPowerCtrl, ST_Flash[_IDF_ST_POWER_TRIAC]); //get_STpower[1-3] tuong ung ST_Flash[_IDF_ST_POWER_TRIAC]
				if(_LIMIT(get_ST_Rxpower,1,_DEF_POWER_TOTAL_NUMMER)) Local_RxPower_compare = get_ST_Rxpower; //update val

				val_EppRom2 = (get_ST_Rxpower<<5) + (ST_Flash[_IDF_ST_ENSAFETY]<<4) + ST_Flash[_IDF_ST_TIMESERVICE];
				RS232_tmp->TxData = val_EppRom2;
				goto ___label_RS232_REPLY;
			}

			//DCS set new value
			get_STtime = GET_STEAM_TIME(RS232_tmp->RxData); //(1-6)~(10-60 mins)
			get_STsafe = GET_SET_SAFETTY(RS232_tmp->RxData);//0 or 1
			get_ST_Rxpower = GET_SET_POWER(RS232_tmp->RxData); //[1-3]

			if(_IS_TIMEOUT_SERVICE(get_STtime) && _LIMIT(get_ST_Rxpower,1,_DEF_POWER_TOTAL_NUMMER)){
				//neu ko thay doi thi tra ve
				if((ST_Flash[_IDF_ST_TIMESERVICE] == get_STtime) &&
					(ST_Flash[_IDF_ST_ENSAFETY] == get_STsafe) &&
					(get_ST_Rxpower == Local_RxPower_compare))	goto ___label_RS232_REPLY;
				//Update gia trị mới
				ST_Flash[_IDF_ST_TIMESERVICE] = get_STtime;
				ST_Flash[_IDF_ST_ENSAFETY] = get_STsafe;
				ST_Flash[_IDF_ST_POWER_TRIAC] = RxPowerCtrl[get_ST_Rxpower]; //get new Flash
				Local_RxPower_compare = get_ST_Rxpower; //update val

				Flash_SetTimer();

			} else RS232_tmp->TxData = _RS232_TX_FAIL;

			goto ___label_RS232_REPLY;
			break;

		//0x25-----------------------------------------------------------------------------
		case _RS232LL_AfCmd_CtrlFuncDCS:
			CheckRxData = RS232_tmp->RxData & 0xF0; //kiem tra 4 bit cao

			//Whirlpool
			if(CheckRxData == _FUNC_WHIRL){
				if(RS232_tmp->RxData == _FUNC_WHIRL_OFF){
					AF_WHIRL_Off();
					wbi(AF_Tmp->Info_AF4[_BYTE0], _INFO_AF4_WHIRLON_Bit, _DISABLE);
					//Nếu RGB,Solenoid cũng tắt thì chuyển section off
					if((rbi(AF_Tmp->Info_AF4[_BYTE0], _INFO_RGB_STT_Bit) == _AFRGB_OFF)
							&& (rbi(AF_Tmp->Info_AF4[_BYTE1], _INFO_SOLAF_Bit) == 0)) AF_Tmp->Section = _DISABLE;
				}
				else if(RS232_tmp->RxData == _FUNC_WHIRL_ON){
					if((CAPSENSOR[_CAPSS_ID_WATER].logicStt == _LOW) && (AF_Tmp->Users_active == _ACTIVE) && (RL_DRAIN_STT==0)){
						AF_WHIRL_On();
						wbi(AF_Tmp->Info_AF4[_BYTE0], _INFO_AF4_WHIRLON_Bit, _ENABLE);
						AF_Tmp->Section = _ENABLE;
					}
					else RS232_tmp->TxData = _RS232_REPONSE_VAL(_FUNC_WHIRL, _RESULT_FUNC_FAIL);
				}
				else RS232_tmp->TxData = _RS232_REPONSE_VAL(_FUNC_WHIRL, _RESULT_FUNC_FAIL);
			}
			//Steamer
			else if(CheckRxData == _FUNC_STEAM){
				if(RS232_tmp->RxData == _FUNC_STEAM_OFF){
					STEAMER_Set(Steamer, _ST_CTRL_OFF);
					goto ___label_RS232_WAIT;
				}
				else if(RS232_tmp->RxData == _FUNC_STEAM_ON){
					STEAMER_Set(Steamer, _ST_CTRL_ON);
					goto ___label_RS232_WAIT;
				}
			}
			else RS232_tmp->TxData = _RS232_TX_FAIL;

			goto ___label_RS232_REPLY;
			break;

		//0x26-----------------------------------------------------------------------------
		case _RS232LL_AfCmd_CtrlFuncLeds:
					CheckRxData = RS232_tmp->RxData & 0xF0; //kiem tra 4 bit cao
					if(CheckRxData == _FUNC_RGB){
						if(RS232_tmp->RxData == _FUNC_RGB_OFF){
							AF_CtrlRGB(&RGB_2020, _AFRGB_OFF, AF_Flash[_IDF_RGB_MASTERCTRL]);
							//Nếu Whirlpool,Solenoid cũng tắt thì chuyển section off
							if((rbi(AF_Tmp->Info_AF4[_BYTE0], _INFO_AF4_WHIRLON_Bit) == _DISABLE)
									&& (rbi(AF_Tmp->Info_AF4[_BYTE1], _INFO_SOLAF_Bit) == 0)) AF_Tmp->Section = _DISABLE;
						}else if(RS232_tmp->RxData == _FUNC_RGB_ON){
							AF_CtrlRGB(&RGB_2020, _AFRGB_ON, AF_Flash[_IDF_RGB_MASTERCTRL]);
							//neu Whirlpool dang OFF thi chuyen qua manual
							if(rbi(AF_Tmp->Info_AF4[_BYTE0], _INFO_AF4_WHIRLON_Bit) == 0) RGB_2020.Mode = _CTRLRGB_MODE_DCSMANUAL;
						}else RS232_tmp->TxData = _RS232_REPONSE_VAL(RS232_tmp->RxData, _RESULT_FUNC_FAIL);
					}
					else if(CheckRxData == _FUNC_SPOT)
					{
						if(RS232_tmp->RxData == _FUNC_SPOT_OFF){
							SPOTLIGHT.SpotPwr = 0;
						}else{
							getSpot = (RS232_tmp->RxData & 0x0F);
							if(getSpot >= 0x0A) SPOTLIGHT.SpotPwr = 100;
							else SPOTLIGHT.SpotPwr = getSpot*10;
						}
						CtrlRGB_Spot(_CTRLSPOT_EFFECT_FADING, SPOTLIGHT.SpotPwr);
					}
					else RS232_tmp->TxData = _RS232_TX_FAIL;

					RS232_tmp->f_Skip_AutoRep = _TRUE; //ko can tu dong cap nhat byte0

					goto ___label_RS232_REPLY;
					break;

		//0x26-----------------------------------------------------------------------------
//		case _RS232LL_AfCmd_RevMassage:
//
//			goto ___label_RS232_REPLY;
//			break;

		//0x2F-----------------------------------------------------------------------------
		case _RS232LL_AfCmd_RstFactory:
			if(RS232_tmp->RxData == 0xFF){
				AF_Flash[_IDF_WTER_LEVEL] = _AF_LV_DEFAULT;
				AF_Flash[_IDF_RGB_MASTERCTRL] = _AFCTRLRGB_FULLCOLOR;
				ST_Flash[_IDF_ST_TIMESERVICE] = _TIMEOUT_SERVICE_DEFAULT;
				ST_Flash[_IDF_ST_ENSAFETY] = _ST_SAFETY_DEFAULT;
				Flash_SetTimer();
			}else RS232_tmp->TxData = _RS232_TX_FAIL;

			goto ___label_RS232_REPLY;
			break;

		//0x30-----------------------------------------------------------------------------
		case _RS232LL_AfCmd_GetByte0:
			//truong hop AF_up len va da duoc nhan OK thi khoi tra loi
			if(_timer_RetryUpdateDCS.status == _timer_on){
				timer_stop(&_timer_RetryUpdateDCS);
				RS232_tmp->state = _RS232LL_NONE;
				return;
			}

			if(RS232_tmp->RxData == 0xFF)
				RS232_tmp->TxData = (uint8_t) (AF_Tmp->Info_AF4[_BYTE0]);
			else
				RS232_tmp->TxData = _RS232_TX_FAIL;

			goto ___label_RS232_REPLY;
			break;

		//0x31-----------------------------------------------------------------------------
		case _RS232LL_AfCmd_GetByte1:
			if(RS232_tmp->RxData == 0xFF)
				RS232_tmp->TxData = (uint8_t)(AF_Tmp->Info_AF4[_BYTE1]);
			else
				RS232_tmp->TxData = _RS232_TX_FAIL;

			goto ___label_RS232_REPLY;
			break;

		//0x32-----------------------------------------------------------------------------
		case _RS232LL_AfCmd_GetByte2:
			if(RS232_tmp->RxData == 0xFF)
				RS232_tmp->TxData =  (uint8_t)(AF_Tmp->Info_AF4[_BYTE2]);
			else
				RS232_tmp->TxData = _RS232_TX_FAIL;

			goto ___label_RS232_REPLY;
			break;
	}
	return;
//==========================================================================================================================
___label_RS232_WAIT:
	timer_set(&_timeout_RS232_Wait, 3*CLOCK_SECOND);
	RS232_tmp->state = _RS232LL_WAIT_GETSTT;
	return;

___label_RS232_REPLY:
	RS232LL_TxReponse(_RS232LL_I_am, 1, RS232_tmp->Backup_RxCmd, RS232_tmp->TxData);
	RS232_tmp->state = _RS232LL_NONE;
																									debug_msg("\nAF_TX=%x%x",RS232_tmp->Backup_RxCmd,RS232_tmp->TxData);
	return;
}

/**	@Purpose Cập nhật tất cả trạng thái của Info_AF4 sau mỗi chu kỳ
  * @brief 	-gồm tất cả 3 BYTE[0],[1],[2]
  * BYTE[0] :
  * 		bit 0 : water sensor connect
  * 		bit 1 : water sensor logic
  * 		bit 2 : drain sensor connect
  * 		bit 3 : drain sensor logic
  * 		bit 4 : RGB ON/OFF					==> ko cần update
  * 		bit 5 : solenoid AutoFill
  * 		bit 6 : solenoid Bình steamer
  * 		bit 7 : AutoFill Section EN/DIS		==> ko cần update
  *BYTE[1] :
  * 		bit 0 : Relay Common ON/OFF
  * 		bit 1 : Relay Whirl ON/OFF
  * 		bit 2: 	Relay Drain ON/OFF
  * 		bit 3: 	Relay Air ON/OFF
  * 		bit 4: 	Relay Steamer Dimmer working
  * 		bit 5: 	Relay Steamer Full power working
  *			bit 6: 	Solenoid Autofill ON/OFF
  * 		bit 7: 	Solenoid Steamer ON/OFF
  *BYTE[2] :
  * 		bit 0 : _INFO_PROBLEM_CURR_WRONG_Bit => Khi Bật riêng từng tải và nếu thấy current bất thường thì bật bit này lên
  * 		bit 1 : _INFO_PROBLEM_CURR_LEAKAGE0_Bit => Chưa bật bất kỳ Relay nào nhưng đã có dòng điện => dính relay tổng hoặc dính chì ở AC PCB
  * 		bit 2: 	_INFO_PROBLEM_CURR_LEAKAGE1_Bit => Mới bật Relay tổng(chưa bật Relay phụ) nhưng đã có dòng điện => Dính relay phụ hoặc dính chì
  * 		bit 3: 	_INFO_PROBLEM_CURR_ZERO_Bit => Đã bật Relay tổng và relay phụ nhưng ko có dòng điện => Chưa cấp nguồn AC(chưa gắn AC Inlet) hoặc chưa gắn đủ tải(AC outlet)
  * 		bit 4: 	_INFO_PROBLEM_CURR_OVERLOAD_Bit => Vượt quá dòng điện định mức cho phép 9.5A
  * 		bit 5: 	_INFO_PROBLEM_DISTEMP1_Bit => disconect PTC thermal in Steamer container
  *			bit 6: 	_INFO_PROBLEM_DISTEMP2_Bit => disconect PTC thermal in AC Diode
  * 		bit 7: 	_INFO_PROBLEM_FAN_Bit => Đang hoạt động nhưng Diode quá nóng (do Fan 24v ko hoạt động)
  *
  * @param  Biến Struct của AF4_RS232, AutoFill ,Steamer
  * @retval None
  */
void AF4_RS232_UpdateStt(RS232_Struct_t *RS232_tmp, AF_Struct_t *AF_Tmp, STEAMER_Struct_t *Steamer){

	static uint8_t 	RL_Stt_Old;

/*BYTE 0*************************************************************************************/
	//bit 0
	wbi(AF_Tmp->Info_AF4[_BYTE0], _INFO_SSWATER_CONNECT_Bit, CAPSENSOR[_CAPSS_ID_WATER].connectStt);
	//bit 1
	wbi(AF_Tmp->Info_AF4[_BYTE0], _INFO_SSDRAIN_CONNECT_Bit, CAPSENSOR[_CAPSS_ID_DRAIN].connectStt);
	//bit 2
	wbi(AF_Tmp->Info_AF4[_BYTE0], _INFO_SSWATER_Logic_Bit, CAPSENSOR[_CAPSS_ID_WATER].logicStt);
	//bit 3
	wbi(AF_Tmp->Info_AF4[_BYTE0], _INFO_SSDRAIN_Logic_Bit, CAPSENSOR[_CAPSS_ID_DRAIN].logicStt);
	//bit 4 => đã tự cập nhập trong thư viện AutoFill
	//bit 5
	if(SPOTLIGHT.SpotPwr) wbi(AF_Tmp->Info_AF4[_BYTE0], _INFO_AF4_LIGHTON_Bit, 1);
	else wbi(AF_Tmp->Info_AF4[_BYTE0], _INFO_AF4_LIGHTON_Bit, 0);
	//bit 6
	if(RGB_2020.EffectColor == _CTRLRGB_EFFECT_OFF) wbi(AF_Tmp->Info_AF4[_BYTE0], _INFO_RGB_STT_Bit, _AFRGB_OFF);
	else wbi(AF_Tmp->Info_AF4[_BYTE0], _INFO_RGB_STT_Bit, _AFRGB_ON);
	//bit 7
	wbi(AF_Tmp->Info_AF4[_BYTE0], _INFO_STEAMER_CTRL_Bit, RL_AIR_STT); //khi steamer run chắc chắn Airpump run

/*BYTE 1*******************************************************************************************************/
	//bit 0
	wbi(AF_Tmp->Info_AF4[_BYTE1], _INFO_RLCOM_Bit, RL_COM_STT);
	//bit 1
	wbi(AF_Tmp->Info_AF4[_BYTE1], _INFO_RLWHIRL_Bit, RL_WHIRL_STT);
	//bit 2
	wbi(AF_Tmp->Info_AF4[_BYTE1], _INFO_RLDRAIN_Bit, RL_DRAIN_STT);
	//bit 3
	wbi(AF_Tmp->Info_AF4[_BYTE1], _INFO_RLAIR_Bit, RL_AIR_STT);
	//bit 4,5
	if(Steamer->Power_Level == 0){
		wbi(AF_Tmp->Info_AF4[_BYTE1], _INFO_DIM_POWER_Bit, 0);
		wbi(AF_Tmp->Info_AF4[_BYTE1], _INFO_FULL_POWER_Bit, 0);
	}else if (Steamer->Power_Level == 100){
		wbi(AF_Tmp->Info_AF4[_BYTE1], _INFO_DIM_POWER_Bit, 0);
		wbi(AF_Tmp->Info_AF4[_BYTE1], _INFO_FULL_POWER_Bit, 1);
	}else {
		wbi(AF_Tmp->Info_AF4[_BYTE1], _INFO_DIM_POWER_Bit, 1);
		wbi(AF_Tmp->Info_AF4[_BYTE1], _INFO_FULL_POWER_Bit, 0);
	}
	//bit 6
	wbi(AF_Tmp->Info_AF4[_BYTE1], _INFO_SOLAF_Bit, RL_SOL_COM_STT && FET_SOL_AF_STT);
	//bit 7
	wbi(AF_Tmp->Info_AF4[_BYTE1], _INFO_SOLST_Bit, RL_SOL_COM_STT && FET_SOL_STEAM_STT);

	//khi có thay đổi trạng thái relay thì cần delay 1 thời gian mới cập nhật chính xác các status , vì Relay phản hồi chậm
	if(RL_Stt_Old != (AF_Tmp->Info_AF4[_BYTE1] && 0x3F)) timer_restart(&_timeout_delayRelay);
	RL_Stt_Old = (AF_Tmp->Info_AF4[_BYTE1] && 0x3F);

/*BYTE 2*******************************************************************************************************/
	//bit 16 17 18 19 20 of CURRENT
	get_CurrentStt_Byte2(GET_RELAYSTT(AF_Tmp->Info_AF4[_BYTE1]), &AF_Tmp->Info_AF4[_BYTE2], TotalCurrent_mA);
	//bit 21
	if(Steamer->tempC_Diode > _TEMPC_DIODE_SO_HOT) wbi(AF_Tmp->Info_AF4[_BYTE2], _INFO_PROBLEM_FAN_Bit, _FAIL);
	else  wbi(AF_Tmp->Info_AF4[_BYTE2], _INFO_PROBLEM_FAN_Bit, _PASS);
	//bit 22
	if(Steamer->tempC == 0xFF) wbi(AF_Tmp->Info_AF4[_BYTE2], _INFO_PROBLEM_DISTEMP1_Bit, _FAIL);
	else wbi(AF_Tmp->Info_AF4[_BYTE2], _INFO_PROBLEM_DISTEMP1_Bit, _PASS);
	//bit 23
	if(Steamer->tempC_Diode == 0xFF) wbi(AF_Tmp->Info_AF4[_BYTE2], _INFO_PROBLEM_DISTEMP2_Bit, _FAIL);
	else wbi(AF_Tmp->Info_AF4[_BYTE2], _INFO_PROBLEM_DISTEMP2_Bit, _PASS);
	//End

/*RESULT => sent to DCS the new BYTE0 =======================================================================================================*/
	autoRepDCS_AF4_Byte0(&Old_UpdateVal, AF_Tmp->Info_AF4[_BYTE0], RS232_tmp);
}

static void autoRepDCS_AF4_Byte0(uint8_t *old_value, uint8_t AF4info_Byte0 ,RS232_Struct_t *rs232_result){
	static uint8_t retryUpdate;

	//bỏ qua Auto Update nếu chương trình yêu cầu
	if(rs232_result->f_Skip_AutoRep == _TRUE || (rs232_result->state == _RS232LL_WAIT_GETSTT)) {
		rs232_result->f_Skip_AutoRep=_FALSE;
		return;
	}

	//tiến hành Update byte0 bình thường
	if(*old_value != GET_VAL_UPDATE(AF4info_Byte0)){
		*old_value = GET_VAL_UPDATE(AF4info_Byte0);
		retryUpdate = _AF4_RS232_RETRY_NUM + 1; //bỏ qua lần 1
		timer_set(&_timer_RetryUpdateDCS, _AF_TIME_RETRY_UPDATE_RS232);
	}

	//timer retry
	if(timer_expired(&_timer_RetryUpdateDCS)){
		if(--retryUpdate){
																									debug_msg("\nAF_UP=>");
			rs232_result->state = _RS232LL_WAIT_FINISHED;
			rs232_result->Backup_RxCmd = _RS232LL_AfCmd_GetByte0;
			rs232_result->TxData = AF4info_Byte0; //get byte 0
			timer_restart(&_timer_RetryUpdateDCS);
		}else
			timer_stop(&_timer_RetryUpdateDCS);
	}
}

static void get_CurrentStt_Byte2(uint8_t allRelaysStt,__IO uint8_t *AF4info_Byte2 ,uint16_t total_Current){
	static uint8_t  checkCurr_retry; 					//quyết định những lỗi phải tắt RL tổng để bảo vệ => check 2 lần
	uint8_t all_Current_Pass;

	all_Current_Pass = *AF4info_Byte2 & 0x1F;

	//Phần chương trình xử lý khi Relay tổng chưa bật
	if(allRelaysStt == 0x00 && (all_Current_Pass == 0x1F)){
		//chờ tính toán total_Current sau ít nhất 1s
		if(timer_expired(&_timer_StartGetCurrt))
		{
			timer_stop(&_timer_StartGetCurrt);
			//Khi chưa bật relay nào mà xuất hiện dòng rò
			if(total_Current > _CONST_VALUE_CURRENT_LEAKAGE){
					wbi(*AF4info_Byte2, _INFO_PROBLEM_CURR_LEAKAGE0_Bit, _FAIL);
																										debug_msg("=>CURR_LEAKAGE0");
					return;
			}
			else if(total_Current > (_CONST_VALUE_CURRENT_LEAKAGE/2)) {
																										debug_msg("total_Current>300=>RST");
				NVIC_SystemReset();
			}
			//nếu OK bật Relay tổng lên => set timer delay relay
			Ctrl_RelayCOM(1);
			FAN_OFF;
			checkCurr_retry = _RETRY_CHECK_ERR_CURRENT;
			timer_set(&_timeout_delayRelay, _TIME_CHECK_ERR_CURRENT);
																										debug_msg("=>RLCOM ON");
		}
	}

	//Phần chương trình xử lý khi Relay tổng đã bật
	if(RL_COM_STT==0) return;

	if(timer_expired(&_timeout_delayRelay))
	{
		//check kiểm tra lỗi liên tục
		timer_set(&_timeout_delayRelay, _TIME_CHECK_ERR_CURRENT);
		//check overload mọi lúc khi Relay tổng bật => sau 3 lần thì tắt RL tổng đi =>lưu ý: phải reset lại mới bật lại đc relay tổng (RL_COM)
		if(total_Current > _CONST_VALUE_CURRENT_OVERLOAD){
																										debug_msg("OVERLOAD1_retry%u ",checkCurr_retry);
			timer_set(&_timeout_delayRelay, _TIME_CHECK_ERR_CURRENT_FASTER); //tăng tốc độ detect lên tránh đứt cầu chì ~ 50ms
			if(--checkCurr_retry == 0){
				Ctrl_RelayCOM(0);
				timer_stop(&_timeout_delayRelay);
				wbi(*AF4info_Byte2, _INFO_PROBLEM_CURR_OVERLOAD_Bit, _FAIL);
																										debug_msg("CURR_OVERLOAD_1");
			}
			return;
		}
		//kiem tra AC Inlet
		if(KETTLE.AC_Duty == 0){
			wbi(*AF4info_Byte2, _INFO_PROBLEM_CURR_ZERO_Bit, _FAIL);
																										//debug_msg("\nErr NONE AC SOURCE");
		}else wbi(*AF4info_Byte2, _INFO_PROBLEM_CURR_ZERO_Bit, _PASS);

		//khi chỉ bật RL tổng , RL phụ đều off
		if(allRelaysStt == 0x01)
		{
			//nếu phát hiện dòng rò thì cũng phải tắt RL tổng đi cho an toàn =>lưu ý: phải reset lại mới bật lại đc relay tổng
			if(total_Current > (_CONST_VALUE_CURRENT_LEAKAGE/2)){
																										debug_msg("LEAKAGE1_retry%u ",checkCurr_retry);
				if(--checkCurr_retry == 0){
					if(total_Current < _CONST_VALUE_CURRENT_LEAKAGE) {
																										debug_msg("total_Current>600=>RST");
						NVIC_SystemReset(); //cho phep reset vi dong dien < 0.6A , > 0.6 la bao loi
					}
					Ctrl_RelayCOM(0);
					timer_stop(&_timeout_delayRelay);
					wbi(*AF4info_Byte2, _INFO_PROBLEM_CURR_LEAKAGE1_Bit, _FAIL);
																										debug_msg("CURR_LEAKAGE1");
				}
				return;
			}

			//update gia tri ADC_Calib0mA khi bat Relay tong => de bo qua ca dong ro di qua tu dien snubber
			if(reCalib_ZeroCurrent){
																										debug_msg("\ntotal_Current=%u",total_Current);
				if(reCalib_ZeroCurrent > _RECALIB_COUNT_SAMPLE){reCalib_ZeroCurrent=0;return;}			//bẫy lỗi
				timer_set(&_timeout_delayRelay, _TIME_CHECK_ERR_CURRENT_FASTER);
				//Nguoc lai thi update gia tri moi de duoc gia tri tot
				ADC_Calib0mA = (ADC_getZeroCurrent(ADC_Avg_Arr[_ID_ADC_CURRENT]) + ADC_Calib0mA/*giá trị cũ hơn*/)/2;
				if(--reCalib_ZeroCurrent==0){
					if(total_Current > 50/*mA*/) {
																										debug_msg("total_Current>50=>RST");
						NVIC_SystemReset();
					}
					else LED1_Blink_INIT_CURRENT_SENSOR();//báo thành công
				}
				return;
			}
		}
		//Kiem tra khi có bật RL bất kì relay phụ
		else if(allRelaysStt > 0x01){
				if(total_Current < _CONST_VALUE_CURRENT_RUN_MIN){
					wbi(*AF4info_Byte2, _INFO_PROBLEM_CURR_WRONG_Bit, _FAIL);
																										debug_msg("\nErr CURR_ZERO");
					return;
				}
				/**** Tiep tuc check cac loi tai khac **/
				//kiem tra current wrong bit => mỗi tải hoạt động độc lập thì phải kiểm tra dòng điện đã define trước đó
				wbi(*AF4info_Byte2, _INFO_PROBLEM_CURR_WRONG_Bit, check_EveryLoads(allRelaysStt, total_Current));
			}

		/***all Current PASS in here***/
		checkCurr_retry = _RETRY_CHECK_ERR_CURRENT; //reset Retry
	}//end if timer
}

static CheckStatus check_EveryLoads(uint8_t RelaysStt, uint16_t in_Current){

	switch(RelaysStt){
		//Chỉ Whirl on
		case 0b00000011:
			if(!_LIMIT(in_Current, _CUR_WHIRL_MIN, _CUR_WHIRL_MAX)){
																										debug_msg("\nErr Curr_Whirl");
				return _FAIL;
			}
			break;
		//Chỉ Drain on
		case 0b00000101:
			if(!_LIMIT(in_Current, _CUR_DRAIN_MIN, _CUR_DRAIN_MAX)){
																										debug_msg("\nErr Curr_Drain");
				return _FAIL;
			}
			break;
		//Chỉ TRIMMING va Air on
		case 0b00011001:

			if((Local_RxPower_compare <= 2) && (in_Current > 5500/*mA*/)){
																										debug_msg("\nErr Triac broken");
				return _FAIL;
			}
			break;
	}

	return _PASS;
}

//================================================================================================================================
void 	AF4_RS232_Main_exe(){
	AF4_RS232_Task(&mainRS232, &AF_BOX4, &KETTLE);
	AF4_RS232_UpdateStt(&mainRS232, &AF_BOX4, &KETTLE);
}
