#include "util_crc.h"

stc8h_u8 util_checksum8(const STC8H_DATA stc8h_u8 *data, stc8h_u16 len)
{
    stc8h_u8 sum;

    sum = 0u;
    while (len != 0u) {
        sum = (stc8h_u8)(sum + *data);
        ++data;
        --len;
    }

    return sum;
}

stc8h_u16 util_crc16_modbus(const STC8H_DATA stc8h_u8 *data, stc8h_u16 len)
{
    stc8h_u16 crc;
    stc8h_u8 i;

    crc = 0xFFFFu;
    while (len != 0u) {
        crc ^= *data;
        ++data;
        --len;

        for (i = 0u; i < 8u; ++i) {
            if ((crc & 0x0001u) != 0u) {
                crc = (stc8h_u16)((crc >> 1) ^ 0xA001u);
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}
