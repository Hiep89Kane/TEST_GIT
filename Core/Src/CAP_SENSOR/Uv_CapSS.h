#ifndef INC_UV_CAPSS_H_
#define INC_UV_CAPSS_H_

//Tao timer cho uart ao 9600 => 103 us
#define _UV_CAPSS_TIMER					_USER_DEFINE_TIM_UVCAPSS

#define _UV_CAPSS_TIMER_SET()			HAL_TIM_Base_Start_IT(&_UV_CAPSS_TIMER)
#define _UV_CAPSS_TIMER_STOP()			HAL_TIM_Base_Stop_IT(&_UV_CAPSS_TIMER)
#define _UV_CAPSS_TIMER_RESET_CNT()		_UV_CAPSS_TIMER.Instance->CNT = 0

#define _UV_CAPSS_BITS_V				1						// nbr of stop bits

#define TX_V_PRINT_WATER(logic)			HAL_GPIO_WritePin(SSWATER_SIGNAL_GPIO_Port, SSWATER_SIGNAL_Pin, logic)
#define TX_V_PRINT_DRAIN(logic)			HAL_GPIO_WritePin(SSDRAIN_SIGNAL_GPIO_Port, SSDRAIN_SIGNAL_Pin, logic)

#define _UV_CAPSS_STOP					(8 + _UV_CAPSS_BITS_V)		 //total nbr.of bits
// bit positions in the Status Register of UV_CAPSS
#define	_UV_CAPSS_TX_BUSYV 				6 		// busy sending data (internal - read only)

void UV_CAPSS_init(unsigned long baud);
void UV_CAPSS_periodic_poll(void);

void UV_CAPSS_putc(char c);
void UV_CAPSS_puts(char *string);
void UV_CAPSS_putArr(unsigned char array[], unsigned int byteCount) ;

#endif /* INC_UV_CAPSS_H_ */
