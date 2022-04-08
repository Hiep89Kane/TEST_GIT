#ifndef INC_STEAMER_H_
#define INC_STEAMER_H_

#define	ST_AIRPUMP_On()				wbi(Global_RLAC_Manager,_RLAIR_Bit,1)
#define	ST_AIRPUMP_Off()			wbi(Global_RLAC_Manager,_RLAIR_Bit,0)
#define ST_AIRPUMP_Stt()			RL_AIR_STT

#define	_AC_FREQUENCY_60HZ			83//166
#define	_AC_FREQUENCY_50HZ			100//200

#define _ST_SAFETY_DIS				0
#define _ST_SAFETY_EN				1
#define _ST_SAFETY_DEFAULT			_ST_SAFETY_EN
#define check_Flash_Safety(val)		((_LIMIT(val, _ST_SAFETY_DIS, _ST_SAFETY_EN))?val:_ST_SAFETY_DEFAULT)

#define _DEF_POWER_TOTAL_NUMMER		3
#define _POWER_DIMER_P1				50 //500w
#define _POWER_DIMER_P2				57 //600w
#define _POWER_DIMER_P3				68 //750w
#define _POWER_DIMER_DEFAULT		_POWER_DIMER_P1

#define _IS_DIMER_POWER(val,min,max)		(_LIMIT(val, min, max))
#define check_Flash_PowerTriac(val,min,max) ((_LIMIT(val, min, max))?val:_POWER_DIMER_DEFAULT)

#define _TEMPC_DIODE_SO_HOT			90

/*dung de kiem tra gia tri temp threshold co dung hay ko ?*/
#define _TEMPC_THRESHOLD_MIN  		60
#define _TEMPC_THRESHOLD_MAX  		100
#define _TEMPC_THRESHOLD_DEFAULT  	85
#define _IS_TEMPC_THRESHOLD(val)		(_LIMIT(val, _TEMPC_THRESHOLD_MIN, _TEMPC_THRESHOLD_MAX))
#define check_Flash_TempThreshold(val)	((_LIMIT(val, _TEMPC_THRESHOLD_MIN, _TEMPC_THRESHOLD_MAX))?val:_TEMPC_THRESHOLD_DEFAULT)

/*dung de kiem tra gia tri timeout service co dung hay ko ?*/
#define _TIMEOUT_SERVICE_MIN		1
#define _TIMEOUT_SERVICE_MAX		6
#define _TIMEOUT_SERVICE_DEFAULT	2
#define _IS_TIMEOUT_SERVICE(val)	(_LIMIT(val, _TIMEOUT_SERVICE_MIN, _TIMEOUT_SERVICE_MAX))
#define check_Flash_TimeSevice(val)	((_LIMIT(val, _TIMEOUT_SERVICE_MIN, _TIMEOUT_SERVICE_MAX))?val:_TIMEOUT_SERVICE_DEFAULT)

/*dung de blink steamer solenoid */
#define _BLINK_STSOL_MAX			15
#define _BLINK_STSOL_MIN			3
#define _BLINK_STSOL_DEFAULT		10
#define _IS_BLINK_STSOL(val)			(_LIMIT(val, _BLINK_STSOL_MIN, _BLINK_STSOL_MAX))
#define check_Flash_BlinkSolenoid(val)	((_LIMIT(val, _BLINK_STSOL_MIN, _BLINK_STSOL_MAX))?val:_BLINK_STSOL_DEFAULT)

#define _TIME_BLINK_DEBUG			(10*CLOCK_SECOND) 										//debug loi
#define _TIME_AUTOCHECK_WATER(min)	(min*60*CLOCK_SECOND) 									//x phut tu dong kiem tra nuoc
#define _TIME_WAIT_QUIET			(10*CLOCK_SECOND)										//thoi gian cho nuoc tinh lang

#define _TIME_FILL_ON				(CLOCK_SECOND/3)										//solenoid fill nuoc luc bat dau
#define _TIME_FILL_OFF				(CLOCK_SECOND*4)										//cho nuoc on dinh
#define _TIME_FILL_CONFIRM			(_TIME_FILL_ON + _TIME_FILL_OFF - 5/*50ms earlier*/)	//xac nhan co nuoc hoac ko nuoc truoc 20 ms

//Control Relay Water Probes
#define ST_RLOldSS_Off()			OUTPUT_set_mode(&_CTRL_RLOldSS, _OUTPUT_mode_off)
#define ST_RLOldSS_On(time)			OUTPUT_set_blink(&_CTRL_RLOldSS, 1, time, 10, 0)
//Control Solenoid fill water of Steamer
#define ST_SOL_Blink(num)			do{\
										if(RL_SOL_COM_STT==0){RL_SOL_COM_ON;delay_ms(100);}\
										OUTPUT_set_blink(&_CTRL_SOLST, num, _TIME_FILL_ON, _TIME_FILL_OFF, 0);\
									}while(0)

#define ST_SOL_Off()				do{\
										OUTPUT_set_mode(&_CTRL_SOLST, _OUTPUT_mode_off);\
										if(_CTRL_SOLAF.mode <= _OUTPUT_mode_off){delay_ms(100); RL_SOL_COM_OFF;}\
									}while(0)

#define ST_SOL_FillFixDry()			OUTPUT_set_blink(&_CTRL_SOLST, 1, CLOCK_SECOND/2, CLOCK_SECOND/2, 0)

