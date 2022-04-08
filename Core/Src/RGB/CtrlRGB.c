#include "myHeader.h"

CtrlRGB_TypeDef_t 	RGB_2020;
CtrlSPOT_TypeDef_t	SPOTLIGHT;

/*Khai bao bien PWM*/
uint16_t	CtrlRGB_CntCompare,
					CtrlRGB_CmpMax;
struct timer CtrlRGB_HoldTimePhase;

static void SpotFading_poll();

RGB_Phase_TypeDef 	CtrlRGB_calculate_phase(CtrlRGB_TypeDef_t *RGB_tmp){
	if (RGB_tmp->EffectColor==_CTRLRGB_EFFECT_ROTATION){
		CtrlRGB_CmpMax = _CTRLRGB_COMPARE_MAX_BRIGHTNESS;
		if (CtrlRGB_CntCompare < CtrlRGB_CmpMax)    {
			CtrlRGB_CntCompare += _CTRLRGB_BRIGHTNESS_DECREASE;
			/*Dat thoi gian giu mau khi chay Full Color => khi dat den 40 000 thi hold mau do them vai second*/
			if (CtrlRGB_CntCompare >= CtrlRGB_CmpMax) timer_set(&CtrlRGB_HoldTimePhase,_CTRLRGB_HOLD_TIME_PHASE);
		}

		if (timer_expired(&CtrlRGB_HoldTimePhase)){
			timer_stop(&CtrlRGB_HoldTimePhase);
			if (++RGB_tmp->Phase_RotateColor>_CTRLRGB_PHASE_BLUESKY) RGB_tmp->Phase_RotateColor=_CTRLRGB_PHASE_WHITE;
			CtrlRGB_CntCompare=_CTRLRGB_COMPARE_MIN_BRIGHTNESS;
		}
	}

	else if (RGB_2020.EffectColor==_CTRLRGB_EFFECT_SINGLE) {
		//chuyen che do Single Color
		RGB_tmp->Phase_RotateColor = RGB_tmp->Phase_SingleColor;

		//Neu White thi 75% , others la Max
		if (RGB_tmp->Phase_RotateColor==_CTRLRGB_PHASE_WHITE) CtrlRGB_CmpMax=_CTRLRGB_COMPARE_MAX_WHITESINGLE;
		else CtrlRGB_CmpMax=_CTRLRGB_COMPARE_MAX_BRIGHTNESS;
		CtrlRGB_CntCompare = CtrlRGB_CmpMax;
	}else if(RGB_tmp->EffectColor==_CTRLRGB_EFFECT_OFF){
		RGB_tmp->Phase_RotateColor=_CTRLRGB_PHASE_OFF;
		CtrlRGB_CntCompare = 0;
	}

    return RGB_tmp->Phase_RotateColor;
}

//---------------------------------------------------------------------------
/*      white   Green   Yellow  Red     Pink    Blue    sky
RED     1       0       1       1       1       0       0
GREEN   1       1       1       0       0       0       1
BLUE    1       0       0       0       1       1       1
*/

void CtrlRGB_periodic_poll(RGB_Phase_TypeDef phase){

	SpotFading_poll();

    switch (phase){
        case    _CTRLRGB_PHASE_WHITE:
            PWM_RED_WriteCompare(CtrlRGB_CntCompare); 					//RED tang tu tu
            PWM_GREEN_WriteCompare(CtrlRGB_CmpMax);   					//GREEN max
            PWM_BLUE_WriteCompare(CtrlRGB_CmpMax);    					//BLUE max
            break;
        case    _CTRLRGB_PHASE_GREEN:
            PWM_RED_WriteCompare(CtrlRGB_CmpMax-CtrlRGB_CntCompare); 	// dang white =>//RED giam tu tu
            PWM_GREEN_WriteCompare(CtrlRGB_CmpMax);                 	//GREEN dang max , giu nguyen
            PWM_BLUE_WriteCompare(CtrlRGB_CmpMax-CtrlRGB_CntCompare);	//BLUE giam tu tu
            break;
        case    _CTRLRGB_PHASE_YELLOW:
            PWM_RED_WriteCompare(CtrlRGB_CntCompare);                	// dang GREEN =>//RED tang tu tu
            PWM_GREEN_WriteCompare(CtrlRGB_CmpMax);                 	//GREEN dang max , giu nguyen
            PWM_BLUE_WriteCompare(_CTRLRGB_COMPARE_MIN_BRIGHTNESS); 	// BLUE tat
            break;
        case    _CTRLRGB_PHASE_RED:
            PWM_RED_WriteCompare(CtrlRGB_CmpMax);                      //dang Yelow => RED max
            PWM_GREEN_WriteCompare(CtrlRGB_CmpMax-CtrlRGB_CntCompare); //GREEN giam tu tu
            PWM_BLUE_WriteCompare(_CTRLRGB_COMPARE_MIN_BRIGHTNESS);    //BLUE tat
            break;
        case    _CTRLRGB_PHASE_PINK:
            PWM_RED_WriteCompare(CtrlRGB_CmpMax);                       //dang RED => RED max
            PWM_GREEN_WriteCompare(_CTRLRGB_COMPARE_MIN_BRIGHTNESS);    //GREEN tat
            PWM_BLUE_WriteCompare(CtrlRGB_CntCompare);                  //BLUE tang tu tu
            break;
        case    _CTRLRGB_PHASE_BLUE:
            PWM_RED_WriteCompare(CtrlRGB_CmpMax-CtrlRGB_CntCompare);    //Dang PinK =>RED giam tu tu
            PWM_GREEN_WriteCompare(_CTRLRGB_COMPARE_MIN_BRIGHTNESS);    //GREEN tat
            PWM_BLUE_WriteCompare(CtrlRGB_CmpMax);                      //BLUE max
            break;
        case    _CTRLRGB_PHASE_BLUESKY:
            PWM_RED_WriteCompare(_CTRLRGB_COMPARE_MIN_BRIGHTNESS);      //Dang BLUE =>RED tat
            PWM_GREEN_WriteCompare(CtrlRGB_CntCompare);                 //GREEN tang tu tu
            PWM_BLUE_WriteCompare(CtrlRGB_CmpMax);                      //BLUE max
            break;                                                      //=>return WHITE
        case    _CTRLRGB_PHASE_OFF :
            PWM_RED_WriteCompare(_CTRLRGB_COMPARE_MIN_BRIGHTNESS); 		//tat
            PWM_GREEN_WriteCompare(_CTRLRGB_COMPARE_MIN_BRIGHTNESS);	//tat
            PWM_BLUE_WriteCompare(_CTRLRGB_COMPARE_MIN_BRIGHTNESS);		//tat
            break;
        default :break;
    }
}

