#include <stdio.h>
#include <string.h>

#include "../../hal/stc8h_chip_identity.c"

volatile unsigned char P_SW2;

static int require(int condition, const char *message)
{
    if (!condition) {
        printf("%s\n", message);
        return 1;
    }
    return 0;
}

int main(void)
{
    const stc8h_u8 chip_id[STC8H_CHIP_ID_SIZE] = {
        0xF7u, 0x84u, 0xC9u, 0x75u, 0x01u, 0xB5u, 0xEEu
    };
    const stc8h_u8 expected[STC8H_DEVICE_UID_SIZE] = {
        0x53u, 0x54u, 0x43u, 0x38u, 0x01u, 0x07u, 0u, 0u, 0u,
        0xF7u, 0x84u, 0xC9u, 0x75u, 0x01u, 0xB5u, 0xEEu
    };
    stc8h_u8 uid[STC8H_DEVICE_UID_SIZE];
    stc8h_u8 invalid[STC8H_CHIP_ID_SIZE];
    int failures;

    failures = 0;
    memset(invalid, 0xFF, sizeof(invalid));
    failures += require(stc8h_chip_identity_is_valid(chip_id) != 0u,
                        "known H8K64U CHIPID must validate");
    failures += require(stc8h_chip_identity_is_valid(invalid) == 0u,
                        "erased CHIPID must fail");
    stc8h_chip_identity_expand_uid(chip_id, uid);
    failures += require(memcmp(uid, expected, sizeof(uid)) == 0,
                        "CHIPID must expand to canonical 16-byte UID");
    return failures == 0 ? 0 : 1;
}