typedef enum
{
	_ST_OFF_OK					=0x00,		//tắt chủ động => OK

	_ST_ERR_CURRT				=0x01,		//Lỗi này là Overload giống Byte 2
	_ST_ERR_SAFETY				=0x02,  	//Bật steamer nhưng ko căm ống ở safety
	_ST_ERR_CLOGGED				=0x03,		//Sau thời gian fill nước 10 lần  =>vẫn ko detect được nước => clogged water
	_ST_ERR_BOILER_DRY			=0x04,		//dang chay nhung binh kho nuoc => relay nhiet cat dien => steamer control OFF
	_ST_ERR_BOILER_PTC			=0x05,		//Lỗi sensor nhiệt ở bình đun
	_ST_ERR_DIODE_PTC			=0x06,		//Lỗi sensor nhiệt ở Diode công suất
	_ST_ERR_DIODE_FAN			=0x07,		//Bật quạt nhưng diode vẫn nóng >80 độ C => hư quạt hoặc chưa gắn quạt
	_ST_ERR_NONE_AIRPUMP		=0x08,		//ko phát hiện dòng điện tối thiểu khi bật Airpump nên ko bật heat cho an toàn
	_ST_TIMEOUT_SERVICE_OFF		=0x09,		//TimeOut thời gian hoạt động tối đa
	//_ST_TIMEOUT_DRAIN_OFF		=0x0A,		//khi drain xả hết nước => hết time phuc vụ khách hàng

	_ST_ERR_NONE_ACSOURCE		=0x0C,		//ko thấy nguồn AC => Lỗi này ko OFF
	_ST_ERR_NONE_STLOAD			=0x0D,		//Có nguồn AC nhưng ko có tải => Lỗi này ko OFF

	_ST_WAIT_DETECTWATER		=0x0E,		//đang chờ fill nước vào
	_ST_ON_OK 					=0x0F		//bật chủ động => OK
} STEAMResponse_TypeDef;

#define _IS_ERR_AUTO_OFF(error)	_LIMIT(error, _ST_ERR_CURRT, _ST_TIMEOUT_SERVICE_OFF)

typedef enum{
	_ST_CTRL_OFF,
	_ST_CTRL_ON,
}STEAM_Ctrl_TypeDef;

typedef enum{
	_STATE_NOCTRL,
	_STATE_START,               //nhận được lệnh ON/OFF => nếu ON thì bật Relay Sensor lên
	_STATE_CHECK_PROBE_WATERSS, //chờ relay Sensor thực sự bật lên chuyển qua ON hoặc fill water
	_STATE_FILL_BOILERWATER,	//trang thai fill nuoc Blink Solenoid n lan
	_STATE_ON,
	_STATE_IDE_WATER			//đang ON sau 1 thời gian thì ko nấu khoảng 10s chờ nước hết dao động
}STEAMState_TypeDef;

typedef struct{
	STEAM_Ctrl_TypeDef		activity;
	STEAMState_TypeDef 		state;

	__IO ProbeWterSS_STT_t	waterStt;
	__IO uint16_t			ADC_SSProbes_Compare;
	__IO uint16_t			ADC_SSProbes_Delta;			//dùng để điều chỉnh water SensorProbes khi gặp lỗi

	uint16_t				backup_Current;				//Dòng điện lưu trữ lại trước khi Steam On để so sánh
	uint8_t					f_CheckAirpumpOk;			//check Air khi vừa bật Steamer
	uint8_t					f_HeatRunOk;				//Steamer đã hoạt động tốt khi vừa bật lên

	__IO LogicStatus		safety_stt;
	__IO uint8_t 			tempC, tempC_Diode;
	uint8_t					RepCauseOFF;        		//nếu Steamer gặp lỗi sẽ tự động OFF , biến này sẽ lưu trữ lai
	uint8_t					Sol_repeat_cnt;

	__IO uint16_t 			AC_Duty, AC_Duty_cnt;		//for AC source
	uint8_t 				Power_Level; 				/*1-100*/

	struct timer 			_timer_STCommon,
							_timer_ConfirmSensor,
							_timer_RepeatCheckRun,		//AdjPower and check Err
							_timeout_Service;
}STEAMER_Struct_t;

/* Global variables ---------------------------------------------------------*/
extern STEAMER_Struct_t 	KETTLE;
//======================================================================================================================
void 			STEAMER_Main_exe(void);
//======================================================================================================================
void 			STEAMER_init(STEAMER_Struct_t *Steamer);
void 			STEAMER_Set(STEAMER_Struct_t *Steamer, STEAM_Ctrl_TypeDef activity);
void			STEAMER_Activity_Task(STEAMER_Struct_t *Steamer, AF_Struct_t *AF_Tmp);

void 			STEAMER_UpdateDuty(Edge_ReturnStatus edge_pule,STEAMER_Struct_t *Steamer);
//In timer 100us
void 			STEAMER_DimerCtrl_periodic_poll(STEAMER_Struct_t *Steamer); //timer interrupt 100us
//In systick
void 			STEAMER_GetStt_Systick(STEAMER_Struct_t *Steamer,__IO uint16_t ADC_Input_Arr[]); //timer interrupt 10ms

#endif /* INC_STEAMER_H_ */
