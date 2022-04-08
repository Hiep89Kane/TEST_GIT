/*
 * User_Define.h
 *
 *  Created on: Nov 6, 2020
 *      Author: NGUYEN VAN HIEP
 */

#ifndef INC_MYDEFINE_H_
#define INC_MYDEFINE_H_

//#define _STEAMER_NONE_STOP	1

#define _THIS_YEAR 			0x22
#define _THIS_MONTH 		0x02	//thang 02
#define _THIS_DAY			0x17	//ngay 	14

#define _CONST_VALUE_CURRENT_LEAKAGE	600 	//dòng điện rò cho phép khi tất cả Relay đều OFF
#define _CONST_VALUE_CURRENT_RUN_MIN	50		//dòng điện nhỏ nhất khi có ít nhất 1 tải hoạt động
#define _CONST_VALUE_CURRENT_OVERLOAD	9200 	//dòng điện quá tải của control box 4 để bảo vệ cầu chì

//=> Số liệu đo được từ thực tế
#define _CUR_WHIRL			550 	/*mA*/
#define _CUR_WHIRL_MIN		300 	/*mA*/
#define _CUR_WHIRL_MAX		900 	/*mA*/

#define _CUR_DRAIN			1000	/*mA*/
#define _CUR_DRAIN_MIN		600		/*mA*/
#define _CUR_DRAIN_MAX		1500	/*mA*/

#define _CUR_AIR_AP25		110		/*mA*/
#define _CUR_AIR_AP45		500		/*mA*/
#define _CUR_AIR_USER		_CUR_AIR_AP25
#define _CUR_AIRPUMP_MIN	80		/*mA*/
#define _CUR_AIRPUMP_MAX	700		/*mA*/
#define _CUR_DETECT_AIRPUMP	80		/*mA*/

//Resistor Disc : 			120v/800W
#define _CUR_ST_P1			2500	/*mA*/
#define _CUR_ST100			6000	/*mA*/

/**
 * Kết Quả Đo các Tải của AF3 thực tế
 * WHirl = 550
 * Drain = 1000
 * AirPump 25 = 120
 * Steamer :
	 * P1:3000 - 45
	 * P2:3600 - 52
	 * P3:4000 - 57
	 * P4:4400 - 62
	 * P5:5900 - 70
 * **/

//----------------------------------------------------USER UARTS----------------------------------------------------------
//#define _INIT_USE_UART1
//#define _INIT_USE_UART2
#define _INIT_USE_UART3
#define _INIT_USE_UART4

//-----------------------------------------------------USER TIMERS--------------------------------------------------------
#define _USER_DEFINE_TIM_RGB 			htim3
#define	_USER_DEFINE_TIM_UVCAPSS		htim6
#define _USER_DEFINE_TIM_DIMER_TRIAC 	htim7

#define _USER_DEFINE_TIM_UVDEBUG		htim14

/*************************************************   Re-define   *************************************************************/
#ifdef _INIT_USE_UART1
	#define _USER_DEFINE_UART_NOUSE		huart1
#endif

#ifdef _INIT_USE_UART2
	#define _USER_DEFINE_UART_DEBUG		huart2
#endif

#ifdef _INIT_USE_UART3
	#define _USER_DEFINE_UART_BLEHC05	huart3
#endif

#ifdef _INIT_USE_UART4
	#define _USER_DEFINE_UART_RS232		huart4
#endif
/* ============================================== USERS PROJECT DEFINE =============================================== */
#define _USERS_DEBUG_ADCS
#define _USERS_DEBUG_AF4STT
#define _USERS_DEBUG_CURRENT
#define _USERS_DEBUG_STEAMER

//*================================================  DEFINE OUTPUT ====================================================-*/
//for Debug
#define TX_V_WritePin(logic)	__NOP()//HAL_GPIO_WritePin(TX_V_GPIO_Port, TX_V_Pin, logic)

#define FAN_ON 					HAL_GPIO_WritePin(FAN_GPIO_Port, FAN_Pin, 1)
#define FAN_OFF 				HAL_GPIO_WritePin(FAN_GPIO_Port, FAN_Pin, 0)
//RELAY
#define RL_COM_ON 				HAL_GPIO_WritePin(RL_COM_GPIO_Port, RL_COM_Pin, 1)
#define RL_COM_OFF 				HAL_GPIO_WritePin(RL_COM_GPIO_Port, RL_COM_Pin, 0)
#define RL_COM_TOG 				HAL_GPIO_TogglePin(RL_COM_GPIO_Port, RL_COM_Pin)
#define RL_COM_STT 				HAL_GPIO_ReadPin(RL_COM_GPIO_Port, RL_COM_Pin)

