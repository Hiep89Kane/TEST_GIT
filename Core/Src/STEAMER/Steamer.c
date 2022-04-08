#include "myHeader.h"

STEAMER_Struct_t KETTLE;
static uint8_t	allRelays_OldStt;

/*******************************************************STATIC FUNCTIONS ********************************************************************************************************/
static void ST_OffPwr_Run(STEAMER_Struct_t *Steamer);
static void ST_IdePwr_Run(STEAMER_Struct_t *Steamer);
static void ST_ReducePwr_Run(STEAMER_Struct_t *Steamer, uint8_t new_Power);
static void ST_FullPwr_Run(STEAMER_Struct_t *Steamer);
//=======================================================//
static void STEAMER_SectionOFF(STEAMER_Struct_t *Steamer);
static void STEAMER_SectionON(STEAMER_Struct_t *Steamer);
static void STEAMER_SectionFILL(STEAMER_Struct_t *Steamer);
//======================================================//
static void STEAMER_AutoFix_DryBoiler(STEAMER_Struct_t *Steamer);
static void STEAMER_AdjPower(STEAMER_Struct_t *Steamer, uint8_t boilerTempC_threshold);
static void STEAMER_Check_Control(STEAMER_Struct_t *Steamer, AF_Struct_t *AF_Tmp);
static void	STEAMER_TimerDebugErr(STEAMER_Struct_t *Steamer);

//================================================================================================================================//
static void ST_OffPwr_Run(STEAMER_Struct_t *Steamer){
	Steamer->Power_Level = 0;
	ST_AIRPUMP_Off();
}

static void ST_IdePwr_Run(STEAMER_Struct_t *Steamer){
	Steamer->Power_Level = 0;
}

static void ST_ReducePwr_Run(STEAMER_Struct_t *Steamer, uint8_t new_Power){
	Steamer->Power_Level = new_Power;
}

static void ST_FullPwr_Run(STEAMER_Struct_t *Steamer){
	Steamer->Power_Level = 100;
}

static void STEAMER_SectionFILL(STEAMER_Struct_t *Steamer){
																												debug_msg("\nSTEAMER_SectionFILL");
	Steamer->RepCauseOFF = _ST_WAIT_DETECTWATER; //add thông tin để gửi cho DCS trước
	Steamer->state = _STATE_FILL_BOILERWATER;

	ST_SOL_Blink(Steamer->Sol_repeat_cnt);
	timer_set(&Steamer->_timer_ConfirmSensor, _TIME_FILL_CONFIRM);
}

static void STEAMER_SectionOFF(STEAMER_Struct_t *Steamer){
																												debug_msg("\nSTEAMER_SectionOFF");
	BUZ_Beep(1);
	Steamer->state = _STATE_NOCTRL;
	Steamer->activity = _ST_CTRL_OFF;
	Steamer->f_HeatRunOk = _CLEAR;
	Steamer->f_CheckAirpumpOk = _CLEAR;
	Steamer->backup_Current = _CLEAR;

	ST_SOL_Off();
	ST_OffPwr_Run(Steamer);

	timer_stop(&Steamer->_timeout_Service);
	timer_stop(&Steamer->_timer_ConfirmSensor);
	timer_stop(&Steamer->_timer_RepeatCheckRun);

	//neu OFF do cac loi [_ST_ERR_CURRT -> _ST_TIMEOUT_DRAIN_OFF] thi bat timer debug loi len
	if(Steamer->RepCauseOFF == _ST_OFF_OK)
		timer_stop(&Steamer->_timer_STCommon);
	else if (_LIMIT(Steamer->RepCauseOFF, _ST_ERR_CURRT, _ST_ERR_NONE_STLOAD)){
		timer_set(&Steamer->_timer_STCommon, _TIME_BLINK_DEBUG);
		//chỉ lưu lại những lỗi quan trọng từ 0x01 đến 0x08
		if(Steamer->RepCauseOFF <= 0x08){
			ST_Report_Flash[Steamer->RepCauseOFF-1]++;//thông kê lỗi
			Flash_SetTimer_SaveReport();
		}
	}
}

