#ifndef SPELL_H
#define SPELL_H

#include "common.h"
#include "actor.h"

void ActivateSpellHand(s16 spell);
void DeactivateSpell(Actor *actor, s16 spell);
void DeactivateSpellHand(s16 spell);
void RemoveHandAndBallBullets(void);
void AddSpellDebris(s16 debrisType, Vec3 *pos, Vec3 *vel, s32 spell);
void AddBulletFlash(Vec3 *pos);

#endif
