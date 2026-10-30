#include "common.h"
#include "crc.h"

extern u32 CRCtable[256];

INCLUDE_ASM("asm/nonmatchings/crc", pad_80139C34);

void InitCRCTable(void) {
    s32 i;
    s32 j;
    u32 CRCaccum;

    for (i = 0; i < 256; i++) {
        CRCaccum = i << 24;
        for (j = 7; j >= 0; j--) {
            if (CRCaccum & 0x80000000) {
                CRCaccum = (CRCaccum << 1) ^ 0x04C11DB7;
            } else {
                CRCaccum <<= 1;
            }
        }
        CRCtable[i] = CRCaccum;
    }
}

u32 UpdateCRC(u8 *ptr) {
    s32 i;
    s32 size;
    u32 CRCaccum;

    size = strlen(ptr);
    CRCaccum = 0;
    for (i = 0; i < size; i++) {
        CRCaccum = (CRCaccum << 8) ^ CRCtable[(CRCaccum >> 24) ^ *ptr++];
    }
    return CRCaccum;
}

u32 UpdateCRCData(u8 *data, s32 len) {
    s32 i;
    u32 crc;

    crc = 0;
    for (i = 0; i < len; i++) {
        crc = (crc << 8) ^ CRCtable[(crc >> 24) ^ *data++];
    }
    return crc;
}

INCLUDE_ASM("asm/nonmatchings/crc", pad_80139D6C);
