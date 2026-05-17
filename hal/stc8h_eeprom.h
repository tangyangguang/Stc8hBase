#ifndef STC8H_EEPROM_H
#define STC8H_EEPROM_H

#include "stc8h_config.h"

#define STC8H_EEPROM_SIZE 4096u
#define STC8H_EEPROM_SECTOR_SIZE 512u

#ifndef STC8H_EEPROM_ENABLE_READ
#define STC8H_EEPROM_ENABLE_READ 1
#endif

#ifndef STC8H_EEPROM_ENABLE_WRITE
#define STC8H_EEPROM_ENABLE_WRITE 1
#endif

#ifndef STC8H_EEPROM_ENABLE_ERASE
#define STC8H_EEPROM_ENABLE_ERASE 1
#endif

#ifndef STC8H_EEPROM_ENABLE_FIXED_BLOCK
#define STC8H_EEPROM_ENABLE_FIXED_BLOCK 0
#endif

#if STC8H_EEPROM_ENABLE_FIXED_BLOCK
#ifndef STC8H_EEPROM_FIXED_ADDR
#error "STC8H_EEPROM_FIXED_ADDR must be defined when fixed block EEPROM is enabled."
#endif

#ifndef STC8H_EEPROM_FIXED_SIZE
#error "STC8H_EEPROM_FIXED_SIZE must be defined when fixed block EEPROM is enabled."
#endif

#if (STC8H_EEPROM_FIXED_ADDR & (STC8H_EEPROM_SECTOR_SIZE - 1u)) != 0
#error "STC8H_EEPROM_FIXED_ADDR must be sector aligned."
#endif

#if (STC8H_EEPROM_FIXED_SIZE == 0u) || (STC8H_EEPROM_FIXED_SIZE > STC8H_EEPROM_SECTOR_SIZE)
#error "STC8H_EEPROM_FIXED_SIZE must be 1..STC8H_EEPROM_SECTOR_SIZE."
#endif

#if (STC8H_EEPROM_FIXED_ADDR + STC8H_EEPROM_FIXED_SIZE) > STC8H_EEPROM_SIZE
#error "STC8H_EEPROM_FIXED range exceeds EEPROM size."
#endif
#endif

#if STC8H_EEPROM_ENABLE_READ
stc8h_status_t stc8h_eeprom_read(stc8h_u16 addr, STC8H_DATA stc8h_u8 *data, stc8h_u16 len);
#endif
#if STC8H_EEPROM_ENABLE_WRITE
stc8h_status_t stc8h_eeprom_write(stc8h_u16 addr, const STC8H_DATA stc8h_u8 *data, stc8h_u16 len);
#endif
#if STC8H_EEPROM_ENABLE_ERASE
stc8h_status_t stc8h_eeprom_erase_sector(stc8h_u16 addr);
#endif
#if STC8H_EEPROM_ENABLE_FIXED_BLOCK
stc8h_status_t stc8h_eeprom_read_fixed(STC8H_DATA stc8h_u8 *data);
stc8h_status_t stc8h_eeprom_save_fixed(const STC8H_DATA stc8h_u8 *data);
#endif

#endif