//void CtrlRGB_RST(CtrlRGB_TypeDef_t *RGB){
//   CtrlRGB_CntCompare=_CTRLRGB_COMPARE_MIN_BRIGHTNESS; //set do sang Min
//
//   RGB->EffectColor			= _CTRLRGB_EFFECT_OFF ;
//   RGB->Phase_RotateColor	= _CTRLRGB_PHASE_WHITE;
//   RGB->Phase_SingleColor	= _CTRLRGB_PHASE_WHITE;
//}

void CtrlRGB_Set(CtrlRGB_TypeDef_t *RGB_tmp, RGB_Effect_TypeDef Set_Effect, RGB_Phase_TypeDef Set_Phase){
	switch(Set_Effect)
	{
		case _CTRLRGB_EFFECT_OFF :
			RGB_tmp->EffectColor = _CTRLRGB_EFFECT_OFF;
			break;

		case _CTRLRGB_EFFECT_ROTATION :
			RGB_tmp->EffectColor = _CTRLRGB_EFFECT_ROTATION;
			RGB_tmp->Phase_RotateColor = Set_Phase;
			break;

		case _CTRLRGB_EFFECT_SINGLE :
			RGB_tmp->EffectColor = _CTRLRGB_EFFECT_SINGLE;
			RGB_tmp->Phase_SingleColor = Set_Phase;
			break;

		default :break;
	}
}

ResultStatus CtrlRGB_Spot(SPOT_Effect_TypeDef effect, uint8_t PerCent_PWR){
//	uint16_t Duty_On;

	if(PerCent_PWR > 100)return _FALSE;
	SPOTLIGHT.Effect_Light = effect;
	SPOTLIGHT.speed = 4;
	if(SPOTLIGHT.Effect_Light == _CTRLSPOT_EFFECT_NORMAL) PWM_SPOT_WriteCompare(PerCent_PWR*_CTRLRGB_PERIOD/100);
	else if(SPOTLIGHT.Effect_Light == _CTRLSPOT_EFFECT_FADING){
		if(PerCent_PWR==0){
			SPOTLIGHT.fading_cnt=0;
			SPOTLIGHT.fading_resume=0;
			PWM_SPOT_WriteCompare(0);
			return _TRUE;
		}
		SPOTLIGHT.fading_cnt = SPOTLIGHT.fading_resume;
		SPOTLIGHT.fading_resume = PerCent_PWR;
	}
	return _TRUE;
}

static void SpotFading_poll(){
	static uint8_t speed_spot=0;


	if(SPOTLIGHT.Effect_Light != _CTRLSPOT_EFFECT_FADING) return;

	if(++speed_spot >= SPOTLIGHT.speed){
		speed_spot = 0;
		if(SPOTLIGHT.fading_cnt > SPOTLIGHT.fading_resume){
			SPOTLIGHT.fading_cnt--;
			PWM_SPOT_WriteCompare(SPOTLIGHT.fading_cnt*_CTRLRGB_PERIOD/100);
		}
		else if(SPOTLIGHT.fading_cnt < SPOTLIGHT.fading_resume) {
			SPOTLIGHT.fading_cnt++;
			PWM_SPOT_WriteCompare(SPOTLIGHT.fading_cnt*_CTRLRGB_PERIOD/100);
		}
	}
}