static void STEAMER_SectionON(STEAMER_Struct_t *Steamer){
	uint8_t minutes_IDE_WATER;

	BUZ_Beep(1);
	Steamer->state = _STATE_ON;
	Steamer->RepCauseOFF = _ST_ON_OK;
	Steamer->f_HeatRunOk = _CLEAR;

	//khi airpump da bat roi thi ko kiem tra nua , tranh truong hop steam on 2 lan lien tiep
	if(ST_AIRPUMP_Stt()){
		Steamer->f_CheckAirpumpOk = _SET;
		Steamer->backup_Current = 0;
	}else {
		Steamer->f_CheckAirpumpOk = _CLEAR;
		Steamer->backup_Current = TotalCurrent_mA;
	}

	ST_RLOldSS_Off();
	ST_SOL_Off();
	timer_stop(&Steamer->_timer_ConfirmSensor);

	if(Steamer->tempC < 35) minutes_IDE_WATER=5;
	else {
		if(ST_Flash[_IDF_ST_POWER_TRIAC] == _POWER_DIMER_P1) minutes_IDE_WATER=4;
		else if(ST_Flash[_IDF_ST_POWER_TRIAC] == _POWER_DIMER_P2) minutes_IDE_WATER=3;
		else if(ST_Flash[_IDF_ST_POWER_TRIAC] == _POWER_DIMER_P3) minutes_IDE_WATER=2;
		else minutes_IDE_WATER=4;
	}
	timer_set(&Steamer->_timer_STCommon, _TIME_AUTOCHECK_WATER(minutes_IDE_WATER));
	timer_set(&Steamer->_timer_RepeatCheckRun, 2*CLOCK_SECOND); //Sau khi run 2s mới có thể xác định đc khô nước hay không
	if(Steamer->_timeout_Service.status == _timer_off)
		timer_set(&Steamer->_timeout_Service, ST_Flash[_IDF_ST_TIMESERVICE]*10*CLOCK_MINUTE);
																												debug_msg("\nSTEAMER_SectionON=>Mid=%u=>IDE=%u mins",ADC_Avg_Arr[_ID_ADC_PROBE_WSENSOR],minutes_IDE_WATER);
}

static void STEAMER_AdjPower(STEAMER_Struct_t *Steamer, uint8_t boilerTempC_threshold){
	 //repeat every 2s
 	 if(Steamer->tempC >= boilerTempC_threshold)
 		 ST_ReducePwr_Run(Steamer, ST_Flash[_IDF_ST_POWER_TRIAC]);//50%
 	 else
 		 ST_FullPwr_Run(Steamer); //100%
}

static void STEAMER_AutoFix_DryBoiler(STEAMER_Struct_t *Steamer){
	static uint8_t f_WterFill_3times_Dry = 0; // khi phát hiện khô nước 2 lần thì tự động fill 3 lần để bẫy lỗi

	if(Steamer->RepCauseOFF != _ST_ERR_BOILER_DRY) return;

	if(++f_WterFill_3times_Dry >= 2){
			Steamer->ADC_SSProbes_Delta = 0;
			f_WterFill_3times_Dry = 0;
			ST_SOL_FillFixDry(); //chủ động bơm thêm 1 ít nước khi gap loi kho nuoc 3 lan
			delay_ms(500);
																												debug_msg("\nFill when dry 2 times");
	}
	else Steamer->ADC_SSProbes_Delta += 300;	//tự động giảm ngưỡng Sensor xuống 1 ít

}

