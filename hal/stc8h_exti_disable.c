#include "stc8h_exti.h"
#include "stc8h_sfr.h"

#define STC8H_EXTI_INT2_MASK 0x10u
#define STC8H_EXTI_INT3_MASK 0x20u
#define STC8H_EXTI_INT4_MASK 0x40u

static stc8h_u8 stc8h_exti_disable_mask(stc8h_exti_line_t line)
{
    if (line == STC8H_EXTI_INT2) {
        return STC8H_EXTI_INT2_MASK;
    }
    if (line == STC8H_EXTI_INT3) {
        return STC8H_EXTI_INT3_MASK;
    }
    if (line == STC8H_EXTI_INT4) {
        return STC8H_EXTI_INT4_MASK;
    }
    return 0u;
}

void stc8h_exti_disable(stc8h_exti_line_t line)
{
    stc8h_u8 mask;

    if (line == STC8H_EXTI_INT0) {
        EX0 = 0;
    } else if (line == STC8H_EXTI_INT1) {
        EX1 = 0;
    } else {
        mask = stc8h_exti_disable_mask(line);
        if (mask != 0u) {
            INTCLKO &= (stc8h_u8)~mask;
        }
    }
}
