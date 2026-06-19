#ifndef STC8H_BOOT_JUMP_H
#define STC8H_BOOT_JUMP_H

#include "stc8h_boot_stub.h"

typedef void (*stc8h_boot_app_entry_t)(void);

void stc8h_boot_jump_to_app(void);
stc8h_status_t stc8h_boot_mark_app_valid(void);

#endif
