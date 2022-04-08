#include "myHeader.h"

AF_Struct_t 	AF_BOX4;
const uint8_t	WaterLevel_Second_Arr[_AF_LV_MAX]={1,3,6,9,12};

/** @Purpose  Khi khởi động MCU phải init hàm này
  * @brief 	- Reset các biến infor của Control box 4
  * 		- Set các timer cần thiết của AutoFill :
  * 				timer check Sensor,
  * 				timer reset uart,
  * 		- load các dữ liệu Flash
  * 		- Spotlight => Off
  * @param  Biến Struct của thư viện AF
  * @retval None
  */
void AF_Init(AF_Struct_t *AF_Tmp){
	uint64_t u64_tmp;

	CAPSENSOR[_CAPSS_ID_WATER].connectStt = _PASS; //sensor init pass
	CAPSENSOR[_CAPSS_ID_DRAIN].connectStt = _PASS; //sensor init pass

	AF_Tmp->Info_AF4[_BYTE0] = 0x0F; //sensor init pass
	AF_Tmp->Info_AF4[_BYTE1] = 0x00; //All relays set off
	AF_Tmp->Info_AF4[_BYTE2] = 0xFF; //Byte 3 all pass

	AF_Tmp->Users_active = _DEACTIVE;
	AF_Tmp->DrainSS_active = _DEACTIVE;

	timer_set(&AF_Tmp->_timer_Check_CapSS, _AF_TIME_CHECK_CAPSS); //check every sensor
	timer_set(&AF_Tmp->_timer_CheckRx, _AF_TIME_RESET_RS232);

	u64_tmp = Flash_ReadDWord(ADDR_FLASH_PAGE_63+ 8*_IDF_DWORD_AF);
	Separate_from_DWord(AF_Flash, u64_tmp);

	AF_Flash[_IDF_WTER_LEVEL] 		= 	check_Flash_WaterLV(AF_Flash[_IDF_WTER_LEVEL]);
	AF_Flash[_IDF_RGB_MASTERCTRL] 	= 	check_Flash_CtrlRGB(AF_Flash[_IDF_RGB_MASTERCTRL]);
	AF_Flash[_IDF_AF_REVERSION] 	= 	_AF4_REVISION;

	AF_SectionOFF(AF_Tmp);
																							debug_msg("AF3 init V%u,Lv=%u,RGB=%u",AF_Flash[_IDF_AF_REVERSION],AF_Flash[_IDF_WTER_LEVEL],AF_Flash[_IDF_RGB_MASTERCTRL]);
}

//********************************************************** AF CONTROL FUNCTIONS *****************************************************************//
/** @Purpose Hàm sử dụng khi tắt vặn Jet swt để tắt
  * @brief 	- LED			=> OFF
  * 		- Solenoid AF 	=> OFF
  * 		- Whirlpool 	=> OFF
  * 		- RGB 			=> OFF
  *			- _timer_AdjLevel, _timeout_Protect_Solenoid = > stop
  *			- JetState		=> NONE
  *			- AF Section    => OFF
  * @param  Biến Struct của thư viện AF
  * @retval None
  */
void AF_SectionOFF(AF_Struct_t *AF_Tmp){
																							debug_msg("=>AF_SectionOFF");
	AF_Tmp->Section = _DISABLE;
	LED_Set_Off();
	AF_Solenoid_Off();
	AF_WHIRL_Off();
	wbi(AF_Tmp->Info_AF4[_BYTE0], _INFO_AF4_WHIRLON_Bit, _DISABLE);

	//Ở mode manual thì ko tắt LED RGB , mode AutoFill mới đc tắt
	if(RGB_2020.Mode == _CTRLRGB_MODE_AUTOFILL)
	{
		AF_CtrlRGB(&RGB_2020, _AFRGB_OFF, AF_Flash[_IDF_RGB_MASTERCTRL]);
		wbi(AF_Tmp->Info_AF4[_BYTE0], _INFO_RGB_STT_Bit, _AFRGB_OFF);
	}

	timer_stop(&AF_Tmp->_timer_AdjLevel);
	timer_stop(&AF_Tmp->_timeout_Protect_Solenoid);
	timer_stop(&AF_Tmp->_timer_Deactive_DrainSS);

	AF_Tmp->JetState = _AF_CONTROL_NONE;
}

