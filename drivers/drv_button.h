#ifndef DRV_BUTTON_H
#define DRV_BUTTON_H

#include "stc8h_config.h"

#ifndef DRV_BUTTON_DEBOUNCE_MS
#define DRV_BUTTON_DEBOUNCE_MS 10u
#endif

#ifndef DRV_BUTTON_LONG_PRESS_MS
#define DRV_BUTTON_LONG_PRESS_MS 800u
#endif

typedef enum {
    DRV_BUTTON_EVENT_NONE = 0,
    DRV_BUTTON_EVENT_PRESS,
    DRV_BUTTON_EVENT_RELEASE,
    DRV_BUTTON_EVENT_SHORT,
    DRV_BUTTON_EVENT_LONG
} drv_button_event_t;

typedef struct {
    stc8h_u8 active_level;
    stc8h_u8 stable_state;
    stc8h_u8 last_raw;
    stc8h_u8 long_reported;
    drv_button_event_t event;
    stc8h_u16 debounce_ms;
    stc8h_u16 debounce_count_ms;
    stc8h_u16 press_ms;
    stc8h_u16 long_press_ms;
} drv_button_t;

void drv_button_init(drv_button_t *button, stc8h_u8 active_level);
void drv_button_set_timing(drv_button_t *button, stc8h_u16 debounce_ms, stc8h_u16 long_press_ms);
void drv_button_scan(drv_button_t *button, stc8h_u8 raw_level, stc8h_u16 elapsed_ms);
drv_button_event_t drv_button_get_event(drv_button_t *button);
stc8h_u16 drv_button_get_press_ms(const drv_button_t *button);

#endif
