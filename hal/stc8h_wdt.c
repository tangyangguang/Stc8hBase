#include "stc8h_wdt.h"
#include "stc8h_sfr.h"

#define STC8H_WDT_FLAG 0x80u
#define STC8H_WDT_ENABLE 0x20u
#define STC8H_WDT_CLEAR 0x10u
#define STC8H_WDT_IDLE_RUN 0x08u
#define STC8H_WDT_SCALE_MASK 0x07u

void stc8h_wdt_enable(stc8h_wdt_scale_t scale, stc8h_u8 run_in_idle)
{
    stc8h_u8 value;

    value = (stc8h_u8)(STC8H_WDT_ENABLE | STC8H_WDT_CLEAR);
    if (run_in_idle != 0u) {
        value |= STC8H_WDT_IDLE_RUN;
    }
    value |= ((stc8h_u8)scale & STC8H_WDT_SCALE_MASK);
    WDT_CONTR = value;
}

void stc8h_wdt_feed(void)
{
    WDT_CONTR |= STC8H_WDT_CLEAR;
}

stc8h_u8 stc8h_wdt_reset_flag(void)
{
    return ((WDT_CONTR & STC8H_WDT_FLAG) != 0u) ? 1u : 0u;
}

void stc8h_wdt_clear_reset_flag(void)
{
    WDT_CONTR &= (stc8h_u8)~STC8H_WDT_FLAG;
}
