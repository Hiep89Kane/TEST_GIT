/*
 * interrupt.h
 *
 *  Created on: Nov 6, 2020
 *      Author: NGUYEN VAN HIEP
 */

#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include "myHeader.h"

//void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);            	//===> Đã khai báo trong :  stm32g0xx_hal_uart.h
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);				//===> Đã khai báo trong :  stm32g0xx_hal_uart.h
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);			//===> Đã khai báo trong :  stm32g0xx_hal_tim.h
//void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);				//===> Đã khai báo trong :  stm32g0xx_hal_adc.h
//void HAL_SYSTICK_Callback(void);										//===> Đã khai báo trong :  stm32g0xx_hal_cortex.h

extern uint8_t 			Uart1_RX,
						Uart2_RX,
						Uart3_RX,
						Uart4_RX;
extern __IO ITStatus 	Uart1_Ready,
						Uart2_Ready,
						Uart3_Ready,
						Uart4_Ready;

#ifdef _INIT_USE_UART1
	int uart1_putc(int c);
	int uart1_puts(char *s);
#endif

#ifdef _INIT_USE_UART2
	int uart2_putc(int c);
	int uart2_puts(char *s);
#endif

#ifdef _INIT_USE_UART3
	int uart3_putc(int c);
	int uart3_puts(char *s);
#endif

#ifdef _INIT_USE_UART4
	int uart4_putc(int c);
	int uart4_puts(char *s);
#endif

#endif /*_INTERRUPT_H_ */