static void STEAMER_Check_Control(STEAMER_Struct_t *Steamer, AF_Struct_t *AF_Tmp){
	static uint8_t retry_Dry;
	uint8_t _RL_other ;
	uint16_t Current_Min_CheckBoilerDry;

	//kiểm tra điều kiện bắt đầu
	if(rbi(AF_Tmp->Info_AF4[_BYTE2], _INFO_PROBLEM_CURR_OVERLOAD_Bit) == _FAIL)	Steamer->RepCauseOFF = _ST_ERR_CURRT;
	if(Steamer->safety_stt == _HIGH)											Steamer->RepCauseOFF = _ST_ERR_SAFETY;
	if(Steamer->tempC == 0xFF)													Steamer->RepCauseOFF = _ST_ERR_BOILER_PTC;
	if(Steamer->tempC_Diode == 0xFF)											Steamer->RepCauseOFF = _ST_ERR_DIODE_PTC;
	if(_LIMIT(Steamer->tempC_Diode, _TEMPC_DIODE_SO_HOT, 110))					Steamer->RepCauseOFF = _ST_ERR_DIODE_FAN;
	if(timer_expired(&Steamer->_timeout_Service)){
		timer_stop(&Steamer->_timeout_Service);

																		#ifndef _STEAMER_NONE_STOP
																				Steamer->RepCauseOFF = _ST_TIMEOUT_SERVICE_OFF;
																				return;
																		#endif

	}

	//Chỉ khi đang Run mới check lỗi Dry boiler
	if(Steamer->state != _STATE_ON) return;

	//Tiến hành Bug lỗi khác
	if(timer_expired(&Steamer->_timer_RepeatCheckRun)){
		timer_set(&Steamer->_timer_RepeatCheckRun, 2*CLOCK_SECOND);

		//Check nguồn 110v AC đầu vào
		if(Steamer->AC_Duty == 0) {
			Steamer->RepCauseOFF = _ST_ERR_NONE_ACSOURCE;
																													debug_msg("=>none ACSOURCE");
			return;
		}

		//Check Airpump hoạt động bình thường mới cho bật heat lên
		if(Steamer->f_CheckAirpumpOk == _CLEAR){
																													debug_msg("=>Check AIRPUMP %u-%u",TotalCurrent_mA,Steamer->backup_Current);
			//nếu ko có sự thay đổi các tải thì mới debug lỗi này
			if(allRelays_OldStt == (AF_Tmp->Info_AF4[_BYTE1] & 0x3F)) {
				//Có tín hiệu nguồn điện AC , dòng điện ko tăng lên
				if(TotalCurrent_mA < (Steamer->backup_Current + _CUR_DETECT_AIRPUMP/*mA*/)) {
					Steamer->RepCauseOFF=_ST_ERR_NONE_AIRPUMP;
																													debug_msg("=>Err None AIRPUMP");
					return;
				}
			}
																													debug_msg("=>AIRPUMP Ok");
			Steamer->f_CheckAirpumpOk = _SET;
			ST_ReducePwr_Run(Steamer, ST_Flash[_IDF_ST_POWER_TRIAC]);
			return;
		}

		//Phải đạt dòng điện hoạt động tối thiểu P1 trước khi kiểm tra lỗi Dry , ngược lại là ko căm tải heat
		if(Steamer->f_HeatRunOk == _CLEAR){
			if(TotalCurrent_mA > _CUR_ST_P1){
				Steamer->f_HeatRunOk = _SET;
				retry_Dry = 3;
																													debug_msg("=>f_HeatRunOk=1");
			}
			else {
				Steamer->RepCauseOFF = _ST_ERR_NONE_STLOAD;
																													debug_msg("=>None STLOAD");
			}
			return;
		}
		//Sau khi đã chạy OK => kiểm tra tiếp lỗi Err Dry
		else
		{
			//chỉ Detect khi công suất >= P1
			if(Steamer->Power_Level >= _POWER_DIMER_P1)
			{
				//RL Air,Drain,Whirl,Com => kiểm tra từng trường hợp riêng biệt
				_RL_other = AF_Tmp->Info_AF4[_BYTE1] & 0x0F;
				Current_Min_CheckBoilerDry = _CUR_ST_P1/3;
				switch(_RL_other)
				{
					case 0b00001001:
						if(TotalCurrent_mA < (_CUR_AIR_USER + 0 + 0 + Current_Min_CheckBoilerDry)){
																													debug_msg("%u-%u-%u=>1001_BOILER_DRY",Steamer->state,Steamer->Power_Level,TotalCurrent_mA);
							goto ___label_STEAMER_ERR_BOILER_DRY;
						}
						break;
					case 0b00001101:
						if(TotalCurrent_mA < (_CUR_AIR_USER + _CUR_DRAIN + 0 + Current_Min_CheckBoilerDry)){
																													debug_msg("%u-%u-%u=>1101_BOILER_DRY",Steamer->state,Steamer->Power_Level,TotalCurrent_mA);
							goto ___label_STEAMER_ERR_BOILER_DRY;
						}
						break;
					case 0b00001011:
						if(TotalCurrent_mA < (_CUR_AIR_USER + 0 + _CUR_WHIRL + Current_Min_CheckBoilerDry)){
																													debug_msg("%u-%u-%u=>1011_BOILER_DRY",Steamer->state,Steamer->Power_Level,TotalCurrent_mA);
							goto ___label_STEAMER_ERR_BOILER_DRY;
						}
						break;
					case 0b00001111:
						if(TotalCurrent_mA < (_CUR_AIR_USER + _CUR_DRAIN + _CUR_WHIRL + Current_Min_CheckBoilerDry)){
																													debug_msg("%u-%u-%u=>1111_BOILER_DRY",Steamer->state,Steamer->Power_Level,TotalCurrent_mA);
							goto ___label_STEAMER_ERR_BOILER_DRY;
						}
						break;
				}//end switch
			}
		}
		//STEAMER run all PASS
		Steamer->RepCauseOFF = _ST_ON_OK;
		STEAMER_AdjPower(Steamer, ST_Flash[_IDF_ST_TEMPTHRES]);
	}//end timer_expired

	return;
//====================================================================================================================================================
___label_STEAMER_ERR_BOILER_DRY :
																													debug_msg("\nretry_Dry%u=%u",retry_Dry,TotalCurrent_mA);
	timer_set(&Steamer->_timer_RepeatCheckRun, CLOCK_SECOND/2); //tăng tốc độ Detect lên
	if(--retry_Dry==0){
	   Steamer->RepCauseOFF = _ST_ERR_BOILER_DRY;
	   timer_stop(&Steamer->_timer_RepeatCheckRun);
   }
}

