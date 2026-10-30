#ifndef BLUR_H
#define BLUR_H

#include "common.h"

typedef struct BlurPoint {
    /* 0x00 */ Vec3 p1;
    /* 0x0C */ Vec3 p2;
    /* 0x18 */ u8 alpha1;
    /* 0x19 */ u8 alpha2;
    /* 0x1A */ u8 unk1A[0x2];
} BlurPoint; /* size = 0x1C */

typedef struct Blur {
    /* 0x000 */ struct Blur *next;
    /* 0x004 */ struct Blur *prev;
    /* 0x008 */ BlurPoint points[15];
    /* 0x1AC */ u8 unk1AC[0x4];
    /* 0x1B0 */ Vtx vtx[2][30];
    /* 0x570 */ u8 buffer;
    /* 0x571 */ u8 alpha1;
    /* 0x572 */ u8 alpha2;
    /* 0x573 */ u8 fade1;
    /* 0x574 */ u8 fade2;
    /* 0x575 */ u8 index;
    /* 0x576 */ u8 oldest;
    /* 0x577 */ u8 idleCount;
} Blur; /* size = 0x578 */

typedef struct PlatformBlur {
    /* 0x000 */ Blur blur;
    /* 0x578 */ f32 innerScale;
    /* 0x57C */ u8 unk57C[0x4];
} PlatformBlur; /* size = 0x580 */

typedef struct BlurList {
    /* 0x000 */ Blur head;
    /* 0x578 */ s32 count;
} BlurList;

extern BlurList blurList;

void StartBlurFX(Blur *blur, Vec3 *p1, Vec3 *p2, u8 alpha1, u8 alpha2, u8 fade1, u8 fade2);
void IncrementBlurFX(Blur *blur, Vec3 *point1, Vec3 *point2);
void DrawBlurFX(Blur *blur);
void InitBlurList(void);
void AddBlur(Blur *blur);
void SubBlur(Blur *blur);
void DrawBlurs(void);
void UpdateBlurs(void);

#endif
