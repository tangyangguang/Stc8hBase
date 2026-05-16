#include "drv_tm1637.h"
#include "stc8h_delay.h"

#ifndef BOARD_TM1637_CLK_HIGH
#error "BOARD_TM1637_CLK_HIGH must be defined by board_pins.h."
#endif

#ifndef BOARD_TM1637_CLK_LOW
#error "BOARD_TM1637_CLK_LOW must be defined by board_pins.h."
#endif

#ifndef BOARD_TM1637_DIO_HIGH
#error "BOARD_TM1637_DIO_HIGH must be defined by board_pins.h."
#endif

#ifndef BOARD_TM1637_DIO_LOW
#error "BOARD_TM1637_DIO_LOW must be defined by board_pins.h."
#endif

#ifndef BOARD_TM1637_DIO_READ
#error "BOARD_TM1637_DIO_READ must be defined by board_pins.h."
#endif

#define DRV_TM1637_CMD_DATA_AUTO 0x40u
#define DRV_TM1637_CMD_ADDR_BASE 0xC0u
#define DRV_TM1637_CMD_DISPLAY   0x88u
#define DRV_TM1637_MAX_DIGITS    6u
#define DRV_TM1637_MINUS         0x40u

#if DRV_TM1637_ENABLE_BRIGHTNESS_STATE
static stc8h_u8 drv_tm1637_brightness = 7u;
#endif
#if DRV_TM1637_ENABLE_SET_DISPLAY
static stc8h_u8 drv_tm1637_display_on = 1u;
#endif

static void drv_tm1637_delay(void)
{
    stc8h_delay_us(DRV_TM1637_DELAY_US);
}

static void drv_tm1637_start(void)
{
    BOARD_TM1637_DIO_HIGH();
    BOARD_TM1637_CLK_HIGH();
    drv_tm1637_delay();
    BOARD_TM1637_DIO_LOW();
    drv_tm1637_delay();
    BOARD_TM1637_CLK_LOW();
}

static void drv_tm1637_stop(void)
{
    BOARD_TM1637_CLK_LOW();
    BOARD_TM1637_DIO_LOW();
    drv_tm1637_delay();
    BOARD_TM1637_CLK_HIGH();
    drv_tm1637_delay();
    BOARD_TM1637_DIO_HIGH();
    drv_tm1637_delay();
}

static stc8h_status_t drv_tm1637_write_byte(stc8h_u8 value)
{
    stc8h_u8 i;
    stc8h_u8 ack;

    for (i = 0u; i < 8u; ++i) {
        BOARD_TM1637_CLK_LOW();
        if ((value & 0x01u) != 0u) {
            BOARD_TM1637_DIO_HIGH();
        } else {
            BOARD_TM1637_DIO_LOW();
        }
        drv_tm1637_delay();
        BOARD_TM1637_CLK_HIGH();
        drv_tm1637_delay();
        value >>= 1;
    }

    BOARD_TM1637_CLK_LOW();
    BOARD_TM1637_DIO_HIGH();
    drv_tm1637_delay();
    BOARD_TM1637_CLK_HIGH();
    drv_tm1637_delay();
    ack = BOARD_TM1637_DIO_READ();
    BOARD_TM1637_CLK_LOW();

    return (ack == 0u) ? STC8H_OK : STC8H_ERROR;
}

static stc8h_status_t drv_tm1637_write_cmd(stc8h_u8 cmd)
{
    stc8h_status_t status;

    drv_tm1637_start();
    status = drv_tm1637_write_byte(cmd);
    drv_tm1637_stop();

    return status;
}

static stc8h_status_t drv_tm1637_write_control(void)
{
    stc8h_u8 cmd;

#if DRV_TM1637_ENABLE_SET_DISPLAY
    if (drv_tm1637_display_on != 0u) {
#endif
#if DRV_TM1637_ENABLE_BRIGHTNESS_STATE
        cmd = (stc8h_u8)(DRV_TM1637_CMD_DISPLAY | (drv_tm1637_brightness & 0x07u));
#else
        cmd = (stc8h_u8)(DRV_TM1637_CMD_DISPLAY | (DRV_TM1637_FIXED_BRIGHTNESS & 0x07u));
#endif
#if DRV_TM1637_ENABLE_SET_DISPLAY
    } else {
        cmd = 0x80u;
    }
#endif

    return drv_tm1637_write_cmd(cmd);
}

#if DRV_TM1637_ENABLE_DISPLAY_NUMBER
static stc8h_u8 drv_tm1637_divmod10(stc8h_u16 *value)
{
    stc8h_u16 quotient;
    stc8h_u16 remainder;

    quotient = 0u;
    remainder = *value;
    while (remainder >= 10u) {
        remainder = (stc8h_u16)(remainder - 10u);
        ++quotient;
    }
    *value = quotient;

    return (stc8h_u8)remainder;
}
#endif

void drv_tm1637_init(void)
{
#if DRV_TM1637_ENABLE_BRIGHTNESS_STATE
    drv_tm1637_brightness = 7u;
#endif
#if DRV_TM1637_ENABLE_SET_DISPLAY
    drv_tm1637_display_on = 1u;
#endif
    BOARD_TM1637_CLK_HIGH();
    BOARD_TM1637_DIO_HIGH();
}

void drv_tm1637_set_brightness(stc8h_u8 brightness)
{
#if DRV_TM1637_ENABLE_BRIGHTNESS_STATE
    drv_tm1637_brightness = brightness & 0x07u;
#else
    (void)brightness;
#endif
}

