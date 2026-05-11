#ifndef STC8H_EXTI_H
#define STC8H_EXTI_H

#include "stc8h_config.h"

typedef enum {
    STC8H_EXTI_INT0 = 0,
    STC8H_EXTI_INT1 = 1,
    STC8H_EXTI_INT2 = 2,
    STC8H_EXTI_INT3 = 3,
    STC8H_EXTI_INT4 = 4
} stc8h_exti_line_t;

typedef enum {
    STC8H_EXTI_MODE_BOTH_EDGES = 0,
    STC8H_EXTI_MODE_FALLING_EDGE = 1
} stc8h_exti_mode_t;

stc8h_status_t stc8h_exti_configure(stc8h_exti_line_t line, stc8h_exti_mode_t mode);
void stc8h_exti_enable(stc8h_exti_line_t line);
void stc8h_exti_disable(stc8h_exti_line_t line);
void stc8h_exti_clear_flag(stc8h_exti_line_t line);

#endif