static void	STEAMER_TimerDebugErr(STEAMER_Struct_t *Steamer){
	//báo lỗi ở đây => _TIME_BLINK_DEBUG
	if(timer_expired(&Steamer->_timer_STCommon)){
		timer_restart(&Steamer->_timer_STCommon);
			BUZ_Beep(Steamer->RepCauseOFF);
			LED_Blue_STDebug(Steamer->RepCauseOFF);
			debug_msg("\nST_stt=%x ",Steamer->RepCauseOFF);
			if(Steamer->RepCauseOFF == _ST_OFF_OK) 															debug_msg("OFF OK");
			else if(Steamer->RepCauseOFF == _ST_ERR_CURRT) 													debug_msg("CURRT");
			else if(Steamer->RepCauseOFF == _ST_ERR_SAFETY) 												debug_msg("SAFETY");
			else if(Steamer->RepCauseOFF == _ST_ERR_CLOGGED) 												debug_msg("CLOGGED Water");
			else if(Steamer->RepCauseOFF == _ST_ERR_BOILER_DRY) 											debug_msg("DRY");
			else if(Steamer->RepCauseOFF == _ST_ERR_BOILER_PTC) 											debug_msg("PTC BOILER");
			else if(Steamer->RepCauseOFF == _ST_ERR_DIODE_PTC) 												debug_msg("PTC HeatSink");
			else if(Steamer->RepCauseOFF == _ST_ERR_DIODE_FAN) 												debug_msg("HOT HeatSink");
			else if(Steamer->RepCauseOFF == _ST_ERR_NONE_AIRPUMP)											debug_msg("NONE AIRPUMP");
			else if(Steamer->RepCauseOFF == _ST_TIMEOUT_SERVICE_OFF) 										debug_msg("TIMEOUT_SERVICE_OFF");
			else if(Steamer->RepCauseOFF == _ST_ERR_NONE_ACSOURCE) 											debug_msg("None AC source");
			else if(Steamer->RepCauseOFF == _ST_ERR_NONE_STLOAD) 											debug_msg("None Steamer Load");
			else if(Steamer->RepCauseOFF == _ST_WAIT_DETECTWATER) 											debug_msg("WAIT_DETECTWATER");
			else if(Steamer->RepCauseOFF == _ST_ON_OK) 														debug_msg("ON OK");
	}
}
/*//=========================================== STEAMER FUNCTION =======================================================================*/
void STEAMER_init(STEAMER_Struct_t *Steamer){
	uint64_t u64_tmp;

	//read Flash
	u64_tmp = Flash_ReadDWord(ADDR_FLASH_PAGE_63 + 8*_IDF_DWORD_ST);
	Separate_from_DWord(ST_Flash, u64_tmp);

	Steamer->activity = 0;
	Steamer->state = 0;
	Steamer->ADC_SSProbes_Delta = 0;

	Steamer->f_HeatRunOk = _CLEAR;
	Steamer->f_CheckAirpumpOk = _CLEAR;
	allRelays_OldStt = 0;

	//kiem tra Flash
	ST_Flash[_IDF_ST_TEMPTHRES] 	= check_Flash_TempThreshold(ST_Flash[_IDF_ST_TEMPTHRES]);
	ST_Flash[_IDF_ST_TIMESERVICE] 	= check_Flash_TimeSevice(ST_Flash[_IDF_ST_TIMESERVICE]);
	ST_Flash[_IDF_ST_BLINKSOL] 		= _BLINK_STSOL_DEFAULT;//check_Flash_BlinkSolenoid(ST_Flash[_IDF_ST_BLINKSOL]);
	ST_Flash[_IDF_ST_ENSAFETY] 		= check_Flash_Safety(ST_Flash[_IDF_ST_ENSAFETY]);
	ST_Flash[_IDF_ST_POWER_TRIAC] 	= check_Flash_PowerTriac(ST_Flash[_IDF_ST_POWER_TRIAC],_POWER_DIMER_P1,_POWER_DIMER_P3);
																											debug_msg("\nSteam init TempC=%u'C timesvice=%umins SolBlink=%u Safety=%u Dim=%u",
																														ST_Flash[_IDF_ST_TEMPTHRES],
																														ST_Flash[_IDF_ST_TIMESERVICE],
																														ST_Flash[_IDF_ST_BLINKSOL],
																														ST_Flash[_IDF_ST_ENSAFETY],
																														ST_Flash[_IDF_ST_POWER_TRIAC]);
}

