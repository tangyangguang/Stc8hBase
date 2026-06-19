#include "util_crc32.h"

#define UTIL_CRC32_MASK 0xFFFFFFFFUL

stc8h_u32 util_crc32_ieee_update(stc8h_u32 finalized_crc, const stc8h_u8 *data, stc8h_u16 len)
{
    stc8h_u32 crc;
    stc8h_u16 i;
    stc8h_u8 bit;

    crc = (~finalized_crc) & UTIL_CRC32_MASK;
    for (i = 0u; i < len; ++i) {
        crc ^= (stc8h_u32)data[i];
        for (bit = 0u; bit < 8u; ++bit) {
            if ((crc & 1UL) != 0UL) {
                crc = (crc >> 1) ^ 0xEDB88320UL;
            } else {
                crc >>= 1;
            }
        }
    }

    return (~crc) & UTIL_CRC32_MASK;
}

#if UTIL_CRC32_ENABLE_ONESHOT
stc8h_u32 util_crc32_ieee(const stc8h_u8 *data, stc8h_u16 len)
{
    return util_crc32_ieee_update(0UL, data, len);
}
#endif
