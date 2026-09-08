#ifndef STC8H_BOOT_CONTROL_IAP_H
#define STC8H_BOOT_CONTROL_IAP_H

#include "stc8h_types.h"

/* Application-side control. request_update() only persists intent; callers
 * must send their protocol ACK before invoking controlled_reset(). */
stc8h_status_t stc8h_boot_request_update(stc8h_u32 session_id);
stc8h_status_t stc8h_boot_cancel_update_request(stc8h_u32 session_id);
stc8h_status_t stc8h_boot_mark_app_valid(void);
void stc8h_boot_controlled_reset(void);

#endif
