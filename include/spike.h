#ifndef SPIKE_H
#define SPIKE_H

#include "types.h"
#include "platform.h"

typedef struct Spike {
    /* 0x00 */ struct Spike *next;
    /* 0x04 */ struct Spike *prev;
    /* 0x08 */ Platform *platform;
} Spike;

typedef struct SpikeList {
    /* 0x00 */ s32 count;
    /* 0x04 */ Spike head;
} SpikeList;

extern SpikeList spikeList;

void UpdateSpikes(void);
void InitSpikeList(void);
void AddSpike(Spike *spike);

#endif
