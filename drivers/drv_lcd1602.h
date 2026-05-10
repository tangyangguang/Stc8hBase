#ifndef DRV_LCD1602_H
#define DRV_LCD1602_H

#include "stc8h_config.h"

void drv_lcd1602_init(stc8h_u8 addr7);
void drv_lcd1602_clear(void);
void drv_lcd1602_set_cursor(stc8h_u8 row, stc8h_u8 col);
void drv_lcd1602_putc(char ch);
void drv_lcd1602_write_string(const char *text);
void drv_lcd1602_write_string_code(const STC8H_CODE char *text);

#endif
