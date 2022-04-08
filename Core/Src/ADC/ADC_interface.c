#include "myHeader.h"

struct Str_Kalman 	Kalman_ProbesWter,
					Kalman_PTC_HeatSink,
					Kalman_PTC_Boiler;

__IO uint16_t 		ADC_Arr[_ID_ADC_TOTAL],
					ADC_Avg_Arr[_ID_ADC_TOTAL];

#define PTC_MAX_TEMPC				111 		//chỉ detect tối đa 111 độ C
const uint16_t PTC_temp[PTC_MAX_TEMPC+1]={
		3138,						/*0*/
		3099,3060,3020,2979,2938,	/*1-5*/
		2896,2853,2810,2766,2722,	/*6-10*/
		2677,2632,2586,2541,2494,	/*11-15*/
		2448,2402,2355,2309,2262,	/*16-20*/
		2216,2169,2123,2077,2031,	/*21-25*/
		1985,1940,1895,1851,1807,	/*26-30*/
		1763,1720,1678,1636,1594,	/*31-35*/
		1553,1513,1474,1435,1397,	/*36-40*/
		1360,1323,1287,1252,1217,	/*41-45*/
		1183,1150,1118,1086,1056,	/*46-50*/
		1026,996,968,940,913,		/*51-55*/
		886,860,835,811,787,		/*56-60*/
		764,742,720,699,678,		/*61-65*/
		658,639,620,601,583,		/*66-70*/
		566,550,533,518,502,		/*71-75*/
		488,473,459,446,433,		/*76-80*/
		420,408,396,384,373,		/*81-85*/
		362,352,342,332,322,		/*86-90*/
		313,304,295,287,279,		/*91-95*/
		271,263,256,249,242,		/*96-100*/
		235,228,222,216,210,		/*101-105*/
		204,199,193,188,183,		/*106-110*/
		178 						/*Val Over tempC*/

//		178,173,168,164,160,		/*111-115*/
//		155,151,147,143,140,		/*116-120*/
//		136,132,129,126,122,		/*121-125*/
//		119,116,113,110,108,		/*126-130*/
//		105,102,100,97,95,			/*131-135*/
//		92,90,88,86,84,				/*136-140*/
//		82,80,78,76,74,				/*141-145*/
//		72,71,69,67,66				/*146-150*/
};

#define ADC_DEVIATION_TEMPC			50												//Độ lệch hoặc sai số cho phép
#define ADC_PTC_MIN					(PTC_temp[PTC_MAX_TEMPC] - ADC_DEVIATION_TEMPC)
#define ADC_PTC_MAX					(PTC_temp[0] + ADC_DEVIATION_TEMPC)				// ~ -1 or -2  do C
#define _Is_PTC_DISABLE(adc)		(adc > ADC_PTC_MAX) || (adc < ADC_PTC_MIN)

/* Kalman Filter FUNCTIONS ====================================================================================================== */
static void  km_init(struct Str_Kalman *kalman, float _varP, float _varM, float _initial);
static float km_process(struct Str_Kalman *kalman, float input);

static void km_init(struct Str_Kalman *kalman, float _varP, float _varM, float _initial)
{
	kalman->varP = _varP;	  //Giá trị ước tính sai số
	kalman->varM = _varM;	  //Giá trị ước tính sai số do nhiễu
	kalman->value = _initial; //Giá trị khởi tạo ban đầu
	kalman->p = 1;			  //Ã†Â°Ã¡Â»â€ºc lÃ†Â°Ã¡Â»Â£ng sai sÃ¡Â»â€˜
	kalman->k = 1;			  //Ã„â€˜Ã¡Â»â„¢ lÃ¡Â»Â£i
}

static float km_process(struct Str_Kalman *kalman, float input)
{
	// compute the value
	kalman->p = kalman->p + kalman->varP;
	kalman->k = kalman->p / (kalman->p + kalman->varM);
	kalman->value = kalman->value + kalman->k * (input - kalman->value);

	// update the estimates => Giá trị ước tính
	kalman->p = (1 - kalman->k) * kalman->p;

	return kalman->value;
}

//Put in Init MCU
void ADC_Init_KalmanFilter(){

	km_init(&Kalman_ProbesWter, 0.01, 20, 4050);	// ADC
	km_init(&Kalman_PTC_HeatSink, 0.01, 20, 2031); 	// ADC of 25 do C
	km_init(&Kalman_PTC_Boiler, 0.01, 20, 2031);	// ADC of 25 do C
}