/** @Purpose Hàm sử dụng Sensor báo đầy nước => tắt solenoid => bật Whirlpool + RGB
  * @brief 	- LED			=> OFF
  * 		- Solenoid AF 	=> OFF
  * 		- Whirlpool 	=> ON => chú ý có điều kiện bảo vệ
  * 		- RGB 			=> ON
  *			- _timer_AdjLevel, _timeout_Protect_Solenoid => stop
  *			- JetState		=> NONE
  *			- AF Section    => ON
  * @param  Biến Struct của thư viện AF
  * @retval None
  */
void AF_SectionFINISH(AF_Struct_t *AF_Tmp){

	AF_Tmp->Section = _ENABLE;
	AF_Tmp->Users_active = _ACTIVE;
																													debug_msg("=>AF_SectionFINISH");
	AF_Solenoid_Off();
	if(RL_DRAIN_STT == 0){
		AF_WHIRL_On();
		wbi(AF_Tmp->Info_AF4[_BYTE0], _INFO_AF4_WHIRLON_Bit, _ENABLE);
	}

	//nếu trước đó off thì bật lên , còn ngược lại : trước đó bật rồi thì khỏi bật lại
	if(RGB_2020.EffectColor == _CTRLRGB_EFFECT_OFF){
		AF_CtrlRGB(&RGB_2020, _AFRGB_ON, AF_Flash[_IDF_RGB_MASTERCTRL]);
		wbi(AF_Tmp->Info_AF4[_BYTE0], _INFO_RGB_STT_Bit, _AFRGB_ON);
	}

	timer_stop(&AF_Tmp->_timer_AdjLevel);
	timer_stop(&AF_Tmp->_timeout_Protect_Solenoid);

	AF_Tmp->JetState = _AF_CONTROL_NONE;
}

/**	@Purpose Hàm sử dụng bắt đầu chuyển qua chế độ chờ fill đầy nước
  * @brief 	- LED						=> ON
  * 		- Solenoid AF 				=> OFF
  * 		- Whirlpool 				=> ON
  * 		- RGB 						=> ON
  *			- _timer_AdjLevel			=> set Level*3s
  *			- _timeout_Protect_Solenoid	=> set _AF_TIME_PROTECT_OVERFLOW ~ 5 mins
  *			- JetState    				=> _AF_CONTROL_WAIT
  * @param  Biến Struct của thư viện AF
  * @retval None
  */
void AF_SectionWAIT(AF_Struct_t *AF_Tmp){
	uint8_t get_second_fill;
																								debug_msg("=>AF_SectionWAIT");
	AF_Tmp->Section = _ENABLE;
	LED_Set_On();
	AF_Solenoid_On();

	timer_set(&AF_Tmp->_timeout_Protect_Solenoid, _AF_TIME_PROTECT_OVERFLOW);
	timer_set(&AF_Tmp->_timer_Auto_Off, _AF_TIME_AUTO_OFF); //Set Auto off all
	timer_set(&AF_Tmp->_timer_Deactive_DrainSS, _TIMEOUT_DEACTIVE_DRAINSS);

	if(rbi(AF_Tmp->Info_AF4[_BYTE0],_INFO_SSWATER_Logic_Bit) == _LOW)
	{
		timer_set(&AF_Tmp->_timer_AdjLevel, _AF_TIME_FILL_MORE);
																								debug_msg("=>Fill More 10s");
	}else{
		get_second_fill = WaterLevel_Second_Arr[AF_Flash[_IDF_WTER_LEVEL]-1];
		timer_set(&AF_Tmp->_timer_AdjLevel,_AF_TIME_STOP_FILL(get_second_fill));
																								debug_msg("=>fill %us",get_second_fill);
	}
	AF_Tmp->JetState = _AF_CONTROL_WAIT;
}

