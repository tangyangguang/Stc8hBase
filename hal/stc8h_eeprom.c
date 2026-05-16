#include "stc8h_eeprom.h"
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

#define STC8H_IAP_CMD_READ 1u
#define STC8H_IAP_CMD_WRITE 2u
#define STC8H_IAP_CMD_ERASE 3u
#define STC8H_IAP_ENABLE 0x80u
#define STC8H_IAP_CMD_FAIL 0x10u

#if STC8H_EEPROM_ENABLE_READ || STC8H_EEPROM_ENABLE_WRITE
static stc8h_u8 stc8h_eeprom_range_ok(stc8h_u16 addr, stc8h_u16 len)
{
    if (len == 0u) {
        return STC8H_TRUE;
    }
    if (addr >= STC8H_EEPROM_SIZE) {
        return STC8H_FALSE;
    }
    if (len > (stc8h_u16)(STC8H_EEPROM_SIZE - addr)) {
        return STC8H_FALSE;
    }
    return STC8H_TRUE;
}
#endif

static void stc8h_eeprom_enable(stc8h_u8 cmd)
{
    IAP_CONTR = STC8H_IAP_ENABLE;
    IAP_TPS = STC8H_IAP_TPS;
    IAP_CMD = cmd;
}

static void stc8h_eeprom_disable(void)
{
    IAP_CONTR = 0u;
    IAP_CMD = 0u;
    IAP_TRIG = 0u;
    IAP_ADDRH = 0xFFu;
    IAP_ADDRL = 0xFFu;
}

static stc8h_status_t stc8h_eeprom_trigger(void)
{
    stc8h_u8 ea_state;

    ea_state = EA;
    EA = 0;
    IAP_TRIG = 0x5Au;
    IAP_TRIG = 0xA5u;
    STC8H_NOP();
    STC8H_NOP();
    EA = ea_state;

    return ((IAP_CONTR & STC8H_IAP_CMD_FAIL) != 0u) ? STC8H_ERROR : STC8H_OK;
}

static void stc8h_eeprom_set_addr(stc8h_u16 addr)
{
    IAP_ADDRH = (stc8h_u8)(addr >> 8);
    IAP_ADDRL = (stc8h_u8)addr;
}

#if STC8H_EEPROM_ENABLE_READ
stc8h_status_t stc8h_eeprom_read(stc8h_u16 addr, STC8H_DATA stc8h_u8 *data, stc8h_u16 len)
{
    stc8h_status_t status;

    if ((data == 0) || (STC8H_IAP_TPS == 0u) || (stc8h_eeprom_range_ok(addr, len) == STC8H_FALSE)) {
        return STC8H_ERROR;
    }

    status = STC8H_OK;
    stc8h_eeprom_enable(STC8H_IAP_CMD_READ);
    while ((len != 0u) && (status == STC8H_OK)) {
        stc8h_eeprom_set_addr(addr);
        status = stc8h_eeprom_trigger();
        *data = IAP_DATA;
        ++data;
        ++addr;
        --len;
    }
    stc8h_eeprom_disable();

    return status;
}
#endif

#if STC8H_EEPROM_ENABLE_WRITE
stc8h_status_t stc8h_eeprom_write(stc8h_u16 addr, const STC8H_DATA stc8h_u8 *data, stc8h_u16 len)
{
    stc8h_status_t status;

    if ((data == 0) || (STC8H_IAP_TPS == 0u) || (stc8h_eeprom_range_ok(addr, len) == STC8H_FALSE)) {
        return STC8H_ERROR;
    }

    status = STC8H_OK;
    stc8h_eeprom_enable(STC8H_IAP_CMD_WRITE);
    while ((len != 0u) && (status == STC8H_OK)) {
        stc8h_eeprom_set_addr(addr);
        IAP_DATA = *data;
        status = stc8h_eeprom_trigger();
        ++data;
        ++addr;
        --len;
    }
    stc8h_eeprom_disable();

    return status;
}
#endif

#if STC8H_EEPROM_ENABLE_ERASE
stc8h_status_t stc8h_eeprom_erase_sector(stc8h_u16 addr)
{
    stc8h_status_t status;

    if ((STC8H_IAP_TPS == 0u) || (addr >= STC8H_EEPROM_SIZE) || ((addr & (STC8H_EEPROM_SECTOR_SIZE - 1u)) != 0u)) {
        return STC8H_ERROR;
    }

    stc8h_eeprom_enable(STC8H_IAP_CMD_ERASE);
    stc8h_eeprom_set_addr(addr);
    status = stc8h_eeprom_trigger();
    stc8h_eeprom_disable();

    return status;
}
#endif

#if STC8H_EEPROM_ENABLE_FIXED_BLOCK
stc8h_status_t stc8h_eeprom_read_fixed(STC8H_DATA stc8h_u8 *data)
{
    stc8h_u16 addr;
#if STC8H_EEPROM_FIXED_SIZE <= 255u
    stc8h_u8 len;
#else
    stc8h_u16 len;
#endif
    stc8h_status_t status;

    if ((data == 0) || (STC8H_IAP_TPS == 0u)) {
        return STC8H_ERROR;
    }

    addr = STC8H_EEPROM_FIXED_ADDR;
    len = STC8H_EEPROM_FIXED_SIZE;
    status = STC8H_OK;
    stc8h_eeprom_enable(STC8H_IAP_CMD_READ);
    while ((len != 0u) && (status == STC8H_OK)) {
        stc8h_eeprom_set_addr(addr);
        status = stc8h_eeprom_trigger();
        *data = IAP_DATA;
        ++data;
        ++addr;
        --len;
    }
    stc8h_eeprom_disable();

    return status;
}

stc8h_status_t stc8h_eeprom_save_fixed(const STC8H_DATA stc8h_u8 *data)
{
    stc8h_u16 addr;
#if STC8H_EEPROM_FIXED_SIZE <= 255u
    stc8h_u8 len;
#else
    stc8h_u16 len;
#endif
    stc8h_status_t status;

    if ((data == 0) || (STC8H_IAP_TPS == 0u)) {
        return STC8H_ERROR;
    }

    stc8h_eeprom_enable(STC8H_IAP_CMD_ERASE);
    stc8h_eeprom_set_addr(STC8H_EEPROM_FIXED_ADDR);
    status = stc8h_eeprom_trigger();
    stc8h_eeprom_disable();
    if (status != STC8H_OK) {
        return status;
    }

    addr = STC8H_EEPROM_FIXED_ADDR;
    len = STC8H_EEPROM_FIXED_SIZE;
    stc8h_eeprom_enable(STC8H_IAP_CMD_WRITE);
    while ((len != 0u) && (status == STC8H_OK)) {
        stc8h_eeprom_set_addr(addr);
        IAP_DATA = *data;
        status = stc8h_eeprom_trigger();
        ++data;
        ++addr;
        --len;
    }
    stc8h_eeprom_disable();

    return status;
}
#endif