/** @Purpose Hàm sử dụng để điều khiển Steamer
  * @brief 	- Hàm thực thi lệnh và trả về các trạng thái OK hoặc lỗi ...
	*					- Kiểm tra các điều kiện để hoạt động như safety device , sensor nuoc
	*
  * @param  1. Biến Struct của thư viện Steamer
	*		2. Yêu cầu điều khiển  : _ST_CTRL_OFF ,_ST_CTRL_ON
  * @retval trả về các Status
  */
void STEAMER_Set(STEAMER_Struct_t *Steamer, STEAM_Ctrl_TypeDef Set_activity){

	switch(Set_activity){
		case _ST_CTRL_OFF:
			Steamer->RepCauseOFF = _ST_OFF_OK; 	//clear variable for OFF
			Steamer->activity = _ST_CTRL_OFF;
			break;
		case _ST_CTRL_ON:
			Steamer->RepCauseOFF = _ST_ON_OK;	//clear variable for ON
			Steamer->activity = _ST_CTRL_ON;	//bat
			break;
		default :
			return;
	}
	Steamer->state	= _STATE_START;
}

void STEAMER_Activity_Task(STEAMER_Struct_t *Steamer, AF_Struct_t *AF_Tmp){

	switch(Steamer->state){
		case _STATE_NOCTRL:
			STEAMER_TimerDebugErr(Steamer);
			return;

		case _STATE_START:
			if(Steamer->activity == _ST_CTRL_OFF) goto ___label_STEAMER_DO_OFF;
			else
			{
																												debug_msg("\nCheck_start");
				//kiểm tra điều kiện bắt đầu
				STEAMER_Check_Control(Steamer, AF_Tmp);
				//Lỗi thì OFF ngay
				if(_IS_ERR_AUTO_OFF(Steamer->RepCauseOFF)){
																												debug_msg("\nStart OFF=%u",Steamer->RepCauseOFF);
					goto ___label_STEAMER_DO_OFF;
				}
				//bật relay Old Sensor để Detect
				ST_RLOldSS_On(ST_Flash[_IDF_ST_BLINKSOL]*(_TIME_FILL_ON + _TIME_FILL_OFF) + 80/*800ms later*/);
				timer_set(&Steamer->_timer_STCommon, CLOCK_SECOND/2); 						//chờ relay sensor bật lên

				Steamer->state = _STATE_CHECK_PROBE_WATERSS;
			}
			break;

		case _STATE_CHECK_PROBE_WATERSS:
			if(timer_expired(&Steamer->_timer_STCommon)){
				timer_stop(&Steamer->_timer_STCommon);
				if(Steamer->waterStt == _ProbeWterSS_FULL){
					goto ___label_STEAMER_DO_ON;
				}
				else if(Steamer->waterStt == _ProbeWterSS_DRY){
					goto ___label_STEAMER_PREPARE_FILLWATER;
				}
			}
			break;

		case _STATE_FILL_BOILERWATER:
			if(timer_expired(&Steamer->_timer_ConfirmSensor)){
				timer_stop(&Steamer->_timer_ConfirmSensor);
																												debug_msg("\r\ncnt=%u Mid=%u-Cmp=%u", Steamer->Sol_repeat_cnt, ADC_Avg_Arr[_ID_ADC_PROBE_WSENSOR],Steamer->ADC_SSProbes_Compare);
				if(Steamer->waterStt == _ProbeWterSS_FULL) goto ___label_STEAMER_DO_ON;

				if(--Steamer->Sol_repeat_cnt == 0){
					Steamer->RepCauseOFF = _ST_ERR_CLOGGED;														debug_msg("=>Water is clogged");
					goto ___label_STEAMER_DO_OFF;
				}
			}

			if(_CTRL_SOLST.state == _OUTPUT_state_ON){
				if(Steamer->_timer_ConfirmSensor.status == _timer_off)
					timer_set(&Steamer->_timer_ConfirmSensor, _TIME_FILL_CONFIRM);
			}
			break;

		case _STATE_ON:
			STEAMER_Check_Control(Steamer, AF_Tmp);

			if(_IS_ERR_AUTO_OFF(Steamer->RepCauseOFF)){
				STEAMER_AutoFix_DryBoiler(Steamer);
																												debug_msg("\nERR_AUTO_OFF=%u",Steamer->RepCauseOFF);
				goto ___label_STEAMER_DO_OFF;
			}
			//timeout kiem tra sensor sau thời gian nhất định
			if(timer_expired(&Steamer->_timer_STCommon)){
				ST_IdePwr_Run(Steamer);
				Steamer->state = _STATE_IDE_WATER;
				timer_stop(&Steamer->_timer_RepeatCheckRun);
				timer_set(&Steamer->_timer_STCommon, _TIME_WAIT_QUIET); //chờ ổn định nước
																												debug_msg("\n_STATE_IDE_WATER");
			}
			break;

		case _STATE_IDE_WATER:
			if(timer_expired(&Steamer->_timer_STCommon)){
				Steamer->state = _STATE_START;
				timer_stop(&Steamer->_timer_STCommon);
				Steamer->ADC_SSProbes_Delta = 0; //gặp tình huống này coi như Boiler ko bị Dry => trở lại bình thường
			}
			break;
	}

	return ;
/*====================================================================================================================================*/
//Nhảy vào đây thực hiện 1 lần duy nhất sau đó chuyển trạng thái ngay
___label_STEAMER_PREPARE_FILLWATER:
		Steamer->Sol_repeat_cnt =  ST_Flash[_IDF_ST_BLINKSOL]; //lấy số lần blink solenoid
		STEAMER_SectionFILL(Steamer);
		goto ___label_RS232_STEAM_RESPONSE;

		return;

___label_STEAMER_DO_ON:
		STEAMER_SectionON(Steamer);
		if(Steamer->f_CheckAirpumpOk == _CLEAR) {
			ST_AIRPUMP_On();
			wbi(AF_Tmp->Info_AF4[_BYTE1], _INFO_RLAIR_Bit,1);
			allRelays_OldStt = AF_Tmp->Info_AF4[_BYTE1] & 0x3F;
		}else ST_ReducePwr_Run(Steamer, ST_Flash[_IDF_ST_POWER_TRIAC]); //bắt đầu từ mức công suất thấp

		goto ___label_RS232_STEAM_RESPONSE;
		return ;

___label_STEAMER_DO_OFF:
		STEAMER_SectionOFF(Steamer);
		goto ___label_RS232_STEAM_RESPONSE;
		return;

//Tự động gửi Rs232 cho DCS nếu Steamer bất kì khi nào ON hoặc OFF
___label_RS232_STEAM_RESPONSE :
		if(mainRS232.f_Skip_AutoRep == _TRUE){mainRS232.f_Skip_AutoRep=_FALSE; return;} //not Reply rs232
		mainRS232.TxData = _RS232_REPONSE_VAL(_FUNC_STEAM, Steamer->RepCauseOFF); 		//lay 4 bit cao la Function + 4 bit thap => TxData goi di
		mainRS232.Backup_RxCmd = _RS232LL_AfCmd_CtrlFuncDCS;
		mainRS232.state = _RS232LL_WAIT_FINISHED;
		//tạm thời không auto Rep trước khi phản hồi lênh ON/OFF
		mainRS232.f_Skip_AutoRep=_TRUE;
}

