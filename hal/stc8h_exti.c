#include "stc8h_exti.h"
#include "stc8h_sfr.h"

stc8h_status_t stc8h_exti_configure(stc8h_exti_line_t line, stc8h_exti_mode_t mode)
{
    if (line == STC8H_EXTI_INT0) {
        EX0 = 0;
        IE0 = 0;
        IT0 = (mode == STC8H_EXTI_MODE_FALLING_EDGE) ? 1 : 0;
        return STC8H_OK;
    }
    if (line == STC8H_EXTI_INT1) {
        EX1 = 0;
        IE1 = 0;
        IT1 = (mode == STC8H_EXTI_MODE_FALLING_EDGE) ? 1 : 0;
        return STC8H_OK;
    }
    return STC8H_ERROR;
}

void stc8h_exti_enable(stc8h_exti_line_t line)
{
    if (line == STC8H_EXTI_INT0) {
        EX0 = 1;
    } else if (line == STC8H_EXTI_INT1) {
        EX1 = 1;
    }
}

void stc8h_exti_disable(stc8h_exti_line_t line)
{
    if (line == STC8H_EXTI_INT0) {
        EX0 = 0;
    } else if (line == STC8H_EXTI_INT1) {
        EX1 = 0;
    }
}

void stc8h_exti_clear_flag(stc8h_exti_line_t line)
{
    if (line == STC8H_EXTI_INT0) {
        IE0 = 0;
    } else if (line == STC8H_EXTI_INT1) {
        IE1 = 0;
    }
}
