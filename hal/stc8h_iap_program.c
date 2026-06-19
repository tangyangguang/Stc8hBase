#include "stc8h_iap_program.h"

#if STC8H_IAP_PROGRAM_ENABLE

#include "stc8h_sfr.h"

#ifndef STC8H_IAP_TPS
#if STC8H_SYSCLK_HZ == 11059200UL
#define STC8H_IAP_TPS 11u
#elif STC8H_SYSCLK_HZ == 12000000UL
#define STC8H_IAP_TPS 12u
#else
#define STC8H_IAP_TPS 0u
#endif
#endif

#define STC8H_IAP_PROGRAM_CMD_READ 1u
#define STC8H_IAP_PROGRAM_CMD_WRITE 2u
#define STC8H_IAP_PROGRAM_CMD_ERASE 3u
#define STC8H_IAP_PROGRAM_ENABLE_BIT 0x80u
#define STC8H_IAP_PROGRAM_CMD_FAIL 0x10u

static stc8h_u8 stc8h_iap_program_range_ok(stc8h_u16 addr, stc8h_u16 len)
{
    stc8h_u32 end_addr;

    if (len == 0u) {
        return STC8H_TRUE;
    }
    if ((addr < STC8H_IAP_PROGRAM_APP_BASE) || (addr > STC8H_IAP_PROGRAM_APP_LIMIT)) {
        return STC8H_FALSE;
    }

    end_addr = (stc8h_u32)addr + (stc8h_u32)len - 1UL;
    if (end_addr > (stc8h_u32)STC8H_IAP_PROGRAM_APP_LIMIT) {
        return STC8H_FALSE;
    }

    return STC8H_TRUE;
}

static stc8h_u16 stc8h_iap_program_to_iap_addr(stc8h_u16 addr)
{
    return (stc8h_u16)(addr - STC8H_IAP_PROGRAM_FLASH_BASE);
}

static void stc8h_iap_program_enable_cmd(stc8h_u8 cmd)
{
    IAP_CONTR = STC8H_IAP_PROGRAM_ENABLE_BIT;
    IAP_TPS = STC8H_IAP_TPS;
    IAP_CMD = cmd;
}

static void stc8h_iap_program_disable(void)
{
    IAP_CONTR = 0u;
    IAP_CMD = 0u;
    IAP_TRIG = 0u;
    IAP_ADDRH = 0xFFu;
    IAP_ADDRL = 0xFFu;
}

static stc8h_status_t stc8h_iap_program_trigger(void)
{
    stc8h_u8 ea_state;

    ea_state = EA;
    EA = 0;
    IAP_TRIG = 0x5Au;
    IAP_TRIG = 0xA5u;
    STC8H_NOP();
    STC8H_NOP();
    EA = ea_state;

    return ((IAP_CONTR & STC8H_IAP_PROGRAM_CMD_FAIL) != 0u) ? STC8H_ERROR : STC8H_OK;
}

static void stc8h_iap_program_set_addr(stc8h_u16 addr)
{
    IAP_ADDRH = (stc8h_u8)(addr >> 8);
    IAP_ADDRL = (stc8h_u8)addr;
}

stc8h_status_t stc8h_iap_program_erase_sector(stc8h_u16 addr)
{
    stc8h_status_t status;
    stc8h_u16 iap_addr;

    if ((STC8H_IAP_TPS == 0u) ||
        (stc8h_iap_program_range_ok(addr, STC8H_IAP_PROGRAM_SECTOR_SIZE) == STC8H_FALSE) ||
        (((stc8h_u16)(addr - STC8H_IAP_PROGRAM_FLASH_BASE) &
          (STC8H_IAP_PROGRAM_SECTOR_SIZE - 1u)) != 0u)) {
        return STC8H_ERROR;
    }

    iap_addr = stc8h_iap_program_to_iap_addr(addr);
    stc8h_iap_program_enable_cmd(STC8H_IAP_PROGRAM_CMD_ERASE);
    stc8h_iap_program_set_addr(iap_addr);
    status = stc8h_iap_program_trigger();
    stc8h_iap_program_disable();

    return status;
}

stc8h_status_t stc8h_iap_program_write(stc8h_u16 addr,
                                       const stc8h_u8 *data,
                                       stc8h_u16 len)
{
    stc8h_status_t status;

    if (((len != 0u) && (data == 0)) ||
        (STC8H_IAP_TPS == 0u) ||
        (stc8h_iap_program_range_ok(addr, len) == STC8H_FALSE)) {
        return STC8H_ERROR;
    }

    status = STC8H_OK;
    stc8h_iap_program_enable_cmd(STC8H_IAP_PROGRAM_CMD_WRITE);
    while ((len != 0u) && (status == STC8H_OK)) {
        stc8h_iap_program_set_addr(stc8h_iap_program_to_iap_addr(addr));
        IAP_DATA = *data;
        status = stc8h_iap_program_trigger();
        ++data;
        ++addr;
        --len;
    }
    stc8h_iap_program_disable();

    return status;
}

stc8h_status_t stc8h_iap_program_read(stc8h_u16 addr,
                                      stc8h_u8 *data,
                                      stc8h_u16 len)
{
    stc8h_status_t status;

    if (((len != 0u) && (data == 0)) ||
        (STC8H_IAP_TPS == 0u) ||
        (stc8h_iap_program_range_ok(addr, len) == STC8H_FALSE)) {
        return STC8H_ERROR;
    }

    status = STC8H_OK;
    stc8h_iap_program_enable_cmd(STC8H_IAP_PROGRAM_CMD_READ);
    while ((len != 0u) && (status == STC8H_OK)) {
        stc8h_iap_program_set_addr(stc8h_iap_program_to_iap_addr(addr));
        status = stc8h_iap_program_trigger();
        *data = IAP_DATA;
        ++data;
        ++addr;
        --len;
    }
    stc8h_iap_program_disable();

    return status;
}

#endif
