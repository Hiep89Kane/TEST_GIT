#include "myHeader.h"

typedef enum{
	_CHECK_CLEAR,
	_CHECK_LO,
	_CHECK_HI,
}CheckSensor_State;

CapSen_Struct_t 	CAPSENSOR[_CAPSS_ID_TOTAL];
uint8_t 			CapSS_selected;  					//bien toan cuc lua chon Sensor de hoat dong
uint8_t 			CapSS_CalibSucess;  				//bien xay ra khi so lan calib sensor thanh cong tuong ung voi so Sensor xu ly (phải < _CAPSS_ID_TOTAL)

static CheckSensor_State Check_State;
static struct timer 	_timer_wait_sensor;


void CapSS_TxSignal_SetOut(uint8_t select_IO_Cap){
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	if(select_IO_Cap == _CAPSS_ID_WATER){
		  //Out Push-pull , Res Pull-up
		  GPIO_InitStruct.Pin = SSWATER_SIGNAL_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_PULLUP;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
		  HAL_GPIO_Init(SSWATER_SIGNAL_GPIO_Port, &GPIO_InitStruct);
	}
	else if(select_IO_Cap == _CAPSS_ID_DRAIN){
		 //Out Push-pull , Res Pull-up
		  GPIO_InitStruct.Pin = SSDRAIN_SIGNAL_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_PULLUP;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
		  HAL_GPIO_Init(SSDRAIN_SIGNAL_GPIO_Port, &GPIO_InitStruct);
	}
}

void CapSS_TxSignal_SetIn(uint8_t select_IO_Cap){
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	if(select_IO_Cap == _CAPSS_ID_WATER){
			//Input : Res Pull-up
		  GPIO_InitStruct.Pin = SSWATER_SIGNAL_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		  GPIO_InitStruct.Pull = GPIO_PULLUP;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
		  HAL_GPIO_Init(SSWATER_SIGNAL_GPIO_Port, &GPIO_InitStruct);
	}
	else if(select_IO_Cap == _CAPSS_ID_DRAIN){
			//Input : Res Pull-up
		  GPIO_InitStruct.Pin = SSDRAIN_SIGNAL_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		  GPIO_InitStruct.Pull = GPIO_PULLUP;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
		  HAL_GPIO_Init(SSDRAIN_SIGNAL_GPIO_Port, &GPIO_InitStruct);
	}
}

/*======================== CapSS_Start  ====================================================
 * Action :	Execuse all Drain Command
 * Param: AF struct
 * */
void CapSS_Start(CapSen_Struct_t *Sensor_Tmp, uint8_t cmd){

	if(cmd == _SS_CHECKSTT_BYTE) {
		Sensor_Tmp->retry_connect = RETRY_NUM_CHECKSTT;															//debug_msg("\n%u-CheckStt",CapSS_selected);
	}
	else if(cmd == _SS_CALIB_BYTE) {
		Sensor_Tmp->retry_connect = RETRY_NUM_CALIB;															//debug_msg("\n%u-Calib",CapSS_selected);
	}
	else return;

	Sensor_Tmp->state = _SENSOR_SENT_START;
	Sensor_Tmp->command = cmd ;
	timer_stop(&_timer_wait_sensor);
}

