#ifndef __TIMER_H__
#define __TIMER_H__
//#pragma used+
#include "stdint.h"

typedef uint32_t clock_time_t;
#define     CLOCK_SECOND 	(100UL)
#define     CLOCK_MINUTE 	(60*CLOCK_SECOND)
#define     CLOCK_HOUR 		(60*CLOCK_MINUTE)

/**
 * A timer.
 *
 * This structure is used for declaring a timer. The timer must be set
 * with timer_set() before it can be used.
 *
 * \hideinitializer
 */

enum {
	_timer_off,
	_timer_on,
	_timer_tick
};

struct timer {
	uint8_t 		status;
	clock_time_t 	start;
	clock_time_t 	interval;
	clock_time_t 	left_time;
};

clock_time_t timer_gettime(void);
void 	timer_periodic_poll(void);
void 	timer_set(struct timer *t, clock_time_t interval);
void 	timer_reset(struct timer *t);
void 	timer_restart(struct timer *t);
uint8_t timer_expired(struct timer *t);
void 	timer_tick(struct timer *t);
void 	timer_stop(struct timer *t);
void 	timer_pause(struct timer *t);
void 	timer_continue(struct timer *t);

//#pragma used-
#endif /* __TIMER_H__ */