/**	@Purpose Hàm Calib sensor
  * @brief	 Calib thành công hay Fail => kiểm tra ở hàm CapSS_Get_Result trong thư viện Sensor
  * @param  None
  * @retval None
  */
void AF_CalibAllCapSS(void){
																								debug_msg("=>Calib Sensors");
	CapSS_CalibSucess = 0;
	CapSS_selected = 0;
	//calib all capSS
	for(uint8_t i = 0; i<_CAPSS_ID_TOTAL; i++){
		CapSS_Start(&CAPSENSOR[i], _SS_CALIB_BYTE);
	}
}

/**
 * Hàm điều khiển RGB của AutoFill
 */
ResultStatus  AF_CtrlRGB(CtrlRGB_TypeDef_t *RGB, AFCtrlRGB_TypeDef_t ON_OFF, AFSetRGB_Typedef ValCtrl){

	if(ON_OFF == _AFRGB_OFF){
		CtrlRGB_Set(RGB,_CTRLRGB_EFFECT_OFF,_CTRLRGB_PHASE_OFF);
		RGB->Mode = _CTRLRGB_MODE_AUTOFILL; // khi RGB OFF thì phải chuyển mode
		return _TRUE;
	}

	if(!_IS_AFCTRLRGB(ValCtrl)) return _FALSE;

	if(ValCtrl == _AFCTRLRGB_FULLCOLOR)
		CtrlRGB_Set(RGB, _CTRLRGB_EFFECT_ROTATION, _CTRLRGB_PHASE_WHITE);
	else
		CtrlRGB_Set(RGB, _CTRLRGB_EFFECT_SINGLE, ValCtrl-1);  /*Because :  RGB_Phase_TypeDef = (AFSetRGB_Typedef -1) */

	return _TRUE;
}

/**	@Purpose Hàm lấy trạng thái điều khiển của Jet Swt => ghi trạng thái điều khiển vào biến JetState
  * @brief 	- trạng thái Click button để Enable or Disable Section
  * 		- trạng thái nhấn giữ nút > 5s để bật Whirlpool và RGB
  * 		- nếu nhấn giữ nút >12s thì tắt hết
  * @param  Biến Struct của thư viện AF
  * @retval None
  */
void AF_JetSW_progress(AF_Struct_t *AF_Tmp){
	static uint8_t 	JetSwt_stt = 0,
					JetSwt_stt_old = 0;

	JetSwt_stt 		= SW_progress(&AF_Tmp->_SW_Jet, !JETSWT_PinStt);

	//ko xử lý khi Stt = 0
	if (JetSwt_stt == 0) return;
	//============== JetSwt_stt status ======================//
	if ((JetSwt_stt == _SW_down_1) || (JetSwt_stt == _SW_down_2) || (JetSwt_stt == _SW_down_3)) JetSwt_stt_old = JetSwt_stt ;
	else if ((JetSwt_stt == _SW_pass_1) || (JetSwt_stt == _SW_pass_2) || (JetSwt_stt == _SW_pass_3))
	{
		if(JetSwt_stt_old == _SW_down_1 || JetSwt_stt_old == _SW_down_2 || JetSwt_stt_old == _SW_down_3){
			JetSwt_stt_old = JetSwt_stt ;
																									//debug_msg("\nClick ");
			if(AF_Tmp->Section == _ENABLE)
				goto ___label_AF_SETCTRL_DISABLE;
			else
				goto ___label_AF_SETCTRL_ENABLE;
		}
	}
	else if(JetSwt_stt == _SW_hold_on_1 && SW_get_hold_time(&AF_Tmp->_SW_Jet) == _AF_TIME_JETSW_ON_ECOJET) {
																								   //debug_msg("\nHold=5s ");
		if(AF_Tmp->Section == _ENABLE){return;}
		AF_WHIRL_On();
		wbi(AF_Tmp->Info_AF4[_BYTE0], _INFO_AF4_WHIRLON_Bit, _ENABLE);
		goto ___label_AF_SETCTRL_ACTIVE;
	}
	else if(JetSwt_stt == _SW_hold_on_1 && SW_get_hold_time(&AF_Tmp->_SW_Jet) == _AF_TIME_JETSW_HOLD_LIMIT){
		AF_Tmp->JetState = _AF_CONTROL_DISABLE ;
																									//debug_msg("\nHold>12s=>");
	}
	return;
/*====================================================================================================================================*/
//Nhảy vào đây thực hiện 1 lần duy nhất sau đó chuyển trạng thái ngay
___label_AF_SETCTRL_DISABLE:
	AF_Tmp->JetState = _AF_CONTROL_DISABLE ;
																									debug_msg("=>JetSwt Disable");
	return;

___label_AF_SETCTRL_ENABLE:
	AF_Tmp->JetState = _AF_CONTROL_ENABLE ;
																									debug_msg("=>JetSwt Enable");
	return;

___label_AF_SETCTRL_ACTIVE:
	AF_Tmp->JetState = _AF_CONTROL_ACTIVE ;
																									debug_msg("=>JetSwt Active");
	return;
}

