#ifndef TIP_H
#define TIP_H

#include "types.h"
#include "overlay.h"

typedef struct Tip {
    /* 0x00 */ struct Tip *next;
    /* 0x04 */ struct Tip *prev;
    /* 0x08 */ Vec3 pos;
    /* 0x14 */ u8 state;
    /* 0x15 */ u8 read;
    /* 0x16 */ s16 animTime;
    /* 0x18 */ s16 tag;
    /* 0x1A */ u8 unk1A[0x6];
    /* 0x20 */ f32 altitude;
    /* 0x24 */ Vec3 *normal;
    /* 0x28 */ u8 unk28[0x4];
    /* 0x2C */ u8 shadowAlpha;
    /* 0x2D */ u8 shadowTarget;
    /* 0x2E */ u8 unk2E[0x4A];
    /* 0x78 */ GameSprite sprite;
    /* 0xB0 */ u16 timer;
    /* 0xB2 */ u8 unkB2[0x6];
} Tip; /* size = 0xB8 */

typedef struct TipList {
    /* 0x00 */ Tip head;
    /* 0xB8 */ s32 count;
} TipList;

extern TipList tipList;

void InitTipList(void);
void AddTip(Tip *tip);
void UpdateTips(void);
void UpdateTip(Tip *tip);

#endif
