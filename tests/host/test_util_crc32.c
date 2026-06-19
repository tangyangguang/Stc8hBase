#include <stdio.h>

#include "../../utils/util_crc32.c"

static int expect_crc(const char *label, stc8h_u32 got, stc8h_u32 expected)
{
    if (got != expected) {
        printf("%s got 0x%08lx expected 0x%08lx\n",
               label,
               (unsigned long)got,
               (unsigned long)expected);
        return 1;
    }
    return 0;
}

int main(void)
{
    static const stc8h_u8 digits[] = "123456789";
    static const stc8h_u8 first[] = "1234";
    static const stc8h_u8 second[] = "56789";
    static const stc8h_u8 bytes[] = {0x00u, 0x01u, 0x02u, 0x03u, 0x04u};
    stc8h_u32 crc;
    int fail;

    fail = 0;
    fail |= expect_crc("empty", util_crc32_ieee(digits, 0u), 0x00000000UL);
    fail |= expect_crc("digits", util_crc32_ieee(digits, 9u), 0xCBF43926UL);

    crc = util_crc32_ieee_update(0UL, first, 4u);
    crc = util_crc32_ieee_update(crc, second, 5u);
    fail |= expect_crc("split digits", crc, 0xCBF43926UL);

    fail |= expect_crc("bytes", util_crc32_ieee(bytes, 5u), 0x515AD3CCUL);

    return fail;
}
