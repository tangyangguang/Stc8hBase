#ifndef UTIL_CRC32_H
#define UTIL_CRC32_H

#include "stc8h_config.h"

stc8h_u32 util_crc32_ieee(const stc8h_u8 *data, stc8h_u16 len);

/*
 * Incremental CRC32/IEEE update.
 * The input crc is the finalized value returned by the previous call.
 */
stc8h_u32 util_crc32_ieee_update(stc8h_u32 finalized_crc, const stc8h_u8 *data, stc8h_u16 len);

#endif
