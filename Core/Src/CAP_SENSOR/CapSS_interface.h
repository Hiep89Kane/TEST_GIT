#ifndef INC_CAPSS_INTERFACE_H_
#define INC_CAPSS_INTERFACE_H_

#include "AF4_RS232.h"

#define RETRY_NUM_CHECKSTT 		6	//6*250ms= 1,5s
#define RETRY_NUM_CALIB			3	//3*500ms= 1,5s

typedef enum {
	_CAPSS_ID_WATER,                            //0
	_CAPSS_ID_DRAIN,                            //1
	//..
	_CAPSS_ID_TOTAL                             //2
} Sensor_index;

typedef enum {
	_SENSOR_CLEAR,			//chua dieu khien
	_SENSOR_SENT_START,		//bat dau tx
	_SENSOR_SENT_DONE,		//wait tx xong -> input
	_SENSOR_BUSY,			//read rx pulses
	_SENSOR_FAIL,			//retry n....0 -> return FAIL
	_SENSOR_SUCESS			//return SUCESS
} Sensor_State;

#define _SENSOR_BYTE_NUM 		5

#define Stx_Index				0
#define Add_Index				1
#define Cmd_Index				2
#define Data_Index				3
#define Etx_Index				4

#define _SS_CALIB_BYTE    		0x60         	//byte 2
#define _SS_CHECKSTT_BYTE 		0x6F

#define _CAL_PULSE_NUM     		10
#define _CHECKSTT_PULSE_NUM		5

typedef struct {
	Sensor_State 		state;
	__IO uint8_t 		pulseCnt;
	__IO CheckStatus 	connectStt;
	__IO LogicStatus 	logicStt;
	uint8_t 			command;
	uint8_t 			retry_connect;
} CapSen_Struct_t;

/* Global variables ---------------------------------------------------------*/
extern CapSen_Struct_t 	CAPSENSOR[_CAPSS_ID_TOTAL];
extern uint8_t 			CapSS_selected;  			//bien toan cuc lua chon Sensor de hoat dong
extern uint8_t 			CapSS_CalibSucess;  		//bien xay ra khi so lan calib sensor thanh cong tuong ung voi so Sensor xu ly (phải < _CAPSS_ID_TOTAL)

/* Functions ---------------------------------------------------------*/
void CapSS_Main_exe(void);

/*********************************************************************/
void CapSS_TxSignal_SetOut(uint8_t CapSS_selected);
void CapSS_TxSignal_SetIn(uint8_t CapSS_selected);

void CapSS_Start(CapSen_Struct_t *Sensor_Tmp, uint8_t cmd);
void CapSS_Task(CapSen_Struct_t *Sensor_Tmp, RS232_Struct_t *rs232);
void CapSS_CheckPulse_Systick(uint8_t Pin_stt, CapSen_Struct_t *Sensor_Tmp);
void CapSS_Get_Result(CapSen_Struct_t *Sensor_Tmp, RS232_Struct_t *rs232);

//In Timer
void CapSS_periodic_poll(uint8_t Value_selectSS, CapSen_Struct_t Sensor_Tmp[_CAPSS_ID_TOTAL]);

#endif /* INC_CAPSS_INTERFACE_H_ */
