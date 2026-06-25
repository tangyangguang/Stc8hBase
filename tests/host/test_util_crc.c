#include <stdio.h>

#define UTIL_CRC16_MODBUS_ENABLE_XDATA 1
#include "../../utils/util_crc.c"

static int expect_crc(const char *label, stc8h_u16 got, stc8h_u16 expected)
{
    if (got != expected) {
        printf("%s got 0x%04x expected 0x%04x\n",
               label,
               (unsigned int)got,
               (unsigned int)expected);
        return 1;
    }
    return 0;
}

int main(void)
{
    static const STC8H_DATA stc8h_u8 digits_data[] = "123456789";
    static const STC8H_XDATA stc8h_u8 digits_xdata[] = "123456789";
    int fail;

    fail = 0;
    fail |= expect_crc("data empty", util_crc16_modbus(digits_data, 0u), 0xFFFFu);
    fail |= expect_crc("data digits", util_crc16_modbus(digits_data, 9u), 0x4B37u);
    fail |= expect_crc("xdata empty", util_crc16_modbus_xdata(digits_xdata, 0u), 0xFFFFu);
    fail |= expect_crc("xdata digits", util_crc16_modbus_xdata(digits_xdata, 9u), 0x4B37u);

    return fail;
}