void CapSS_Task(CapSen_Struct_t *Sensor_Tmp, RS232_Struct_t *rs232){
	uint16_t CheckSumByte;
	static uint8_t 	string_cmd[_SENSOR_BYTE_NUM];
	static uint8_t 	Condition_nummer ;
	static uint32_t count_systick;

	switch(Sensor_Tmp->state){
		case _SENSOR_SENT_START :
			Check_State = _CHECK_CLEAR;

			string_cmd[Stx_Index] 	= 0x1E;
			string_cmd[Add_Index] 	= 0x05;
			string_cmd[Cmd_Index] 	= Sensor_Tmp->command;
			CheckSumByte			= string_cmd[Stx_Index] + string_cmd[Add_Index] + string_cmd[Cmd_Index];
			string_cmd[Data_Index] 	= CheckSumByte & 0xFF ;
			string_cmd[Etx_Index] 	= 0x1F;

			Sensor_Tmp->pulseCnt = 0 ;

			CapSS_TxSignal_SetOut(CapSS_selected);
			count_systick = HAL_GetTick();
			UV_CAPSS_putArr(string_cmd, _SENSOR_BYTE_NUM);

			if(Sensor_Tmp->command == _SS_CHECKSTT_BYTE)timer_set(&_timer_wait_sensor, CLOCK_SECOND/4); 	//it nhat 100 ms
			else if(Sensor_Tmp->command == _SS_CALIB_BYTE)timer_set(&_timer_wait_sensor, CLOCK_SECOND/2); 	//it nhat 400 ms => vi Calib tra loi cham

			Sensor_Tmp->state = _SENSOR_SENT_DONE;
			break;

		case _SENSOR_SENT_DONE :
			if((HAL_GetTick() - count_systick) > 15/*ms*/) {
				Sensor_Tmp->state = _SENSOR_BUSY;
				CapSS_TxSignal_SetIn(CapSS_selected);
			}

			break;

		case _SENSOR_BUSY :

			if(timer_expired(&_timer_wait_sensor)){
				timer_stop(&_timer_wait_sensor);

				if( Sensor_Tmp->command == _SS_CALIB_BYTE){
					Condition_nummer = _CAL_PULSE_NUM;
																									//debug_msg("\n%u-Calib.pulseCnt=%u",CapSS_selected, Sensor_Tmp->pulseCnt);
				}
				else if( Sensor_Tmp->command == _SS_CHECKSTT_BYTE){
					Condition_nummer = _CHECKSTT_PULSE_NUM;
																									//debug_msg("\n%u-Check.pulseCnt=%u",CapSS_selected, Sensor_Tmp->pulseCnt);
				}

				if(_LIMIT(Sensor_Tmp->pulseCnt, Condition_nummer-4, Condition_nummer+4)){
					Sensor_Tmp->state = _SENSOR_SUCESS;
					Sensor_Tmp->connectStt = _PASS;
																									//debug_msg("=>OK");
				}else {
					if(--Sensor_Tmp->retry_connect == 0){
						Sensor_Tmp->state = _SENSOR_FAIL; 			//must Clear then continous
						if( Sensor_Tmp->command == _SS_CHECKSTT_BYTE){
							Sensor_Tmp->connectStt = _FAIL;											//debug_msg("=>FAIL");
						}
					}else Sensor_Tmp->state = _SENSOR_SENT_START; 	//must Clear then continous
																									//debug_msg("\n%u-Retry=%u",CapSS_selected,Sensor_Tmp->retry_connect);
				}
			}
			break;
		default : break;
	}

	CapSS_Get_Result(Sensor_Tmp, rs232);
}

