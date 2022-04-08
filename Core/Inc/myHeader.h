/*
 * myHeader.h
 *
 *  Created on: Nov 6, 2020
 *      Author: NGUYEN VAN HIEP
 */
#ifndef INC_MYHEADER_H_
#define INC_MYHEADER_H_

#include "main.h"

#include "stdint.h"
#include "string.h"
#include "stdarg.h"

#include "myDefine.h"

#include "common.h"
#include "timer.h"
#include "SW_interface.h"
#include "OUTPUT_ctrl_interface.h"
#include "get_string.h"

#include "printf.h"
#include "debug_v.h"
#include "ADC_interface.h"
#include "flash.h"
#include "Uv_CapSS.h"
#include "interrupt.h"

#include "init_MCU.h"
#include "CtrlRGB.h"
#include "CapSS_interface.h"
#include "RS232LL.h"
#include "AF4_RS232.h"
#include "Steamer.h"
#include "AF_Control.h"
#include "BLE_HC05.h"
#include "handle_com_wifi_c.h"

/************************************************   get information extern from main.c  *************************************************************/
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

extern IWDG_HandleTypeDef hiwdg;

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim14;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4;

#endif /* INC_MYHEADER_H_ */
