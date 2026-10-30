#ifndef FOOTPRINT_H
#define FOOTPRINT_H

#include "types.h"
#include "platform.h"
#include "texture.h"

typedef struct Footprint {
    /* 0x00 */ struct Footprint *next;
    /* 0x04 */ struct Footprint *prev;
    /* 0x08 */ Texture *texture;
    /* 0x0C */ Platform *parent;
    /* 0x10 */ Vec3 *normalPtr;
    /* 0x14 */ Vec3 pos;
    /* 0x20 */ Vec3 normal;
    /* 0x2C */ u8 r;
    /* 0x2D */ u8 g;
    /* 0x2E */ u8 b;
    /* 0x2F */ u8 alpha;
    /* 0x30 */ u8 maxAlpha;
    /* 0x31 */ u8 flags;
    /* 0x34 */ f32 scaleSpeed;
    /* 0x38 */ f32 width;
    /* 0x3C */ f32 length;
    /* 0x40 */ s16 life;
    /* 0x42 */ s16 age;
    /* 0x44 */ u8 unk44[0x4];
    /* 0x48 */ Vtx vtx[4];
    /* 0x88 */ s16 texScale;
    /* 0x8A */ u8 alphaStep;
    /* 0x8B */ u8 timer;
    /* 0x8C */ u8 unk8C[0x4];
} Footprint; /* size = 0x90 */

typedef struct FootprintList {
    /* 0x00 */ Footprint head;
    /* 0x90 */ s32 count;
} FootprintList;

extern FootprintList footprintList;

s32 SetupFootprint(Footprint *footprint, Vec3 *pos, Vec3 *plane, f32 size, f32 lifetime, Texture *texture, s32 flags, s32 texScale);
void AddFootprint(Footprint *footprint);

void InitFootprintList(void);
void RemoveFootprint(Footprint *ptr);
void UpdateFootprints(void);
void DrawFootprints(void);
void KillFootprint(Footprint *ptr);
void DrawFootprint(Footprint *ptr);

#endif
