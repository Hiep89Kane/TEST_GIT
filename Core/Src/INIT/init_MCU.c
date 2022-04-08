#include "myHeader.h"

__IO uint8_t Global_RLAC_Manager = 0, ZeroPulse_delay = 0;
struct timer  _timer_StartGetCurrt;  	//Chờ tính toán dòng điện ban đầu khi reset MCU
__IO uint16_t TotalCurrent_mA = 0;		//mA
__IO uint16_t ADC_Calib0mA = 0;			//~2048
uint8_t 	  reCalib_ZeroCurrent;

OUTPUT_struct_t 	_CTRL_LED1_YE,
					_CTRL_LED2_BL,
					_CTRL_BUZ,
					_CTRL_RLCOM,
					_CTRL_SEN5V,
					_CTRL_SOLAF,
					_CTRL_RLOldSS,
					_CTRL_SOLST;
//Note: must check => OUTPUT_number_max = 8
static struct timer 		_timer_Debugv;
extern __IO Current_Shape_t Cur_Shape;

get_string_str Uart3_StrBuff;

static void string_Uart3_report(char *string, uint16_t size);

static void string_Uart3_report(char *string, uint16_t size){

	raw_data((com_protocol_t* )string, size);
//	debug_msg("\r\nUartRx=%u",size);
//	for(uint8_t i=0; i<size; i++){
//		debug_msg(" %u", string[i]);
//	}
}


/*FUNCTIONS COMMON */
void LED1_BaseCtrl(uint8_t ON_OFF) {
	if (ON_OFF) LED1_ON;
	else LED1_OFF;
}

void LED2_BaseCtrl(uint8_t ON_OFF) {
	if (ON_OFF) LED2_ON;
	else LED2_OFF;
}

void BUZ_BaseCtrl(uint8_t ON_OFF) {
	if (ON_OFF) BUZ_ON;
	else BUZ_OFF;
}
void AF_RLCOM_BaseCtrl(uint8_t ON_OFF) {
	if (ON_OFF) RL_COM_ON;
	else RL_COM_OFF;
}

void AF_SEN5V_BaseCtrl(uint8_t OFF_ON) {
	if (OFF_ON) SS_5V_CTRL_OFF;
	else SS_5V_CTRL_ON;
}

void AF_SOL_BaseCtrl(uint8_t ON_OFF) {
	if (ON_OFF) FET_SOL_AF_ON;
	else FET_SOL_AF_OFF;
}

void AF_WHIRL_BaseCtrl(uint8_t ON_OFF) {
	if (ON_OFF) RL_WHIRL_On;
	else RL_WHIRL_Off;
}

void AF_DRAIN_BaseCtrl(uint8_t ON_OFF) {
	if (ON_OFF) RL_DRAIN_On;
	else RL_DRAIN_Off;
}

void ST_RLOldSS_BaseCtrl(uint8_t ON_OFF) {
	if (ON_OFF) RL_SS_STEAM_ON;
	else RL_SS_STEAM_OFF;
}

void ST_SOL_BaseCtrl(uint8_t ON_OFF) {
	if (ON_OFF) FET_SOL_STEAM_ON;
	else FET_SOL_STEAM_OFF;
}

void ST_AIRPUMP_BaseCtrl(uint8_t ON_OFF) {
	if (ON_OFF) RL_AIR_ON;
	else RL_AIR_OFF;
}

void ST_RL100_BaseCtrl(uint8_t ON_OFF) {
	if (ON_OFF) RL_STEAM100_ON;
	else RL_STEAM100_OFF;
}
//=============================================FUNCTIONS INIT MCU ========================*/
void init_MCU_TIM(){
	HAL_TIM_Base_Start_IT(&_USER_DEFINE_TIM_DIMER_TRIAC);
}

void init_MCU_get_MAC(uint64_t *out_MAC, uint32_t ADR){
	*out_MAC = Flash_ReadDWord(ADR);
}

void CtrlRLAC_periodic_poll(__IO uint16_t AC_Zero_point,__IO uint8_t RLAC_Manager_Var){
	if(AC_Zero_point != ZeroPulse_delay) return;
	AF_WHIRL_BaseCtrl(rbi(RLAC_Manager_Var,_RLWHIRL_Bit));
	AF_DRAIN_BaseCtrl(rbi(RLAC_Manager_Var,_RLDRAIN_Bit));
	ST_AIRPUMP_BaseCtrl(rbi(RLAC_Manager_Var,_RLAIR_Bit));
}

