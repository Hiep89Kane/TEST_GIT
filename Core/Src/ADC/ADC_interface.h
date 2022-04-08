#ifndef INC_ADC_INTERFACE_H_
#define INC_ADC_INTERFACE_H_

#define ACS711_SEN					15		//(25/2) //12.5A - độ nhạy 110v/1A
#ifdef ACS711_SEN
	#define _ADC_0A_DEFAULT			2047u
#endif

/* NOTE : data sheet ACS711-12AB: "110mV/1A with Vcc = 3.3V"
 *
 * 	3300 mV     3300*1000/110=30000 mA			4095
 *  110  mv		1000 	    		mA			4095*1000/30000 = 4095/30
 *				x*1000*30/4095	 (?)mA			x (result ADC measured)
 *
 */
#define _TOLERANCE					40
#define _IS_CALIB_CURRENT(adc)		_LIMIT(adc, _ADC_0A_DEFAULT-_TOLERANCE, _ADC_0A_DEFAULT+_TOLERANCE)
#define _AMPS_RMS 					712 	// 0.707*A => 707 	mA
#define _AMPS_PP 					1000 	// 1*A     => 1000 	mA

typedef enum{
	_CURRENT_SHAPE_NONE,
	_CURRENT_SHAPE_POS,
	_CURRENT_SHAPE_NEG,
	_CURRENT_SHAPE_SINE,
}Current_Shape_t;

#define _ADC_LOGIC_COMPARE(adc,threshold)	((adc > threshold)?1:0)

#define _ADC_SAMPLE_NUM				30

#define _ADC_WTER_PROBE_THESHOLD10	4000 	// 0-10 độ C
#define _ADC_WTER_PROBE_THESHOLD29	3915 	// 11-29 độ C
#define _ADC_WTER_PROBE_THESHOLD50	3700 	// 30-50 độ C
#define _ADC_WTER_PROBE_THESHOLD75	3500	// 51-75 độ C
#define _ADC_WTER_PROBE_THESHOLD100	3300    // 76-100.. độ C

/* =============================================== DMA ADC FUNCTIONS =============================================== */
typedef enum{
	_ID_ADC_PROBE_WSENSOR,						/*!< chanel 0 in DMA array result                          */
	_ID_ADC_PTC_DIODE,							/*!< chanel 1 in DMA array result                          */
	_ID_ADC_PTC_BOILER,							/*!< chanel 2 in DMA array result                          */
	_ID_ADC_CURRENT,							/*!< chanel 3 in DMA array result                          */

	_ID_ADC_TOTAL
}DMA_ADCposition_t;

typedef enum{
	_ProbeWterSS_RLOFF,							/*!< khi relay ko bat*/
	_ProbeWterSS_DRY,							/*!< Khi Mid = LOW*/
	_ProbeWterSS_FULL,							/*!< khi Mid = HIGH*/
	_ProbeWterSS_UNKNOW
}ProbeWterSS_STT_t;

typedef enum{
	_Measure_Curr_None,
	_Measure_Curr_Start,
	_Measure_Curr_Done,
}MeasureCurrState_TypeDef;

struct Str_Kalman
{
	float varP;
	float varM;
	float value;
	float p;
	float k;
};

/* Global variables ---------------------------------------------------------*/
extern __IO uint16_t 		ADC_Arr[_ID_ADC_TOTAL],
					 	 	ADC_Avg_Arr[_ID_ADC_TOTAL];

extern struct Str_Kalman 	Kalman_ProbesWter,
							Kalman_PTC_HeatSink,
							Kalman_PTC_Boiler;

void  				ADC_Init_KalmanFilter();
/* ADC FUNCTIONS ====================================================================================================== */
/*Tính giá trị trung bình của tất cả các kênh ADC ,num_Samples phải bội số của 10 => put in IRQ DMA ADC*/
void 				ADC_Calculate_AVG(__IO uint16_t *InPut, __IO uint16_t *OutPut, uint16_t num_Samples/*x10*/);

/*Trả về giá trị ngưỡng so sánh của đầu dò mực nước tùy từng nhiệt đô trong bình nấu */
uint16_t			ADC_get_WterProbeThreshold(uint8_t param_tempC);

/*Trả về nhiệt độ trong bình nấu */
uint8_t	 			ADC_2TempC(uint16_t param_ADC);										/*!< put in anywhere  */
ProbeWterSS_STT_t	ADC_2WaterStt(uint8_t In_RL_stt, LogicStatus In_Mid_logic);			/*!< put in anywhere */

uint16_t		 	ADC_getZeroCurrent(uint16_t Adc_param);								/*!< Chi lay 1 lan duy nhat khi Reset MCU */

void 				ADC_2CurrentAC(uint8_t param_HalfCycle_AC,
									uint16_t param_ZeroCurrentADC,
									__IO uint16_t paramCurrentADC,
									__IO uint16_t *OutPut_mA,
									__IO Current_Shape_t *OutPut_Shape);

#endif /* INC_ADC_INTERFACE_H_ */