#define RL_SS_STEAM_ON 			HAL_GPIO_WritePin(RL_SS_STEAM_GPIO_Port, RL_SS_STEAM_Pin, 1)
#define RL_SS_STEAM_OFF 		HAL_GPIO_WritePin(RL_SS_STEAM_GPIO_Port, RL_SS_STEAM_Pin, 0)
#define RL_SS_STEAM_TOG 		HAL_GPIO_TogglePin(RL_SS_STEAM_GPIO_Port, RL_SS_STEAM_Pin)
#define RL_SS_STEAM_STT 		HAL_GPIO_ReadPin(RL_SS_STEAM_GPIO_Port, RL_SS_STEAM_Pin)

#define RL_DRAIN_On 			HAL_GPIO_WritePin(RL_DRAIN_GPIO_Port, RL_DRAIN_Pin, 1)
#define RL_DRAIN_Off 			HAL_GPIO_WritePin(RL_DRAIN_GPIO_Port, RL_DRAIN_Pin, 0)
#define RL_DRAIN_TOG 			HAL_GPIO_TogglePin(RL_DRAIN_GPIO_Port, RL_DRAIN_Pin)
#define RL_DRAIN_STT 			HAL_GPIO_ReadPin(RL_DRAIN_GPIO_Port, RL_DRAIN_Pin)

#define RL_WHIRL_On 			HAL_GPIO_WritePin(RL_WHIRL_GPIO_Port, RL_WHIRL_Pin, 1)
#define RL_WHIRL_Off 			HAL_GPIO_WritePin(RL_WHIRL_GPIO_Port, RL_WHIRL_Pin, 0)
#define RL_WHIRL_TOG 			HAL_GPIO_TogglePin(RL_WHIRL_GPIO_Port, RL_WHIRL_Pin)
#define RL_WHIRL_STT 			HAL_GPIO_ReadPin(RL_WHIRL_GPIO_Port, RL_WHIRL_Pin)

#define RL_AIR_ON 				HAL_GPIO_WritePin(RL_AIR_GPIO_Port, RL_AIR_Pin, 1)
#define RL_AIR_OFF 				HAL_GPIO_WritePin(RL_AIR_GPIO_Port, RL_AIR_Pin, 0)
#define RL_AIR_TOG 				HAL_GPIO_TogglePin(RL_AIR_GPIO_Port, RL_AIR_Pin)
#define RL_AIR_STT				HAL_GPIO_ReadPin(RL_AIR_GPIO_Port, RL_AIR_Pin)

#define RL_STEAM100_ON 			HAL_GPIO_WritePin(RL_STEAM100_GPIO_Port, RL_STEAM100_Pin, 1)
#define RL_STEAM100_OFF 		HAL_GPIO_WritePin(RL_STEAM100_GPIO_Port, RL_STEAM100_Pin, 0)
#define RL_STEAM100_TOG 		HAL_GPIO_TogglePin(RL_STEAM100_GPIO_Port, RL_STEAM100_Pin)
#define RL_STEAM100_STT 		HAL_GPIO_ReadPin(RL_STEAM100_GPIO_Port, RL_STEAM100_Pin)

#define RL_STEAM50_ON 			HAL_GPIO_WritePin(RL_STEAM50_GPIO_Port, RL_STEAM50_Pin, 1)
#define RL_STEAM50_OFF 			HAL_GPIO_WritePin(RL_STEAM50_GPIO_Port, RL_STEAM50_Pin, 0)
#define RL_STEAM50_TOG 			HAL_GPIO_TogglePin(RL_STEAM50_GPIO_Port, RL_STEAM50_Pin)
#define RL_STEAM50_STT 			HAL_GPIO_ReadPin(RL_STEAM50_GPIO_Port, RL_STEAM50_Pin)

#define RL_SOL_COM_ON 			HAL_GPIO_WritePin(RL_SOL_COM_GPIO_Port, RL_SOL_COM_Pin, 1)
#define RL_SOL_COM_OFF 			HAL_GPIO_WritePin(RL_SOL_COM_GPIO_Port, RL_SOL_COM_Pin, 0)
#define RL_SOL_COM_TOG 			HAL_GPIO_TogglePin(RL_SOL_COM_GPIO_Port, RL_SOL_COM_Pin)
#define RL_SOL_COM_STT 			HAL_GPIO_ReadPin(RL_SOL_COM_GPIO_Port, RL_SOL_COM_Pin)

//OUTPUT FET
#define FET_SPOT_LEFT_ON 		HAL_GPIO_WritePin(FET_SPOT_LEFT_GPIO_Port, FET_SPOT_LEFT_Pin, 0)
#define FET_SPOT_LEFT_OFF 		HAL_GPIO_WritePin(FET_SPOT_LEFT_GPIO_Port, FET_SPOT_LEFT_Pin, 1)
#define FET_SPOT_LEFT_TOG 		HAL_GPIO_TogglePin(FET_SPOT_LEFT_GPIO_Port, FET_SPOT_LEFT_Pin)
#define FET_SPOT_LEFT_STT 		!HAL_GPIO_ReadPin(FET_SPOT_LEFT_GPIO_Port, FET_SPOT_LEFT_Pin)

