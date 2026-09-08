#ifndef STC8H_CHIP_IDENTITY_H
#define STC8H_CHIP_IDENTITY_H

#include "stc8h_types.h"

#define STC8H_CHIP_ID_BASE 0xFDE0u
#define STC8H_CHIP_ID_SIZE 7u
#define STC8H_DEVICE_UID_SIZE 16u
#define STC8H_DEVICE_UID_MAPPING_VERSION 1u

stc8h_status_t stc8h_chip_identity_read(
    stc8h_u8 chip_id[STC8H_CHIP_ID_SIZE]);
stc8h_u8 stc8h_chip_identity_is_valid(
    const stc8h_u8 chip_id[STC8H_CHIP_ID_SIZE]);
void stc8h_chip_identity_expand_uid(
    const stc8h_u8 chip_id[STC8H_CHIP_ID_SIZE],
    stc8h_u8 uid[STC8H_DEVICE_UID_SIZE]);
stc8h_status_t stc8h_chip_identity_read_uid(
    stc8h_u8 uid[STC8H_DEVICE_UID_SIZE]);

#endif
