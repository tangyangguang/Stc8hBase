#include "stc8h_i2c.h"
#include "stc8h_delay.h"

#ifndef BOARD_I2C_SDA_HIGH
#error "BOARD_I2C_SDA_HIGH must be defined by STC8H_PINS_INCLUDE"
#endif
#ifndef BOARD_I2C_SDA_LOW
#error "BOARD_I2C_SDA_LOW must be defined by STC8H_PINS_INCLUDE"
#endif
#ifndef BOARD_I2C_SCL_HIGH
#error "BOARD_I2C_SCL_HIGH must be defined by STC8H_PINS_INCLUDE"
#endif
#ifndef BOARD_I2C_SCL_LOW
#error "BOARD_I2C_SCL_LOW must be defined by STC8H_PINS_INCLUDE"
#endif
#ifndef BOARD_I2C_SDA_READ
#error "BOARD_I2C_SDA_READ must be defined by STC8H_PINS_INCLUDE"
#endif

#ifndef STC8H_I2C_DELAY_US
#define STC8H_I2C_DELAY_US 5u
#endif

static void stc8h_i2c_delay(void)
{
    stc8h_delay_us(STC8H_I2C_DELAY_US);
}

static void stc8h_i2c_start(void)
{
    BOARD_I2C_SDA_HIGH();
    BOARD_I2C_SCL_HIGH();
    stc8h_i2c_delay();
    BOARD_I2C_SDA_LOW();
    stc8h_i2c_delay();
    BOARD_I2C_SCL_LOW();
}

static void stc8h_i2c_stop(void)
{
    BOARD_I2C_SDA_LOW();
    stc8h_i2c_delay();
    BOARD_I2C_SCL_HIGH();
    stc8h_i2c_delay();
    BOARD_I2C_SDA_HIGH();
    stc8h_i2c_delay();
}

static stc8h_u8 stc8h_i2c_write_byte(stc8h_u8 value)
{
    stc8h_u8 i;
    stc8h_u8 ack;

    for (i = 0u; i < 8u; ++i) {
        if ((value & 0x80u) != 0u) {
            BOARD_I2C_SDA_HIGH();
        } else {
            BOARD_I2C_SDA_LOW();
        }
        stc8h_i2c_delay();
        BOARD_I2C_SCL_HIGH();
        stc8h_i2c_delay();
        BOARD_I2C_SCL_LOW();
        value <<= 1;
    }

    BOARD_I2C_SDA_HIGH();
    stc8h_i2c_delay();
    BOARD_I2C_SCL_HIGH();
    stc8h_i2c_delay();
    ack = (BOARD_I2C_SDA_READ() == 0u) ? 1u : 0u;
    BOARD_I2C_SCL_LOW();
    return ack;
}

static stc8h_u8 stc8h_i2c_read_byte(stc8h_u8 ack)
{
    stc8h_u8 i;
    stc8h_u8 value;

    value = 0u;
    BOARD_I2C_SDA_HIGH();
    for (i = 0u; i < 8u; ++i) {
        value <<= 1;
        BOARD_I2C_SCL_HIGH();
        stc8h_i2c_delay();
        if (BOARD_I2C_SDA_READ() != 0u) {
            value |= 1u;
        }
        BOARD_I2C_SCL_LOW();
        stc8h_i2c_delay();
    }

    if (ack != 0u) {
        BOARD_I2C_SDA_LOW();
    } else {
        BOARD_I2C_SDA_HIGH();
    }
    stc8h_i2c_delay();
    BOARD_I2C_SCL_HIGH();
    stc8h_i2c_delay();
    BOARD_I2C_SCL_LOW();
    BOARD_I2C_SDA_HIGH();

    return value;
}

void stc8h_i2c_init(void)
{
    BOARD_I2C_SDA_HIGH();
    BOARD_I2C_SCL_HIGH();
}

stc8h_status_t stc8h_i2c_write(stc8h_u8 addr7, const stc8h_u8 *data, stc8h_u8 len)
{
    stc8h_u8 i;

    if (addr7 > 0x7Fu) {
        return STC8H_ERROR;
    }
    if ((data == 0) && (len != 0u)) {
        return STC8H_ERROR;
    }

    stc8h_i2c_start();
    if (stc8h_i2c_write_byte((stc8h_u8)(addr7 << 1)) == 0u) {
        stc8h_i2c_stop();
        return STC8H_ERROR;
    }

    for (i = 0u; i < len; ++i) {
        if (stc8h_i2c_write_byte(data[i]) == 0u) {
            stc8h_i2c_stop();
            return STC8H_ERROR;
        }
    }

    stc8h_i2c_stop();
    return STC8H_OK;
}

stc8h_status_t stc8h_i2c_read(stc8h_u8 addr7, stc8h_u8 *data, stc8h_u8 len)
{
    stc8h_u8 i;

    if (addr7 > 0x7Fu) {
        return STC8H_ERROR;
    }
    if ((data == 0) && (len != 0u)) {
        return STC8H_ERROR;
    }

    stc8h_i2c_start();
    if (stc8h_i2c_write_byte((stc8h_u8)((addr7 << 1) | 1u)) == 0u) {
        stc8h_i2c_stop();
        return STC8H_ERROR;
    }

    for (i = 0u; i < len; ++i) {
        data[i] = stc8h_i2c_read_byte((i + 1u) < len);
    }

    stc8h_i2c_stop();
    return STC8H_OK;
}

stc8h_u8 stc8h_i2c_probe(stc8h_u8 addr7)
{
    stc8h_u8 ack;

    if (addr7 > 0x7Fu) {
        return 0u;
    }

    stc8h_i2c_start();
    ack = stc8h_i2c_write_byte((stc8h_u8)(addr7 << 1));
    stc8h_i2c_stop();
    return ack;
}