void CapSS_Get_Result(CapSen_Struct_t *Sensor_Tmp, RS232_Struct_t *rs232){
	if(!_LIMIT(Sensor_Tmp->state, _SENSOR_FAIL, _SENSOR_SUCESS)) return;

	switch(Sensor_Tmp->command){
		case _SS_CHECKSTT_BYTE :
			if(Sensor_Tmp->state == _SENSOR_SUCESS) Sensor_Tmp->connectStt = _PASS;
			else if(Sensor_Tmp->state == _SENSOR_FAIL) Sensor_Tmp->connectStt = _FAIL;
			break;
		case _SS_CALIB_BYTE :

			if(Sensor_Tmp->state == _SENSOR_SUCESS){CapSS_CalibSucess++;}
			else if(Sensor_Tmp->state == _SENSOR_FAIL);

			//Khi Gap Sensor cuoi cung
			if(CapSS_selected == (_CAPSS_ID_TOTAL-1)){
				if(CapSS_CalibSucess < _CAPSS_ID_TOTAL){
					if(rs232->state == _RS232LL_WAIT_GETSTT) goto ___label_RS232_CALIB_FINISHED_FAIL;
					LED1_Blink_CALIBSS_FAIL();
				}else{
					if(rs232->state == _RS232LL_WAIT_GETSTT) goto ___label_RS232_CALIB_FINISHED_SUCESS;
					LED1_Blink_CALIBSS_OK();
				}
				CapSS_CalibSucess = 0;
				CapSS_selected = 0;
			}
			//chua phai sensor cuoi cung thi tang len
			else{CapSS_selected++;}

			break;
		default: break;
	}
	Sensor_Tmp->command = 0x00;
	Sensor_Tmp->state = _SENSOR_CLEAR;
return;

/*Thuc hien 1 lan roi thoat ra **********************************************************************/
___label_RS232_CALIB_FINISHED_FAIL :
	rs232->state = _RS232LL_WAIT_FINISHED;
	rs232->TxData = _RS232_TX_FAIL;
	return;

___label_RS232_CALIB_FINISHED_SUCESS :
	rs232->state = _RS232LL_WAIT_FINISHED;
	rs232->TxData = _RS232_TX_OK;
	return;
}

void CapSS_CheckPulse_Systick(uint8_t Pin_stt, CapSen_Struct_t *Sensor_Tmp){
	static uint8_t Pin_stt_old;
	static uint32_t tickstart;

	if(Sensor_Tmp->state != _SENSOR_BUSY){Pin_stt_old = Pin_stt;return;}

	if(Pin_stt == 0 && Pin_stt_old == 1){
	//falling
		if(Check_State == _CHECK_CLEAR){
			tickstart = HAL_GetTick();
			Check_State = _CHECK_LO;
		}else if(Check_State == _CHECK_HI){
			if((HAL_GetTick() - tickstart) < 3) Check_State = _CHECK_CLEAR; //bo qua
		}
	}
	else if(Pin_stt == 1 && Pin_stt_old == 0){
	//rising
		if(Check_State == _CHECK_LO){
			if(_LIMIT((HAL_GetTick() - tickstart) ,3/*ms*/,7/*ms*/)){
				tickstart = HAL_GetTick();
				Check_State = _CHECK_HI;
			}else {
				Check_State = _CHECK_CLEAR;
			}
		}
	}
	else if(Pin_stt == 1 && Pin_stt_old == 1){
		if(Check_State == _CHECK_HI){
			if(((HAL_GetTick() - tickstart) >= 3)){
				Sensor_Tmp->pulseCnt++;
				Check_State = _CHECK_CLEAR;
			}
		}
	}
	Pin_stt_old = Pin_stt;
}

void CapSS_periodic_poll(uint8_t Value_selectSS, CapSen_Struct_t Sensor_Tmp[_CAPSS_ID_TOTAL]){

  	if(Value_selectSS == _CAPSS_ID_WATER){
  		CapSS_CheckPulse_Systick(WATERSS_SIGNAL_STT, &Sensor_Tmp[_CAPSS_ID_WATER]);
  	}else if(Value_selectSS == _CAPSS_ID_DRAIN){
  		CapSS_CheckPulse_Systick(DRAINSS_SIGNAL_STT, &Sensor_Tmp[_CAPSS_ID_DRAIN]);
  	}

  	Sensor_Tmp[_CAPSS_ID_DRAIN].logicStt = Systick_ReadInput_poll(_Input_SSDrain_Logic, SSDRAIN_Logic_PinStt, 200, 200);
  	Sensor_Tmp[_CAPSS_ID_WATER].logicStt = Systick_ReadInput_poll(_Input_SSWater_Logic, SSWATER_Logic_PinStt, 500, 100);
}

void CapSS_Main_exe(void){

	 CapSS_Task(&CAPSENSOR[CapSS_selected], &mainRS232);
}
