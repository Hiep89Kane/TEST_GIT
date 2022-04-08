#ifndef __OUTPUT_CTRL_INTERFACE_H__
#define __OUTPUT_CTRL_INTERFACE_H__
#include "common.h"
#include "timer.h"

#define OUTPUT_number_max  8 //user define

typedef enum {
  _OUTPUT_mode_noctrl,
  _OUTPUT_mode_off,
  _OUTPUT_mode_on,
  _OUTPUT_mode_blink,

  _OUTPUT_mode_max
}OUTPUT_mode_t;

typedef enum {
  _OUTPUT_state_START,
  _OUTPUT_state_ON,
  _OUTPUT_state_OFF,
  _OUTPUT_state_REPEAT
} OUTPUT_state_t;

typedef struct {
	OUTPUT_mode_t	mode; 			//mode dieu khien
	OUTPUT_state_t  state; 			//trang thai dieu khien LED trong ngat timer

	uint8_t			repeat, 		//so lan lap lai neu mode la blink
					repeat_count;	//bien dem de kiem tra repeat

	clock_time_t	ON_duty,		//thoi gian LED ON	neu blink
  	  	  	  		OFF_duty,		//thoi gian LED OFF	neu blink
  	  	  	  		repeat_duty;

	struct  timer 	timeout,
  					duty_timeout; 	//neu <> 0 => thi lap lai lien tuc

	void (*OUTPUT_BaseCtrl)(uint8_t ON_OFF);//0 is off, >0 is on

}OUTPUT_struct_t;

uint8_t  OUTPUT_set_mode(OUTPUT_struct_t *state, uint8_t mode);

/*					<---ON duty---> <---OFF duty---><--Ide-->
 *
 * 	Pulse   |--------------|_______________|_________| => repeat again
 *
 *			<--------repeat_duty timeout------------->
 *
 *			if (repeat_duty ==0) just repeat n times
 *			if (repeat_duty < (ON + OFF duty)) => IDE = 1 and "none stop"
 *			if (repeat_duty > (ON + OFF duty)) => IDE = (repeat_duty - (ON + OFF duty))  and "none stop"
 */

//repeat: number of blinking in each duty
//ON_duty: LED on duty in each blink
//OFF_duty: LED off duty in each blink
//repeat_duty: timeout that the LED stop to blink
uint8_t  OUTPUT_set_blink(OUTPUT_struct_t *state, uint8_t repeat, clock_time_t ON_duty, clock_time_t OFF_duty, clock_time_t repeat_duty);

//Config a new led automatic control slot, return the order number of automatic slot, return 0 if all slots is full
uint8_t  OUTPUT_config_new_control(OUTPUT_struct_t *state, void (*OUTPUT_BaseCtrl_pointer)(uint8_t ON_OFF));

//Place this in timer interrupt (like 10ms interrupt)
void  	OUTPUT_periodic_poll(void);

ResultStatus OUTPUT_get_event(OUTPUT_struct_t *OUTPUT_tmp, OUTPUT_state_t check_event);

#endif  //__OUTPUT_CTRL_INTERFACE_H__
