#ifndef WIND_H
#define WIND_H

#include "types.h"

typedef struct Wind {
    /* 0x00 */ f32 pos[3];
    /* 0x0C */ f32 size[3];
    /* 0x18 */ f32 dir[3];
    /* 0x24 */ s32 type;
    /* 0x28 */ f32 strength;
    /* 0x2C */ f32 radius;
    /* 0x30 */ s32 tag;
    /* 0x34 */ struct Wind *next;
    /* 0x38 */ struct Wind *prev;
} Wind; /* size = 0x3C */

typedef struct WindList {
    /* 0x00 */ Wind head;
    /* 0x3C */ s32 count;
} WindList;

extern WindList windList;

void InitWindList(void);
void AddWind(Wind *wind);
void UpdateWinds(void);
Wind * FindTaggedWind(Wind *startWind, s32 tag);

#endif
