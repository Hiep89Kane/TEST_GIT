#ifndef _DEBUG_V
#define _DEBUG_V

//Tao timer cho uart ao debug 9600 => 103 us
#define _DEBUGV_TIMER					_USER_DEFINE_TIM_UVDEBUG
#define _DEBUGV_TIMER_SET()				HAL_TIM_Base_Start_IT(&_DEBUGV_TIMER)
#define _DEBUGV_TIMER_STOP()			HAL_TIM_Base_Stop_IT(&_DEBUGV_TIMER)
#define _DEBUGV_TIMER_RESET_CNT()		(_DEBUGV_TIMER.Instance->CNT = 0)

#define _DEBUGV_STOP_BITS				1						// nbr of stop bits

#define _DEBUGV_TX(logic)				TX_V_WritePin(logic)	//in I/O file

#define DEBUGV_STOP						(8 + _DEBUGV_STOP_BITS)		 	// total nbr. of bits
#define	DEBUGV_TX_BUSYV 				6 								// busy sending data (internal - read only)

void debugv_init(unsigned long baud);
void debugv_putc(char c);
void debugv_puts(char *string);
void debugv_putArr(unsigned char array[], unsigned char byteCount) ;
void debugv_periodic_poll(void);

#endif