//10ms poll systick
void STEAMER_GetStt_Systick(STEAMER_Struct_t *Steamer, __IO uint16_t ADC_Input_Arr[]){
	uint16_t  Adc_Threshold;

	Steamer->tempC 			= ADC_2TempC(ADC_Input_Arr[_ID_ADC_PTC_BOILER]);
	Steamer->tempC_Diode 	= ADC_2TempC(ADC_Input_Arr[_ID_ADC_PTC_DIODE]);
	Adc_Threshold        	= ADC_get_WterProbeThreshold(Steamer->tempC);
	if(Steamer->tempC < 33) Steamer->ADC_SSProbes_Delta = 0; //ko fix khi nhiet do < 31 độ C (lúc này ko bị Dry)
	Steamer->ADC_SSProbes_Compare = Adc_Threshold - Steamer->ADC_SSProbes_Delta;

	Steamer->waterStt = ADC_2WaterStt(RL_SS_STEAM_STT, _ADC_LOGIC_COMPARE(ADC_Input_Arr[_ID_ADC_PROBE_WSENSOR], Steamer->ADC_SSProbes_Compare));

	if(ST_Flash[_IDF_ST_ENSAFETY] == _ST_SAFETY_DIS) {Steamer->safety_stt = _LOW; return;}
	else Steamer->safety_stt = Systick_ReadInput_poll(_Input_Safety_Logic, SAFETY_PinStt, 20, 20);
}

