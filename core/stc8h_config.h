#ifndef STC8H_CONFIG_H
#define STC8H_CONFIG_H

#include "stc8h_compiler.h"
#include "stc8h_types.h"

#ifdef STC8H_CONFIG_INCLUDE
#include STC8H_CONFIG_INCLUDE
#endif

#ifndef STC8H_CHIP_STC8H1K08
#define STC8H_CHIP_STC8H1K08 1
#endif

#ifndef STC8H_SYSCLK_HZ
#define STC8H_SYSCLK_HZ 11059200UL
#endif

#ifndef STC8H_UART1_BAUD
#define STC8H_UART1_BAUD 115200UL
#endif

#ifndef STC8H_GPIO_PORT_COUNT
#define STC8H_GPIO_PORT_COUNT 6u
#endif

#ifdef STC8H_PINS_INCLUDE
#include STC8H_PINS_INCLUDE
#endif

#endif
