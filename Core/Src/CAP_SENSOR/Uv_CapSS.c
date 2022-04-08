#include "myHeader.h"

extern TIM_HandleTypeDef _USER_DEFINE_TIM_UVCAPSS;

unsigned char Capuv_buffer; // serial buffer
unsigned char Capuv_bit_nbr; // bit counter
unsigned char Capuv_status; // status register

extern uint8_t CapSS_selected;  	//bien toan cuc lua chon Sensor de hoat dong

// UV_CAPSS initialization function (we MUST call this function BEFORE using UV_CAPSS)
void UV_CAPSS_init(unsigned long baud) // UV_CAPSS = software UART
{
	//TX output for PRINT
	if (CapSS_selected == _CAPSS_ID_WATER)
		TX_V_PRINT_WATER(1); 					//logic 1 (high level = STOP TX)
	else if (CapSS_selected == _CAPSS_ID_DRAIN)
		TX_V_PRINT_DRAIN(1);
	//Init int Timer 103 us
	if (baud == 9600u) {
		_UV_CAPSS_TIMER_SET();
		_UV_CAPSS_TIMER_STOP();
	}
}

void UV_CAPSS_putc(char c) {
	uint32_t	start_stick;

	start_stick = HAL_GetTick();
	while (Capuv_status & (1 << _UV_CAPSS_TX_BUSYV)){
		if((HAL_GetTick()-start_stick) > 100/*ms*/) return;
	} // wait while UV_CAPSS is busy with sending
	/************************/
	Capuv_status = (1 << _UV_CAPSS_TX_BUSYV); // set TX busy flag (clear all others)
	Capuv_buffer = c;		  			 // copy data to buffer
	Capuv_bit_nbr = 0xFF;	  		     // erase bit counter (set all bits)
	//  Interrupt_TC2_ClearPending();
	_UV_CAPSS_TIMER_RESET_CNT();
	// Set Priority !!
	_UV_CAPSS_TIMER_SET();

	if (CapSS_selected == _CAPSS_ID_WATER) TX_V_PRINT_WATER(0); // logic 1 (high level = STOP TX)
	else if (CapSS_selected == _CAPSS_ID_DRAIN) TX_V_PRINT_DRAIN(0);

	start_stick = HAL_GetTick();
	while (__HAL_TIM_GET_IT_SOURCE(&_UV_CAPSS_TIMER,TIM_IT_UPDATE) == SET){
		if(( HAL_GetTick()-start_stick) > 100/*ms*/) return;
	}
}

void UV_CAPSS_puts(char *string) {
	uint8_t bufIndex = 0;
	/* Blocks the control flow until all data has been sent */
	while (string[bufIndex] != 0) {
		UV_CAPSS_putc(string[bufIndex]);
		bufIndex++;
		if (bufIndex > 50)
		return;
	}
}

void UV_CAPSS_putArr(unsigned char array[], unsigned int byteCount) {
	unsigned char arrayIndex;

	for (arrayIndex = 0u; arrayIndex < byteCount; arrayIndex++) {
		UV_CAPSS_putc(array[arrayIndex]);
	}
}

void UV_CAPSS_periodic_poll(void) {
	Capuv_bit_nbr++;
	/*** check what are we doing: send or receive ? ***/
	if (Capuv_status & (1 << _UV_CAPSS_TX_BUSYV))     // transmit process
			{
		if (Capuv_bit_nbr < 8)              // data bits (bit 0...7)
				{

			if (CapSS_selected == _CAPSS_ID_WATER)
				TX_V_PRINT_WATER(Capuv_buffer & 0x01);
			else if (CapSS_selected == _CAPSS_ID_DRAIN)
				TX_V_PRINT_DRAIN(Capuv_buffer & 0x01);

			Capuv_buffer >>= 1;             // next bit, please !
		} else {

			if (CapSS_selected == _CAPSS_ID_WATER)
				TX_V_PRINT_WATER(1);
			else if (CapSS_selected == _CAPSS_ID_DRAIN)
				TX_V_PRINT_DRAIN(1);

			if (Capuv_bit_nbr >= _UV_CAPSS_STOP) // ready! stop bit(s) sent
			{
				_UV_CAPSS_TIMER_STOP();
				Capuv_status = 0x00;       // clear UV_CAPSS status register
			}
		}
	}
}