#if DRV_TM1637_ENABLE_SET_DISPLAY
void drv_tm1637_set_display(stc8h_u8 on)
{
    drv_tm1637_display_on = on ? 1u : 0u;
}
#endif

#if DRV_TM1637_ENABLE_DISPLAY_RAW4 || !DRV_TM1637_ENABLE_DISPLAY_RAW
#define DRV_TM1637_RAW_FN static stc8h_status_t drv_tm1637_display_raw_len
#define DRV_TM1637_DISPLAY_RAW_INTERNAL drv_tm1637_display_raw_len
#else
#define DRV_TM1637_RAW_FN stc8h_status_t drv_tm1637_display_raw
#define DRV_TM1637_DISPLAY_RAW_INTERNAL drv_tm1637_display_raw
#endif

DRV_TM1637_RAW_FN(const stc8h_u8 *segments, stc8h_u8 len)
{
    stc8h_u8 i;
    stc8h_status_t status;

#if DRV_TM1637_ENABLE_RAW_LEN_CHECK
    if ((segments == 0) || (len == 0u) || (len > DRV_TM1637_MAX_DIGITS)) {
        return STC8H_ERROR;
    }
#endif

    status = drv_tm1637_write_cmd(DRV_TM1637_CMD_DATA_AUTO);
    if (status != STC8H_OK) {
        return status;
    }

    drv_tm1637_start();
    status = drv_tm1637_write_byte(DRV_TM1637_CMD_ADDR_BASE);
    for (i = 0u; (i < len) && (status == STC8H_OK); ++i) {
        status = drv_tm1637_write_byte(segments[i]);
    }
    drv_tm1637_stop();
    if (status != STC8H_OK) {
        return status;
    }

    return drv_tm1637_write_control();
}

#if DRV_TM1637_ENABLE_DISPLAY_RAW && DRV_TM1637_ENABLE_DISPLAY_RAW4
stc8h_status_t drv_tm1637_display_raw(const stc8h_u8 *segments, stc8h_u8 len)
{
    return drv_tm1637_display_raw_len(segments, len);
}
#endif

#if DRV_TM1637_ENABLE_DISPLAY_RAW4
stc8h_status_t drv_tm1637_display_raw4(const stc8h_u8 segments[4])
{
    return DRV_TM1637_DISPLAY_RAW_INTERNAL(segments, 4u);
}
#endif

#if DRV_TM1637_ENABLE_DISPLAY_DIGITS
stc8h_status_t drv_tm1637_display_digits(const stc8h_u8 *digits, stc8h_u8 len)
{
    stc8h_u8 i;
    stc8h_u8 segments[DRV_TM1637_MAX_DIGITS];

    if ((digits == 0) || (len == 0u) || (len > DRV_TM1637_DIGITS) || (len > DRV_TM1637_MAX_DIGITS)) {
        return STC8H_ERROR;
    }

    for (i = 0u; i < len; ++i) {
        segments[i] = drv_tm1637_encode_digit(digits[i]);
    }

    return DRV_TM1637_DISPLAY_RAW_INTERNAL(segments, len);
}
#endif

#if DRV_TM1637_ENABLE_DISPLAY_NUMBER
stc8h_status_t drv_tm1637_display_number(stc8h_s16 value)
{
    stc8h_u8 segments[DRV_TM1637_DIGITS];
    stc8h_u8 pos;
    stc8h_u16 magnitude;

    for (pos = 0u; pos < DRV_TM1637_DIGITS; ++pos) {
        segments[pos] = 0u;
    }

    if (value < 0) {
        magnitude = (stc8h_u16)(0 - (stc8h_s16)(value + 1));
        ++magnitude;
    } else {
        magnitude = (stc8h_u16)value;
    }

    pos = DRV_TM1637_DIGITS;
    do {
        --pos;
        segments[pos] = drv_tm1637_encode_digit(drv_tm1637_divmod10(&magnitude));
    } while ((magnitude != 0u) && (pos != 0u));

    if (value < 0) {
        if (pos == 0u) {
            segments[0] = DRV_TM1637_MINUS;
        } else {
            segments[(stc8h_u8)(pos - 1u)] = DRV_TM1637_MINUS;
        }
    }

    return DRV_TM1637_DISPLAY_RAW_INTERNAL(segments, DRV_TM1637_DIGITS);
}
#endif

#if DRV_TM1637_ENABLE_ENCODE_DIGIT
stc8h_u8 drv_tm1637_encode_digit(stc8h_u8 digit)
{
    static const STC8H_CODE stc8h_u8 table[10] = {
        0x3Fu, 0x06u, 0x5Bu, 0x4Fu, 0x66u,
        0x6Du, 0x7Du, 0x07u, 0x7Fu, 0x6Fu
    };

    if (digit > 9u) {
        return 0u;
    }

    return table[digit];
}
#endif

#if DRV_TM1637_ENABLE_CLEAR
stc8h_status_t drv_tm1637_clear(void)
{
    stc8h_u8 segments[DRV_TM1637_DIGITS];
    stc8h_u8 i;

    for (i = 0u; i < DRV_TM1637_DIGITS; ++i) {
        segments[i] = 0u;
    }

    return DRV_TM1637_DISPLAY_RAW_INTERNAL(segments, DRV_TM1637_DIGITS);
}
#endif
