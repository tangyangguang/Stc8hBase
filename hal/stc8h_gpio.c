#include "stc8h_gpio.h"
#include "stc8h_sfr.h"

#ifndef STC8H_GPIO_PORT_MASK
#define STC8H_GPIO_PORT_MASK 0x3Fu
#endif

#define STC8H_GPIO_PORT_ENABLED(port) ((STC8H_GPIO_PORT_MASK & (1u << (port))) != 0u)

static stc8h_u8 stc8h_gpio_mask(stc8h_u8 pin)
{
    return (stc8h_u8)(1u << (pin & 0x07u));
}

static stc8h_u8 stc8h_gpio_read_port(stc8h_u8 port)
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

static void stc8h_gpio_write_port(stc8h_u8 port, stc8h_u8 value)
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

static stc8h_u8 stc8h_gpio_read_m0(stc8h_u8 port)
{
    switch (port) {
#if STC8H_GPIO_PORT_ENABLED(0)
    case 0u: return P0M0;
#endif
#if STC8H_GPIO_PORT_ENABLED(1)
    case 1u: return P1M0;
#endif
#if STC8H_GPIO_PORT_ENABLED(2)
    case 2u: return P2M0;
#endif
#if STC8H_GPIO_PORT_ENABLED(3)
    case 3u: return P3M0;
#endif
#if STC8H_GPIO_PORT_ENABLED(4)
    case 4u: return P4M0;
#endif
#if STC8H_GPIO_PORT_ENABLED(5)
    case 5u: return P5M0;
#endif
    default: return 0u;
    }
}

static void stc8h_gpio_write_m0(stc8h_u8 port, stc8h_u8 value)
{
    switch (port) {
#if STC8H_GPIO_PORT_ENABLED(0)
    case 0u: P0M0 = value; break;
#endif
#if STC8H_GPIO_PORT_ENABLED(1)
    case 1u: P1M0 = value; break;
#endif
#if STC8H_GPIO_PORT_ENABLED(2)
    case 2u: P2M0 = value; break;
#endif
#if STC8H_GPIO_PORT_ENABLED(3)
    case 3u: P3M0 = value; break;
#endif
#if STC8H_GPIO_PORT_ENABLED(4)
    case 4u: P4M0 = value; break;
#endif
#if STC8H_GPIO_PORT_ENABLED(5)
    case 5u: P5M0 = value; break;
#endif
    default: break;
    }
}

static stc8h_u8 stc8h_gpio_read_m1(stc8h_u8 port)
{
    switch (port) {
#if STC8H_GPIO_PORT_ENABLED(0)
    case 0u: return P0M1;
#endif
#if STC8H_GPIO_PORT_ENABLED(1)
    case 1u: return P1M1;
#endif
#if STC8H_GPIO_PORT_ENABLED(2)
    case 2u: return P2M1;
#endif
#if STC8H_GPIO_PORT_ENABLED(3)
    case 3u: return P3M1;
#endif
#if STC8H_GPIO_PORT_ENABLED(4)
    case 4u: return P4M1;
#endif
#if STC8H_GPIO_PORT_ENABLED(5)
    case 5u: return P5M1;
#endif
    default: return 0u;
    }
}

static void stc8h_gpio_write_m1(stc8h_u8 port, stc8h_u8 value)
{
    switch (port) {
#if STC8H_GPIO_PORT_ENABLED(0)
    case 0u: P0M1 = value; break;
#endif
#if STC8H_GPIO_PORT_ENABLED(1)
    case 1u: P1M1 = value; break;
#endif
#if STC8H_GPIO_PORT_ENABLED(2)
    case 2u: P2M1 = value; break;
#endif
#if STC8H_GPIO_PORT_ENABLED(3)
    case 3u: P3M1 = value; break;
#endif
#if STC8H_GPIO_PORT_ENABLED(4)
    case 4u: P4M1 = value; break;
#endif
#if STC8H_GPIO_PORT_ENABLED(5)
    case 5u: P5M1 = value; break;
#endif
    default: break;
    }
}

void stc8h_gpio_set_mode(stc8h_u8 port, stc8h_u8 pin, stc8h_gpio_mode_t mode)
{
    stc8h_u8 m0;
    stc8h_u8 m1;
    stc8h_u8 mask;

    if ((port >= STC8H_GPIO_PORT_COUNT) || (pin > 7u)) {
        return;
    }

    mask = stc8h_gpio_mask(pin);
    m0 = stc8h_gpio_read_m0(port);
    m1 = stc8h_gpio_read_m1(port);

    switch (mode) {
    case STC8H_GPIO_MODE_QUASI:
        m0 &= (stc8h_u8)~mask;
        m1 &= (stc8h_u8)~mask;
        break;
    case STC8H_GPIO_MODE_PUSH_PULL:
        m0 |= mask;
        m1 &= (stc8h_u8)~mask;
        break;
    case STC8H_GPIO_MODE_INPUT_ONLY:
        m0 &= (stc8h_u8)~mask;
        m1 |= mask;
        break;
    case STC8H_GPIO_MODE_OPEN_DRAIN:
        m0 |= mask;
        m1 |= mask;
        break;
    default:
        break;
    }

    stc8h_gpio_write_m0(port, m0);
    stc8h_gpio_write_m1(port, m1);
}

void stc8h_gpio_write(stc8h_u8 port, stc8h_u8 pin, stc8h_u8 value)
{
    stc8h_u8 p;
    stc8h_u8 mask;

    if ((port >= STC8H_GPIO_PORT_COUNT) || (pin > 7u)) {
        return;
    }

    p = stc8h_gpio_read_port(port);
    mask = stc8h_gpio_mask(pin);
    if (value != 0u) {
        p |= mask;
    } else {
        p &= (stc8h_u8)~mask;
    }
    stc8h_gpio_write_port(port, p);
}

stc8h_u8 stc8h_gpio_read(stc8h_u8 port, stc8h_u8 pin)
{
    stc8h_u8 p;
    stc8h_u8 mask;

    if ((port >= STC8H_GPIO_PORT_COUNT) || (pin > 7u)) {
        return 0u;
    }

    p = stc8h_gpio_read_port(port);
    mask = stc8h_gpio_mask(pin);
    return ((p & mask) != 0u) ? 1u : 0u;
}

void stc8h_gpio_toggle(stc8h_u8 port, stc8h_u8 pin)
{
    stc8h_u8 p;

    if ((port >= STC8H_GPIO_PORT_COUNT) || (pin > 7u)) {
        return;
    }

    p = stc8h_gpio_read_port(port);
    p ^= stc8h_gpio_mask(pin);
    stc8h_gpio_write_port(port, p);
}