void STEAMER_UpdateDuty(Edge_ReturnStatus edge_pulse,STEAMER_Struct_t *Steamer){
/*Note : frequency 60 hz - 50hz => duty 16ms-20ms
 * Use Opto EL814 :
 * Input pulse--->        	 	...____|________________|________________|______...
 *           			       		   <---------Full Sin wave----------->
 *           			       		   <------------16ms-20ms------------>
 *           			       		   <-----8-10ms---->|<-----8-10ms---->
 * Ac_Duty 	  --->                     <---80--100us--->                       =>Actually Ac_Duty = 79(with 60hz) or 98(with 50hz)
 *
 */

	if(edge_pulse != _RISING) return;
	// chon canh len
	Steamer->AC_Duty = Steamer->AC_Duty_cnt;
	Steamer->AC_Duty_cnt = 0;

}

void STEAMER_DimerCtrl_periodic_poll(STEAMER_Struct_t *Steamer){
	//timer interrupt 100us

	unsigned int  x1;

	//Measure the Duty of EL814 (= half duty of Sin wave)
	if(++Steamer->AC_Duty_cnt >= (_AC_FREQUENCY_50HZ + 20)){
		Steamer->AC_Duty = 0;
		Steamer->AC_Duty_cnt = 0;
		return;
	}
	else STEAMER_UpdateDuty(Get_Edge(INPUT_AC_STT), Steamer);

	//case Triac OFF
	if((Steamer->state != _STATE_ON) || (Steamer->Power_Level < 10))
	{
		DIMMER_CTRL_OFF;
		if(RL_COM_STT) FAN_OFF; //Chi khi Relay COM bat len moi tat Fan de bay loi sau khi khoi dong
		return;
	}

	FAN_ON;

	//case Triac ON full
	if(Steamer->Power_Level >= 100){
		DIMMER_CTRL_ON;
		return;
	}

	//case Triac ON dimmer
	x1 = Steamer->AC_Duty - (Steamer->Power_Level*Steamer->AC_Duty)/100;

	if(_LIMIT(Steamer->AC_Duty_cnt,x1,x1+10)) DIMMER_CTRL_ON; //10 ~ _ZERO_AC_POINT ~ 1ms
	else DIMMER_CTRL_OFF;
}

void STEAMER_Main_exe(void){

	if(reCalib_ZeroCurrent !=0) return;

	STEAMER_Activity_Task(&KETTLE, &AF_BOX4);
}
