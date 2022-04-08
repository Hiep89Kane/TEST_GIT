#include "myHeader.h"

//AT+NAME=Elite_02$0D$0A
#define  							_AT				"AT"
#define								_AT_DEFAULT	  	"AT+DEFAULT"
#define								_AT_BAUD	  	"AT+BAUD"
#define								_AT_RESET	  	"AT+RESET"
#define								_AT_TYPE	  	"AT+TYPE"
#define								_AT_ROLE	  	"AT+ROLE"
#define								_AT_POWE	  	"AT+POWE"
#define								_AT_NAME	  	"AT+NAME"
#define								_AT_LADDR  		"AT+LADDR"
#define								_AT_UUID	  	"AT+UUID"
#define								_AT_DISC	  	"AT+DISC"
#define								_AT_ADVEN	  	"AT+ADVEN"
#define								_AT_ADVI	  	"AT+ADVI"
#define								_AT_NINTERVAL	"AT+NINTERVAL"
#define								_AT_PIN	  		"AT+PIN"
#define								_AT_VERSION	 	"AT+VERSION"
//								_AT_CHAR[]	  	={"AT+CHAR"},
//								_AT_RSLV[]	  	={"AT+RSLV"},
//								_AT_CONN[]	  	={"AT+CONN"},
//								_AT_CONA[]	  	={"AT+CONA"},
//								_AT_BAND[]	  	={"AT+BAND"},
//								_AT_CLRBAND[]	  ={"AT+CLRBAND"},
//								_AT_GETDCN[]	  ={"AT+GETDCN"},
//								_AT_HELP[]	    ={"AT+HELP"},

#define								_AT_SLEEP	    "AT+SLEEP"

struct timer		_BLE_timeout;
uint8_t 			BLE_RxBuff[BLE_BUFFER_MAX];
BLERxStt_TypeDef 	BLE_RX_Stt;

static BLEStt_TypeDef 	BLE_SendCmd(char *AT_StrCmd, char Param);
static void 			BLE_RstBuffer();
/*===========================================================================================*/
static BLEStt_TypeDef BLE_SendCmd(char *AT_StrCmd, char Param){
	char str_send[50];

	if(Param == 0)sprintf(str_send,"%s\r\n",AT_StrCmd);
	else if(isnumber(Param)){sprintf(str_send,"%s%c\r\n",AT_StrCmd,Param);}
	else return _BLE_ERR;

	uart3_puts(str_send);
	return _BLE_OK;
}

static void BLE_RstBuffer(){
	BLE_RxBuff[0] = 0;
	BLE_RX_Stt = _BLE_NONE;
}

void BLE_GetBuffer(char RxData){
	static uint8_t cntBuff;

	if(!isreadable(RxData) || cntBuff >= BLE_BUFFER_MAX) BLE_RstBuffer();

	switch(BLE_RX_Stt){
		case _BLE_NONE:
			if(RxData == '@'){
				timer_set(&_BLE_timeout, CLOCK_SECOND/2);
				cntBuff = 0;
				BLE_RX_Stt = _BLE_BUSY;
			}
			break;
		case _BLE_BUSY:
			if(RxData == '#'){
				BLE_RxBuff[cntBuff] = 0;
				cntBuff = 0;
				BLE_RX_Stt = _BLE_FINISHED;

			}
			else {
				BLE_RxBuff[cntBuff] = RxData;
				cntBuff++;
				return;
			}
			break;
		default:break;
	}
}

