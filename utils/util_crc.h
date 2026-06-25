#ifndef UTIL_CRC_H
#define UTIL_CRC_H

#include "stc8h_config.h"

#ifndef UTIL_CRC16_MODBUS_ENABLE_XDATA
#define UTIL_CRC16_MODBUS_ENABLE_XDATA 0
#endif

stc8h_u8 util_checksum8(const STC8H_DATA stc8h_u8 *data, stc8h_u16 len);
stc8h_u16 util_crc16_modbus(const STC8H_DATA stc8h_u8 *data, stc8h_u16 len);
#if UTIL_CRC16_MODBUS_ENABLE_XDATA
stc8h_u16 util_crc16_modbus_xdata(const STC8H_XDATA stc8h_u8 *data, stc8h_u16 len);
#endif

#endif