//OUTPUT FET
#define FET_SPOT_RIGHT_ON 		HAL_GPIO_WritePin(FET_SPOT_RIGHT_GPIO_Port, FET_SPOT_RIGHT_Pin, 0)
#define FET_SPOT_RIGHT_OFF 		HAL_GPIO_WritePin(FET_SPOT_RIGHT_GPIO_Port, FET_SPOT_RIGHT_Pin, 1)
#define FET_SPOT_RIGHT_TOG 		HAL_GPIO_TogglePin(FET_SPOT_RIGHT_GPIO_Port, FET_SPOT_RIGHT_Pin)
#define FET_SPOT_RIGHT_STT 		!HAL_GPIO_ReadPin(FET_SPOT_RIGHT_GPIO_Port, FET_SPOT_RIGHT_Pin)

#define FET_SOL_AF_ON 			HAL_GPIO_WritePin(FET_SOL_AF_GPIO_Port, FET_SOL_AF_Pin, 0)
#define FET_SOL_AF_OFF 			HAL_GPIO_WritePin(FET_SOL_AF_GPIO_Port, FET_SOL_AF_Pin, 1)
#define FET_SOL_AF_TOG 			HAL_GPIO_TogglePin(FET_SOL_AF_GPIO_Port, FET_SOL_AF_Pin)
#define FET_SOL_AF_STT 			!HAL_GPIO_ReadPin(FET_SOL_AF_GPIO_Port, FET_SOL_AF_Pin)

#define FET_SOL_STEAM_ON 		HAL_GPIO_WritePin(FET_SOL_STEAM_GPIO_Port, FET_SOL_STEAM_Pin, 0)
#define FET_SOL_STEAM_OFF 		HAL_GPIO_WritePin(FET_SOL_STEAM_GPIO_Port, FET_SOL_STEAM_Pin, 1)
#define FET_SOL_STEAM_TOG 		HAL_GPIO_TogglePin(FET_SOL_STEAM_GPIO_Port, FET_SOL_STEAM_Pin)
#define FET_SOL_STEAM_STT 		!HAL_GPIO_ReadPin(FET_SOL_STEAM_GPIO_Port, FET_SOL_STEAM_Pin)

//OUTPUT SENSOR
#define DRAINSS_SIGNAL_HI 		HAL_GPIO_WritePin(SSDRAIN_SIGNAL_GPIO_Port, SSDRAIN_SIGNAL_Pin, 1)
#define DRAINSS_SIGNAL_LOW 		HAL_GPIO_WritePin(SSDRAIN_SIGNAL_GPIO_Port, SSDRAIN_SIGNAL_Pin, 0)
#define DRAINSS_SIGNAL_TOG 		HAL_GPIO_TogglePin(SSDRAIN_SIGNAL_GPIO_Port, SSDRAIN_SIGNAL_Pin)
#define DRAINSS_SIGNAL_STT 		HAL_GPIO_ReadPin(SSDRAIN_SIGNAL_GPIO_Port, SSDRAIN_SIGNAL_Pin)

#define WATERSS_SIGNAL_HI 		HAL_GPIO_WritePin(SSWATER_SIGNAL_GPIO_Port, SSWATER_SIGNAL_Pin, 1)
#define WATERSS_SIGNAL_LOW 		HAL_GPIO_WritePin(SSWATER_SIGNAL_GPIO_Port, SSWATER_SIGNAL_Pin, 0)
#define WATERSS_SIGNAL_TOG 		HAL_GPIO_TogglePin(SSWATER_SIGNAL_GPIO_Port, SSWATER_SIGNAL_Pin)
#define WATERSS_SIGNAL_STT 		HAL_GPIO_ReadPin(SSWATER_SIGNAL_GPIO_Port, SSWATER_SIGNAL_Pin)

//OUTPUT IC SWITCH
#define SS_5V_CTRL_ON 			HAL_GPIO_WritePin(SS_5V_CTRL_GPIO_Port, SS_5V_CTRL_Pin, 1)
#define SS_5V_CTRL_OFF 			HAL_GPIO_WritePin(SS_5V_CTRL_GPIO_Port, SS_5V_CTRL_Pin, 0)
#define SS_5V_CTRL_TOG 			HAL_GPIO_TogglePin(SS_5V_CTRL_GPIO_Port, SS_5V_CTRL_Pin)
#define SS_5V_CTIn_STT 			HAL_GPIO_ReadPin(SS_5V_CTRL_GPIO_Port, SS_5V_CTRL_Pin)

