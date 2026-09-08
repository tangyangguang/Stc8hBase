#include "stc8h_boot_stub.h"

#define BOOT_ENTRY 0x0200u
#define LJMP_HI(addr) ((stc8h_u8)((addr) >> 8))
#define LJMP_LO(addr) ((stc8h_u8)(addr))
#define APP_VECTOR(offset) ((stc8h_u16)(STC8H_BOOT_APP_BASE + (offset)))
#define DEFINE_VECTOR(name, offset) \
    __code __at (offset) const stc8h_u8 name[3] = { \
        0x02u, LJMP_HI(APP_VECTOR(offset)), LJMP_LO(APP_VECTOR(offset)) \
    }

__code __at (0x0000) const stc8h_u8 h8k64u_ota_reset_vector[3] = {
    0x02u, LJMP_HI(BOOT_ENTRY), LJMP_LO(BOOT_ENTRY)
};

/* SDCC relocates application interrupt vectors by --code-loc. The protected
 * low table forwards all H8K interrupt slots 0..44 to the same offsets in the
 * application image. The bootloader itself runs with interrupts disabled. */
DEFINE_VECTOR(h8k64u_ota_vector_0,  0x0003);
DEFINE_VECTOR(h8k64u_ota_vector_1,  0x000B);
DEFINE_VECTOR(h8k64u_ota_vector_2,  0x0013);
DEFINE_VECTOR(h8k64u_ota_vector_3,  0x001B);
DEFINE_VECTOR(h8k64u_ota_vector_4,  0x0023);
DEFINE_VECTOR(h8k64u_ota_vector_5,  0x002B);
DEFINE_VECTOR(h8k64u_ota_vector_6,  0x0033);
DEFINE_VECTOR(h8k64u_ota_vector_7,  0x003B);
DEFINE_VECTOR(h8k64u_ota_vector_8,  0x0043);
DEFINE_VECTOR(h8k64u_ota_vector_9,  0x004B);
DEFINE_VECTOR(h8k64u_ota_vector_10, 0x0053);
DEFINE_VECTOR(h8k64u_ota_vector_11, 0x005B);
DEFINE_VECTOR(h8k64u_ota_vector_12, 0x0063);
DEFINE_VECTOR(h8k64u_ota_vector_13, 0x006B);
DEFINE_VECTOR(h8k64u_ota_vector_14, 0x0073);
DEFINE_VECTOR(h8k64u_ota_vector_15, 0x007B);
DEFINE_VECTOR(h8k64u_ota_vector_16, 0x0083);
DEFINE_VECTOR(h8k64u_ota_vector_17, 0x008B);
DEFINE_VECTOR(h8k64u_ota_vector_18, 0x0093);
DEFINE_VECTOR(h8k64u_ota_vector_19, 0x009B);
DEFINE_VECTOR(h8k64u_ota_vector_20, 0x00A3);
DEFINE_VECTOR(h8k64u_ota_vector_21, 0x00AB);
DEFINE_VECTOR(h8k64u_ota_vector_22, 0x00B3);
DEFINE_VECTOR(h8k64u_ota_vector_23, 0x00BB);
DEFINE_VECTOR(h8k64u_ota_vector_24, 0x00C3);
DEFINE_VECTOR(h8k64u_ota_vector_25, 0x00CB);
DEFINE_VECTOR(h8k64u_ota_vector_26, 0x00D3);
DEFINE_VECTOR(h8k64u_ota_vector_27, 0x00DB);
DEFINE_VECTOR(h8k64u_ota_vector_28, 0x00E3);
DEFINE_VECTOR(h8k64u_ota_vector_29, 0x00EB);
DEFINE_VECTOR(h8k64u_ota_vector_30, 0x00F3);
DEFINE_VECTOR(h8k64u_ota_vector_31, 0x00FB);
DEFINE_VECTOR(h8k64u_ota_vector_32, 0x0103);
DEFINE_VECTOR(h8k64u_ota_vector_33, 0x010B);
DEFINE_VECTOR(h8k64u_ota_vector_34, 0x0113);
DEFINE_VECTOR(h8k64u_ota_vector_35, 0x011B);
DEFINE_VECTOR(h8k64u_ota_vector_36, 0x0123);
DEFINE_VECTOR(h8k64u_ota_vector_37, 0x012B);
DEFINE_VECTOR(h8k64u_ota_vector_38, 0x0133);
DEFINE_VECTOR(h8k64u_ota_vector_39, 0x013B);
DEFINE_VECTOR(h8k64u_ota_vector_40, 0x0143);
DEFINE_VECTOR(h8k64u_ota_vector_41, 0x014B);
DEFINE_VECTOR(h8k64u_ota_vector_42, 0x0153);
DEFINE_VECTOR(h8k64u_ota_vector_43, 0x015B);
DEFINE_VECTOR(h8k64u_ota_vector_44, 0x0163);


