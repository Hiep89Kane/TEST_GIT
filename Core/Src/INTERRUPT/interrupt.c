#include "myHeader.h"

uint8_t 		Uart1_RX,
				Uart2_RX,
				Uart3_RX,
				Uart4_RX;
__IO ITStatus 	Uart1_Ready = RESET,
				Uart2_Ready = RESET,
				Uart3_Ready = RESET,
				Uart4_Ready = RESET;

__IO Current_Shape_t Cur_Shape;

/* =============================================== UART FUNCTIONS ========================*/
#ifdef _INIT_USE_UART1
	int uart1_putc(int c){
		HAL_UART_Transmit_IT(&huart1, (uint8_t*)&c, 1);
		while(Uart1_Ready != SET);
		Uart1_Ready = RESET;
		return c;
	}

	int uart1_puts(char *s){
		HAL_UART_Transmit_IT(&huart1, (uint8_t*)s, strlen(s));
		while(Uart1_Ready != SET);
		Uart1_Ready = RESET;
		return strlen(s);
	}
#endif

#ifdef _INIT_USE_UART2
	int uart2_putc(int c){
		HAL_UART_Transmit_IT(&huart2, (uint8_t*)&c, 1);
		while (Uart2_Ready != SET);
		Uart2_Ready = RESET;
		return c;
	}

	int uart2_puts(char *s){
		HAL_UART_Transmit_IT(&huart2, (uint8_t*)s, strlen(s));
		while(Uart2_Ready != SET);
		Uart2_Ready = RESET;
		return strlen(s);
	}
#endif

#ifdef _INIT_USE_UART3
	int uart3_putc(int c){
		uint32_t	start_stick;

		start_stick = HAL_GetTick();
		HAL_UART_Transmit_IT(&huart3, (uint8_t*)&c, 1);
		while(Uart3_Ready != SET){
			if(( HAL_GetTick()-start_stick) > 100/*ms*/) break;
		}
		Uart3_Ready = RESET;
		return c;
	}

	int uart3_puts(char *s){
		uint32_t	start_stick;

		start_stick = HAL_GetTick();
		HAL_UART_Transmit_IT(&huart3, (uint8_t*)s, strlen(s));
		while(Uart3_Ready != SET){
			if(( HAL_GetTick()-start_stick) > 100/*ms*/)  break;
		}
		Uart3_Ready = RESET;
		return strlen(s);
	}
#endif

#ifdef _INIT_USE_UART4
	int uart4_putc(int c){
		uint32_t	start_stick;

		start_stick = HAL_GetTick();
		HAL_UART_Transmit_IT(&huart4, (uint8_t*)&c, 1);
		while (Uart4_Ready != SET){
			if(( HAL_GetTick()-start_stick) > 1000/*ms*/)  break;
		}
		Uart4_Ready = RESET;
		return c;
	}

	int uart4_puts(char *s){
		uint32_t	start_stick;

		//start_stick = HAL_GetTick();
		HAL_UART_Transmit_IT(&huart4, (uint8_t*)s, strlen(s));
		while(Uart4_Ready != SET){
			if(( HAL_GetTick()-start_stick) > 1000/*ms*/)  break;
		}
		Uart4_Ready = RESET;
		return strlen(s);
	}
#endif

/****
 * Description : hàm Interrupt Tx Uart (khi có sự kiện ngắt Tx Uart nào đó đều nhảy vào đây thực hiện)
 * Param : tham số là con trỏ huart
 * Postion : ở đây
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	#ifdef _INIT_USE_UART1
		if(huart->Instance == huart1.Instance) Uart1_Ready = SET;
	#endif

	#ifdef _INIT_USE_UART2
		if(huart->Instance == huart2.Instance) Uart2_Ready = SET;
	#endif

	#ifdef _INIT_USE_UART3
		if(huart->Instance == huart3.Instance) Uart3_Ready = SET;
	#endif

	#ifdef _INIT_USE_UART4
		if(huart->Instance == huart4.Instance) Uart4_Ready = SET;
	#endif
}
/**
  * @brief  Rx Transfer completed callback.
  * @param  huart UART handle.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){

	#ifdef _INIT_USE_UART1
		if(huart->Instance == huart1.Instance){HAL_UART_Receive_IT(&huart1, &Uart1_RX, 1);}
	#endif

	#ifdef _INIT_USE_UART2
		if(huart->Instance == huart2.Instance){HAL_UART_Receive_IT(&huart2, &Uart2_RX, 1);}
	#endif

	#ifdef _INIT_USE_UART3
		if(huart->Instance == _USER_DEFINE_UART_BLEHC05.Instance){
			HAL_UART_Receive_IT(&_USER_DEFINE_UART_BLEHC05, &Uart3_RX, 1);
			get_string_get_input(&Uart3_StrBuff, Uart3_RX);

			//BLE_GetBuffer(Uart3_RX);
		}
	#endif

	#ifdef _INIT_USE_UART4
		if(huart->Instance == _USER_DEFINE_UART_RS232.Instance){
			HAL_UART_Receive_IT(&_USER_DEFINE_UART_RS232, &Uart4_RX, 1);
			RS232LL_RxGetBuff(Uart4_RX);
			//kiem tra Irq lien tuc
			timer_set(&AF_BOX4._timer_CheckRx, _AF_TIME_RESET_RS232);
		}
	#endif
}

/**
  * @brief  Conversion complete callback in non-blocking mode.
  * @param hadc ADC handle
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	//~170 us
	if(hadc->Instance == hadc1.Instance){
		//Average Filter
		ADC_Calculate_AVG(ADC_Arr, ADC_Avg_Arr, _ADC_SAMPLE_NUM);
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance == _USER_DEFINE_TIM_RGB.Instance){CtrlRGB_periodic_poll(CtrlRGB_calculate_phase(&RGB_2020));}
	if(htim->Instance == _USER_DEFINE_TIM_UVCAPSS.Instance){UV_CAPSS_periodic_poll();}

	//100us
	if(htim->Instance == _USER_DEFINE_TIM_DIMER_TRIAC.Instance){
		ADC_2CurrentAC(KETTLE.AC_Duty, ADC_Calib0mA, ADC_Avg_Arr[_ID_ADC_CURRENT], &TotalCurrent_mA, &Cur_Shape);
		CtrlRLAC_periodic_poll(KETTLE.AC_Duty_cnt, Global_RLAC_Manager);
		STEAMER_DimerCtrl_periodic_poll(&KETTLE);
	}

	if(htim->Instance == htim14.Instance){
		//debugv_periodic_poll();
	}
}


/**
  * @brief  SYSTICK callback.
  * @retval None
  */
void HAL_SYSTICK_Callback(void){
	static uint8_t 	tx10;

	//1ms
	CapSS_periodic_poll(CapSS_selected, CAPSENSOR);

	//10ms
	if(++tx10 >= 10){
	   tx10 = 0;
	   timer_periodic_poll();
	   OUTPUT_periodic_poll();
	   STEAMER_GetStt_Systick(&KETTLE, ADC_Avg_Arr);
	}
}
