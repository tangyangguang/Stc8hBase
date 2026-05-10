#include "drv_lcd1602.h"
#include "stc8h_i2c.h"
#include "stc8h_delay.h"

#ifndef DRV_LCD1602_I2C_BIT_RS
#define DRV_LCD1602_I2C_BIT_RS 0u
#endif
#ifndef DRV_LCD1602_I2C_BIT_RW
#define DRV_LCD1602_I2C_BIT_RW 1u
#endif
#ifndef DRV_LCD1602_I2C_BIT_EN
#define DRV_LCD1602_I2C_BIT_EN 2u
#endif
#ifndef DRV_LCD1602_I2C_BIT_BL
#define DRV_LCD1602_I2C_BIT_BL 3u
#endif
#ifndef DRV_LCD1602_I2C_BIT_D4
#define DRV_LCD1602_I2C_BIT_D4 4u
#endif
#ifndef DRV_LCD1602_I2C_BIT_D5
#define DRV_LCD1602_I2C_BIT_D5 5u
#endif
#ifndef DRV_LCD1602_I2C_BIT_D6
#define DRV_LCD1602_I2C_BIT_D6 6u
#endif
#ifndef DRV_LCD1602_I2C_BIT_D7
#define DRV_LCD1602_I2C_BIT_D7 7u
#endif

#ifndef DRV_LCD1602_BACKLIGHT_DEFAULT
#define DRV_LCD1602_BACKLIGHT_DEFAULT 1u
#endif

static stc8h_u8 drv_lcd1602_addr7;
static stc8h_u8 drv_lcd1602_backlight;

static stc8h_u8 drv_lcd1602_bit(stc8h_u8 bit)
{
    return (stc8h_u8)(1u << bit);
}

static stc8h_u8 drv_lcd1602_pack_nibble(stc8h_u8 nibble, stc8h_u8 rs)
{
    stc8h_u8 value;

    value = 0u;
    if ((nibble & 0x01u) != 0u) {
        value |= drv_lcd1602_bit(DRV_LCD1602_I2C_BIT_D4);
    }
    if ((nibble & 0x02u) != 0u) {
        value |= drv_lcd1602_bit(DRV_LCD1602_I2C_BIT_D5);
    }
    if ((nibble & 0x04u) != 0u) {
        value |= drv_lcd1602_bit(DRV_LCD1602_I2C_BIT_D6);
    }
    if ((nibble & 0x08u) != 0u) {
        value |= drv_lcd1602_bit(DRV_LCD1602_I2C_BIT_D7);
    }
    if (rs != 0u) {
        value |= drv_lcd1602_bit(DRV_LCD1602_I2C_BIT_RS);
    }
    if (drv_lcd1602_backlight != 0u) {
        value |= drv_lcd1602_bit(DRV_LCD1602_I2C_BIT_BL);
    }
    return value;
}

static void drv_lcd1602_write_raw(stc8h_u8 value)
{
    (void)stc8h_i2c_write(drv_lcd1602_addr7, &value, 1u);
}

static void drv_lcd1602_pulse(stc8h_u8 value)
{
    drv_lcd1602_write_raw((stc8h_u8)(value | drv_lcd1602_bit(DRV_LCD1602_I2C_BIT_EN)));
    stc8h_delay_us(1u);
    drv_lcd1602_write_raw((stc8h_u8)(value & (stc8h_u8)~drv_lcd1602_bit(DRV_LCD1602_I2C_BIT_EN)));
    stc8h_delay_us(50u);
}

static void drv_lcd1602_write_nibble(stc8h_u8 nibble, stc8h_u8 rs)
{
    drv_lcd1602_pulse(drv_lcd1602_pack_nibble((stc8h_u8)(nibble & 0x0Fu), rs));
}

static void drv_lcd1602_write_byte(stc8h_u8 value, stc8h_u8 rs)
{
    drv_lcd1602_write_nibble((stc8h_u8)(value >> 4), rs);
    drv_lcd1602_write_nibble(value, rs);
}

static void drv_lcd1602_command(stc8h_u8 command)
{
    drv_lcd1602_write_byte(command, 0u);
    if ((command == 0x01u) || (command == 0x02u)) {
        stc8h_delay_ms(2u);
    }
}

void drv_lcd1602_init(stc8h_u8 addr7)
{
    drv_lcd1602_addr7 = addr7;
    drv_lcd1602_backlight = DRV_LCD1602_BACKLIGHT_DEFAULT;

    stc8h_i2c_init();
    stc8h_delay_ms(50u);

    drv_lcd1602_write_nibble(0x03u, 0u);
    stc8h_delay_ms(5u);
    drv_lcd1602_write_nibble(0x03u, 0u);
    stc8h_delay_us(150u);
    drv_lcd1602_write_nibble(0x03u, 0u);
    drv_lcd1602_write_nibble(0x02u, 0u);

    drv_lcd1602_command(0x28u);
    drv_lcd1602_command(0x08u);
    drv_lcd1602_clear();
    drv_lcd1602_command(0x06u);
    drv_lcd1602_command(0x0Cu);
}

void drv_lcd1602_clear(void)
{
    drv_lcd1602_command(0x01u);
}

void drv_lcd1602_set_cursor(stc8h_u8 row, stc8h_u8 col)
{
    stc8h_u8 addr;

    addr = (row == 0u) ? 0x00u : 0x40u;
    addr = (stc8h_u8)(addr + col);
    drv_lcd1602_command((stc8h_u8)(0x80u | addr));
}

void drv_lcd1602_putc(char ch)
{
    drv_lcd1602_write_byte((stc8h_u8)ch, 1u);
}

void drv_lcd1602_write_string(const char *text)
{
    if (text == 0) {
        return;
    }

    while (*text != '\0') {
        drv_lcd1602_putc(*text);
        ++text;
    }
}

void drv_lcd1602_write_string_code(const STC8H_CODE char *text)
{
    if (text == 0) {
        return;
    }

    while (*text != '\0') {
        drv_lcd1602_putc(*text);
        ++text;
    }
}
