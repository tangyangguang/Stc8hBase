#include "stc8h_exti.h"
#include "stc8h_sfr.h"

#define STC8H_EXTI_INT2_MASK 0x10u
#define STC8H_EXTI_INT3_MASK 0x20u
#define STC8H_EXTI_INT4_MASK 0x40u

static stc8h_u8 stc8h_exti_mask(stc8h_exti_line_t line)
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

stc8h_status_t stc8h_exti_configure(stc8h_exti_line_t line, stc8h_exti_mode_t mode)
{
    stc8h_u8 it_value;
    stc8h_u8 mask;

    if (mode == STC8H_EXTI_MODE_BOTH_EDGES) {
        it_value = 0u;
    } else if (mode == STC8H_EXTI_MODE_FALLING_EDGE) {
        it_value = 1u;
    } else {
        return STC8H_ERROR;
    }

    if (line == STC8H_EXTI_INT0) {
        EX0 = 0;
        IE0 = 0;
        IT0 = it_value;
        return STC8H_OK;
    }
    if (line == STC8H_EXTI_INT1) {
        EX1 = 0;
        IE1 = 0;
        IT1 = it_value;
        return STC8H_OK;
    }
    mask = stc8h_exti_mask(line);
    if (mask != 0u) {
        if (mode != STC8H_EXTI_MODE_FALLING_EDGE) {
            return STC8H_ERROR;
        }
        INTCLKO &= (stc8h_u8)~mask;
        AUXINTIF &= (stc8h_u8)~mask;
        return STC8H_OK;
    }
    return STC8H_ERROR;
}

void stc8h_exti_enable(stc8h_exti_line_t line)
{
    stc8h_u8 mask;

    if (line == STC8H_EXTI_INT0) {
        EX0 = 1;
    } else if (line == STC8H_EXTI_INT1) {
        EX1 = 1;
    } else {
        mask = stc8h_exti_mask(line);
        if (mask != 0u) {
            INTCLKO |= mask;
        }
    }
}

void stc8h_exti_disable(stc8h_exti_line_t line)
{
    stc8h_u8 mask;

    if (line == STC8H_EXTI_INT0) {
        EX0 = 0;
    } else if (line == STC8H_EXTI_INT1) {
        EX1 = 0;
    } else {
        mask = stc8h_exti_mask(line);
        if (mask != 0u) {
            INTCLKO &= (stc8h_u8)~mask;
        }
    }
}

void stc8h_exti_clear_flag(stc8h_exti_line_t line)
{
    stc8h_u8 mask;

    if (line == STC8H_EXTI_INT0) {
        IE0 = 0;
    } else if (line == STC8H_EXTI_INT1) {
        IE1 = 0;
    } else {
        mask = stc8h_exti_mask(line);
        if (mask != 0u) {
            AUXINTIF &= (stc8h_u8)~mask;
        }
    }
}
