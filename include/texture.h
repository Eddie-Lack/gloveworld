#ifndef TEXTURE_H
#define TEXTURE_H

#include "common.h"

typedef struct Texture {
    /* 0x00 */ u32 crc;
    /* 0x04 */ u8 cycleStart;
    /* 0x05 */ u8 cycleEnd;
    /* 0x06 */ s16 flags;
    /* 0x08 */ s16 cycleSpeed;
    /* 0x0A */ s16 cycleCount;
    /* 0x0C */ u16 width;
    /* 0x0E */ u16 height;
    /* 0x10 */ u16 maskS;
    /* 0x12 */ u16 maskT;
    /* 0x14 */ s32 size;
    /* 0x18 */ u16 fmt;
    /* 0x1A */ u16 siz;
    /* 0x1C */ u8 *data;
    /* 0x20 */ s32 *palette;
} Texture;

void LoadTexture(Texture *tex);

#endif
