#ifndef DRV_TM1637_H
#define DRV_TM1637_H

#include "stc8h_config.h"

#ifndef DRV_TM1637_DIGITS
#define DRV_TM1637_DIGITS 4u
#endif

#ifndef DRV_TM1637_DELAY_US
#define DRV_TM1637_DELAY_US 5u
#endif

#ifndef DRV_TM1637_ENABLE_DISPLAY_DIGITS
#define DRV_TM1637_ENABLE_DISPLAY_DIGITS 1
#endif

#ifndef DRV_TM1637_ENABLE_DISPLAY_RAW4
#define DRV_TM1637_ENABLE_DISPLAY_RAW4 0
#endif

#ifndef DRV_TM1637_ENABLE_DISPLAY_RAW
#define DRV_TM1637_ENABLE_DISPLAY_RAW 1
#endif

#ifndef DRV_TM1637_ENABLE_SET_DISPLAY
#define DRV_TM1637_ENABLE_SET_DISPLAY 1
#endif

#ifndef DRV_TM1637_ENABLE_BRIGHTNESS_STATE
#define DRV_TM1637_ENABLE_BRIGHTNESS_STATE 1
#endif

#ifndef DRV_TM1637_FIXED_BRIGHTNESS
#define DRV_TM1637_FIXED_BRIGHTNESS 7u
#endif

#ifndef DRV_TM1637_ENABLE_RAW_LEN_CHECK
#define DRV_TM1637_ENABLE_RAW_LEN_CHECK 1
#endif

#ifndef DRV_TM1637_ENABLE_DISPLAY_NUMBER
#define DRV_TM1637_ENABLE_DISPLAY_NUMBER 1
#endif

#ifndef DRV_TM1637_ENABLE_ENCODE_DIGIT
#define DRV_TM1637_ENABLE_ENCODE_DIGIT 1
#endif

#ifndef DRV_TM1637_ENABLE_CLEAR
#define DRV_TM1637_ENABLE_CLEAR 1
#endif

#if (DRV_TM1637_DIGITS < 1u) || (DRV_TM1637_DIGITS > 6u)
#error "DRV_TM1637_DIGITS must be 1..6."
#endif

#if DRV_TM1637_ENABLE_DISPLAY_DIGITS && !DRV_TM1637_ENABLE_ENCODE_DIGIT
#error "DRV_TM1637_ENABLE_DISPLAY_DIGITS requires DRV_TM1637_ENABLE_ENCODE_DIGIT."
#endif

#if DRV_TM1637_ENABLE_DISPLAY_NUMBER && !DRV_TM1637_ENABLE_ENCODE_DIGIT
#error "DRV_TM1637_ENABLE_DISPLAY_NUMBER requires DRV_TM1637_ENABLE_ENCODE_DIGIT."
#endif

void drv_tm1637_init(void);
void drv_tm1637_set_brightness(stc8h_u8 brightness);
#if DRV_TM1637_ENABLE_SET_DISPLAY
void drv_tm1637_set_display(stc8h_u8 on);
#endif
#if DRV_TM1637_ENABLE_DISPLAY_RAW
stc8h_status_t drv_tm1637_display_raw(const stc8h_u8 *segments, stc8h_u8 len);
#endif
#if DRV_TM1637_ENABLE_DISPLAY_RAW4
stc8h_status_t drv_tm1637_display_raw4(const stc8h_u8 segments[4]);
#endif
#if DRV_TM1637_ENABLE_DISPLAY_DIGITS
stc8h_status_t drv_tm1637_display_digits(const stc8h_u8 *digits, stc8h_u8 len);
#endif
#if DRV_TM1637_ENABLE_DISPLAY_NUMBER
stc8h_status_t drv_tm1637_display_number(stc8h_s16 value);
#endif
#if DRV_TM1637_ENABLE_ENCODE_DIGIT
stc8h_u8 drv_tm1637_encode_digit(stc8h_u8 digit);
#endif
#if DRV_TM1637_ENABLE_CLEAR
stc8h_status_t drv_tm1637_clear(void);
#endif

#endif
