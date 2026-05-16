#include "stc8h_exti.h"
#include "stc8h_sfr.h"

#define STC8H_EXTI_INT2_MASK 0x10u
#define STC8H_EXTI_INT3_MASK 0x20u
#define STC8H_EXTI_INT4_MASK 0x40u

void stc8h_exti_clear_flags(stc8h_u8 mask)
{
    if ((mask & (1u << STC8H_EXTI_INT0)) != 0u) {
        IE0 = 0;
    }
    if ((mask & (1u << STC8H_EXTI_INT1)) != 0u) {
        IE1 = 0;
    }
    if ((mask & (1u << STC8H_EXTI_INT2)) != 0u) {
        AUXINTIF &= (stc8h_u8)~STC8H_EXTI_INT2_MASK;
    }
    if ((mask & (1u << STC8H_EXTI_INT3)) != 0u) {
        AUXINTIF &= (stc8h_u8)~STC8H_EXTI_INT3_MASK;
    }
    if ((mask & (1u << STC8H_EXTI_INT4)) != 0u) {
        AUXINTIF &= (stc8h_u8)~STC8H_EXTI_INT4_MASK;
    }
}
