#include "stc8h_iap_ota_params.h"

#if STC8H_IAP_OTA_PARAMS_ENABLE

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

#define STC8H_IAP_OTA_PARAMS_CMD_READ 1u
#define STC8H_IAP_OTA_PARAMS_CMD_WRITE 2u
#define STC8H_IAP_OTA_PARAMS_CMD_ERASE 3u
#define STC8H_IAP_OTA_PARAMS_ENABLE_BIT 0x80u
#define STC8H_IAP_OTA_PARAMS_CMD_FAIL 0x10u

static stc8h_u8 stc8h_iap_ota_params_in_sector(stc8h_u16 addr,
                                                stc8h_u16 len,
                                                stc8h_u16 base)
{
    stc8h_u32 end_addr;
    stc8h_u32 sector_end;

    if (len == 0u) {
        return STC8H_TRUE;
    }
    if (addr < base) {
        return STC8H_FALSE;
    }

    end_addr = (stc8h_u32)addr + (stc8h_u32)len - 1UL;
    sector_end = (stc8h_u32)base + (stc8h_u32)STC8H_IAP_OTA_PARAM_SECTOR_SIZE - 1UL;
    return (end_addr <= sector_end) ? STC8H_TRUE : STC8H_FALSE;
}

static stc8h_u8 stc8h_iap_ota_params_range_ok(stc8h_u16 addr, stc8h_u16 len)
{
    if (stc8h_iap_ota_params_in_sector(addr, len, STC8H_IAP_OTA_PARAM_A_BASE) == STC8H_TRUE) {
        return STC8H_TRUE;
    }
    return stc8h_iap_ota_params_in_sector(addr, len, STC8H_IAP_OTA_PARAM_B_BASE);
}

static void stc8h_iap_ota_params_enable_cmd(stc8h_u8 cmd)
{
    IAP_CONTR = STC8H_IAP_OTA_PARAMS_ENABLE_BIT;
    IAP_TPS = STC8H_IAP_TPS;
    IAP_CMD = cmd;
}

static void stc8h_iap_ota_params_disable(void)
{
    IAP_CONTR = 0u;
    IAP_CMD = 0u;
    IAP_TRIG = 0u;
    IAP_ADDRH = 0xFFu;
    IAP_ADDRL = 0xFFu;
}

static stc8h_status_t stc8h_iap_ota_params_trigger(void)
{
    stc8h_u8 ea_state;

    ea_state = EA;
    EA = 0;
    IAP_TRIG = 0x5Au;
    IAP_TRIG = 0xA5u;
    STC8H_NOP();
    STC8H_NOP();
    EA = ea_state;

    return ((IAP_CONTR & STC8H_IAP_OTA_PARAMS_CMD_FAIL) != 0u) ? STC8H_ERROR : STC8H_OK;
}

static void stc8h_iap_ota_params_set_addr(stc8h_u16 addr)
{
    IAP_ADDRH = (stc8h_u8)(addr >> 8);
    IAP_ADDRL = (stc8h_u8)addr;
}

stc8h_status_t stc8h_iap_ota_params_erase(stc8h_u16 addr)
{
    stc8h_status_t status;

    if ((STC8H_IAP_TPS == 0u) ||
        ((addr != STC8H_IAP_OTA_PARAM_A_BASE) && (addr != STC8H_IAP_OTA_PARAM_B_BASE))) {
        return STC8H_ERROR;
    }

    stc8h_iap_ota_params_enable_cmd(STC8H_IAP_OTA_PARAMS_CMD_ERASE);
    stc8h_iap_ota_params_set_addr(addr);
    status = stc8h_iap_ota_params_trigger();
    stc8h_iap_ota_params_disable();

    return status;
}

stc8h_status_t stc8h_iap_ota_params_write(stc8h_u16 addr,
                                          const stc8h_u8 *data,
                                          stc8h_u16 len)
{
    stc8h_status_t status;

    if (((len != 0u) && (data == 0)) ||
        (STC8H_IAP_TPS == 0u) ||
        (stc8h_iap_ota_params_range_ok(addr, len) == STC8H_FALSE)) {
        return STC8H_ERROR;
    }

    status = STC8H_OK;
    stc8h_iap_ota_params_enable_cmd(STC8H_IAP_OTA_PARAMS_CMD_WRITE);
    while ((len != 0u) && (status == STC8H_OK)) {
        stc8h_iap_ota_params_set_addr(addr);
        IAP_DATA = *data;
        status = stc8h_iap_ota_params_trigger();
        ++data;
        ++addr;
        --len;
    }
    stc8h_iap_ota_params_disable();

    return status;
}

stc8h_status_t stc8h_iap_ota_params_read(stc8h_u16 addr,
                                         stc8h_u8 *data,
                                         stc8h_u16 len)
{
    stc8h_status_t status;

    if (((len != 0u) && (data == 0)) ||
        (STC8H_IAP_TPS == 0u) ||
        (stc8h_iap_ota_params_range_ok(addr, len) == STC8H_FALSE)) {
        return STC8H_ERROR;
    }

    status = STC8H_OK;
    stc8h_iap_ota_params_enable_cmd(STC8H_IAP_OTA_PARAMS_CMD_READ);
    while ((len != 0u) && (status == STC8H_OK)) {
        stc8h_iap_ota_params_set_addr(addr);
        status = stc8h_iap_ota_params_trigger();
        *data = IAP_DATA;
        ++data;
        ++addr;
        --len;
    }
    stc8h_iap_ota_params_disable();

    return status;
}

#endif
