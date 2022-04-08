/* ========================================
 * File Name: timer.c
 * Version 1.0
 * Copyright LACLONG Ltd, 2016
 * Description:
 *  This file contains API to enable Set timer by User.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF LACLONG Ltd.
 *
 * ========================================
*/

#include "timer.h"

clock_time_t clock_time;

clock_time_t  timer_gettime(){
  return clock_time;
}


/*---------------------------------------------------------------------------*/
/**
 * count the timer.
 *
 * This function is must be call in a timer interrupt
 *
 */
void timer_periodic_poll(void){
  clock_time++;
}
/*---------------------------------------------------------------------------*/
/**
 * Set a timer.
 *
 * This function is used to set a timer for a time sometime in the
 * future. The function timer_expired() will evaluate to true after
 * the timer has expired.
 *
 * \param t A pointer to the timer
 * \param interval The interval before the timer expires.
 *
 */
void
timer_set(struct timer *t, clock_time_t interval)
{
  t->interval = interval;
  t->start = clock_time;
  t->status = _timer_on;
  t->left_time=0;
}
/*---------------------------------------------------------------------------*/
/**
 * Reset the timer with the same interval.
 *
 * This function resets the timer with the same interval that was
 * given to the timer_set() function. The start point of the interval
 * is the exact time that the timer last expired. Therefore, this
 * function will cause the timer to be stable over time, unlike the
 * timer_rester() function.
 *
 * \param t A pointer to the timer.
 *
 * \sa timer_restart()
 */
void
timer_reset(struct timer *t){
  t->start += t->interval;
  t->status = _timer_on;
}
/*---------------------------------------------------------------------------*/
/**
 * Restart the timer from the current point in time
 *
 * This function restarts a timer with the same interval that was
 * given to the timer_set() function. The timer will start at the
 * current time.
 *
 * \note A periodic timer will drift if this function is used to reset
 * it. For preioric timers, use the timer_reset() function instead.
 *
 * \param t A pointer to the timer.
 *
 * \sa timer_reset()
 */
void
timer_restart(struct timer *t){
  t->start = clock_time;
  t->status = _timer_on;
}
/*---------------------------------------------------------------------------*/
/**
 * Check if a timer has expired.
 *
 * This function tests if a timer has expired and returns true or
 * false depending on its status.
 *
 * \param t A pointer to the timer
 *
 * \return Non-zero if the timer has expired, zero otherwise.
 *
 */
uint8_t timer_expired(struct timer *t){
  char i = t->status;
  //WDR();    ==>Cho nay can them vao sau de chong treo nut
  if (i == _timer_off)return 0;
  else if (i == _timer_tick)return 1;
  else if ((clock_time_t)(clock_time - t->start) >= (clock_time_t)t->interval){
    t->status = _timer_tick;
    return 1;
  };
  return 0;
}
/*---------------------------------------------------------------------------*/
void timer_stop(struct timer *t){
  t->status = _timer_off;
  t->left_time=0;
}

void
timer_tick(struct timer *t)
{
  t->start = clock_time - t->interval;
  t->status = _timer_tick;
}

void timer_pause(struct timer *t){
    if (t->status==_timer_on){
        t->status=_timer_off;
        t->left_time=t->interval - (clock_time - t->start);
    }
}

void timer_continue(struct timer *t){
    if (t->left_time > 0){
        t->status=_timer_on;
        t->interval=t->left_time;
        t->left_time=0;
        t->start=clock_time;
    }
}
