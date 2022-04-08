/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SSDRAIN_LOGIC_Pin GPIO_PIN_11
#define SSDRAIN_LOGIC_GPIO_Port GPIOC
#define SSDRAIN_SIGNAL_Pin GPIO_PIN_12
#define SSDRAIN_SIGNAL_GPIO_Port GPIOC
#define IN_SAFETY_Pin GPIO_PIN_13
#define IN_SAFETY_GPIO_Port GPIOC
#define PC14_Pin GPIO_PIN_14
#define PC14_GPIO_Port GPIOC
#define SS_5V_CTRL_Pin GPIO_PIN_15
#define SS_5V_CTRL_GPIO_Port GPIOC
#define LED2_BL_Pin GPIO_PIN_1
#define LED2_BL_GPIO_Port GPIOC
#define BUZ_Pin GPIO_PIN_2
#define BUZ_GPIO_Port GPIOC
#define LED1_YE_Pin GPIO_PIN_3
#define LED1_YE_GPIO_Port GPIOC
#define FAN_Pin GPIO_PIN_6
#define FAN_GPIO_Port GPIOA
#define RL_SS_STEAM_Pin GPIO_PIN_7
#define RL_SS_STEAM_GPIO_Port GPIOA
#define RL_DRAIN_Pin GPIO_PIN_0
#define RL_DRAIN_GPIO_Port GPIOB
#define RL_WHIRL_Pin GPIO_PIN_1
#define RL_WHIRL_GPIO_Port GPIOB
#define RL_AIR_Pin GPIO_PIN_2
#define RL_AIR_GPIO_Port GPIOB
#define RL_STEAM100_Pin GPIO_PIN_10
#define RL_STEAM100_GPIO_Port GPIOB
#define RL_STEAM50_Pin GPIO_PIN_11
#define RL_STEAM50_GPIO_Port GPIOB
#define RL_COM_Pin GPIO_PIN_12
#define RL_COM_GPIO_Port GPIOB
#define BUG_IO_Pin GPIO_PIN_13
#define BUG_IO_GPIO_Port GPIOB
#define IN_BUTTON_Pin GPIO_PIN_14
#define IN_BUTTON_GPIO_Port GPIOB
#define IN_EN_DEBUG_Pin GPIO_PIN_15
#define IN_EN_DEBUG_GPIO_Port GPIOB
#define TO_MASS_Pin GPIO_PIN_8
#define TO_MASS_GPIO_Port GPIOA
#define IN_EL814_Pin GPIO_PIN_9
#define IN_EL814_GPIO_Port GPIOA
#define DIM_TRIAC_Pin GPIO_PIN_6
#define DIM_TRIAC_GPIO_Port GPIOC
#define BLE_STATE_Pin GPIO_PIN_7
#define BLE_STATE_GPIO_Port GPIOC
#define BLE_EN_Pin GPIO_PIN_10
#define BLE_EN_GPIO_Port GPIOA
#define PA11_Pin GPIO_PIN_11
#define PA11_GPIO_Port GPIOA
#define PA12_Pin GPIO_PIN_12
#define PA12_GPIO_Port GPIOA
#define TIM3_CH3_R_Pin GPIO_PIN_8
#define TIM3_CH3_R_GPIO_Port GPIOC
#define TIM3_CH4_SPOT_Pin GPIO_PIN_9
#define TIM3_CH4_SPOT_GPIO_Port GPIOC
#define PD0_Pin GPIO_PIN_0
#define PD0_GPIO_Port GPIOD
#define FET_SPOT_RIGHT_Pin GPIO_PIN_1
#define FET_SPOT_RIGHT_GPIO_Port GPIOD
#define FET_SPOT_LEFT_Pin GPIO_PIN_2
#define FET_SPOT_LEFT_GPIO_Port GPIOD
#define FET_SOL_AF_Pin GPIO_PIN_3
#define FET_SOL_AF_GPIO_Port GPIOD
#define FET_SOL_STEAM_Pin GPIO_PIN_4
#define FET_SOL_STEAM_GPIO_Port GPIOD
#define PB3_Pin GPIO_PIN_3
#define PB3_GPIO_Port GPIOB
#define TIM3_CH1_B_Pin GPIO_PIN_4
#define TIM3_CH1_B_GPIO_Port GPIOB
#define TIM3_CH2_G_Pin GPIO_PIN_5
#define TIM3_CH2_G_GPIO_Port GPIOB
#define RL_SOL_COM_Pin GPIO_PIN_6
#define RL_SOL_COM_GPIO_Port GPIOB
#define IN_JETSW_Pin GPIO_PIN_7
#define IN_JETSW_GPIO_Port GPIOB
#define SSWATER_SIGNAL_Pin GPIO_PIN_8
#define SSWATER_SIGNAL_GPIO_Port GPIOB
#define IN_DRAINSW_Pin GPIO_PIN_9
#define IN_DRAINSW_GPIO_Port GPIOB
#define SSWATER_LOGIC_Pin GPIO_PIN_10
#define SSWATER_LOGIC_GPIO_Port GPIOC
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
