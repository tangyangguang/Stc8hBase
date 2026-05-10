#ifndef STC8H_TIMER_H
#define STC8H_TIMER_H

#include "stc8h_config.h"

typedef enum {
    STC8H_TIMER0 = 0,
    STC8H_TIMER1,
    STC8H_TIMER2
} stc8h_timer_id_t;

stc8h_status_t stc8h_timer_init_1ms(stc8h_timer_id_t timer);
void stc8h_timer_start(stc8h_timer_id_t timer);
void stc8h_timer_stop(stc8h_timer_id_t timer);
void stc8h_timer_enable_interrupt(stc8h_timer_id_t timer);
void stc8h_timer_disable_interrupt(stc8h_timer_id_t timer);
void stc8h_timer_clear_flag(stc8h_timer_id_t timer);

#endif
