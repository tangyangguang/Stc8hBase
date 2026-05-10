#include "stc8h_interrupt.h"
#include "stc8h_sfr.h"

#if defined(STC8H_HOSTED)
static stc8h_u8 stc8h_hosted_ea;
#endif

void stc8h_interrupt_enable_global(void)
{
#if defined(STC8H_HOSTED)
    stc8h_hosted_ea = 1u;
#else
    EA = 1;
#endif
}

void stc8h_interrupt_disable_global(void)
{
#if defined(STC8H_HOSTED)
    stc8h_hosted_ea = 0u;
#else
    EA = 0;
#endif
}