void init_MCU_IO_Ctrl(){

/*COMMON IO CTRL*/
	OUTPUT_config_new_control(&_CTRL_LED1_YE, &LED1_BaseCtrl);
	OUTPUT_config_new_control(&_CTRL_LED2_BL, &LED2_BaseCtrl);
	OUTPUT_config_new_control(&_CTRL_BUZ, &BUZ_BaseCtrl);

	OUTPUT_config_new_control(&_CTRL_RLCOM, &AF_RLCOM_BaseCtrl); //Relay tong

/*AF IO CTRL*/
	//For DC
	OUTPUT_config_new_control(&_CTRL_SEN5V, &AF_SEN5V_BaseCtrl);
	OUTPUT_config_new_control(&_CTRL_SOLAF, &AF_SOL_BaseCtrl);

/*STEAMER IO CTRL*/
	//For DC
	OUTPUT_config_new_control(&_CTRL_RLOldSS, &ST_RLOldSS_BaseCtrl);
	OUTPUT_config_new_control(&_CTRL_SOLST, &ST_SOL_BaseCtrl);
}

/****
 * Description : hàm khởi tạo Uart , sử dụng define để chỉ khởi tạo Uart nào cần dùng
 * Postion : đặt trong trước while(1)
 */
void init_MCU_UART(UART_HandleTypeDef *huart){
	#ifdef _INIT_USE_UART1
		if(huart->Instance == huart1.Instance){
			HAL_UART_Init(&huart1);
			HAL_UART_Receive_IT(&huart1, &Uart1_RX, 1);
			__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
			__HAL_UART_ENABLE_IT(&huart1, UART_IT_TC);
		}
	#endif

	#ifdef _INIT_USE_UART2
		if(huart->Instance == huart2.Instance){
			HAL_UART_Init(&huart2);
			HAL_UART_Receive_IT(&huart2, &Uart2_RX, 1);
			__HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
			__HAL_UART_ENABLE_IT(&huart2, UART_IT_TC);
		}
	#endif

	#ifdef _INIT_USE_UART3
		if(huart->Instance == huart3.Instance){
			HAL_UART_Init(&huart3);
			HAL_UART_Receive_IT(&huart3, &Uart3_RX, 1);
			__HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);
			__HAL_UART_ENABLE_IT(&huart3, UART_IT_TC);
		}
	#endif

	#ifdef _INIT_USE_UART4
		if(huart->Instance == huart4.Instance){
			HAL_UART_Init(&huart4);
			HAL_UART_Receive_IT(&huart4, &Uart4_RX, 1);
			__HAL_UART_ENABLE_IT(&huart4, UART_IT_RXNE);
			__HAL_UART_ENABLE_IT(&huart4, UART_IT_TC);
		}
	#endif

	//thông báo khi nhận đủ chuỗi
	get_string_init_notify(&Uart3_StrBuff, &string_Uart3_report);
}

/****
 * Description:	Hàm khởi tạo PWM
 * Param : 		none
 * Postion:		trước while(1)
 */
void init_MCU_PWM(){
	HAL_TIM_Base_Start_IT(&_CTRLRGB_TIMER_PWM);
	if(HAL_TIM_PWM_Start(&_CTRLRGB_TIMER_PWM, _TIM_PWM_CH_RED) != HAL_OK) Error_Handler();
	if(HAL_TIM_PWM_Start(&_CTRLRGB_TIMER_PWM, _TIM_PWM_CH_GREEN) != HAL_OK) Error_Handler();
	if(HAL_TIM_PWM_Start(&_CTRLRGB_TIMER_PWM, _TIM_PWM_CH_BLUE) != HAL_OK) Error_Handler();
	if(HAL_TIM_PWM_Start(&_CTRLRGB_TIMER_PWM, _TIM_PWM_CH_SPOT) != HAL_OK) Error_Handler();
}

/****
 * Description:	Ham khoi tao DMA of ADC
 * Param : 		none
 * Postion:		truoc While
 */
