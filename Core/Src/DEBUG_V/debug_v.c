#include "myHeader.h"

static unsigned char debugv_buffer ; // serial buffer
static unsigned char debugv_bit_nbr; // bit counter
static unsigned char debugv_status ; // status register

void debugv_init(unsigned long baud)
{
    //TX output for PRINT
	_DEBUGV_TX(1);
    //Init int Timer 103 us
    if(baud == 9600u){
      _DEBUGV_TIMER_SET();
      _DEBUGV_TIMER_STOP();
    }
}

void debugv_putc(char c){
	uint32_t	start_stick;

	start_stick = HAL_GetTick();
	while(debugv_status & (1<<DEBUGV_TX_BUSYV)){
		if((HAL_GetTick()-start_stick) > 100/*ms*/) return;
	} // wait while UART v is busy with sending
    /************************/
	debugv_status  = (1<<DEBUGV_TX_BUSYV); 	     // set TX busy flag (clear all others)
	debugv_buffer  = c;		  			 // copy data to buffer
	debugv_bit_nbr = 0xFF;	  		     // erase bit counter (set all bits)
  //  Interrupt_TC2_ClearPending();
		_DEBUGV_TIMER_RESET_CNT();
    // Set Priority !!
		_DEBUGV_TIMER_SET();
		_DEBUGV_TX(0);

	start_stick = HAL_GetTick();
	while(__HAL_TIM_GET_IT_SOURCE(&_DEBUGV_TIMER,TIM_IT_UPDATE) == SET){
		if((HAL_GetTick()-start_stick) > 100/*ms*/) return;
	}
}

void debugv_puts(char *string){
	uint8_t bufIndex = 0;
    /* Blocks the control flow until all data has been sent */
    while(string[bufIndex] != 0)
    {
    	debugv_putc(string[bufIndex]);
    	bufIndex++;
    	if(bufIndex > 200)return; //toi da 200 ky tu
    }
}

void debugv_putArr(unsigned char array[], unsigned char byteCount)
{
    unsigned char arrayIndex;

    for (arrayIndex = 0u; arrayIndex < byteCount; arrayIndex++)
    {
		debugv_putc(array[arrayIndex]);
    }
}

void debugv_periodic_poll(void){

  debugv_bit_nbr++;
      /*** check what are we doing: send or receive ? ***/
  if(debugv_status & (1<<DEBUGV_TX_BUSYV))     // transmit process
  {
     if(debugv_bit_nbr < 8)              // data bits (bit 0...7)
     {
    	_DEBUGV_TX(debugv_buffer & 0x01);
        debugv_buffer >>= 1;             // next bit, please !
     }else{
		_DEBUGV_TX(1);

		if(debugv_bit_nbr >= DEBUGV_STOP) // ready! stop bit(s) sent
		{
			_DEBUGV_TIMER_STOP();
			debugv_status = 0x00;       // clear UART V status register
		}
     }
  }
}