//OUTPUT BUZZER
#define BUZ_ON 					HAL_GPIO_WritePin(BUZ_GPIO_Port, BUZ_Pin, 1)
#define BUZ_OFF 				HAL_GPIO_WritePin(BUZ_GPIO_Port, BUZ_Pin, 0)
#define BUZ_TOG 				HAL_GPIO_TogglePin(BUZ_GPIO_Port, BUZ_Pin)
#define BUZ_STT 				HAL_GPIO_ReadPin(BUZ_GPIO_Port, BUZ_Pin)
//OUTPUT LED1
#define LED1_ON		 			HAL_GPIO_WritePin(LED1_YE_GPIO_Port, LED1_YE_Pin, 1)
#define LED1_OFF 				HAL_GPIO_WritePin(LED1_YE_GPIO_Port, LED1_YE_Pin, 0)
#define LED1_TOG 				HAL_GPIO_TogglePin(LED1_YE_GPIO_Port, LED1_YE_Pin)
#define LED1_STT 				HAL_GPIO_ReadPin(LED1_YE_GPIO_Port, LED1_YE_Pin)

//OUTPUT LED1
#define LED2_ON		 			HAL_GPIO_WritePin(LED2_BL_GPIO_Port, LED2_BL_Pin, 1)
#define LED2_OFF 				HAL_GPIO_WritePin(LED2_BL_GPIO_Port, LED2_BL_Pin, 0)
#define LED2_TOG 				HAL_GPIO_TogglePin(LED2_BL_GPIO_Port, LED2_BL_Pin)
#define LED2_STT 				HAL_GPIO_ReadPin(LED2_BL_GPIO_Port, LED2_BL_Pin)

//OUTPUT BLE
#define BLE_EN_ON 				HAL_GPIO_WritePin(BLE_EN_GPIO_Port, BLE_EN_Pin, 1)
#define BLE_EN_OFF 				HAL_GPIO_WritePin(BLE_EN_GPIO_Port, BLE_EN_Pin, 0)
#define BLE_EN_TOG 				HAL_GPIO_TogglePin(BLE_EN_GPIO_Port, BLE_EN_Pin)
#define BLE_EN_STT 				HAL_GPIO_ReadPin(BLE_EN_GPIO_Port, BLE_EN_Pin)

#define BUG_IO_ON 				HAL_GPIO_WritePin(BUG_IO_GPIO_Port, BUG_IO_Pin, 1)
#define BUG_IO_OFF 				HAL_GPIO_WritePin(BUG_IO_GPIO_Port, BUG_IO_Pin, 0)
#define BUG_IO_TOG 				HAL_GPIO_TogglePin(BUG_IO_GPIO_Port, BUG_IO_Pin)
#define BUG_IO_STT 				HAL_GPIO_ReadPin(BUG_IO_GPIO_Port, BUG_IO_Pin)

#define DIMMER_CTRL_ON   		HAL_GPIO_WritePin(DIM_TRIAC_GPIO_Port, DIM_TRIAC_Pin,1)
#define DIMMER_CTRL_OFF  		HAL_GPIO_WritePin(DIM_TRIAC_GPIO_Port, DIM_TRIAC_Pin,0)
#define DIMMER_CTRL_STT  		HAL_GPIO_ReadPin(DIM_TRIAC_GPIO_Port, DIM_TRIAC_Pin)
#define DIMMER_CTRL_TOG			HAL_GPIO_TogglePin(DIM_TRIAC_GPIO_Port, DIM_TRIAC_Pin)

//*====================================================================  DEFINE INPUT ====================================================================-*//
#define BUTTON_PinStt 			HAL_GPIO_ReadPin(IN_BUTTON_GPIO_Port, IN_BUTTON_Pin)
#define SAFETY_PinStt 			HAL_GPIO_ReadPin(IN_SAFETY_GPIO_Port, IN_SAFETY_Pin)
#define JETSWT_PinStt 			HAL_GPIO_ReadPin(IN_JETSW_GPIO_Port, IN_JETSW_Pin)
#define DRAINSWT_PinStt 		HAL_GPIO_ReadPin(IN_DRAINSW_GPIO_Port, IN_DRAINSW_Pin)
#define SSWATER_Logic_PinStt 	HAL_GPIO_ReadPin(SSWATER_LOGIC_GPIO_Port, SSWATER_LOGIC_Pin)
#define SSDRAIN_Logic_PinStt 	HAL_GPIO_ReadPin(SSDRAIN_LOGIC_GPIO_Port, SSDRAIN_LOGIC_Pin)

#define INPUT_AC_STT			HAL_GPIO_ReadPin(IN_EL814_GPIO_Port, IN_EL814_Pin)

#define DEBUG_ENA_PinStt 		HAL_GPIO_ReadPin(IN_EN_DEBUG_GPIO_Port, IN_EN_DEBUG_Pin)

#endif /* INC_MYDEFINE_H_ */
