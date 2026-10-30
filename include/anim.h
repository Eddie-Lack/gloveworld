#ifndef ANIM_H
#define ANIM_H

#include "common.h"
#include "actor.h"

void InitActorAnim(Actor *tempActor);
void SetAnimationSpeed(Actor *myActor, f32 speed);
void FlushQueue(Actor *myActor);
void UpdateAnimations(Actor *actor);
s32 QueryAnimTime(Actor *actor, f32 time);

#endif
