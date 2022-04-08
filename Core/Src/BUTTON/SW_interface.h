#ifndef   __SW_INTERFACE_H__
#define   __SW_INTERFACE_H__
#include  "timer.h"

#define _SW_pass_time   (CLOCK_SECOND/8)     				//pass longer than this -> click timeout -> not a multi-click
#define _SW_hold_time   (CLOCK_SECOND*2-CLOCK_SECOND/10)   	//hold longer than this -> not a multi-click
#define _SW_init_time   (CLOCK_SECOND/10)    				//hold longer this to make a click down

enum  SW_status{
  _SW_null,
  _SW_init_1,
  _SW_down_1,
  _SW_hold_1,
  _SW_hold_on_1,
  _SW_init_up_1,
  _SW_pass_1,
  _SW_single_click,
  _SW_init_2,
  _SW_down_2,
  _SW_hold_2,
  _SW_hold_on_2,
  _SW_init_up_2,
  _SW_pass_2,
  _SW_double_click,
  _SW_init_3,
  _SW_down_3,
  _SW_hold_3,
  _SW_hold_on_3,
  _SW_init_up_3,
  _SW_pass_3,
  _SW_triple_click,
  _SW_hold_on_pass       //hold on pass is not need to know how many click
};
struct SW_state{
  uint8_t  		status;
  clock_time_t  hold_start;
  struct  timer timeout;
};

uint8_t  		SW_progress(struct SW_state *SW, uint8_t is_click);
clock_time_t  	SW_get_hold_time(struct SW_state *SW);

#endif  //__SW_INTERFACE_H__