/**	@Purpose Task xu ly nhung trang thai hoat dong cua AF
  * @brief 	gom cac trang thai sau
  	  	  * _AF_CONTROL_NONE
  	  	  * _AF_CONTROL_ENABLE
  	  	  * _AF_CONTROL_WAIT
  	  	  * _AF_CONTROL_DISABLE
  	  	  * _AF_CONTROL_ACTIVE
  *
  * @param  * Biến Struct của thư viện AF
  	  	  	* Trang thai cua Water sensor
  * @retval None
  */
void AF_JetSwtCmd_Task(AF_Struct_t *AF_Tmp, LogicStatus Water_logic){

	switch(AF_Tmp->JetState){
		case _AF_CONTROL_NONE :
			break;
		case _AF_CONTROL_ENABLE :
																									//debug_msg("=>task ENABLE");
					goto ___label_AF_DO_WAIT;
			break;
		case _AF_CONTROL_WAIT:
			//co nuoc
			if(Water_logic == _LOW){
				if(timer_expired(&AF_Tmp->_timer_AdjLevel)){
																									//debug_msg("=>task WAIT=finished");
					goto ___label_AF_DO_FINISH;
				}
			}
			//ko nuoc
			else {
				timer_restart(&AF_Tmp->_timer_AdjLevel);
				if(timer_expired(&AF_Tmp->_timeout_Protect_Solenoid)){
																									//debug_msg("=>task WAIT=timeout protect");
					goto ___label_AF_DO_OFF;
				}
			}
			break;
		case _AF_CONTROL_DISABLE :
																									//debug_msg("=>task DISABLE");
					goto ___label_AF_DO_OFF;
			break;
		case _AF_CONTROL_ACTIVE :
																									//debug_msg("=>task ACTIVE");
					goto ___label_AF_DO_FINISH;
			break;
		default : break;
	}
	return;
/*====================================================================================================================================*/
//Nhảy vào đây thực hiện 1 lần duy nhất sau đó chuyển trạng thái ngay
___label_AF_DO_OFF:
	AF_SectionOFF(AF_Tmp);
	STEAMER_Set(&KETTLE, _ST_CTRL_OFF); //tat steamer luon
	return;

___label_AF_DO_WAIT:
	AF_SectionWAIT(AF_Tmp);
	return;

___label_AF_DO_FINISH:
	AF_SectionFINISH(AF_Tmp);
	return;
}

