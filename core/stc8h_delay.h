#ifndef STC8H_DELAY_H
#define STC8H_DELAY_H

#include "stc8h_config.h"

void stc8h_delay_us(stc8h_u16 us);
void stc8h_delay_ms(stc8h_u16 ms);
stc8h_status_t stc8h_delay_timer0_1t_init(void);
void stc8h_delay_timer0_1t_us(stc8h_u16 us);

#endif