BLECtrl_Typedef BLE_GetCtrl(STEAMER_Struct_t *Steamer){
	uint8_t stt , *str_tmp ,getNummer ;

	//truong BUSY qua lau thi xoa di
	if(timer_expired(&_BLE_timeout)){
		timer_stop(&_BLE_timeout);
		BLE_RX_Stt = _BLE_NONE;
		return _BLECTRL_NULL;
	}

	if(BLE_RX_Stt != _BLE_FINISHED) return 0;

	if(strcmp(BLE_RxBuff, _STEAM_ON)==0) 			stt = _BLECTRL_STEAM_ON;
	else if(strcmp(BLE_RxBuff, _STEAM_OFF)==0) 		stt = _BLECTRL_STEAM_OFF;
	else if(strcmp(BLE_RxBuff, _STEAM_ENSAFE)==0) 	stt = _BLECTRL_ST_SAFETY_EN;
	else if(strcmp(BLE_RxBuff, _STEAM_DISSAFE)==0)	stt = _BLECTRL_ST_SAFETY_DIS;
	else if(strncmp(BLE_RxBuff, _STEAM_SETTEMP, strlen(_STEAM_SETTEMP))==0) {
		str_tmp = &BLE_RxBuff[strlen(_STEAM_SETTEMP)];
		getNummer = Str2Num(str_tmp);
		if(_LIMIT(getNummer,0,100)){
			ZeroPulse_delay = getNummer;
																								debug_msg("set fix ZeroPulse=%u", getNummer);
			stt = _BLECTRL_SETTEMPC;
		}
	}
	else if(strncmp(BLE_RxBuff, _STEAM_SETPWR, strlen(_STEAM_SETPWR))==0) {
//		str_tmp = &BLE_RxBuff[strlen(_STEAM_SETPWR)];
//		getNummer = Str2Num(str_tmp);
//		if(_IS_DIMER_POWER(getNummer,15,100)){
//			if(getNummer == ST_Flash[_IDF_ST_POWER_TRIAC])
//				if(++ST_Flash[_IDF_ST_POWER_TRIAC] > RxPowerCtrl[3]) ST_Flash[_IDF_ST_POWER_TRIAC] = RxPowerCtrl[1];
//			else ST_Flash[_IDF_ST_POWER_TRIAC] = getNummer;
//																								debug_msg("PWR=%u", ST_Flash[_IDF_ST_POWER_TRIAC]);
//			stt = _BLECTRL_SETPWR;
//		}
	}
	else if(strcmp(BLE_RxBuff, _STEAM_DEBUGADC)==0) stt = _BLECTRL_ADCDEBUG;
	//AF
	else if(strcmp(BLE_RxBuff, _AUTOFILL_REV)==0) stt = _BLECTRL_AUTOFILL_REV;
	else if(strcmp(BLE_RxBuff, _AUTOFILL_ON)==0) stt = _BLECTRL_AUTOFILL_ON;
	else if(strcmp(BLE_RxBuff, _AUTOFILL_OFF)==0) stt = _BLECTRL_AUTOFILL_OFF;
	else if(strcmp(BLE_RxBuff, _MASSAGE_REV)==0) stt = _BLECTRL_MASSAGE_REV;
	else stt = _BLECTRL_NULL;

	BLE_RX_Stt = _BLE_NONE;

	return stt;
}

void BLE_Ctrl_Task(uint8_t cmd, STEAMER_Struct_t *Steamer, AF_Struct_t *AF_Tmp){

	switch(cmd){
		case _BLECTRL_STEAM_ON:
			STEAMER_Set(Steamer, _ST_CTRL_ON);
			break;
		case _BLECTRL_STEAM_OFF:
			STEAMER_Set(Steamer, _ST_CTRL_OFF);
			goto ___label_BLE_RESPONSE;
			break;
		case _BLECTRL_ST_SAFETY_EN:
			ST_Flash[_IDF_ST_ENSAFETY] = _ST_SAFETY_EN;
			Flash_SetTimer();
			goto ___label_BLE_RESPONSE;
			break;
		case _BLECTRL_ST_SAFETY_DIS:

			ST_Flash[_IDF_ST_ENSAFETY] = _ST_SAFETY_DIS;
			Flash_SetTimer();
			goto ___label_BLE_RESPONSE;
			break;
		case _BLECTRL_SETTEMPC:
			Flash_SetTimer();
			goto ___label_BLE_RESPONSE;
			break;
		case _BLECTRL_SETPWR:
			Flash_SetTimer();
			goto ___label_BLE_RESPONSE;
			break;
		case _BLECTRL_ADCDEBUG:
			OUTPUT_set_blink(&_CTRL_RLOldSS,1,CLOCK_MINUTE,1,0);
			break;

		case _BLECTRL_AUTOFILL_REV:
			if(AF_Tmp->Section == _ENABLE)	{
				AF_Tmp->JetState = _AF_CONTROL_DISABLE ;
			}else{
				AF_Tmp->JetState = _AF_CONTROL_ENABLE ;
			}
			break;
		case _BLECTRL_AUTOFILL_ON:
			AF_Tmp->JetState = _AF_CONTROL_ENABLE ;
			break;
		case _BLECTRL_AUTOFILL_OFF:
			AF_Tmp->JetState = _AF_CONTROL_DISABLE ;
			break;
		case _BLECTRL_MASSAGE_REV:
			mainRS232.state=_RS232LL_WAIT_FINISHED;
			mainRS232.Backup_RxCmd=_RS232LL_AfCmd_RevMassage;
			break;
	}
return ;

___label_BLE_RESPONSE :
	if(Steamer->RepCauseOFF == _ST_ON_OK){
																								debug_msg("=>_ST_ON_OK");
	}else {
																								debug_msg("=>_ST_Err%u",Steamer->RepCauseOFF);
	}
}

