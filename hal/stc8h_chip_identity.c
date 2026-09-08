#include "stc8h_chip_identity.h"

#include "stc8h_sfr.h"

stc8h_u8 stc8h_chip_identity_is_valid(
    const stc8h_u8 chip_id[STC8H_CHIP_ID_SIZE])
{
    stc8h_u8 index;
    stc8h_u8 all_zero;
    stc8h_u8 all_ff;

    if ((chip_id == 0) || (chip_id[0] != 0xF7u) ||
        (chip_id[1] != 0x84u)) {
        return 0u;
    }
    all_zero = 1u;
    all_ff = 1u;
    for (index = 0u; index < STC8H_CHIP_ID_SIZE; ++index) {
        if (chip_id[index] != 0u) {
            all_zero = 0u;
        }
        if (chip_id[index] != 0xFFu) {
            all_ff = 0u;
        }
    }
    return (stc8h_u8)((all_zero == 0u) && (all_ff == 0u));
}

stc8h_status_t stc8h_chip_identity_read(
    stc8h_u8 chip_id[STC8H_CHIP_ID_SIZE])
{
    stc8h_u8 index;
    stc8h_u8 saved_p_sw2;

    if (chip_id == 0) {
        return STC8H_ERROR;
    }
    saved_p_sw2 = P_SW2;
    P_SW2 |= 0x80u;
    for (index = 0u; index < STC8H_CHIP_ID_SIZE; ++index) {
        chip_id[index] = STC8H_SFRX(STC8H_CHIP_ID_BASE + index);
    }
    P_SW2 = saved_p_sw2;
    return stc8h_chip_identity_is_valid(chip_id) != 0u ?
           STC8H_OK : STC8H_ERROR;
}

void stc8h_chip_identity_expand_uid(
    const stc8h_u8 chip_id[STC8H_CHIP_ID_SIZE],
    stc8h_u8 uid[STC8H_DEVICE_UID_SIZE])
{
    stc8h_u8 index;

    if ((chip_id == 0) || (uid == 0)) {
        return;
    }
    uid[0] = 0x53u;
    uid[1] = 0x54u;
    uid[2] = 0x43u;
    uid[3] = 0x38u;
    uid[4] = STC8H_DEVICE_UID_MAPPING_VERSION;
    uid[5] = STC8H_CHIP_ID_SIZE;
    uid[6] = 0u;
    uid[7] = 0u;
    uid[8] = 0u;
    for (index = 0u; index < STC8H_CHIP_ID_SIZE; ++index) {
        uid[9u + index] = chip_id[index];
    }
}

stc8h_status_t stc8h_chip_identity_read_uid(
    stc8h_u8 uid[STC8H_DEVICE_UID_SIZE])
{
    stc8h_u8 chip_id[STC8H_CHIP_ID_SIZE];

    if ((uid == 0) || (stc8h_chip_identity_read(chip_id) != STC8H_OK)) {
        return STC8H_ERROR;
    }
    stc8h_chip_identity_expand_uid(chip_id, uid);
    return STC8H_OK;
}