/**	@Purpose Xử lý tất cả các timeout của AutoFill
  * 		_timer_Check_CapSS 			: kiểm tra Cap Sensor định kỳ
  * 		_timeout_Protect_Solenoid 	: timeout chống tràn nước sau 5-6 phút
  * 		_timer_Auto_Off   			: timeout tự động tắt toàn bộ sau 1h-2h
  * 		_timeout_AF_DRAIN_OffAll	: sau 10s ko user ko tắt Drain thì tự động tắt RGB và Whirlpool đi
  * 		_timeout_Protect_Drain  	: tự động tắt Drain sau 5 phút sử dụng nếu ko có Sensor Drain hoặc Drain xả hoài ko hết nước
  *										: tự động tắt Drain sau 20s nếu sensor báo hết nước
  * @param  Biến Struct của thư viện AF
  * @retval None
  */
void AF_Timeout_progress(AF_Struct_t *AF_Tmp){
	static uint8_t retry_checkCapSS = 0;

	//Kiểm tra liên tục Caps sensor
	  if(timer_expired(&AF_Tmp->_timer_Check_CapSS)){
		  timer_restart(&AF_Tmp->_timer_Check_CapSS);
		  if(++CapSS_selected >= _CAPSS_ID_TOTAL){
			  CapSS_selected = 0;
			  if((rbi(AF_Tmp->Info_AF4[_BYTE0],_INFO_SSWATER_CONNECT_Bit) == _FAIL) ||
			  			(rbi(AF_Tmp->Info_AF4[_BYTE0],_INFO_SSDRAIN_CONNECT_Bit) == _FAIL)){
				  if(++retry_checkCapSS >=20){
					  retry_checkCapSS = 0;
					  if(AF_Tmp->JetState == _AF_CONTROL_NONE) RST_Source5vCapSS();
				  }
			  }
		  }
		  CapSS_Start(&CAPSENSOR[CapSS_selected], _SS_CHECKSTT_BYTE);
	  }

	  //thời gian hoạt động tối đa của solenoid => chống tràn nước
	  if(timer_expired(&AF_Tmp->_timeout_Protect_Solenoid)){
		  timer_stop(&AF_Tmp->_timeout_Protect_Solenoid);
		  AF_Tmp->JetState = _AF_CONTROL_DISABLE;
		  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	debug_msg("\n_timeout_Protect_Solenoid");
	  }

	  //thời gian hoạt động tối đa của hệ thống Autofill
	  if(timer_expired(&AF_Tmp->_timer_Auto_Off)){
		  timer_stop(&AF_Tmp->_timer_Auto_Off);
		  AF_Tmp->JetState = _AF_CONTROL_DISABLE;
		  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	debug_msg("\n_timer_Auto_Off");
	  }

	  //sau 1s ko tat Drain thi tat RGB và whirlpool di
	  if(timer_expired(&AF_Tmp->_timeout_AF_DRAIN_OffAll)){
		  timer_stop(&AF_Tmp->_timeout_AF_DRAIN_OffAll);

		  //tắt Steamer
		  STEAMER_Set(&KETTLE, _ST_CTRL_OFF); //tat steamer luon , cau Long Update - 210806

		  //tắt Whirlpool
		  AF_WHIRL_Off();
		  wbi(AF_Tmp->Info_AF4[_BYTE0], _INFO_AF4_WHIRLON_Bit, _DISABLE);
		  wbi(AF_Tmp->Info_AF4[_BYTE1], _INFO_RLWHIRL_Bit, 0);

		  //tắt RGB, chuyển mode
		  AF_CtrlRGB(&RGB_2020, _AFRGB_OFF, AF_Flash[_IDF_RGB_MASTERCTRL]);
		  wbi(AF_Tmp->Info_AF4[_BYTE0], _INFO_RGB_STT_Bit, _AFRGB_OFF);

		  //neu solenoid bat thi xac dinh section=1 va nguoc lai
		  if(rbi(AF_Tmp->Info_AF4[_BYTE1], _INFO_SOLAF_Bit)) AF_Tmp->Section = _ENABLE;
		  else AF_Tmp->Section = _DISABLE;
																									debug_msg("\nDrain Off All");
	  }

	  //timer tắt Drainpump
	  if(timer_expired(&AF_Tmp->_timeout_Protect_Drain)){
	  	timer_stop(&AF_Tmp->_timeout_Protect_Drain);
	  	AF_Tmp->DrainState = _DRAIN_HOLD_TIMEOUT;
	  	AF_Tmp->Users_active = _DEACTIVE;
																									debug_msg("\n_timeout_Protect_Drain");
	  }

	  //reset Uart nếu ko có tín hiệu từ DCS
	  if(timer_expired(&AF_Tmp->_timer_CheckRx)){
	  	timer_restart(&AF_Tmp->_timer_CheckRx);
	  	reset_UART(&_USER_DEFINE_UART_RS232);
	  	LED1_Blink_RST_UART();
	  																								debug_msg("\nRST Uart");
	  }
}