///* =============================================== ADC FUNCTIONS =============================================== */
void ADC_Calculate_AVG (__IO uint16_t *InPut, __IO uint16_t *OutPut, uint16_t num_Samples/*x10*/){
	uint8_t i=0;
	static uint16_t c = 0;
	static uint32_t _total[_ID_ADC_TOTAL];

	if(++c <= num_Samples){
		for(i=0; i<_ID_ADC_TOTAL; i++) _total[i] += (uint32_t)InPut[i];

		//Lay ket qua ADC_CURRENT som hon 1/10 so voi cac ADC khac
		if((c % (num_Samples/10))==0) {
			OutPut[_ID_ADC_CURRENT] = _total[_ID_ADC_CURRENT]*10/num_Samples;
			_total[_ID_ADC_CURRENT] = 0;
		}
		return;
	}
	else
	{
		for(i=0; i<_ID_ADC_TOTAL; i++) {
			if(i==_ID_ADC_CURRENT) continue;
			OutPut[i] = (uint16_t)(_total[i]/num_Samples);
			_total[i] = 0;
		}
		c = 0;

        //Kalman filter
		OutPut[_ID_ADC_PROBE_WSENSOR]= (uint16_t) (km_process(&Kalman_ProbesWter, 	(float) OutPut[_ID_ADC_PROBE_WSENSOR]));
		OutPut[_ID_ADC_PTC_DIODE]    = (uint16_t) (km_process(&Kalman_PTC_HeatSink, (float) OutPut[_ID_ADC_PTC_DIODE]));
		OutPut[_ID_ADC_PTC_BOILER]	  = (uint16_t)(km_process(&Kalman_PTC_Boiler,   (float) OutPut[_ID_ADC_PTC_BOILER]));
	}
}

uint16_t	ADC_get_WterProbeThreshold(uint8_t param_tempC){

   //thong so nay lay tu ket qua testing
   if(_LIMIT(param_tempC,0,10/*độ C*/)) return(_ADC_WTER_PROBE_THESHOLD10);
   else if(_LIMIT(param_tempC,11,29)) 	return(_ADC_WTER_PROBE_THESHOLD29);
   else if(_LIMIT(param_tempC,30,50)) 	return(_ADC_WTER_PROBE_THESHOLD50);
   else if(_LIMIT(param_tempC,51,75)) 	return(_ADC_WTER_PROBE_THESHOLD75);

   return(_ADC_WTER_PROBE_THESHOLD100);
}

//return tempC of PTC sensor
uint8_t ADC_2TempC(uint16_t param_ADC){

	if(_Is_PTC_DISABLE(param_ADC)) return 0xFF; //Wrong

	for(uint8_t temp = 1; temp < PTC_MAX_TEMPC; temp++){
		if((param_ADC) > (PTC_temp[temp])) return (temp-1);
	}
	return PTC_MAX_TEMPC;
}


//return Old sensor status
ProbeWterSS_STT_t ADC_2WaterStt(uint8_t In_RL_stt, LogicStatus In_Mid_logic){

	if(In_RL_stt == 0) return _ProbeWterSS_RLOFF;

	if(In_Mid_logic == _LOW) return _ProbeWterSS_FULL;
	else if(In_Mid_logic == _HIGH) return _ProbeWterSS_DRY;

	return _ProbeWterSS_UNKNOW;
}

//chi lay 1 lan duy nhat
uint16_t ADC_getZeroCurrent( uint16_t Adc_param){

	if(_IS_CALIB_CURRENT(Adc_param)) return Adc_param;
	return _ADC_0A_DEFAULT;
}

//Đo dòng điện
void ADC_2CurrentAC(uint8_t param_TimeHalfCycle,
					uint16_t param_ZeroCurrentADC,
					__IO uint16_t paramCurrentADC,
					__IO uint16_t *OutPut_mA,
					__IO Current_Shape_t *OutPut_Shape){

	static MeasureCurrState_TypeDef state = 0;
	static uint8_t cnt_Samples, get_HalfCycle_AC;
	static uint32_t Pos_ADC_total=0, Neg_ADC_total=0;

	switch(state){
		case _Measure_Curr_None:

			if(_LIMIT(param_TimeHalfCycle,_AC_FREQUENCY_60HZ-10,_AC_FREQUENCY_50HZ+10)) get_HalfCycle_AC = param_TimeHalfCycle; 		//~98
			else get_HalfCycle_AC = _AC_FREQUENCY_50HZ ;//default for Lexor

			cnt_Samples = 2*get_HalfCycle_AC; 					//biến lấy mẫu full duty cua Sinwave
			Pos_ADC_total = 0;
			Neg_ADC_total = 0;
			state = _Measure_Curr_Start;
			break;
		case _Measure_Curr_Start:
			if(paramCurrentADC > param_ZeroCurrentADC) Pos_ADC_total += (paramCurrentADC - param_ZeroCurrentADC);
			else Neg_ADC_total += (param_ZeroCurrentADC - paramCurrentADC);

			if(--cnt_Samples == 0) state = _Measure_Curr_Done;
			break;
		case _Measure_Curr_Done:
			Pos_ADC_total 	= (uint16_t)(Pos_ADC_total/(2*get_HalfCycle_AC));
			Neg_ADC_total 	= (uint16_t)(Neg_ADC_total/(2*get_HalfCycle_AC));
			*OutPut_mA 		= (uint16_t)((Pos_ADC_total + Neg_ADC_total)*35*_AMPS_PP/4095);

			state = _Measure_Curr_None;

			if((Pos_ADC_total < 30/*mA*/) && (Neg_ADC_total < 30/*mA*/)){
				//*OutPut_mA = 0;
				*OutPut_Shape = _CURRENT_SHAPE_NONE;
				return;
			}
			else if(Pos_ADC_total < 50/*mA*/) *OutPut_Shape = _CURRENT_SHAPE_NEG;
			else if(Neg_ADC_total < 50/*mA*/) *OutPut_Shape = _CURRENT_SHAPE_POS;
			else *OutPut_Shape = _CURRENT_SHAPE_SINE;

			break;
	}

}
