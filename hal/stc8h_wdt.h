#ifndef STC8H_WDT_H
#define STC8H_WDT_H

#include "stc8h_config.h"

typedef enum {
    STC8H_WDT_SCALE_2 = 0,
    STC8H_WDT_SCALE_4 = 1,
    STC8H_WDT_SCALE_8 = 2,
    STC8H_WDT_SCALE_16 = 3,
    STC8H_WDT_SCALE_32 = 4,
    STC8H_WDT_SCALE_64 = 5,
    STC8H_WDT_SCALE_128 = 6,
    STC8H_WDT_SCALE_256 = 7
} stc8h_wdt_scale_t;

void stc8h_wdt_enable(stc8h_wdt_scale_t scale, stc8h_u8 run_in_idle);
void stc8h_wdt_feed(void);
stc8h_u8 stc8h_wdt_reset_flag(void);
void stc8h_wdt_clear_reset_flag(void);

#endif
