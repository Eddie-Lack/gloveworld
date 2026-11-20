#ifndef CAMEO_H
#define CAMEO_H

#include "types.h"

typedef struct CameoItem {
    /* 0x00 */ s16 type;
    /* 0x02 */ s16 time;
    /* 0x04 */ s16 duration;
    /* 0x06 */ s16 trigger;
    /* 0x08 */ s16 done;
    /* 0x0A */ s16 target;
    union {
        /* 0x0C */ Vec3 pos;
        /* 0x0C */ s16 args[2];
    } u;
    /* 0x18 */ f32 speed;
} CameoItem; /* size = 0x1C */

typedef struct Cameo {
    /* 0x00 */ struct Cameo *next;
    /* 0x04 */ struct Cameo *prev;
    /* 0x08 */ s32 active;
    /* 0x0C */ s32 count;
    /* 0x10 */ struct CameoItem *items[0x14];
} Cameo; /* size = 0x60 */

typedef struct CameoList {
    /* 0x00 */ Cameo head;
    /* 0x60 */ s32 count;
} CameoList;

extern CameoList cameoList;

void InitCameoList(void);
void AddCameo(Cameo *cameo);
void StartCameo(s32 num);
void UpdateCameos(void);
void UpdateCameo(Cameo *cameo);
void ProcessCameoInstruction(Cameo *cameo, CameoItem *instruction);

#endif
