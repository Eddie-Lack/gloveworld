#include "common.h"
#include "ident.h"
#include "decompress.h"

extern u8 *decompOut;
extern s32 decompSize;
extern u8 decompWindow[0x1000];
extern s16 decompWindowPos;

void memset(u8 *dst, u8 c, s32 n) {
    s32 i;

    for (i = 0; i < n; i++) {
        dst[i] = c;
    }
}

void DecompressWriteByte(u8 c) {
    *decompOut++ = c;
    decompSize++;
    decompWindow[decompWindowPos] = c;
    decompWindowPos = (decompWindowPos + 1) & 0xFFF;
}

s32 DecompressBuffer(u8 *src, u8 *dst) {
    s16 flags = 0;
    s16 count = 0;
    s16 offset = 0;
    s16 i = 0;

    decompOut = dst;
    decompSize = 0;
    decompWindowPos = 0;
    memset(decompWindow, 0, 0x1000);
    while (1) {
        flags = *src++;
        for (i = 0; i != 8; i++) {
            if (flags & 0x80) {
                if ((count = *src++) == 0) {
                    return decompSize;
                } else {
                    offset = 0x1000 - ((count & 0xF0) * 16 + *src++);
                    count &= 0xF;
                    count += 2;
                    while (count != 0) {
                        DecompressWriteByte(decompWindow[(decompWindowPos + offset) & 0xFFF]);
                        count--;
                    }
                }
            } else {
                DecompressWriteByte(*src++);
            }
            flags += flags;
        }
    }
    return decompSize;
}

INCLUDE_ASM("asm/nonmatchings/decompress", pad_8012C5D8);

INCLUDE_RODATA("asm/nonmatchings/decompress", pad_801019C4);
