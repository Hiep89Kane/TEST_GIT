#include  <SW_interface.h>

uint8_t  SW_progress(struct SW_state *SW, uint8_t is_click){
  uint8_t  status;
  status=SW->status;
  if (is_click){
    if (status==_SW_null){
      status=_SW_init_1;
      timer_set(&SW->timeout,_SW_init_time);
      SW->hold_start=timer_gettime();
    }
    else if (status==_SW_init_1){
      if (timer_expired(&SW->timeout))status=_SW_down_1;
    }
    else if (status==_SW_down_1){
      status=_SW_hold_1;
      timer_set(&SW->timeout,_SW_hold_time);
    }
    else if (status==_SW_hold_1){
      if (timer_expired(&SW->timeout))status=_SW_hold_on_1;
    }
    else if (status==_SW_pass_1){
      status=_SW_init_2;
      timer_set(&SW->timeout,_SW_init_time);
      SW->hold_start=timer_gettime();
    }
    else if (status==_SW_init_2){
      if (timer_expired(&SW->timeout))status=_SW_down_2;
    }
    else if (status==_SW_down_2){
      status=_SW_hold_2;
      timer_set(&SW->timeout,_SW_hold_time);
    }
    else if (status==_SW_hold_2){
      if (timer_expired(&SW->timeout))status=_SW_hold_on_2;
    }
    else if (status==_SW_pass_2){
      status=_SW_init_3;
      timer_set(&SW->timeout,_SW_init_time);
      SW->hold_start=timer_gettime();
    }
    else if (status==_SW_init_3){
      if (timer_expired(&SW->timeout))status=_SW_down_3;
    }
    else if (status==_SW_down_3){
      status=_SW_hold_3;
      timer_set(&SW->timeout,_SW_hold_time);
    }
    else if (status==_SW_hold_3){
      if (timer_expired(&SW->timeout))status=_SW_hold_on_3;
    }
    #ifdef  _USE_SW_INIT_UP
    else if (status==_SW_init_up_1){
      status=_SW_hold_1;
    }
    else if (status==_SW_init_up_2){
      status=_SW_hold_2;
    }
    else if (status==_SW_init_up_3){
      status=_SW_hold_3;
    }
    #endif
  }
  else {
    if (status==_SW_pass_1){
      if (timer_expired(&SW->timeout))status=_SW_single_click;
    }
    #ifdef  _USE_SW_INIT_UP
    else if (status==_SW_hold_1){
      status=_SW_init_up_1;
      timer_set(&SW->timeout,_SW_init_time);
    }
    else if (status==_SW_hold_2){
      status=_SW_init_up_2;
      timer_set(&SW->timeout,_SW_init_time);
    }
    else if (status==_SW_hold_3){
      status=_SW_init_up_3;
      timer_set(&SW->timeout,_SW_init_time);
    }
    else if (status==_SW_init_up_1){
      if (timer_expired(&SW->timeout)){
        status=_SW_pass_1;
        timer_set(&SW->timeout,_SW_pass_time);
      }
    }
    else if (status==_SW_init_up_2){
      if (timer_expired(&SW->timeout)){
        status=_SW_pass_2;
        timer_set(&SW->timeout,_SW_pass_time);
      }
    }
    else if (status==_SW_init_up_3){
      if (timer_expired(&SW->timeout)){
        status=_SW_pass_3;
        timer_set(&SW->timeout,_SW_pass_time);
      }
    }
    #else
    else if (status==_SW_hold_1){
      status=_SW_pass_1;
      timer_set(&SW->timeout,_SW_pass_time);
    }
    else if (status==_SW_hold_2){
      status=_SW_pass_2;
      timer_set(&SW->timeout,_SW_pass_time);
    }
    else if (status==_SW_hold_3){
      status=_SW_pass_3;
      timer_set(&SW->timeout,_SW_pass_time);
    }
    #endif
    else if (status==_SW_pass_2){
      if (timer_expired(&SW->timeout))status=_SW_double_click;
    }
    else if (status==_SW_pass_3){
      if (timer_expired(&SW->timeout))status=_SW_triple_click;
    }
    else if ((status==_SW_hold_on_1)||(status==_SW_hold_on_2)||(status==_SW_hold_on_3)){
      status=_SW_hold_on_pass;
    }
    else status=_SW_null;
  }
  SW->status=status;
  return  status;
}

clock_time_t  SW_get_hold_time(struct SW_state *SW){
  return  (timer_gettime() - SW->hold_start);
}
