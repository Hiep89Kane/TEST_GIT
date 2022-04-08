#ifndef INC_INIT_MCU_H_
#define INC_INIT_MCU_H_

#define _AF4_REVISION						3//TRIAC

#define _ZERO_AC_POINT						9 //trễ 9x(100us)~1ms so với tín hiệu từ Opto E814

#define LED1_Blink_Revision()				OUTPUT_set_blink(&_CTRL_LED1_YE, _AF4_REVISION, CLOCK_SECOND/4, CLOCK_SECOND/4, 0)
#define LED1_Blink_WaterLevel(level)		OUTPUT_set_blink(&_CTRL_LED1_YE, level, CLOCK_SECOND/3, CLOCK_SECOND/3, 0)
#define LED1_Blink_RST_FACTORY()			OUTPUT_set_blink(&_CTRL_LED1_YE, 1, CLOCK_SECOND*3, CLOCK_SECOND/10, 0)
#define LED1_Blink_CALIBSS_OK()				OUTPUT_set_blink(&_CTRL_LED1_YE, 1, CLOCK_SECOND*3, CLOCK_SECOND/20, 0)
#define LED1_Blink_CALIBSS_FAIL()			OUTPUT_set_blink(&_CTRL_LED1_YE, 10, CLOCK_SECOND/20, CLOCK_SECOND/20, 0)
#define LED1_Blink_INIT_CURRENT_SENSOR()	OUTPUT_set_blink(&_CTRL_LED1_YE, 10, CLOCK_SECOND/50, CLOCK_SECOND/50, 0)
#define LED1_Blink_RST_UART()				OUTPUT_set_blink(&_CTRL_LED1_YE, 5, CLOCK_SECOND/50, CLOCK_SECOND/50, 0)

#define Ctrl_RelayCOM(ON_OFF)				((ON_OFF) ? OUTPUT_set_mode(&_CTRL_RLCOM, _OUTPUT_mode_on) : OUTPUT_set_mode(&_CTRL_RLCOM, _OUTPUT_mode_off))

enum{
	_AfDebug_WterSS_Disconnect=1,
	_AfDebug_DrainSS_Disconnect=2,
	_AfDebug_Current_Wrong=3,
	_AfDebug_Current_Leak0=4,
	_AfDebug_Current_Leak1=5,
	_AfDebug_Current_Zero=6,
	_AfDebug_Current_OverLoad=7,
	_AfDebug_DisTem1_Boiler=8,
	_AfDebug_DisTem2_AcDiode=9,
	_AfDebug_Fan_Hot=10,
};

//bit of Global_RLAC_Manager variable
enum {
	_RL_NONE,							//0
	_RLWHIRL_Bit,                       //1
	_RLDRAIN_Bit,                       //2
	_RLAIR_Bit,                         //3
}RLAC_ManagerBit;

#define LED_Green_AfDebug(times)	OUTPUT_set_blink(&_CTRL_LED1_YE, times, CLOCK_SECOND/6, CLOCK_SECOND/3, 0)

#define LED_Set_On()				OUTPUT_set_mode(&_CTRL_LED1_YE, _OUTPUT_mode_on)
#define LED_Set_Off()				OUTPUT_set_mode(&_CTRL_LED1_YE, _OUTPUT_mode_off)

#define BUZ_Beep(times)				OUTPUT_set_blink(&_CTRL_BUZ, times, CLOCK_SECOND/5, CLOCK_SECOND/5, 0)
#define LED_Blue_STDebug(times)		OUTPUT_set_blink(&_CTRL_LED2_BL, times, CLOCK_SECOND/4, CLOCK_SECOND/4, 0)

#define RST_Source5vCapSS()			OUTPUT_set_blink(&_CTRL_SEN5V,1,CLOCK_SECOND,CLOCK_SECOND/10,0)

#define _RECALIB_COUNT_SAMPLE		5	//ADC_Calib0mA = trung bình cộng của 5 lần sau khi RL_COM on

/* Global variables ---------------------------------------------------------*/
extern __IO uint8_t 		Global_RLAC_Manager, ZeroPulse_delay;
extern struct timer 	 	_timer_StartGetCurrt;
extern __IO uint16_t 		TotalCurrent_mA;
extern __IO uint16_t 		ADC_Calib0mA;
extern uint8_t 				reCalib_ZeroCurrent;

extern	 OUTPUT_struct_t 	_CTRL_LED1_YE,
							_CTRL_LED2_BL,
							_CTRL_BUZ,
							_CTRL_RLCOM,
							_CTRL_SEN5V,
							_CTRL_SOLAF,
							_CTRL_RLOldSS,
							_CTRL_SOLST;

extern get_string_str Uart3_StrBuff;

/*//=========================================== BASE IO FUNCTIONS for OUTPUT CONTROL =============================================================================*/
void LED1_BaseCtrl(uint8_t ON_OFF);
void LED2_BaseCtrl(uint8_t ON_OFF);
void BUZ_BaseCtrl(uint8_t ON_OFF);
//AutoFill IO
void AF_RLCOM_BaseCtrl(uint8_t ON_OFF);
void AF_SEN5V_BaseCtrl(uint8_t OFF_ON);
void AF_SOL_BaseCtrl(uint8_t ON_OFF);
void AF_WHIRL_BaseCtrl(uint8_t ON_OFF);
void AF_DRAIN_BaseCtrl(uint8_t ON_OFF);
//STEAMER IO
void ST_RLOldSS_BaseCtrl(uint8_t ON_OFF);
void ST_SOL_BaseCtrl(uint8_t ON_OFF);
void ST_AIRPUMP_BaseCtrl(uint8_t ON_OFF);
void ST_RL100_BaseCtrl(uint8_t ON_OFF);
/*//===========================================END DECLARE =============================================================================*/
void init_MCU_UART(UART_HandleTypeDef *huart);
void init_MCU_PWM();
void init_MCU_ADC1_DMA();
void init_MCU_IO_Ctrl();
void init_MCU_TIM();
void init_MCU_get_MAC(uint64_t *out_MAC, uint32_t ADR);

void init_MCU_AllProjet(void);

void Debug_AllProject_Main_exe();

void init_DebugCauseRstMCU();
void Led1_AFdebug_poll(uint8_t info_byte0, uint8_t info_byte2);
void reset_UART(UART_HandleTypeDef *huart);

void CtrlRLAC_periodic_poll(__IO uint16_t AC_Zero_point,__IO uint8_t RLAC_Manager_Var);

#endif /* INC_INIT_MCU_H_ */
