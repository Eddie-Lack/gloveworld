#include "common.h"
#include "ident.h"
#include "actor.h"
#include "macros.h"
#include "anim.h"

INCLUDE_ASM("asm/nonmatchings/anim", pad_801338F8);

void InitActorAnim(Actor *tempActor) {
    *tempActor->anim = *tempActor->inst->anim;
    tempActor->anim->current = -1;
    tempActor->anim->queue[0] = -1;
    tempActor->anim->queue[1] = -1;
    tempActor->anim->queue[2] = -1;
    tempActor->anim->queue[3] = -1;
    tempActor->anim->queue[4] = -1;
    tempActor->anim->loop = 0;
    tempActor->anim->queueLen = 0;
    tempActor->anim->time = 0.0f;
    tempActor->anim->finishedAnim = 0;
}

void AnimateActor(Actor *actor, s32 animNum, u8 loop, u8 queue, f32 speed) {
    AnimState *state = actor->anim;
    AnimInfo *info;
    f32 actualSpeed;

    if (state == NULL || state->anims == NULL) {
        return;
    }
    if (state->current == animNum && !queue && !state->finishedAnim) {
        if ((speed == 0.0f && state->speed == state->anims[animNum].speed) || speed == state->speed) {
            state->loop = loop;
            return;
        }
    }
    if (animNum > state->numAnims) {
        animNum = 0;
    }
    if (speed == 0.0f) {
        actualSpeed = actor->anim->anims[animNum].speed;
    } else {
        actualSpeed = speed;
    }
    if (!queue) {
        state->current = animNum;
        state->loop = loop;
        state->speed = actualSpeed;
        state->finishedAnim = 0;
        info = &state->anims[state->current];
        state->time = info->start;
        FlushQueue(actor);
        if (actualSpeed < 0.0f) {
            state->time = info->end;
        }
    } else if (state->current == -1) {
        state->current = animNum;
        state->loop = loop;
        state->speed = actualSpeed;
    } else if (state->queueLen != 5) {
        state->queue[state->queueLen] = animNum;
        state->queueLoop[state->queueLen] = loop;
        state->queueSpeed[state->queueLen] = actualSpeed;
        state->queueLen++;
    }
}

void SetAnimationSpeed(Actor *myActor, f32 speed) {
    myActor->anim->speed = speed;
}

void FlushQueue(Actor *myActor) {
    s32 i;

    myActor->anim->queueLen = 0;
    for (i = 0; i < 5; i++) {
        myActor->anim->queue[i] = -1;
    }
}

void UpdateAnimations(Actor *actor) {
    AnimState *anim = actor->anim;
    AnimInfo *info;
    s32 i;

    if (anim == NULL || anim->anims == NULL) {
        return;
    }
    info = &anim->anims[anim->current];
    if (anim->current < 0) {
        return;
    }
    anim->finishedAnim = 0;
    anim->time += anim->speed;
    if ((anim->time > info->end || anim->time < info->start) && !anim->loop) {
        anim->time = MIN(MAX(anim->time, info->start), info->end);
        anim->finishedAnim = anim->current + 1;
        if (anim->queue[0] != -1) {
            anim->current = anim->queue[0];
            anim->loop = anim->queueLoop[0];
            anim->speed = anim->queueSpeed[0];
            if (anim->speed < 0.0f) {
                anim->time = anim->anims[anim->current].end;
            } else {
                anim->time = anim->anims[anim->current].start;
            }
            for (i = 0; i < 4; i++) {
                anim->queue[i] = anim->queue[i + 1];
                anim->queueLoop[i] = anim->queueLoop[i + 1];
                anim->queueSpeed[i] = anim->queueSpeed[i + 1];
            }
            anim->queue[4] = -1;
            anim->queueLoop[4] = -1;
            anim->queueSpeed[4] = -1.0f;
            if (anim->queueLen > 0) {
                anim->queueLen--;
            }
        }
    } else if (anim->time > info->end) {
        anim->time = info->start;
    } else if (anim->time < info->start) {
        anim->time = info->end;
    }
}

s32 QueryAnimTime(Actor *actor, f32 time) {
    if (time <= actor->anim->time && actor->anim->time < actor->anim->speed + time) {
        return TRUE;
    }
    return FALSE;
}

INCLUDE_RODATA("asm/nonmatchings/anim", pad_80105938);
