#ifndef STC8H_INTERRUPT_H
#define STC8H_INTERRUPT_H

#include "stc8h_config.h"

#define STC8H_VECTOR_INT0 0
#define STC8H_VECTOR_TIMER0 1
#define STC8H_VECTOR_INT1 2
#define STC8H_VECTOR_TIMER1 3
#define STC8H_VECTOR_UART1 4

void stc8h_interrupt_enable_global(void);
void stc8h_interrupt_disable_global(void);

#endif
