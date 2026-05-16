#include "stc8h_gpio.h"
#include "stc8h_sfr.h"

#ifndef STC8H_GPIO_PORT_MASK
#define STC8H_GPIO_PORT_MASK 0x3Fu
#endif

#define STC8H_GPIO_PORT_ENABLED(port) ((STC8H_GPIO_PORT_MASK & (1u << (port))) != 0u)

static stc8h_u8 stc8h_gpio_toggle_read_port(stc8h_u8 port)
{
    switch (port) {
#if STC8H_GPIO_PORT_ENABLED(0)
    case 0u: return P0;
#endif
#if STC8H_GPIO_PORT_ENABLED(1)
    case 1u: return P1;
#endif
#if STC8H_GPIO_PORT_ENABLED(2)
    case 2u: return P2;
#endif
#if STC8H_GPIO_PORT_ENABLED(3)
    case 3u: return P3;
#endif
#if STC8H_GPIO_PORT_ENABLED(4)
    case 4u: return P4;
#endif
#if STC8H_GPIO_PORT_ENABLED(5)
    case 5u: return P5;
#endif
    default: return 0u;
    }
}

static void stc8h_gpio_toggle_write_port(stc8h_u8 port, stc8h_u8 value)
{
    switch (port) {
#if STC8H_GPIO_PORT_ENABLED(0)
    case 0u: P0 = value; break;
#endif
#if STC8H_GPIO_PORT_ENABLED(1)
    case 1u: P1 = value; break;
#endif
#if STC8H_GPIO_PORT_ENABLED(2)
    case 2u: P2 = value; break;
#endif
#if STC8H_GPIO_PORT_ENABLED(3)
    case 3u: P3 = value; break;
#endif
#if STC8H_GPIO_PORT_ENABLED(4)
    case 4u: P4 = value; break;
#endif
#if STC8H_GPIO_PORT_ENABLED(5)
    case 5u: P5 = value; break;
#endif
    default: break;
    }
}

void stc8h_gpio_toggle(stc8h_u8 port, stc8h_u8 pin)
{
    stc8h_u8 p;

    if ((port >= STC8H_GPIO_PORT_COUNT) || (pin > 7u)) {
        return;
    }

    p = stc8h_gpio_toggle_read_port(port);
    p ^= (stc8h_u8)(1u << (pin & 0x07u));
    stc8h_gpio_toggle_write_port(port, p);
}
