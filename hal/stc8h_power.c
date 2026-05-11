#include "stc8h_power.h"
#include "stc8h_compiler.h"
#include "stc8h_sfr.h"

#define STC8H_PCON_IDLE 0x01u
#define STC8H_PCON_POWER_DOWN 0x02u

void stc8h_power_idle(void)
{
    PCON |= STC8H_PCON_IDLE;
    STC8H_NOP();
    STC8H_NOP();
}

void stc8h_power_down(void)
{
    PCON |= STC8H_PCON_POWER_DOWN;
    STC8H_NOP();
    STC8H_NOP();
    STC8H_NOP();
    STC8H_NOP();
}