/**	@Purpose Hàm lấy trạng thái điều khiển của nut Drain Swt => ghi trạng thái điều khiển vào biến DrainState
  * @brief
  * @param  Biến Struct của thư viện AF
  * @retval None
  */
void AF_DrainSW_progress(AF_Struct_t *AF_Tmp){
	static uint8_t Dra_stt ;

	Dra_stt = SW_progress(&AF_Tmp->_SW_Drain, !DRAINSWT_PinStt);

	//relay COM phai duoc bat truoc , neu Relay Drainpump bat truoc se ko kiem tra duoc loi de bat relay COM
	if(RL_COM_STT==0) return;

	if(Dra_stt==_SW_null){
    	//Dieu kien OFF section
    	AF_Tmp->DrainState = _DRAIN_NORMAL;
    	timer_stop(&AF_Tmp->_timeout_Protect_Drain);
    	timer_stop(&AF_Tmp->_timeout_AF_DRAIN_OffAll);
	}
	else if((Dra_stt == _SW_hold_on_1) && SW_get_hold_time(&AF_Tmp->_SW_Drain) == _TIMEOUT_RST_USERS_ACTIVE){
		AF_Tmp->Users_active = _DEACTIVE; //>30s
	}
	else if(_LIMIT(Dra_stt, _SW_hold_1,_SW_hold_on_pass)){
		if(AF_Tmp->DrainState == _DRAIN_NORMAL){
			timer_set(&AF_Tmp->_timeout_AF_DRAIN_OffAll, _AF_TIME_HOLDDRAIN_OFFALL);
			timer_set(&AF_Tmp->_timeout_Protect_Drain, _AF_TIME_HOLDDRAIN_NOSENSOR);				debug_msg("\n\rSet Drain 5 mins");
			AF_Tmp->DrainState = _DRAIN_HOLD;
			AF_DRAIN_On();
		}
    }
}

/**	@Purpose Xử lý tất cả các State Drain điều khiển của AutoFill
  * @brief 	Đặt trong while(1)

  * @param  Biến Struct của thư viện AF
  * @retval None
  */
void AF_DrainSwtCmd_Task(AF_Struct_t *AF_Tmp, CheckStatus Dra_connect, LogicStatus Dra_logic){

	//Active DrainSS
	if(Dra_logic == _LOW) AF_Tmp->DrainSS_active = _ACTIVE;
	//Deactive DrainSS
	if(timer_expired(&AF_Tmp->_timer_Deactive_DrainSS)){
		timer_stop(&AF_Tmp->_timer_Deactive_DrainSS);
		if((Dra_logic == _HIGH) && (AF_Tmp->JetState==_AF_CONTROL_WAIT)) AF_Tmp->DrainSS_active = _DEACTIVE;
	}

	switch(AF_Tmp->DrainState){
		case _DRAIN_NORMAL:
			AF_DRAIN_Off();
			break;
		case _DRAIN_HOLD:
			if(Dra_connect == _PASS && Dra_logic == _HIGH && (AF_Tmp->DrainSS_active == _ACTIVE)){
				AF_Tmp->DrainState = _DRAIN_HOLD_SSDRAIN;
				timer_set(&AF_Tmp->_timeout_Protect_Drain, _AF_TIME_HOLDDRAIN_SENSOR);
																									debug_msg("=>set 5s");
			}
			break;
		case _DRAIN_HOLD_SSDRAIN:
			//neu mat ket noi thi set lai 5 mins => _DRAIN_HOLD => du dk thi set lai 20s
			if(Dra_connect == _FAIL || Dra_logic == _LOW){
				AF_Tmp->DrainState = _DRAIN_HOLD;
				timer_set(&AF_Tmp->_timeout_Protect_Drain, _AF_TIME_HOLDDRAIN_NOSENSOR);
																									debug_msg("=>reset 5 mins");
			}
			break;
		case _DRAIN_HOLD_TIMEOUT:
			AF_DRAIN_Off();
			break;
		default : break;
	}
}

