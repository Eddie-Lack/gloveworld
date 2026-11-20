#ifndef AMBIENT_H
#define AMBIENT_H

#include "types.h"
#include "sound.h"

typedef struct AmbientSound {
    /* 0x00 */ struct AmbientSound *next;
    /* 0x04 */ struct AmbientSound *prev;
    /* 0x08 */ ContinuousSample sample;
    /* 0x24 */ Vec3 offset;
    /* 0x30 */ Vec3 pos;
    /* 0x3C */ struct Platform *platform;
    /* 0x40 */ s16 minDelay;
    /* 0x42 */ s16 randDelay;
    /* 0x44 */ s16 timer;
    /* 0x46 */ s16 duration;
    /* 0x48 */ s16 volume;
    /* 0x4A */ s16 tag;
} AmbientSound; /* size = 0x4C */

typedef struct AmbientSoundList {
    /* 0x00 */ AmbientSound head;
    /* 0x4C */ s32 count;
} AmbientSoundList;

extern AmbientSoundList ambientSoundList;

void SubAmbientSound(AmbientSound *ambientSound);
void InitAmbientSoundList(void);
AmbientSound *FindTaggedAmbientSound(AmbientSound *start, s32 tag);

#endif