void init_MCU_ADC1_DMA(){
	if(HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK) Error_Handler();
	if(HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_Arr, _ID_ADC_TOTAL) != HAL_OK) Error_Handler();
}

/****
 * Description : hàm reset_UART chủ động
 * Param : tham số là huartx cần reset
 * Postion : đặt ở vị trí cần Reset uart
 */
void reset_UART(UART_HandleTypeDef *huart){
	if(HAL_UART_Init(huart) != HAL_OK) Error_Handler();

	__HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(huart, UART_IT_TC);
	#ifdef _INIT_USE_UART1
		if(huart->Instance == huart1.Instance) HAL_UART_Receive_IT(&huart1, &Uart1_RX, 1);
	#endif

	#ifdef _INIT_USE_UART2
		if(huart->Instance == huart2.Instance) HAL_UART_Receive_IT(&huart2, &Uart2_RX, 1);
	#endif

	#ifdef _INIT_USE_UART3
		if(huart->Instance == huart3.Instance) HAL_UART_Receive_IT(&huart3, &Uart3_RX, 1);
	#endif

	#ifdef _INIT_USE_UART4
		if(huart->Instance == huart4.Instance) HAL_UART_Receive_IT(&huart4, &Uart4_RX, 1);
	#endif
}

/****
 * Description : ham kiem tra nguyen nhan reset MCU
 */
void init_DebugCauseRstMCU(){
	uint8_t i;

	debug_msg("\nMAC=");
	for(i=0; i<8 ; i++){
		debug_msg("%x ",(uint8_t)(AF4_MAC_u64_Flash>>(i*8)));
	}
	debug_msg("\n");
	if(__HAL_RCC_GET_FLAG(RCC_FLAG_LSIRDY)) debug_msg("LSI Ready");
	else if(__HAL_RCC_GET_FLAG(RCC_FLAG_OBLRST)) debug_msg("Option Byte Loader reset flag");
	else if(__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) == SET) debug_msg("PIN RST");
	else if(__HAL_RCC_GET_FLAG(RCC_FLAG_PWRRST)) debug_msg("POR/PDR RST");
	else if(__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST)) debug_msg("Software RST");
	else if(__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)) debug_msg("IWDG RST");
	else if(__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST)) debug_msg("Window WDG RST");
	else if(__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST)) debug_msg("Low Power RST");
	else debug_msg("Unknow RST");
	__HAL_RCC_CLEAR_RESET_FLAGS();

	timer_set(&_timer_Debugv, 12*CLOCK_SECOND);
}

void Led1_AFdebug_poll(uint8_t info_byte0, uint8_t info_byte2){
	uint8_t blink_num;

	blink_num = 0;
	//check byte 2
	if(rbi(info_byte2,_INFO_PROBLEM_FAN_Bit) == _FAIL)			blink_num = _AfDebug_Fan_Hot;
	if(rbi(info_byte2,_INFO_PROBLEM_DISTEMP2_Bit) == _FAIL) 	blink_num = _AfDebug_DisTem2_AcDiode;
	if(rbi(info_byte2,_INFO_PROBLEM_DISTEMP1_Bit) == _FAIL) 	blink_num = _AfDebug_DisTem1_Boiler;
	if(rbi(info_byte2,_INFO_PROBLEM_CURR_OVERLOAD_Bit) == _FAIL)blink_num = _AfDebug_Current_OverLoad;
	if(rbi(info_byte2,_INFO_PROBLEM_CURR_ZERO_Bit) == _FAIL)	blink_num = _AfDebug_Current_Zero;
	if(rbi(info_byte2,_INFO_PROBLEM_CURR_LEAKAGE1_Bit) == _FAIL)blink_num = _AfDebug_Current_Leak1;
	if(rbi(info_byte2,_INFO_PROBLEM_CURR_LEAKAGE0_Bit) == _FAIL)blink_num = _AfDebug_Current_Leak0;
	if(rbi(info_byte2,_INFO_PROBLEM_CURR_WRONG_Bit) == _FAIL)	blink_num = _AfDebug_Current_Wrong;

	//check byte 0
	if(rbi(info_byte0,_INFO_SSDRAIN_CONNECT_Bit) == _FAIL) blink_num = _AfDebug_DrainSS_Disconnect;
	if(rbi(info_byte0,_INFO_SSWATER_CONNECT_Bit) == _FAIL) blink_num = _AfDebug_WterSS_Disconnect;

	if(blink_num==0)return;
	LED_Green_AfDebug(blink_num);
}