/**	@Purpose Hàm xử lý hoạt động của button Swt
  * @brief
  * @param  Biến Struct của thư viện AF
  * @retval None
  */
void AF_ButtonSW_progress(AF_Struct_t *AF_Tmp){
	static uint8_t Btn_stt ;
	uint8_t i;

	Btn_stt = SW_progress(&AF_Tmp->_SW_Button, !BUTTON_PinStt);

	if(Btn_stt == _SW_single_click){
		if(++AF_Flash[_IDF_WTER_LEVEL] > _AF_LV_MAX) AF_Flash[_IDF_WTER_LEVEL] = _AF_LV_MIN;
		Flash_SetTimer();
		LED1_Blink_WaterLevel(AF_Flash[_IDF_WTER_LEVEL]);

		mainRS232.state=_RS232LL_WAIT_FINISHED;
		mainRS232.Backup_RxCmd=_RS232LL_AfCmd_RevMassage;
		mainRS232.TxData = 0x00;

	}
	if(Btn_stt == _SW_hold_on_1 && SW_get_hold_time(&AF_Tmp->_SW_Button) == _AF_TIME_FACTORY_RST){
		timer_restart(&AF_Tmp->_timer_Check_CapSS);
		AF_Flash[_IDF_WTER_LEVEL] = _AF_LV_DEFAULT;
		AF_Flash[_IDF_RGB_MASTERCTRL] = _AFCTRLRGB_FULLCOLOR;
		AF_Flash[_IDF_AF_REVERSION] = _AF4_REVISION;

		ST_Flash[_IDF_ST_TEMPTHRES] = _TEMPC_THRESHOLD_DEFAULT;
		ST_Flash[_IDF_ST_TIMESERVICE] = _TIMEOUT_SERVICE_DEFAULT;
		ST_Flash[_IDF_ST_BLINKSOL] = _BLINK_STSOL_DEFAULT;
		ST_Flash[_IDF_ST_ENSAFETY] = _ST_SAFETY_DEFAULT;
		ST_Flash[_IDF_ST_POWER_TRIAC] = _POWER_DIMER_DEFAULT;

		for(i=0;i<8;i++) ST_Report_Flash[i]=0;

		Flash_SetTimer();
		LED1_Blink_RST_FACTORY();
		//Calib Cap Sensor
		timer_restart(&AF_Tmp->_timer_Check_CapSS);
		AF_CalibAllCapSS();
	}
}

void AF_Main_exe(void){

	if(reCalib_ZeroCurrent !=0) return;

	  AF_JetSW_progress(&AF_BOX4);
	  AF_DrainSW_progress(&AF_BOX4);
	  AF_ButtonSW_progress(&AF_BOX4);

	  AF_JetSwtCmd_Task(&AF_BOX4, CAPSENSOR[_CAPSS_ID_WATER].logicStt);
	  AF_DrainSwtCmd_Task(&AF_BOX4, CAPSENSOR[_CAPSS_ID_DRAIN].connectStt, CAPSENSOR[_CAPSS_ID_DRAIN].logicStt);

	  AF_Timeout_progress(&AF_BOX4);
}
