#ifndef VENT_H
#define VENT_H

#include "types.h"
#include "platform.h"
#include "sound.h"

typedef struct Vent {
    /* 0x00 */ struct Vent *next;
    /* 0x04 */ struct Vent *prev;
    /* 0x08 */ s16 type;
    /* 0x0A */ s16 tag;
    /* 0x0C */ u8 bulletType;
    /* 0x0D */ u8 active;
    /* 0x0E */ u8 on;
    /* 0x0F */ u8 numCycles;
    /* 0x10 */ s16 framesOff[6];
    /* 0x1C */ s16 framesOn[6];
    /* 0x28 */ s16 timer;
    /* 0x2A */ s16 cycle;
    /* 0x2C */ Vec3 velocity;
    /* 0x38 */ Vec3 origin;
    /* 0x44 */ Platform *parent;
    /* 0x48 */ ContinuousSample sound;
    /* 0x64 */ Vec3 soundPos;
} Vent; /* size = 0x70 */

typedef struct VentList {
    /* 0x00 */ Vent head;
    /* 0x70 */ s32 count;
} VentList;

extern VentList ventList;

void InitVentList(void);
void AddVent(Vent *vent);
void SubVent(Vent *vent);
void UpdateVents(void);
void UpdateVent(Vent *vent);
Vent *FindTaggedVent(Vent *startVent, s32 tag);

#endif
