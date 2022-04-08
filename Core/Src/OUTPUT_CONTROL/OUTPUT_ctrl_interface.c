#include "OUTPUT_ctrl_interface.h"

OUTPUT_struct_t *OUTPUT_struct_t_buf[OUTPUT_number_max];

//return <>0 mean some thing is wrong
uint8_t  OUTPUT_set_mode(OUTPUT_struct_t *OUTPUT_tmp, uint8_t mode){
  if (mode >= _OUTPUT_mode_max)return 1;
  OUTPUT_tmp->mode = mode;
  OUTPUT_tmp->state=_OUTPUT_state_START;
  if (mode==_OUTPUT_mode_blink){
    if (OUTPUT_tmp->ON_duty==0){
    	OUTPUT_tmp->mode=_OUTPUT_mode_off;
    	OUTPUT_tmp->repeat_count=0;
      return 2;
    }
  }
  return 0;
}

uint8_t  OUTPUT_set_blink(OUTPUT_struct_t *OUTPUT_tmp, uint8_t repeat, clock_time_t ON_duty, clock_time_t OFF_duty, clock_time_t repeat_duty){
  OUTPUT_tmp->mode=_OUTPUT_mode_blink;
  OUTPUT_tmp->state=_OUTPUT_state_START;
  if (ON_duty==0){
	OUTPUT_tmp->mode =_OUTPUT_mode_off;
    return 2; //On=0
  }
  OUTPUT_tmp->ON_duty = ON_duty;
  OUTPUT_tmp->OFF_duty = OFF_duty;
  OUTPUT_tmp->repeat = repeat;
  OUTPUT_tmp->repeat_count = 0;
  OUTPUT_tmp->repeat_duty = repeat_duty;
  return 0;
}

void  OUTPUT_periodic_poll(void){

  for (uint8_t i = 0;i < OUTPUT_number_max;i++){

    if (OUTPUT_struct_t_buf[i]==0)continue;

    if (OUTPUT_struct_t_buf[i]->mode==_OUTPUT_mode_noctrl)continue;

    //bat dau
    if (OUTPUT_struct_t_buf[i]->state==_OUTPUT_state_START){
      OUTPUT_struct_t_buf[i]->state=_OUTPUT_state_ON;
      switch (OUTPUT_struct_t_buf[i]->mode){
        case _OUTPUT_mode_off:
			OUTPUT_struct_t_buf[i]->OUTPUT_BaseCtrl(0);
			break;
        case _OUTPUT_mode_on:
			OUTPUT_struct_t_buf[i]->OUTPUT_BaseCtrl(1);
			break;
        case _OUTPUT_mode_blink:
        	//bat dau la ON truoc
			OUTPUT_struct_t_buf[i]->OUTPUT_BaseCtrl(1);
			timer_set(&OUTPUT_struct_t_buf[i]->timeout, OUTPUT_struct_t_buf[i]->ON_duty);
			if (OUTPUT_struct_t_buf[i]->repeat_duty!=0)
			{
				timer_set(&OUTPUT_struct_t_buf[i]->duty_timeout,OUTPUT_struct_t_buf[i]->repeat_duty);
			}
			break;
      }
    }
    //neu blink thi lam
    else if (OUTPUT_struct_t_buf[i]->mode==_OUTPUT_mode_blink){
      if (timer_expired(&OUTPUT_struct_t_buf[i]->timeout)){
        switch (OUTPUT_struct_t_buf[i]->state){
          case _OUTPUT_state_ON:
			  OUTPUT_struct_t_buf[i]->state=_OUTPUT_state_OFF;
			  OUTPUT_struct_t_buf[i]->OUTPUT_BaseCtrl(0);
			  //neu thoi gian OFF_duty => chuyen qua tat LED
			  if (OUTPUT_struct_t_buf[i]->OFF_duty==0){
				OUTPUT_struct_t_buf[i]->mode=_OUTPUT_mode_off;
				OUTPUT_struct_t_buf[i]->state=_OUTPUT_state_ON;
			  }
			  else timer_set(&OUTPUT_struct_t_buf[i]->timeout,OUTPUT_struct_t_buf[i]->OFF_duty);
			  break;

          case _OUTPUT_state_OFF:
			  //Neu dat toi so lan lap lai thi cho chu ky moi hoac tat
			  if (++OUTPUT_struct_t_buf[i]->repeat_count==OUTPUT_struct_t_buf[i]->repeat){
				OUTPUT_struct_t_buf[i]->repeat_count=0;
				//Neu ko su dung chu ky lap lai thi tra ve che do off
				if (OUTPUT_struct_t_buf[i]->repeat_duty==0){
				  OUTPUT_struct_t_buf[i]->state=_OUTPUT_state_START;
				  OUTPUT_struct_t_buf[i]->mode=_OUTPUT_mode_noctrl;      //############
				}
				//Neu su dung chu ky lap lai thi reset timer chu ky
				else
				{
				  OUTPUT_struct_t_buf[i]->state=_OUTPUT_state_REPEAT;
				  OUTPUT_struct_t_buf[i]->timeout = OUTPUT_struct_t_buf[i]->duty_timeout; // ???
				}
			  }
			  //chop LED lap lai
			  else {
				OUTPUT_struct_t_buf[i]->state=_OUTPUT_state_ON;
				OUTPUT_struct_t_buf[i]->OUTPUT_BaseCtrl(1);
				timer_set(&OUTPUT_struct_t_buf[i]->timeout,OUTPUT_struct_t_buf[i]->ON_duty);
			  }
			  break;

          case _OUTPUT_state_REPEAT:
			  //Thoi gian bat dau chu ky moi da den
			  OUTPUT_struct_t_buf[i]->state=_OUTPUT_state_ON;
			  OUTPUT_struct_t_buf[i]->OUTPUT_BaseCtrl(1);
			  timer_set(&OUTPUT_struct_t_buf[i]->timeout,OUTPUT_struct_t_buf[i]->ON_duty);
			  timer_reset(&OUTPUT_struct_t_buf[i]->duty_timeout);
			  break;

        }//end switch state
      } //end timer_expired--> co the la ON_duty hoac OFF duty
    }//end if mode = blink
  }//end for
}

uint8_t  OUTPUT_config_new_control(OUTPUT_struct_t *state, void (*OUTPUT_BaseCtrl_pointer)(uint8_t ON_OFF)){

  for (uint8_t i = 0;i < OUTPUT_number_max;i++){
    if (OUTPUT_struct_t_buf[i]==0){
      OUTPUT_struct_t_buf[i]=state;
      OUTPUT_struct_t_buf[i]->OUTPUT_BaseCtrl=OUTPUT_BaseCtrl_pointer;
      return i+1;
    }
  }
  return 0;
}

ResultStatus OUTPUT_get_event(OUTPUT_struct_t *OUTPUT_tmp, OUTPUT_state_t check_event){
	static OUTPUT_state_t evt_old_t;

	if(OUTPUT_tmp->state == evt_old_t)return _FALSE;
	evt_old_t = OUTPUT_tmp->state;
	if(evt_old_t != check_event)return _FALSE;

	return _TRUE;
}
