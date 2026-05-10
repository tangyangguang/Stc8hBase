#include "drv_button.h"

static void drv_button_add_ms(stc8h_u16 *value, stc8h_u16 elapsed_ms)
{
    stc8h_u16 next;

    next = (stc8h_u16)(*value + elapsed_ms);
    if (next < *value) {
        next = 0xFFFFu;
    }
    *value = next;
}

void drv_button_init(drv_button_t *button, stc8h_u8 active_level)
{
    if (button == 0) {
        return;
    }

    button->active_level = active_level ? 1u : 0u;
    button->stable_state = 0u;
    button->last_raw = 0u;
    button->long_reported = 0u;
    button->event = DRV_BUTTON_EVENT_NONE;
    button->debounce_ms = DRV_BUTTON_DEBOUNCE_MS;
    button->debounce_count_ms = 0u;
    button->press_ms = 0u;
    button->long_press_ms = DRV_BUTTON_LONG_PRESS_MS;
}

void drv_button_scan(drv_button_t *button, stc8h_u8 raw_level, stc8h_u16 elapsed_ms)
{
    stc8h_u8 raw_active;

    if (button == 0) {
        return;
    }

    raw_active = (raw_level == button->active_level) ? 1u : 0u;
    if (raw_active != button->last_raw) {
        button->last_raw = raw_active;
        button->debounce_count_ms = 0u;
        return;
    }

    if (button->debounce_count_ms < button->debounce_ms) {
        drv_button_add_ms(&button->debounce_count_ms, elapsed_ms);
        if (button->debounce_count_ms < button->debounce_ms) {
            return;
        }
    }

    if (button->stable_state != raw_active) {
        button->stable_state = raw_active;
        if (raw_active != 0u) {
            button->press_ms = 0u;
            button->long_reported = 0u;
            button->event = DRV_BUTTON_EVENT_PRESS;
        } else {
            button->event = (button->long_reported == 0u) ? DRV_BUTTON_EVENT_SHORT : DRV_BUTTON_EVENT_RELEASE;
        }
        return;
    }

    if (button->stable_state != 0u) {
        drv_button_add_ms(&button->press_ms, elapsed_ms);
        if ((button->long_reported == 0u) && (button->press_ms >= button->long_press_ms)) {
            button->long_reported = 1u;
            button->event = DRV_BUTTON_EVENT_LONG;
        }
    }
}

drv_button_event_t drv_button_get_event(drv_button_t *button)
{
    drv_button_event_t event;

    if (button == 0) {
        return DRV_BUTTON_EVENT_NONE;
    }

    event = button->event;
    button->event = DRV_BUTTON_EVENT_NONE;
    return event;
}
