#ifndef WATER_H
#define WATER_H

#include "types.h"

typedef struct Water {
    /* 0x00 */ struct Water *next;
    /* 0x04 */ struct Water *prev;
    /* 0x08 */ struct Actor *actor;
    /* 0x0C */ Box box;
    /* 0x24 */ f32 currentX;
    /* 0x28 */ f32 currentY;
    /* 0x2C */ f32 currentZ;
    /* 0x30 */ f32 height;
    /* 0x34 */ f32 targetHeight;
    /* 0x38 */ s16 tag;
    /* 0x3A */ u8 unk3A[0x2];
} Water; /* size = 0x3C */

typedef struct WaterList {
    /* 0x00 */ Water head;
    /* 0x3C */ s32 count;
} WaterList;

extern WaterList waterList;

Water *FindTaggedWater(s32 tagNum);

void InitWaterList(void);
void AddWater(Water *water);
void DeformTextures(void);
void UpdateWater(void);
void CheckCameraForWater(void);

#endif
