#ifndef STC8H_QEI_H
#define STC8H_QEI_H

#include "stc8h_config.h"

#define STC8H_QEI_MODE_TI6_EDGES 1u
#define STC8H_QEI_MODE_TI5_EDGES 2u
#define STC8H_QEI_MODE_BOTH_EDGES 3u

#ifndef STC8H_QEI_PWMB_TI5_PIN_SELECT
#error "Define STC8H_QEI_PWMB_TI5_PIN_SELECT for the target board."
#endif

#ifndef STC8H_QEI_PWMB_TI6_PIN_SELECT
#error "Define STC8H_QEI_PWMB_TI6_PIN_SELECT for the target board."
#endif

#ifndef STC8H_QEI_PWMB_MODE
#error "Define STC8H_QEI_PWMB_MODE for the target encoder."
#endif

#ifndef STC8H_QEI_PWMB_FILTER
#error "Define STC8H_QEI_PWMB_FILTER with the hardware filter encoding (0..15)."
#endif

#if !STC8H_CHIP_STC8H8K64U
#error "stc8h_qei currently supports only STC8H8K64U."
#endif

#if (STC8H_QEI_PWMB_TI5_PIN_SELECT > 3u) || (STC8H_QEI_PWMB_TI6_PIN_SELECT > 3u)
#error "STC8H QEI PWMB pin selection must be 0..3."
#endif

#if (STC8H_QEI_PWMB_MODE < STC8H_QEI_MODE_TI6_EDGES) || \
    (STC8H_QEI_PWMB_MODE > STC8H_QEI_MODE_BOTH_EDGES)
#error "STC8H QEI PWMB mode must be 1, 2 or 3."
#endif

#if STC8H_QEI_PWMB_FILTER > 15u
#error "STC8H QEI PWMB filter encoding must be 0..15."
#endif

void stc8h_qei_pwmb_init(void);
stc8h_u16 stc8h_qei_pwmb_read(void);
void stc8h_qei_pwmb_stop(void);

#endif
