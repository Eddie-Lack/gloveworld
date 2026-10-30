#ifndef PERSONAL_H
#define PERSONAL_H

#include "common.h"
#include "actor.h"

void ProcessPersonalLoop(void);
void ProcessInputPersonal(void);
void updateLava(u8 *buf, u32 width, u32 height);
void AdjustColours(u8 *r, u8 *g, u8 *b);

#endif