void BLE_init(){
	uint64_t u64_tmp;

	 BLE_EN_ON;
	 //BLE_SendCmd(_AT_RESET,0);

	u64_tmp = Flash_ReadDWord(ADDR_FLASH_PAGE_63+ 8*_IDF_DWORD_ST_REPORT);
	//if(u64_tmp == 0xFFFFFFFFFFFFFFFF) {u64_tmp = 0; Flash_SetTimer();} //nếu lần đầu chưa ghi gì hết => xóa tất cả về 0
	Separate_from_DWord(ST_Report_Flash, u64_tmp);
}

void WIFI_Ctrl_Task(button_status_t *btn_tmp, uint8_t lever, RS232_Struct_t *RS232_tmp){

	LED1_Blink_Revision();

	RS232_tmp->state = _RS232LL_START;

	//WHIRLPOOL
	if(btn_tmp->_01){
		if(lever != 0){
			RS232_tmp->RxCmd=_RS232LL_AfCmd_CtrlFuncDCS;
			RS232_tmp->RxData=_FUNC_WHIRL_ON;
		}else {
			RS232_tmp->RxCmd=_RS232LL_AfCmd_CtrlFuncDCS;
			RS232_tmp->RxData=_FUNC_WHIRL_OFF;
		}
		//uart_printf("Btn1\n\r");
	}

	//STEAM HEAT
	if(btn_tmp->_02){
		if(lever != 0){
			RS232_tmp->RxCmd=_RS232LL_AfCmd_CtrlFuncDCS;
			RS232_tmp->RxData=_FUNC_STEAM_ON;
		}else {
			RS232_tmp->RxCmd=_RS232LL_AfCmd_CtrlFuncDCS;
			RS232_tmp->RxData=_FUNC_STEAM_OFF;
		}
		//uart_printf("Btn2\n\r");
	}

	//AURORA
	if(btn_tmp->_03){
		if(lever != 0){
			RS232_tmp->RxCmd=_RS232LL_AfCmd_CtrlFuncLeds;
			RS232_tmp->RxData=_FUNC_RGB_ON;
		}else {
			RS232_tmp->RxCmd=_RS232LL_AfCmd_CtrlFuncLeds;
			RS232_tmp->RxData=_FUNC_RGB_OFF;
		}
		//uart_printf("Btn3\n\r");
	}

	//LIGHT
	if(btn_tmp->_04){
		if(lever != 0){
			RS232_tmp->RxCmd=_RS232LL_AfCmd_CtrlFuncLeds;
			RS232_tmp->RxData=_FUNC_SPOT_ON;
		}else {
			RS232_tmp->RxCmd=_RS232LL_AfCmd_CtrlFuncLeds;
			RS232_tmp->RxData=_FUNC_SPOT_OFF;
		}
		//uart_printf("Btn4\n\r");
	}

	//MASSAGE
	if(btn_tmp->_05){
		RS232_tmp->Backup_RxCmd = _RS232LL_AfCmd_RevMassage;
		RS232_tmp->TxData = 0x00;
		RS232_tmp->state = _RS232LL_WAIT_FINISHED;
//		if(lever != 0){
//			RS232_tmp->RxCmd=_RS232LL_AfCmd_RevMassage;
//		}else {
//			RS232_tmp->RxCmd=_RS232LL_AfCmd_RevMassage;
//		}
		//uart_printf("Btn5\n\r");
	}

}

void WIFI_Ctrl_Task_Bt(button_status_t btn_tmp, uint8_t lever){
	WIFI_Ctrl_Task(&btn_tmp, lever, &mainRS232);
}
