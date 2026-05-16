#include "stc8h_power.h"
#include "stc8h_compiler.h"
#include "stc8h_sfr.h"

#define STC8H_PCON_IDLE 0x01u

void stc8h_power_idle(void)
{
    PCON |= STC8H_PCON_IDLE;
    STC8H_NOP();
    STC8H_NOP();
}