void Debug_AllProject_Main_exe(){

	if(timer_expired(&_timer_Debugv) == 0) return;
	timer_restart(&_timer_Debugv);
//	Led1_AFdebug_poll(AF_BOX4.Info_AF4[_BYTE0], AF_BOX4.Info_AF4[_BYTE2]);
//
//	#ifdef	_USERS_DEBUG_CURRENT
//		debug_msg("\n-------------------");
//		debug_msg("\nShape=%u-Cur=%x-Zero=%b", Cur_Shape, TotalCurrent_mA, ADC_Calib0mA);
//	#endif
//
//	#ifdef _USERS_DEBUG_ADCS
//		debug_msg("\nADC[]={%u,%u,%u,%u}",	ADC_Avg_Arr[0],
//											ADC_Avg_Arr[1],
//											ADC_Avg_Arr[2],
//											ADC_Avg_Arr[3]);
//	#endif
//
//	#ifdef _USERS_DEBUG_AF4STT
//		debug_msg("\nInfo Byte210=0x %x %x %x", (AF_BOX4.Info_AF4[_BYTE2]),
//												(AF_BOX4.Info_AF4[_BYTE1]),
//												(AF_BOX4.Info_AF4[_BYTE0]));
//	#endif
//
//	#ifdef	_USERS_DEBUG_STEAMER
//		debug_msg("\nTemp=%u-%u AC=%u Pwr=%u-%u", 	KETTLE.tempC,
//													KETTLE.tempC_Diode,
//													KETTLE.AC_Duty,
//													KETTLE.Power_Level,
//													ST_Flash[_IDF_ST_POWER_TRIAC]);
//	#endif
//
//	debug_msg("\n-------------------");
}

void init_MCU_AllProjet(void){

	  /*<*! README :
	  *	In Users_Define.h ***!!
	  *
	  	*	huart1 	=> none use
	  	*	huart2 	=> none use
	  	*	huart3 	=> use for Bluetooth HC05
		*	huart4 	=> use for RS232 main
		* 	htim3	=> PWM
		*	htim6 	=> CapSS
		*	htim14 	=> Debug_v
		*	htim7   => 100us check AC source duty
	  */
	  ADC_Init_KalmanFilter();
	  init_MCU_UART(&_USER_DEFINE_UART_BLEHC05);
	  init_MCU_UART(&_USER_DEFINE_UART_RS232);
	  init_MCU_PWM();
	  init_MCU_ADC1_DMA();
	  init_MCU_TIM();
	  init_MCU_IO_Ctrl();

	  Global_RLAC_Manager=0;
	  ZeroPulse_delay = _ZERO_AC_POINT;//~1ms

	  //Lấy địa chỉ MAC từ Flash
	  init_MCU_get_MAC(&AF4_MAC_u64_Flash, (ADDR_FLASH_PAGE_63+ 8*_IDF_DWORD_MAC_ADDRESS));
	  init_DebugCauseRstMCU();

	  FAN_ON; delay_ms(200);//se OFF khi Ctrl_RelayCOM(1) => neu ko co loi

	  //Lấy giá trị ADC tương ứng với 0 mA lúc bắt đầu
	  ADC_Calib0mA = ADC_getZeroCurrent(ADC_Avg_Arr[_ID_ADC_CURRENT]);									debug_msg("\nZeroCurt=%u",ADC_Calib0mA);
	  TotalCurrent_mA = 0;
	  reCalib_ZeroCurrent = _RECALIB_COUNT_SAMPLE;
	  timer_set(&_timer_StartGetCurrt, CLOCK_SECOND);

	  BLE_init();
	  AF_Init(&AF_BOX4);
	  STEAMER_init(&KETTLE);

	  //LED1_Blink_Revision();
	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	debug_msg("\nAF3 STEAMER date:%x%x%x ..",_THIS_YEAR,_THIS_MONTH,_THIS_DAY);
}
