#ifndef STC8H_CONFIG_H
#define STC8H_CONFIG_H

#include "stc8h_compiler.h"
#include "stc8h_types.h"

#ifdef STC8H_CONFIG_INCLUDE
#include STC8H_CONFIG_INCLUDE
#endif

#ifndef STC8H_CHIP_STC8H8K64U
#define STC8H_CHIP_STC8H8K64U 0
#endif

#ifndef STC8H_CHIP_STC8H1K08
#define STC8H_CHIP_STC8H1K08 0
#endif

#if ((STC8H_CHIP_STC8H1K08 != 0) && (STC8H_CHIP_STC8H1K08 != 1)) || \
    ((STC8H_CHIP_STC8H8K64U != 0) && (STC8H_CHIP_STC8H8K64U != 1))
#error "STC8H chip profile macros must be 0 or 1."
#endif

#if (STC8H_CHIP_STC8H1K08 + STC8H_CHIP_STC8H8K64U) != 1
#error "Select exactly one STC8H chip profile."
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
