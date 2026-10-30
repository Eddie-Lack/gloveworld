#ifndef BOSS_H
#define BOSS_H

#include "common.h"
#include "actor.h"

void DoBoss(void);
void DoPiratesBoss(void);
void DoCarnivalBoss(void);
void DoFortressBoss(void);
void SetPlatformPoint(s16 num, s16 point);
void SetPlatPoint(Platform *plat, s32 point);
void DoPiratesBonus(void);
void DoWayRoom(void);
void DoPrehistBoss(void);
void DoSpaceBoss3(void);
void DoFlythru2Screen(void);
void SetupFlythru2Screen(void);

#endif
