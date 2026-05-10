#ifndef STC8H_EEPROM_H
#define STC8H_EEPROM_H

#include "stc8h_config.h"

#define STC8H_EEPROM_SIZE 4096u
#define STC8H_EEPROM_SECTOR_SIZE 512u

stc8h_status_t stc8h_eeprom_read(stc8h_u16 addr, STC8H_DATA stc8h_u8 *data, stc8h_u16 len);
stc8h_status_t stc8h_eeprom_write(stc8h_u16 addr, const STC8H_DATA stc8h_u8 *data, stc8h_u16 len);
stc8h_status_t stc8h_eeprom_erase_sector(stc8h_u16 addr);

#endif
