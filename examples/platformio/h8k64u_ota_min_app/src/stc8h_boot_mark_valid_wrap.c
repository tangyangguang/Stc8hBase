#include "stc8h_boot_jump.h"

stc8h_status_t stc8h_boot_mark_app_valid(void)
{
    /*
     * The minimal link-contract app provides the call site only.
     * The production app wires this to the OTA parameter store after
     * the H8K64U parameter-area IAP ops are integrated.
     */
    return STC8H_OK;
}
