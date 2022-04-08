#ifndef INC_CTRLRGB_H_
#define INC_CTRLRGB_H_

extern TIM_HandleTypeDef htim3;

/*Dinh nghia cac thong so chu ki ngat PWM (uint16)*/
//gia tri cang cao thi chu ki cang cham
#define     _CTRLRGB_TIMER_PWM			    		_USER_DEFINE_TIM_RGB
#define     _CTRLRGB_PERIOD                         _USER_DEFINE_TIM_RGB.Init.Period /*38000*/

#define 	_TIM_PWM_CH_BLUE						TIM_CHANNEL_1
#define 	_TIM_PWM_CH_GREEN						TIM_CHANNEL_2
#define 	_TIM_PWM_CH_RED							TIM_CHANNEL_3
#define 	_TIM_PWM_CH_SPOT						TIM_CHANNEL_4

#define 	PWM_RED_WriteCompare(CMP)				__HAL_TIM_SET_COMPARE(&_CTRLRGB_TIMER_PWM,_TIM_PWM_CH_RED,CMP)
#define 	PWM_GREEN_WriteCompare(CMP)				__HAL_TIM_SET_COMPARE(&_CTRLRGB_TIMER_PWM,_TIM_PWM_CH_GREEN,CMP)
#define 	PWM_BLUE_WriteCompare(CMP)				__HAL_TIM_SET_COMPARE(&_CTRLRGB_TIMER_PWM,_TIM_PWM_CH_BLUE,CMP)
#define 	PWM_SPOT_WriteCompare(CMP)				__HAL_TIM_SET_COMPARE(&_CTRLRGB_TIMER_PWM,_TIM_PWM_CH_SPOT,CMP)

#define     _CTRLRGB_COMPARE_MAX_WHITESINGLE        ((_CTRLRGB_PERIOD*17)/20)	//85%
/*Dinh nghia thoi gian Giu mau Phase khi Full Color*/
#define     _CTRLRGB_HOLD_TIME_PHASE                (2*CLOCK_SECOND)      	//Unit: MSecond

/*Dinh nghia cac thong so sanh voi bo dem PERIOD PWM (uint16)*/
//Gia tri <= PERIOD
#define     _CTRLRGB_COMPARE_MIN_BRIGHTNESS         0               			//Cang thap thi cang lau sang
#define     _CTRLRGB_COMPARE_MAX_BRIGHTNESS         _CTRLRGB_PERIOD           	//Cang cao thi cang lau tat
#define     _CTRLRGB_BRIGHTNESS_DECREASE            20//(_CTRLRGB_PERIOD/1600)  //Cang nho thi do sang tang (giam) cang min (BAT BUOC la boi cua 5)

typedef enum{
	_CTRLRGB_PHASE_WHITE,                   // 0
	_CTRLRGB_PHASE_GREEN,                  	// 1
    _CTRLRGB_PHASE_YELLOW,                 	// 2
    _CTRLRGB_PHASE_RED,                    	// 3
    _CTRLRGB_PHASE_PINK,                 	// 4
    _CTRLRGB_PHASE_BLUE,                   	// 5
    _CTRLRGB_PHASE_BLUESKY,               	// 6
    _CTRLRGB_PHASE_OFF,                		// 7
}RGB_Phase_TypeDef;

/*Dinh nghia che do he thong*/
typedef enum{
	_CTRLRGB_MODE_AUTOFILL, 	//0 => chế độ tắt mở tự động theo JetSwt => chuyển qua mode này khi DCS điều khiển tắt RGB
	_CTRLRGB_MODE_DCSMANUAL,	//1 => không tự động tắt mở theo Jet Swt => chuyển qua mode này khi "DCS bật RGB"(bật cách khác ko tính) và "Whirlpool off"
}RGB_Mode_TypeDef;

/*Dinh nghia Hieu ung RGB*/
typedef enum{
	_CTRLRGB_EFFECT_OFF,					//0
	_CTRLRGB_EFFECT_ROTATION,               //1
	_CTRLRGB_EFFECT_SINGLE                 	//2
}RGB_Effect_TypeDef;

typedef struct{
	RGB_Mode_TypeDef   	Mode;				//_CTRLRGB_MODE_AUTOFILL  _CTRLRGB_MODE_DCSMANUAL ;
	RGB_Effect_TypeDef  EffectColor;		//_CTRLRGB_EFFECT_OFF, _CTRLRGB_EFFECT_ROTATION, _CTRLRGB_EFFECT_SINGLE
	RGB_Phase_TypeDef   Phase_RotateColor,	//xac dinh Phase color dat trong ngat timer
						Phase_SingleColor;	//WHITE -> GREEN_ -> YELOW -> ...-> ALL OFF
}CtrlRGB_TypeDef_t;

typedef enum{
	_CTRLSPOT_EFFECT_NORMAL,
	_CTRLSPOT_EFFECT_FADING
}SPOT_Effect_TypeDef;

typedef struct {
	SPOT_Effect_TypeDef	Effect_Light;
	uint8_t			SpotPwr;
	uint8_t 		speed;
	__IO uint32_t 	fading_cnt;
	__IO uint32_t 	fading_resume;
}CtrlSPOT_TypeDef_t;

/* Global variables ---------------------------------------------------------*/
extern CtrlRGB_TypeDef_t 	RGB_2020;
extern CtrlSPOT_TypeDef_t	SPOTLIGHT;

//**************** RGB Functions *********************************************/
RGB_Phase_TypeDef 	CtrlRGB_calculate_phase(CtrlRGB_TypeDef_t *RGB_tmp);
void 				CtrlRGB_periodic_poll(RGB_Phase_TypeDef phase);

//void 				CtrlRGB_RST(CtrlRGB_TypeDef_t *RGB);
void 				CtrlRGB_Set(CtrlRGB_TypeDef_t *RGB,
								RGB_Effect_TypeDef Set_Effect,
								RGB_Phase_TypeDef Set_Phase);
ResultStatus 		CtrlRGB_Spot(SPOT_Effect_TypeDef effect, uint8_t PerCent_PWR);

#endif /* INC_CTRLRGB_H_ */
