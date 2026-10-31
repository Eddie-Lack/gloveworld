#include "common.h"
#include "ident.h"
#include "actor.h"
#include "maths.h"
#include "jalloc.h"
#include "debug.h"
#include "ball.h"
#include "hand.h"
#include "overlay.h"
#include "texture.h"
#include "macros.h"
#include "platform.h"
#include "camera.h"
#include "debris.h"
#include "controller.h"
#include "game.h"
#include "banks.h"
#include "collect.h"
#include "sound.h"
#include "collision.h"
#include "enemy.h"

extern Quat vertQ;
extern Quat zeroQuat;
extern Vec3 zero;
extern f32 gravity;
extern u8 inertia;
extern s8 rotOrder[3];

extern char *nmeObjectName[];
u32 UpdateCRC(char *ptr);
void NMEGetNewInstruction(Enemy *enemy, s32 instructionNum, s32 overrideConditions);
void UpdateActorPlatform(Vec3 *actorPos, Vec3 *actorActualOldpos, Vec3 *actorVel, Vec3 *actorRot, Vec3 *actorRotaim, Quat *actorQRot, s8 raimy, s32 flags,
                         Platform *actorPlatform, Platform *actorOldPlatform);
extern s32 gravMode;
extern u8 ActiveController;
extern Vec3 globalRot[];
extern f32 frontprobey;
void DisableControl(s16 time);
void SquashActor(Actor *actor, Vec3 *dir, f32 power, f32 maxSquash);

inline void GetQuaternionFromXYZRot(Quat *quat, Vec3 *rot) {
    Quat q2;
    Quat q3;
    s32 i;

    *quat = zeroQuat;
    for (i = 0; i < 3; i++) {
        if ((&rot->x)[rotOrder[i]] != 0.0f) {
            switch (rotOrder[i]) {
                case 0:
                    q2.x = 1.0f;
                    q2.y = 0.0f;
                    q2.z = 0.0f;
                    break;
                case 1:
                    q2.x = 0.0f;
                    q2.y = 1.0f;
                    q2.z = 0.0f;
                    break;
                case 2:
                    q2.x = 0.0f;
                    q2.y = 0.0f;
                    q2.z = 1.0f;
                    break;
            }
            q2.w = (&rot->x)[rotOrder[i]];
            GetQuaternionFromRotation(&q3, &q2);
            QuaternionMultiply(quat, quat, &q3);
        }
    }
}

inline s32 IsHandHoldingBall(void) {
    if (handControl.releaseTimer) {
        return FALSE;
    }
    if (ball.link == &hand) {
        if (hand.state->mode == HAND_JOINED || hand.state->mode == HAND_PREWHACK) {
            return TRUE;
        }
        if (hand.state->mode == HAND_THROWAIM || hand.state->mode == HAND_THROW2CUP) {
            return TRUE;
        }
        if (hand.state->mode == HAND_BOUNCE || hand.state->mode == HAND_BOUNCE2CUP) {
            return TRUE;
        }
        if (hand.state->mode == HAND_AIRBOURNEWHACK || hand.state->mode == HAND_SPIN) {
            return TRUE;
        }
        if (hand.state->mode == HAND_ROTOR) {
            return TRUE;
        }
    }
    return FALSE;
}

void AddObjectsSpritesToSpriteList(Object *obj) {
    GameSprite *sprite;
    s32 x;

    if (obj->numSprites > 0) {
        for (x = 0; x < obj->numSprites; x++) {
            sprite = JallocAlloc(sizeof(GameSprite), 1, "Obj_Sprite");
            sprite->tex = obj->sprites[x].tex;
            sprite->pos = zero;
            if (!(obj->sprites[x].flags & 8)) {
                obj->sprites[x].flags |= 8;
                if (sprite->tex->width == 16) {
                    obj->sprites[x].width *= 2;
                }
                if (sprite->tex->height == 16) {
                    obj->sprites[x].height *= 2;
                }
            }
            sprite->r = sprite->g = sprite->b = 255;
            sprite->alpha = 128;
            if (obj->alpha[0] < 255) {
                sprite->flags = 1;
            } else {
                sprite->flags = 0;
            }
            sprite->offsetX = -sprite->tex->width / 2;
            sprite->offsetY = -sprite->tex->height / 2;
            AddSprite(sprite, 0);
            obj->sprites[x].sprite = sprite;
        }
    }
    if (obj->child) {
        AddObjectsSpritesToSpriteList(obj->child);
    }
    if (obj->sibling) {
        AddObjectsSpritesToSpriteList(obj->sibling);
    }
}

void InitActorList(void) {
    actorList.count = 0;
    actorList.head.next = actorList.head.prev = &actorList.head;
    collisionList.count = 0;
    collisionList.head.collNext = collisionList.head.collPrev = &collisionList.head;
    distList.count = 0;
    distList.head.distNext = distList.head.distPrev = &distList.head;
    distList.head.distSq = 0.0f;
}

void AddActorToDistList(Actor *actor) {
    Actor *ptr;

    if (distList.count != 0) {
        for (ptr = &distList.head; ptr != distList.head.distPrev; ptr = ptr->distNext) {
            if (actor->distSq < ptr->distSq) {
                break;
            }
        }
    } else {
        ptr = &distList.head;
    }
    distList.count++;
    actor->distNext = ptr;
    actor->distPrev = ptr->distPrev;
    ptr->distPrev->distNext = actor;
    ptr->distPrev = actor;
}

void AddActor(Actor *actor) {
    Actor *ptr = actorList.head.next;
    Actor *cPtr;

    if (actor->next == NULL) {
        if (!(actor->flags & 4)) {
            cPtr = collisionList.head.collNext;
            collisionList.count++;
            actor->collNext = cPtr;
            actor->collPrev = cPtr->collPrev;
            cPtr->collPrev->collNext = actor;
            cPtr->collPrev = actor;
        }
        if (actor->inst) {
            if (actor->inst->obj->flags & 2) {
                ptr = &actorList.head;
            }
            if (actor->inst->obj) {
                AddObjectsSpritesToSpriteList(actor->inst->obj);
            }
        }
        actorList.count++;
        actor->next = ptr;
        actor->prev = ptr->prev;
        ptr->prev->next = actor;
        ptr->prev = actor;
    }
}

void CalcMaxActorRadius(Actor *actor) {
    Vec3 d;
    f32 max = 0.0f;
    f32 temp;
    s32 i;

    for (i = 0; i < actor->obj->numSpheres; i++) {
        if (actor->obj->spheres[i].flags & 0x10) {
            SUB_VECTORS(&d, &actor->pos, &actor->obj->spheres[i].pos);
            temp = LENGTH(&d);
        } else {
            temp = LENGTH(&actor->obj->spheres[i].pos);
        }
        temp += actor->obj->spheres[i].radius;
        if (temp > max) {
            max = temp;
        }
    }
    actor->radius = max;
}

inline void RemoveObjectSprites(Object *obj, s32 freeit) {
    s32 i;

    if (obj->sprites) {
        for (i = 0; i < obj->numSprites; i++) {
            if (obj->sprites[i].sprite) {
                SubSprite(obj->sprites[i].sprite);
            }
            if (freeit) {
                JallocFree(&obj->sprites[i].sprite);
            }
        }
    }
    if (obj->child) {
        RemoveObjectSprites(obj->child, freeit);
    }
    if (obj->sibling) {
        RemoveObjectSprites(obj->sibling, freeit);
    }
}

void SubActor(Actor *actor) {
    if (actor->next) {
        actor->prev->next = actor->next;
        actor->next->prev = actor->prev;
        actorList.count--;
        actor->next = NULL;
    }
    if (actor->collNext) {
        actor->collPrev->collNext = actor->collNext;
        actor->collNext->collPrev = actor->collPrev;
        collisionList.count--;
        actor->collNext = NULL;
    }
    if (actor->inst && actor->inst->obj) {
        RemoveObjectSprites(actor->inst->obj, FALSE);
    }
}

void AddConveyorSpeedToVel(Vec3 *vel, Vec3 *add) {
    s32 i;

    if (vel->x * add->x + vel->y * add->y + vel->z * add->z < 0.0f) {
        ADD_TO_VECTOR(vel, add);
    } else {
        for (i = 0; i < 3; i++) {
            if (((&add->x)[i] > 0.0f && (&vel->x)[i] < (&add->x)[i]) ||
                ((&add->x)[i] < 0.0f && (&vel->x)[i] > (&add->x)[i])) {
                (&vel->x)[i] = (&add->x)[i];
            }
        }
    }
}

void SubConveyorSpeedFromVel(Vec3 *vel, Vec3 *add) {
    s32 i;

    if (vel->x * add->x + vel->y * add->y + vel->z * add->z < 0.0f) {
        SUB_FROM_VECTOR(vel, add);
    } else {
        for (i = 0; i < 3; i++) {
            if (((&add->x)[i] > 0.0f && (&vel->x)[i] < (&add->x)[i]) ||
                ((&add->x)[i] < 0.0f && (&vel->x)[i] > (&add->x)[i])) {
                (&vel->x)[i] = 0.0f;
            } else {
                (&vel->x)[i] -= (&add->x)[i];
            }
        }
    }
}

void AddConveyorSpeed(Vec3 *vector, Platform *platform, Vec3 *pos, s32 set) {
    Vec3 tempVect;
    f32 m;

    if (platform->pullSpeed == 0.0f) {
        if (platform->motion) {
            RotateVectorByQuaternion(&tempVect, &platform->conveyorVel, &platform->actor.rotQuat);
        } else {
            tempVect = platform->conveyorVel;
        }
    } else {
        AddVector2D(&tempVect, &platform->actor.pos, &platform->conveyorVel);
        SubVector2D(&tempVect, &tempVect, pos);
        m = LENGTH_2D(&tempVect);
        if (m == 0.0f) {
            return;
        }
        SCALE_VECTOR_2D(&tempVect, 1.0f / m);
        if (platform->pullSpeed > 0.0f) {
            m = platform->pullSpeed < m ? platform->pullSpeed : m;
        } else {
            m = platform->pullSpeed < -m ? -m : platform->pullSpeed;
        }
        tempVect.y = 0.0f;
        SCALE_VECTOR_2D(&tempVect, m);
    }
    if (set) {
        *vector = tempVect;
    } else {
        ADD_TO_VECTOR(vector, &tempVect);
    }
}

void UpdateActor(Actor *actor) {
    f32 speed; // also holds the squash sine value, as in the original
    f32 val;
    s32 j;
    s32 i;
    s32 k;

    if (actor->pos.y < -10000.0f) {
        actor->pos.y = -10000.0f;
    }
    if (actor->active == 2) {
        actor->active = 0;
        SubActor(actor);
        if (actor->state != NULL && actor->state->index != -1 && enemies[actor->state->index].brain.spellTimer[2]) {
            RemoveUniqueActor(actor, NME_FROG);
            FindObject(&actor->inst, UpdateCRC(nmeObjectName[enemies[actor->state->index].actor.type]),
                       nmeObjectName[enemies[actor->state->index].actor.type], 1);
            enemies[actor->state->index].brain.flags &= ~0x10;
            TurnEnemyInto(&enemies[actor->state->index], actor->type);
            NMEGetNewInstruction(&enemies[actor->state->index], 0, 1);
            actor->anim->time = 1.0f;
            actor->active = 0;
            SubActor(actor);
            enemies[actor->state->index].brain.spellTimer[2] = 0;
        }
        return;
    }
    if (actor->state != NULL && actor->state->squash != 0.0f) {
        j = 0;
        if (actor->state->squashTimer >= 0) {
            if (actor->phys->squashPeriod > 0.0f) {
                speed = SineWave(actor->phys->squashPeriod, actor->phys->squashPeriod * 1.570796327 + actor->state->squashTimer * 10, 0);
            } else {
                speed = SineWave(-actor->phys->squashPeriod, actor->phys->squashPeriod * -0.5235987756666667 + actor->state->squashTimer * 10, 0);
            }
            actor->state->squash *= actor->phys->squashDecay;
            if (actor->state->squashTimer++ >= 5 && actor->state->squash < actor->phys->squashMin) {
                actor->state->squash = 0.0f;
                actor->state->squashXZ = 1.0f;
                actor->state->squashY = 1.0f;
            } else {
                speed *= actor->state->squash;
                actor->state->squashXZ = speed + 1.0f;
                actor->state->squashY = 1.0f - speed;
            }
        } else {
            for (i = 0; i < 2; i++) {
                (&actor->state->squashXZ)[i] -= ((&actor->state->squashXZ)[i] - 1.0f) / 2.0f;
                if (ABS((&actor->state->squashXZ)[i] - 1.0f) < 0.01) {
                    j++;
                }
            }
            if (j == 2) {
                actor->state->squash = 0.0f;
                actor->state->squashXZ = 1.0f;
                actor->state->squashY = 1.0f;
            }
        }
    }
    actor->prevVel = actor->vel;
    if ((actor->flags & 0x8000) || (!(actor->flags & 0x1000000) && actor->type == BULLET)) {
        speed = LENGTH(&actor->vel);
        if (speed > actor->phys->maxSpeed) {
            actor->vel.x *= actor->phys->maxSpeed / speed;
            actor->vel.y *= actor->phys->maxSpeed / speed;
            actor->vel.z *= actor->phys->maxSpeed / speed;
        }
    } else if (!(actor->flags & 0x1000000)) {
        speed = LENGTH_2D(&actor->vel);
        if (speed > actor->phys->maxSpeed) {
            actor->vel.x *= actor->phys->maxSpeed / speed;
            actor->vel.z *= actor->phys->maxSpeed / speed;
        }
    }
    if (actor->obj != NULL) {
        for (k = 0; k < actor->obj->numSpheres; k++) {
            if (actor->obj->spheres[k].targetRadius != actor->obj->spheres[k].radius) {
                actor->obj->spheres[k].radius +=
                    MIN(MAX(-actor->phys->radiusRate, (actor->obj->spheres[k].targetRadius - actor->obj->spheres[k].radius) / 4.0f),
                        actor->phys->radiusRate);
            }
        }
        UpdateActorPlatform(&actor->pos, &actor->platPrevPos, &actor->vel, &actor->rot, &actor->rotAim, &actor->rotQuat,
                            actor->phys->rotAim[1], actor->flags, actor->obj->platform, actor->obj->prevPlatform);
        actor->obj->contactFlags = 0;
    }
    if (actor->state != NULL && actor->state->teleport != NULL && actor->type != BULLET && UpdateActorTeleport(actor)) {
        return;
    }
    if (actor == &hand && handControl.catapult != NULL && handControl.catapult->catapult->launchTimer > 0) {
        if (hand.state->mode == HAND_JOINED) {
            ZERO_VECTOR(&ball.vel);
        } else {
            ZERO_VECTOR(&hand.vel);
        }
    }
    UpdateActorPosAndRot(actor);
    UpdateActorCollisionAndGravity(actor);
    if ((actor->flags & 0x8000) || actor->type == BULLET) {
        speed = LENGTH(&actor->vel);
        if (!(actor->flags & 0x1000000) && speed > actor->phys->maxSpeed) {
            actor->vel.x *= actor->phys->maxSpeed / speed;
            actor->vel.y *= actor->phys->maxSpeed / speed;
            actor->vel.z *= actor->phys->maxSpeed / speed;
        }
    } else {
        speed = LENGTH_2D(&actor->vel);
        if (!(actor->flags & 0x1000000) && speed > actor->phys->maxSpeed) {
            actor->vel.x *= actor->phys->maxSpeed / speed;
            actor->vel.z *= actor->phys->maxSpeed / speed;
        }
    }
    if ((actor != &hand || hand.state->mode != HAND_SLIDING) && (actor->flags & 0x10)
        && speed < actor->phys->minSpeed * 1.5
        && (speed == 0.0f || actor->friction < actor->phys->friction
            || (actor->obj->planes[0] != NULL && terrain[((CollFace *)actor->obj->planes[0])->terrain].slideSlopeScale != 1.0f
                && actor != &hand))
        && actor->obj->planes[0] != NULL) {
        val = actor->phys->minSpeed / terrain[((CollFace *)actor->obj->planes[0])->terrain].slideSpeedScale;
        if (speed < val * 1.5
            && 1.0f - (1.0f - actor->phys->slideLimit) / terrain[((CollFace *)actor->obj->planes[0])->terrain].slideSlopeScale
                   < actor->obj->planes[0]->y
            && actor->obj->resting[0] == 1) {
            if (speed < val) {
                SUB_VECTORS(&actor->vel, &zero, actor->obj->planes[0]);
                SCALE_VECTOR(&actor->vel, gravity);
            } else {
                SCALE_VECTOR(&actor->vel, 0.95);
            }
        }
    }
    if (!(actor->flags & 0x1000000) && actor->vel.y > 15.0) {
        actor->vel.y = 15.0f;
    }
    actor->speed = LENGTH(&actor->vel);
    if (actor == &hand && (hand.flags & 0x8000) && hand.obj->planes[0] != NULL) {
        hand.obj->onGround = 1;
    } else if (actor->obj != NULL) {
        actor->obj->wasOnGround = actor->obj->onGround;
        if (actor->flags & 0x10) {
            actor->obj->onGround = 0;
            i = 0;
            if (actor->obj->planes[0] == NULL && (actor->obj->contactFlags & 2)) {
                actor->obj->onGround = 1;
            } else {
                while (actor->obj->planes[i] != NULL && i < 4) {
                    if (actor->obj->planes[i]->y < 0.75
                        && terrain[((CollFace *)actor->obj->planes[i])->terrain].gravityScale >= 1.0f) {
                        i++;
                    } else {
                        actor->obj->onGround = 1;
                        break;
                    }
                }
            }
            if (actor == &hand) {
                if (hand.state->mode != HAND_SLIDING && hand.state->mode != HAND_SKIDDING && handControl.frog == 0
                    && (hand.obj->planes[0] == NULL
                        || terrain[((CollFace *)hand.obj->planes[0])->terrain].slip <= 1.0f)) {
                    hand.friction *= 0.99;
                }
                if (hand.obj->onGround == 0 && hand.obj->contactFlags
                    && DistanceBetweenPointsSquared(&hand.pos, &hand.prevPos) < 0.01) {
                    hand.obj->onGround = 1;
                }
            }
        } else {
            actor->obj->onGround = 0;
        }
    }
    if (actor->state != NULL && actor->obj != NULL && actor->state->teleport != NULL) {
        actor->obj->onGround = 0;
        actor->obj->contactFlags = 0;
    }
    if (actor == &hand && hand.state->mode != HAND_SLIDING && hand.obj->onGround) {
        if (hand.obj->planes[0] != NULL
            && handControl.inputAccel.x * hand.obj->planes[0]->x + handControl.inputAccel.z * hand.obj->planes[0]->z < 0.0f) {
            AddToOneScaledVector2D(&actor->vel, (Vec3 *)hand.obj->planes[0], -gravity);
        }
        if (hand.state->mode != HAND_HURT && handControl.actionTime >= 3 && hand.state->onHurtPlatform == 0) {
            handControl.lastGroundPlatform = hand.obj->platform;
            if (hand.obj->platform != NULL) {
                SUB_VECTORS(&handControl.lastGroundPos, &hand.pos, &hand.obj->platform->actor.pos);
            } else {
                handControl.lastGroundPos = hand.pos;
            }
        }
    }
    if (inertia == 0) {
        ZERO_VECTOR(&actor->vel);
        actor->speed = 0.0f;
    }
    if (!(actor->flags & 0x4000)) {
        GetQuaternionFromXYZRot(&actor->rotQuat, &actor->rot);
    }
}

s32 UpdateActorCollisionAndGravity(Actor *actor) {
    Vec3 q;
    Vec3 sum;
    Vec3 v;
    f32 actorGravity;
    f32 terminalVelocity;
    f32 dist;
    s32 j;
    s32 num;
    s32 onSticky;
    s32 ok;

    onSticky = FALSE;
    num = 0;
    if (actor->flags & 0x18) {
        actor->speed = LENGTH(&actor->vel);
        actor->obj->prevPlatform = actor->obj->platform;
        actor->obj->platform = NULL;
        actor->obj->prevContactPlatform = actor->obj->contactPlatform;
        actor->obj->contactPlatform = NULL;
        if (actor->state != NULL) {
            actor->state->onHurtPlatform = 0;
        }
        actor->obj->contactFlags = 0;
        CalcCollisionSpheres(actor);
        CalcMaxActorRadius(actor);
        if (actor->flags & 0x10) {
            CheckCollisions(actor);
        }
        if (actor->flags & 8) {
            CheckActorCollisions(actor);
        }
        if (actor->flags & 0x20) {
            if (actor->type == BALL) {
                if (handControl.catapult == NULL && hand.state->mode != HAND_THROWAIM && hand.state->mode != HAND_BALLWALK) {
                    if (actor->obj->planes[0] != NULL) {
                        ActorStartRoll(actor, 0.0f, 0.0f);
                    } else if (actor->obj->contactFlags) {
                        ActorStartRoll(actor, actor->vel.x, actor->vel.z);
                    }
                }
            } else {
                RollActor(actor);
            }
        }
        if (actor == &hand) {
            if (handControl.catapult != NULL && handControl.catapult->catapult->launchTimer > 0) {
                if (hand.state->mode == HAND_JOINED) {
                    ball.obj->platform = hand.obj->platform = handControl.catapult;
                } else {
                    hand.obj->platform = handControl.catapult;
                }
            }
            if (handControl.climbPlatform != NULL) {
                hand.obj->platform = handControl.climbPlatform;
            }
            if (handControl.edgePlatform != NULL) {
                actor->obj->platform = handControl.edgePlatform;
            }
        }
    }
    if (!(actor->flags & 1) || inertia != 1) {
        return onSticky;
    }
    if (actor->flags & 0x40) {
        actorGravity = gravity / 2.0f;
    } else {
        actorGravity = gravity;
    }
    if (actor->flags & 0x01000000) {
        terminalVelocity = -100000.0f;
    } else if (!(actor->flags & 0x200000) && actor->state != NULL && actor->state->waterState) {
        terminalVelocity = -3.75f;
        actorGravity /= 2.5;
    } else {
        terminalVelocity = -15.0f;
    }
    if (actor == &ball) {
        if (ballControl.state == BALL_HELD_SPECIAL) {
            CalcMaxActorRadius(actor);
            ADD_TO_VECTOR(&ball.pos, &ball.link->pos);
            SUB_FROM_VECTOR(&ball.pos, &ball.link->prevPos);
            // dist holds the radius first. It is rounded to float before the squash term.
            dist = ball.link->radius - 3.5;
            CollideActorInsideSphere(actor, &ball.link->pos, dist - ball.link->state->squash * 16.0f);
            ActorStartRoll(actor, ball.vel.x, ball.vel.z);
        }
        for (j = 0; j < actor->obj->numSpheres; j++) {
            if (actor->obj->planes[j] != NULL && actor->obj->contactSphere[j]) {
                SUB_VECTORS(&v, &actor->pos, &actor->obj->contactPos[j]);
                if (actor->obj->resting[j]) {
                    if (j == 0) {
                        SCALE_VECTOR(&v, actor->obj->planes[0]->y * gravity / 15.0f);
                        AddToVector2D(&actor->vel, &v);
                    }
                } else {
                    SCALE_VECTOR(&v, actor->speed * gravity / 20.0f);
                    AddToVector2D(&actor->vel, &v);
                    ActorStartRoll(actor, actor->vel.x, actor->vel.z);
                }
            }
        }
    }
    if (actor->obj == NULL) {
        actor->vel.y = MAX(actor->vel.y - actorGravity, terminalVelocity);
        return FALSE;
    }
    for (j = 0; j < 4; j++) {
        if (actor->obj->planes[j] != NULL && actor->obj->planes[j]->y > 0.0f
            && (actor->obj->resting[j] == 1 || actor->phys->bounce == 0.0f)) {
            break;
        }
    }
    if (actor == &hand && (hand.flags & 0x10) && handControl.suctionPlane != NULL) {
        if (pad[ActiveController].mag == 0.0f && hand.obj->planes[0] != NULL) {
            AddOneScaledVector(&actor->vel, &zero, handControl.suctionPlane, -gravity);
            return onSticky;
        }
        AddToOneScaledVector(&hand.vel, handControl.suctionPlane, -gravity);
        return onSticky;
    }
    if (j == 4 || !(actor->flags & 0x10)) {
        actor->vel.y = MAX(actor->vel.y - actorGravity, terminalVelocity);
        return onSticky;
    }
    if (gravMode) {
        if (actor->obj->planes[0] != NULL && terrain[((CollFace *)actor->obj->planes[0])->terrain].gravityScale < 1.0f) {
            if (LENGTH(&actor->vel) < actor->phys->minSpeed) {
                AddToOneScaledVector(&actor->vel, (Vec3 *)actor->obj->planes[0], -actorGravity);
            }
            onSticky = TRUE;
            if ((actor == &hand && pad[ActiveController].mag != 0.0f)
                || (actor == &ball && (hand.state->mode == HAND_JOINED || hand.state->mode == HAND_BALLWALK) && pad[ActiveController].mag != 0.0f)) {
                // empty: the negated form gives a different comparison sequence
            } else {
                actor->vel.y = MAX(actor->vel.y - actorGravity / 2.0f, terminalVelocity);
            }
        } else {
            actor->vel.y = MAX(actor->vel.y - actorGravity, terminalVelocity);
        }
    }
    ZERO_VECTOR(&sum);
    for (j = 0; j < 4; j++) {
        if (actor->obj->planes[j] != NULL && ((CollFace *)actor->obj->planes[j])->plane.y > 0.0f
            && (actor->obj->resting[j] == 1 || actor->phys->bounce == 0.0f)) {
            if (terrain[((CollFace *)actor->obj->planes[j])->terrain].gravityScale < 1.0f
                || (actor == &hand && hand.state->mode == HAND_SLIDING)
                || ((CollFace *)actor->obj->planes[j])->plane.y
                       <= 1.0f - (1.0f - actor->phys->slideLimit)
                              / terrain[((CollFace *)actor->obj->planes[j])->terrain].slideSlopeScale) {
                ok = TRUE;
            } else if (actor->vel.y > 0.0f) {
                ok = TRUE;
            } else {
                ok = FALSE;
            }
            if (ok) {
                ADD_VECTORS(&v, &actor->pos, &globalRot[actor->obj->contactSphere[j]]);
                dist = -(v.x * actor->obj->planes[j]->x + (v.y - actorGravity) * actor->obj->planes[j]->y + v.z * actor->obj->planes[j]->z + actor->obj->planes[j]->d);
                q.x = v.x + dist * actor->obj->planes[j]->x;
                if ((actor == &hand && pad[ActiveController].mag != 0.0f && hand.state->mode != HAND_SLIDING)
                    || (actor == &ball && (hand.state->mode == HAND_JOINED || hand.state->mode == HAND_BALLWALK) && pad[ActiveController].mag != 0.0f)) {
                    q.y = v.y - actorGravity * 0.65 * terrain[((CollFace *)actor->obj->planes[j])->terrain].gravityScale + dist * actor->obj->planes[j]->y;
                } else {
                    q.y = v.y - actorGravity * terrain[((CollFace *)actor->obj->planes[j])->terrain].gravityScale + dist * actor->obj->planes[j]->y;
                }
                q.z = v.z + dist * actor->obj->planes[j]->z;
                SUB_VECTORS(&v, &q, &actor->obj->contactPos[j]);
                if (v.y < 0.0f) {
                    num++;
                    ADD_TO_VECTOR(&sum, &v);
                }
            }
        }
    }
    if (num != 0) {
        if (gravMode) {
            AddToOneScaledVector2D(&actor->vel, &sum, 1.0 / (f32)num);
            return onSticky;
        }
        AddToOneScaledVector(&actor->vel, &sum, 1.0 / (f32)num);
    }
    return onSticky;
}

void UpdateActorPosAndRot(Actor *actor) {
    Quat tempQ;
    s32 i;
    f32 mul;

    for (i = 0; i < 3; i++) {
        (&actor->prevPos.x)[i] = (&actor->pos.x)[i];
        (&actor->pos.x)[i] += (&actor->vel.x)[i];
        if (actor->obj && !actor->obj->planes[0] && (actor->type != BULLET || (actor->flags & 0x200000)) &&
            actor->type != CAMERA) {
            (&actor->vel.x)[i] *= 1.0f - (1.0f - actor->phys->friction) / 4.0f;
        } else if (i == 1 && !(actor->flags & 0x8000)) {
            actor->vel.y *= actor->phys->friction;
        } else if (actor->obj && actor->obj->planes[0]) {
            mul = 1.0f - actor->friction;
            (&actor->vel.x)[i] *= terrain[((CollFace *)actor->obj->planes[0])->terrain].slip < 1.0f
                                      ? 1.0f - mul
                                      : 1.0f - mul / terrain[((CollFace *)actor->obj->planes[0])->terrain].slip;
        } else {
            (&actor->vel.x)[i] *= actor->friction;
        }
    }
    if (actor->flags & 0x4000) {
        GetQuaternionFromRotation(&tempQ, (Quat *)&actor->spinAxis);
        QuaternionMultiply(&actor->rotQuat, &tempQ, &actor->rotQuat);
        actor->spinSpeed *= actor->phys->rotDrag;
        return;
    }
    for (i = 0; i < 3; i++) {
        (&actor->rot.x)[i] += (&actor->rotVel.x)[i];
        (&actor->rot.x)[i] = Aabs((&actor->rot.x)[i]);
        if (actor->phys->rotAim[i]) {
            ActorHandleRotaim(actor, i);
        }
        (&actor->rotVel.x)[i] *= actor->phys->rotDrag;
        if (!inertia) {
            (&actor->rotVel.x)[i] = 0.0f;
        }
    }
}

s32 UpdateActorTeleport(Actor *actor) {
    Vec3 tempVect;
    PlatformTeleport *teleport;
    s32 i;
    s32 r;

    if (actor->state == NULL || actor->state->teleport == NULL) {
        return FALSE;
    }
    for (i = 0; i < 1; i++) {
        tempVect = actor->pos;
        r = Random(31) - 15;
        tempVect.x += r;
        r = Random(31) - 15;
        tempVect.y += r;
        r = Random(31) - 15;
        tempVect.z += r;
        AddDebris(TRAIL, &tempVect, &zero, NULL, NULL);
    }
    if (actor->state->teleportTimer == 0) {
        teleport = actor->state->teleport;
        actor->state->teleportTimer = -teleport->growTime;
        if (actor->state->teleportTimer == 0) {
            if (actor == &ball) {
                ball.scale.x = ball.scale.y = ball.scale.z = ball.obj->spheres[0].radius * 0.05 / 9.0;
            } else {
                actor->scale.x = actor->scale.y = actor->scale.z = 0.05f;
            }
            actor->state->teleport = NULL;
        }
        if (teleport->target != NULL) {
            if (teleport->target->motion != NULL) {
                if (teleport->target->motion->rot.y != 0.0f) {
                    RotateVector2D(&tempVect, &teleport->pos, teleport->target->motion->rot.y);
                } else if (teleport->target->motion->rot.x != 0.0f) {
                    RotateVector2DXYZ(&tempVect, &teleport->pos, teleport->target->motion->rot.x, 0);
                } else if (teleport->target->motion->rot.z != 0.0f) {
                    RotateVector2DXYZ(&tempVect, &teleport->pos, teleport->target->motion->rot.z, 2);
                }
                ADD_VECTORS(&actor->pos, &tempVect, &teleport->target->actor.pos);
            } else {
                ADD_VECTORS(&actor->pos, &teleport->pos, &teleport->target->actor.pos);
            }
            if (teleport->flags & 0x20) {
                actor->obj->prevPlatform = actor->obj->platform = teleport->target;
                if (teleport->target->scale != NULL) {
                    actor->pos.y = teleport->target->actor.pos.y
                                   + teleport->target->extent.y * teleport->target->actor.scale.y / teleport->target->scale->min[1];
                } else {
                    actor->pos.y = teleport->target->actor.pos.y + teleport->target->extent.y;
                }
            }
        } else if (teleport->flags & 0x40) {
            if (actor == &hand) {
                tempVect = ball.pos;
                ball.pos = hand.pos;
                hand.pos = tempVect;
                ball.state->teleportTimer = -teleport->growTime;
                ball.state->teleport = teleport;
                ball.obj->spheres[0].radius = ball.obj->spheres[0].targetRadius = 0.0f;
            } else if (actor == &ball) {
                tempVect = ball.pos;
                ball.pos = hand.pos;
                hand.pos = tempVect;
                hand.state->teleportTimer = -teleport->growTime;
                hand.state->teleport = teleport;
                ZERO_VECTOR(&hand.scale);
            }
        } else {
            actor->pos = teleport->pos;
        }
        if (cameraInfo.focus == &actor->pos) {
            ADD_TO_VECTOR(&camera.pos, &actor->pos);
            SUB_FROM_VECTOR(&camera.pos, &actor->prevPos);
            ADD_TO_VECTOR(&cameraInfo.actor.pos, &actor->pos);
            SUB_FROM_VECTOR(&cameraInfo.actor.pos, &actor->prevPos);
        }
        if (teleport->flags & 0x10) {
            ZERO_VECTOR(&actor->vel);
        }
        actor->pos.y += actor->radius;
        actor->prevPos = actor->pos;
        PlaySample(318, &actor->pos, 100, 128);
        if (actor == &hand) {
            DeactivateCollectable(SUCTION);
        }
        return FALSE;
    }
    if (actor->state->teleportTimer > 0) {
        actor->state->teleportTimer--;
        if (actor == &ball) {
            ball.scale.x = ball.scale.y = ball.scale.z =
                (f32)(ball.state->teleportTimer * 0.05 * ball.obj->spheres[0].radius) / (ball.state->teleport->shrinkTime * 9.0);
        } else {
            actor->scale.x = actor->scale.y = actor->scale.z =
                (f32)(actor->state->teleportTimer * 0.05) / (f32)actor->state->teleport->shrinkTime;
        }
        return TRUE;
    }
    actor->state->teleportTimer++;
    if (actor == &ball) {
        ball.scale.x = ball.scale.y = ball.scale.z =
            (f32)((ball.state->teleport->growTime + ball.state->teleportTimer) * 0.05 * ball.obj->spheres[0].radius)
            / (ball.state->teleport->growTime * 9.0);
    } else {
        actor->scale.x = actor->scale.y = actor->scale.z =
            (f32)(actor->state->teleport->growTime + actor->state->teleportTimer) * 0.05 / actor->state->teleport->growTime;
    }
    if (actor->state->teleportTimer == 0) {
        if (actor == &ball) {
            ball.scale.x = ball.scale.y = ball.scale.z = ball.obj->spheres[0].radius * 0.05 / 9.0;
        } else {
            actor->scale.x = actor->scale.y = actor->scale.z = 0.05f;
        }
        actor->state->teleport = NULL;
    }
    return FALSE;
}

void UpdateActorPlatform(Vec3 *actorPos, Vec3 *actorActualOldpos, Vec3 *actorVel, Vec3 *actorRot, Vec3 *actorRotaim, Quat *actorQRot, s8 raimy, s32 flags,
                         Platform *actorPlatform, Platform *actorOldPlatform) {
    f32 d[3];
    Quat tempQ;
    Vec3 tempVect;
    Vec3 tempVel;
    Platform *p;
    Platform *pp;
    f32 dist;
    f32 ang;
    s32 k;
    s32 axis;
    f32 *cp;
    s32 n;

    if (actorPlatform != NULL && (actorPlatform->flags & 0x200)) {
        if (actorPlatform->motion != NULL) {
            SUB_VECTORS(&tempVect, actorPos, &actorPlatform->actor.pos);
            RotateVector2D(&tempVect, &tempVect, actorPlatform->motion->rot.y - actorPlatform->motion->oldRot.y);
            ADD_VECTORS(actorActualOldpos, &tempVect, &actorPlatform->actor.pos);
        } else {
            *actorActualOldpos = *actorPos;
        }
        AddConveyorSpeed(&tempVel, actorPlatform, actorPos, 1);
        ADD_TO_VECTOR(actorPos, &tempVel);
        if (actorOldPlatform != actorPlatform) {
            SubConveyorSpeedFromVel(actorVel, &tempVel);
        }
    }
    if (actorOldPlatform != NULL && actorPlatform != actorOldPlatform && (actorOldPlatform->flags & 0x200)) {
        AddConveyorSpeed(&tempVel, actorOldPlatform, actorPos, 1);
        if (DOT_PRODUCT(actorVel, &tempVel) < 0.0f) {
            ADD_TO_VECTOR(actorVel, &tempVel);
        } else {
            cp = &tempVel.x;
            for (k = 0; k < 3; k++) {
                if ((cp[k] > 0.0f && (&actorVel->x)[k] < cp[k]) ||
                    (cp[k] < 0.0f && cp[k] < (&actorVel->x)[k])) {
                    (&actorVel->x)[k] = cp[k];
                }
            }
        }
    }
    // Each platform is tested before it is assigned to p. An if/else that assigns first
    // leaves a NULL test on p after the join. The ROM does not have that test.
    if (((actorPlatform != NULL && (p = actorPlatform)) || (actorOldPlatform != NULL && (p = actorOldPlatform)))
        && p->actor.pos.y - p->actor.prevPos.y > -10.0f) {
        ADD_TO_VECTOR(actorPos, &p->actor.pos);
        SUB_FROM_VECTOR(actorPos, &p->actor.prevPos);
        for (pp = p->parent; pp != NULL; pp = pp->parent) {
            ADD_TO_VECTOR(actorPos, &pp->actor.pos);
            SUB_FROM_VECTOR(actorPos, &pp->actor.prevPos);
        }
        if (p->catapult != NULL || (p->flags & 0xA000)) {
            if (p->motion->rot.y != p->motion->oldRot.y) {
                RotateVector2D(actorVel, actorVel, p->motion->rot.y - p->motion->oldRot.y);
                if (!(flags & 0x20)) {
                    actorRot->y += p->motion->rot.y - p->motion->oldRot.y;
                    actorRotaim->y += p->motion->rot.y - p->motion->oldRot.y;
                }
                d[0] = actorPos->x - p->actor.pos.x;
                d[1] = actorPos->z - p->actor.pos.z;
                dist = sqrtf(d[0] * d[0] + d[1] * d[1]);
                ang = Aabs(atan2(d[1], d[0]) - (p->motion->rot.y - p->motion->oldRot.y));
                d[0] = dist * __cosf(ang);
                d[1] = dist * __sinf(ang);
                actorPos->x = d[0] + p->actor.pos.x;
                actorPos->z = d[1] + p->actor.pos.z;
            }
            if (p->motion->rot.x != p->motion->oldRot.x) {
                tempVect.x = actorPos->x - p->actor.pos.x;
                tempVect.y = actorPos->y - p->actor.pos.y;
                tempVect.z = actorPos->z - p->actor.pos.z;
                RotateVector2DXYZ(&tempVect, &tempVect, -p->motion->rot.y, 1);
                d[0] = tempVect.y;
                d[1] = tempVect.z;
                dist = sqrtf(tempVect.y * tempVect.y + tempVect.z * tempVect.z);
                ang = Aabs(atan2(d[1], d[0]) + (p->motion->rot.x - p->motion->oldRot.x));
                d[0] = dist * __cosf(ang);
                d[1] = dist * __sinf(ang);
                tempVect.y = d[0];
                tempVect.z = d[1];
                RotateVector2DXYZ(&tempVect, &tempVect, p->motion->rot.y, 1);
                actorPos->x = p->actor.pos.x + tempVect.x;
                actorPos->y = p->actor.pos.y + tempVect.y;
                actorPos->z = p->actor.pos.z + tempVect.z;
            }
            if (p->motion->rot.z != p->motion->oldRot.z) {
                tempVect.x = actorPos->x - p->actor.pos.x;
                tempVect.y = actorPos->y - p->actor.pos.y;
                tempVect.z = actorPos->z - p->actor.pos.z;
                RotateVector2DXYZ(&tempVect, &tempVect, -p->motion->rot.y, 1);
                d[0] = tempVect.x;
                d[1] = tempVect.y;
                dist = sqrtf(tempVect.x * tempVect.x + tempVect.y * tempVect.y);
                ang = Aabs(atan2(d[1], d[0]) + (p->motion->rot.z - p->motion->oldRot.z));
                d[0] = dist * __cosf(ang);
                d[1] = dist * __sinf(ang);
                tempVect.x = d[0];
                tempVect.y = d[1];
                RotateVector2DXYZ(&tempVect, &tempVect, p->motion->rot.y, 1);
                actorPos->x = p->actor.pos.x + tempVect.x;
                actorPos->y = p->actor.pos.y + tempVect.y;
                actorPos->z = p->actor.pos.z + tempVect.z;
            }
        } else if (p->motion != NULL) {
            if (p->motion->rot.y != p->motion->oldRot.y) {
                RotateVector2D(actorVel, actorVel, p->motion->rot.y - p->motion->oldRot.y);
                if (flags & 0x4000) {
                    vertQ.w = p->motion->rot.y - p->motion->oldRot.y;
                    GetQuaternionFromRotation(&tempQ, &vertQ);
                    QuaternionMultiply(actorQRot, &tempQ, actorQRot);
                } else if (raimy) {
                    actorRot->y += p->motion->rot.y - p->motion->oldRot.y;
                    actorRotaim->y += p->motion->rot.y - p->motion->oldRot.y;
                }
            }
            axis = 10;
            n = 0;
            for (k = 0; k < 3; k++) {
                if ((&p->motion->rot.x)[k] == (&p->motion->oldRot.x)[k]) {
                    d[n++] = (&actorPos->x)[k] - (&p->actor.pos.x)[k];
                } else {
                    axis = k;
                }
            }
            if (axis != 10) {
                tempVect.x = actorPos->x - p->actor.pos.x;
                tempVect.y = actorPos->y - p->actor.pos.y;
                tempVect.z = actorPos->z - p->actor.pos.z;
                RotateVector2DXYZ(&tempVect, &tempVect, (&p->motion->rot.x)[axis] - (&p->motion->oldRot.x)[axis], axis);
                actorPos->x = p->actor.pos.x + tempVect.x;
                actorPos->y = p->actor.pos.y + tempVect.y;
                actorPos->z = p->actor.pos.z + tempVect.z;
            }
        }
    }
}

void ActorHandleRotaim(Actor *actor, s32 i) {
    f32 val = 1.0f;

    if (actor->phys->rotAim[i] == 1) {
        val = 2.0f;
    }
    (&actor->rotAim.x)[i] = Aabs((&actor->rotAim.x)[i]);
    if (FindShortestAngle((&actor->rot.x)[i], (&actor->rotAim.x)[i]) <= 0.01) {
        (&actor->rot.x)[i] = (&actor->rotAim.x)[i];
    } else {
        if ((&actor->rot.x)[i] < (&actor->rotAim.x)[i]) {
            if ((&actor->rotAim.x)[i] - (&actor->rot.x)[i] > PI) {
                (&actor->rot.x)[i] =
                    (&actor->rot.x)[i] -
                    (actor->phys->turnSpeed > FindShortestAngle((&actor->rot.x)[i], (&actor->rotAim.x)[i]) / val
                         ? FindShortestAngle((&actor->rot.x)[i], (&actor->rotAim.x)[i]) / val
                         : actor->phys->turnSpeed);
            } else {
                (&actor->rot.x)[i] =
                    (&actor->rot.x)[i] +
                    (actor->phys->turnSpeed > FindShortestAngle((&actor->rot.x)[i], (&actor->rotAim.x)[i]) / val
                         ? FindShortestAngle((&actor->rot.x)[i], (&actor->rotAim.x)[i]) / val
                         : actor->phys->turnSpeed);
            }
        } else if ((&actor->rotAim.x)[i] < (&actor->rot.x)[i]) {
            if ((&actor->rot.x)[i] - (&actor->rotAim.x)[i] > PI) {
                (&actor->rot.x)[i] =
                    (&actor->rot.x)[i] +
                    (actor->phys->turnSpeed > FindShortestAngle((&actor->rot.x)[i], (&actor->rotAim.x)[i]) / val
                         ? FindShortestAngle((&actor->rot.x)[i], (&actor->rotAim.x)[i]) / val
                         : actor->phys->turnSpeed);
            } else {
                (&actor->rot.x)[i] =
                    (&actor->rot.x)[i] -
                    (actor->phys->turnSpeed > FindShortestAngle((&actor->rot.x)[i], (&actor->rotAim.x)[i]) / val
                         ? FindShortestAngle((&actor->rot.x)[i], (&actor->rotAim.x)[i]) / val
                         : actor->phys->turnSpeed);
            }
        }
    }
    (&actor->rot.x)[i] = Aabs((&actor->rot.x)[i]);
}

void AccelerateActor2D(Actor *actor, f32 speed, f32 maxSpeed) {
    f32 angle = actor->rot.y;

    actor->vel.x += speed * __sinf(angle);
    actor->vel.z += speed * __cosf(angle);
    actor->speed = LENGTH(&actor->vel);
    if (!(actor->flags & 0x1000000)) {
        speed = LENGTH_2D(&actor->vel); // reusing speed matches the original registers
        if (speed > maxSpeed) {
            SCALE_VECTOR_2D(&actor->vel, maxSpeed / speed);
            actor->speed = LENGTH(&actor->vel);
        }
    }
}

void AccelerateActor(Actor *actor, f32 speed, f32 maxSpeed) {
    Vec3 tempVect;
    f32 angle;

    tempVect.y = tempVect.x = 0.0f;
    tempVect.z = 1.0f;
    if (actor->flags & 0x4000) {
        RotateVectorByQuaternion(&tempVect, &tempVect, &actor->rotQuat);
    } else {
        angle = actor->rot.y;
        RotateVector2DXYZ(&tempVect, &tempVect, actor->rot.x, 0);
        RotateVector2D(&tempVect, &tempVect, angle);
    }
    tempVect.x *= speed;
    tempVect.y *= speed;
    tempVect.z *= speed;
    actor->vel.x += tempVect.x;
    actor->vel.y += tempVect.y;
    actor->vel.z += tempVect.z;
    ActorMaxOutVel(actor, maxSpeed);
}

inline void ActorMaxOutVel(Actor *actor, f32 maxSpeed) {
    actor->speed = LENGTH(&actor->vel);
    if (!(actor->flags & 0x1000000) && actor->speed > maxSpeed) {
        SCALE_VECTOR(&actor->vel, maxSpeed / actor->speed);
        actor->speed =
            LENGTH(&actor->vel);
    }
}

inline void ActorMaxOutVel2D(Actor *actor, f32 maxSpeed) {
    if (!(actor->flags & 0x1000000) &&
        LENGTH_2D(&actor->vel) > maxSpeed) {
        SCALE_VECTOR_2D(&actor->vel, maxSpeed / actor->speed);
        actor->speed =
            LENGTH(&actor->vel);
    }
}

void AccelerateActorFromQuaternion(Actor *actor, f32 speed, f32 maxSpeed, Quat *quat, s32 is3D) {
    Vec3 tempVect;

    tempVect.y = tempVect.x = 0.0f;
    tempVect.z = 1.0f;
    RotateVectorByQuaternion(&tempVect, &tempVect, quat);
    tempVect.x *= speed;
    tempVect.y *= speed;
    tempVect.z *= speed;
    actor->vel.x += tempVect.x;
    actor->vel.y += tempVect.y;
    actor->vel.z += tempVect.z;
    if (is3D) {
        ActorMaxOutVel(actor, maxSpeed);
    } else {
        ActorMaxOutVel2D(actor, maxSpeed);
    }
}

void AccelerateActorFromActor(Actor *sourceActor, Actor *targetActor, f32 speed, f32 maxSpeed) {
    targetActor->vel.x += speed * __sinf(sourceActor->rot.y);
    targetActor->vel.z += speed * __cosf(sourceActor->rot.y);
    ActorMaxOutVel(targetActor, maxSpeed);
}

void AccelerateActorFromActor2D(Actor *from, Actor *to, f32 speed, f32 max) {
    to->vel.x += speed * __sinf(from->rot.y);
    to->vel.z += speed * __cosf(from->rot.y);
    ActorMaxOutVel2D(to, max);
}

s32 IsActorFacingActor(Actor *actor, Actor *target, f32 range) {
    Vec3 d;

    SubVector2D(&d, &target->pos, &actor->pos);
    return FindShortestAngle(actor->rot.y, Aabs(atan2(d.x, d.z))) < range;
}

s32 IsActorFacingPoint(Actor *actor, Vec3 *target, f32 range) {
    Vec3 d;

    SubVector2D(&d, target, &actor->pos);
    return FindShortestAngle(actor->rot.y, Aabs(atan2(d.x, d.z))) < range;
}

void UpdateActorPositionDependencies(void) {
    Vec3 temp;
    Vec3 temp2;
    Quat tempQ;
    f32 tempf;
    f32 val;
    f32 dist;
    f32 camSpeed;
    s32 i;

    cameraInfo.actor.prevPos = cameraInfo.actor.pos;
    if (gameInfo.level >= INTRO) {
        return;
    }
    temp.x = temp.y = 0.0f;
    temp.z = 5.0f;
    RotateVector2D(&temp, &temp, hand.rot.y);
    ADD_VECTORS(&handControl.edgeProbe.pos, &hand.pos, &temp);
    temp.x = 0.0f;
    temp.z = 18.0f;
    temp.y = frontprobey;
    RotateVector2D(&temp, &temp, hand.rot.y);
    ADD_VECTORS(&handControl.frontProbe.pos, &hand.pos, &temp);
    if (hand.state->mode == HAND_CLING) {
        if (handControl.actionTime < 12) {
            for (i = 0; i < 4; i++) {
                handControl.clingPlanes[i] = hand.obj->planes[i];
            }
        } else {
            for (i = 0; i < 4; i++) {
                if (hand.obj->planes[i] != NULL && hand.obj->planes[i] != handControl.clingPlanes[i]) {
                    ApplyAction(HAND_DROPDOWN);
                    break;
                }
            }
        }
        ZERO_VECTOR(&hand.vel);
    }
    if (hand.state->mode == HAND_BALLWALK) {
        SetVector2D(&hand.pos, &ball.pos);
        tempf = ball.obj->spheres[0].radius + hand.obj->spheres[0].radius + 2.0;
        if (hand.pos.y - ball.pos.y < tempf) {
            hand.pos.y = ball.pos.y + tempf - 5.0;
            if (ball.vel.y > 0.0f) {
                hand.vel.y = ball.vel.y * 1.2;
            } else {
                hand.vel.y = 0.0f;
            }
        }
    }
    if (ballControl.state == BALL_HELD) {
        ballControl.fallTime = 0;
        if (ballControl.enemySphere == -1) {
            if (ballControl.enemy->type == CHUCK) {
                SetVector2D(&ballControl.enemy->pos, &ball.pos);
                SetVector2D(&ball.vel, &ballControl.enemy->vel);
                ballControl.enemy->vel.y = ball.vel.y;
                ballControl.enemy->pos.y = ball.pos.y + ball.obj->spheres[0].radius
                                           + ballControl.enemy->obj->spheres[0].radius
                                           - ballControl.enemy->obj->spheres[0].pos.y - 5.0f;
            } else {
                RotateVector2D(&temp2, &ballControl.enemyOffset, ballControl.enemy->rot.y);
                ADD_VECTORS(&temp, &ballControl.enemy->pos, &temp2);
                SUB_VECTORS(&temp2, &ball.pos, &temp);
                temp2.x *= 0.5;
                temp2.y *= 0.5;
                temp2.z *= 0.5;
                SUB_FROM_VECTOR(&ball.pos, &temp2);
            }
        } else {
            COPY_VECTOR(&ball.pos, &ballControl.enemy->obj->spheres[ballControl.enemySphere].pos);
            RotateVectorByQuaternion(&temp, &ballControl.enemyOffset, &ballControl.enemy->rotQuat);
            ADD_TO_VECTOR(&ball.pos, &temp);
            ADD_TO_VECTOR(&ball.pos, &ballControl.enemy->vel);
        }
        if (ballControl.enemy->type == BUGLE) {
            MakeUnit(&temp2);
            SquashActor(&ball, &temp2, -3.0f, 0.5f);
            if (DistanceBetweenPoints(&ball.pos, &ballControl.enemy->pos) < ballControl.enemy->radius - 4.0f) {
                ball.flags |= 1;
                ballControl.state = BALL_HELD_SPECIAL;
                ball.flags &= ~0x18;
                SUB_VECTORS(&ball.vel, &ball.pos, &ball.prevPos);
            }
        } else {
            vertQ.w = ballControl.enemy->rot.y - ballControl.enemyRot.y;
            GetQuaternionFromRotation(&tempQ, &vertQ);
            QuaternionMultiply(&ball.rotQuat, &tempQ, &ball.rotQuat);
        }
    }
    if (handControl.status == HAND_HELD || handControl.status == HAND_HOLDING) {
        if (handControl.status == HAND_HELD) {
            DisableControl(2);
            handControl.actionLock = 2;
            hand.rotAim.y = handControl.attachedTo->rot.y;
        }
        if (handControl.attachTag == -1) {
            if (handControl.attachedTo->flags & 0x4000) {
                RotateVectorByQuaternion(&temp2, &handControl.attachOffset, &handControl.attachedTo->rotQuat);
            } else {
                RotateVector2D(&temp2, &handControl.attachOffset, handControl.attachedTo->rot.y);
            }
            ADD_VECTORS(&temp, &handControl.attachedTo->pos, &temp2);
            SUB_VECTORS(&temp2, &hand.pos, &temp);
            if (handControl.status != HAND_HOLDING) {
                temp2.x *= 0.5;
                temp2.y *= 0.5;
                temp2.z *= 0.5;
            }
            SUB_FROM_VECTOR(&hand.pos, &temp2);
        } else {
            COPY_VECTOR(&hand.pos, &handControl.attachedTo->obj->spheres[handControl.attachTag].pos);
            RotateVectorByQuaternion(&temp, &handControl.attachOffset, &handControl.attachedTo->rotQuat);
            ADD_TO_VECTOR(&hand.pos, &temp);
        }
        hand.vel = handControl.attachedTo->vel;
    }
    if (hand.state->mode == HAND_JOINED || hand.state->mode == HAND_THROW2CUP || hand.state->mode == HAND_BOUNCE2CUP) {
        MoveHandToBall();
    }
    if (hand.state->mode == HAND_THROWAIM || hand.state->mode == HAND_SPIN) {
        MoveBallToHand();
        ball.vel = hand.vel;
    }
    if (hand.state->mode == HAND_BOUNCE) {
        SetVector2D(&hand.pos, &ball.pos);
        SetVector2D(&hand.vel, &ball.vel);
        if (ballControl.cancerTimer == 0 && pad[ActiveController].mag < 5.0f) {
            SCALE_VECTOR_2D(&ball.vel, 0.7);
        }
        val = ball.pos.y - 6.0 + ball.obj->spheres[0].radius;
        if (hand.pos.y < val) {
            hand.pos.y = val;
            if (hand.vel.y < 0.0f) {
                hand.vel.y = 0.0f;
            }
        }
    } else if (cameraInfo.mode && (cameraInfo.focus != &hand.pos || hand.state->mode != HAND_THROWAIM)) {
        AddToOneScaledVector(&cameraInfo.actor.pos, cameraInfo.focusVel, 0.4f);
    }
    if (ball.speed != 0.0f && !ball.obj->wasOnGround && hand.state->mode != HAND_JOINED && hand.state->mode != HAND_BOUNCE
        && ball.obj->onGround && ball.obj->planes[0] != NULL && ball.obj->planes[0]->y > 0.9) {
        val = ABS(DOT_PRODUCT(&ball.vel, ball.obj->planes[0]));
        val /= ball.speed;
        SCALE_VECTOR_2D(&ball.vel, MIN(1.6 - val, 0.9));
    }
    if (cameraInfo.mode) {
        CalcCameraRot();
        if (cameraInfo.overrideTimer == 0 && cameraInfo.targetFrozen == 0) {
            if (cameraInfo.focus == &hand.pos && TrackBall()) {
                cameraInfo.focus = &ball.pos;
                cameraInfo.focusVel = &ball.vel;
            } else if (cameraInfo.focus == &ball.pos
                       && (hand.state->mode == HAND_THROWAIM || hand.state->mode == HAND_AIRBOURNEWHACK)) {
                cameraInfo.focus = &hand.pos;
                cameraInfo.focusVel = &hand.vel;
            }
            if ((cameraInfo.focus == &hand.pos || cameraInfo.focus == &ball.pos)
                && hand.state->mode == HAND_BOUNCE) {
                cameraInfo.targetY = ball.pos.y + (hand.pos.y - ball.pos.y) / 2.0f;
            } else {
                cameraInfo.targetY = cameraInfo.focus->y + cameraInfo.targetOffsetY;
            }
            cameraInfo.targetOffsetY = cameraInfo.distance / 11.0f;
            if (cameraInfo.actor.pos.y < cameraInfo.targetY) {
                cameraInfo.actor.pos.y += (cameraInfo.followSpeed / 2.0f > (cameraInfo.targetY - cameraInfo.actor.pos.y) / 3.0f) ? (cameraInfo.targetY - cameraInfo.actor.pos.y) / 3.0f : cameraInfo.followSpeed / 2.0f;
            }
            if (cameraInfo.actor.pos.y > cameraInfo.targetY) {
                cameraInfo.actor.pos.y -= (cameraInfo.followSpeed / 2.0f > (cameraInfo.actor.pos.y - cameraInfo.targetY) / 3.0f) ? (cameraInfo.actor.pos.y - cameraInfo.targetY) / 3.0f : cameraInfo.followSpeed / 2.0f;
            }
            camera.vel.y *= 0.8;
            if (cameraInfo.sideOffsetEnabled && gameInfo.cameoActive == 0) {
                if (hand.state->mode != HAND_BALLWALK) {
                    val = FindShortestAngleSigned(camera.rot.y, Aabs(hand.rot.y + PI));
                } else {
                    val = FindShortestAngleSigned(camera.rot.y, hand.rot.y);
                }
                tempf = ABS(PI - ABS(val));
                if (tempf < 0.4 || tempf > PI - 0.4) {
                    cameraInfo.sideOffsetAim = 0.0f;
                } else if ((tempf > 0.5 && tempf < PI - 0.5) || (tempf < PI - 0.5 && tempf > 0.5)) {
                    if (val > 0.0f) {
                        cameraInfo.sideOffsetAim = MIN(cameraInfo.distance / 10.0f, 40.0f);
                    } else {
                        cameraInfo.sideOffsetAim = MAX(-40.0f, -cameraInfo.distance / 10.0f);
                    }
                }
                val = (cameraInfo.sideOffsetAim - cameraInfo.sideOffset) / 10.0f;
                if (val > 0.0f) {
                    val = MIN(val, 0.5);
                } else {
                    val = MAX(-0.5, val);
                }
                cameraInfo.sideOffsetVel = (cameraInfo.sideOffsetVel + val) * 0.6;
                cameraInfo.sideOffset += cameraInfo.sideOffsetVel;
                temp.x = cameraInfo.sideOffset;
                temp.y = 0.0f;
                temp.z = 0.0f;
                RotateVector2D(&temp, &temp, camera.rot.y);
            } else {
                ZERO_VECTOR(&temp);
            }
        } else {
            ZERO_VECTOR(&temp);
        }
        if (cameraInfo.targetFrozen == 0) {
            SubVector2D(&temp2, cameraInfo.focus, &cameraInfo.actor.pos);
            AddToVector2D(&temp2, &temp);
            if (cameraInfo.followTimer != 0) {
                cameraInfo.followTimer--;
            } else if (cameraInfo.followSpeed < 20.0f) {
                cameraInfo.followSpeed += MIN((20.0f - cameraInfo.followSpeed) / 8.0f, 1.0f);
            }
            camSpeed = cameraInfo.followSpeed;
            dist = LENGTH_2D(&temp2);
            if (dist > camSpeed) {
                AddToOneScaledVector2D(&cameraInfo.actor.pos, &temp2, camSpeed / dist);
            } else {
                AddToVector2D(&cameraInfo.actor.pos, &temp2);
            }
        }
        cameraInfo.sideOffsetEnabled = 1;
    }
}

s32 CheckActorForTeleport(Actor *actor, Platform *platform) {
    PlatformTeleport *teleport;

    if (platform->teleport != NULL && platform->teleport->active == 1) {
        teleport = platform->teleport;
        if ((actor->type == HAND && (teleport->flags & 2) && !IsHandHoldingBall() && hand.state->mode != HAND_BALLWALK
             && hand.state->mode != HAND_WHACK)
            || (actor->type == BALL
                && (((teleport->flags & 1) && hand.state->mode != HAND_PREWHACK && hand.state->mode != HAND_JOINED)
                    || (hand.state->mode == HAND_JOINED && (teleport->flags & 8))))
            || ((teleport->flags & 4) && actor->type != DEBRIS && actor->type != BULLET)) {
            if (actor == &ball) {
                switch (hand.state->mode) {
                    case HAND_BOUNCE:
                        ApplyAction(HAND_RELEASE);
                        hand.flags &= ~0x40;
                        break;
                    case HAND_BALLWALK:
                        ApplyAction(HAND_WALK);
                        ball.link = NULL;
                        break;
                }
            } else if (actor == &hand) {
                player.hurtTimer = 0;
                hand.alpha = 100;
            }
            if (hand.state->mode == HAND_JOINED && (actor == &hand || actor == &ball)) {
                if (hand.state->teleport != NULL || ball.state->teleport != NULL) {
                    return FALSE;
                }
                hand.state->teleport = teleport;
                hand.state->teleportTimer = teleport->shrinkTime;
                ball.state->teleport = teleport;
                ball.state->teleportTimer = teleport->shrinkTime;
            } else {
                if (teleport->flags & 0x40) {
                    handControl.joinTimer = teleport->shrinkTime + teleport->growTime;
                    if (actor == &hand) {
                        ball.state->teleport = teleport;
                        ball.state->teleportTimer = teleport->shrinkTime + 1;
                    } else if (actor == &ball) {
                        hand.state->teleport = teleport;
                        hand.state->teleportTimer = teleport->shrinkTime + 1;
                    }
                }
                actor->state->teleport = teleport;
                actor->state->teleportTimer = teleport->shrinkTime;
            }
            PlaySample(317, &actor->pos, 100, 128);
            return TRUE;
        }
    }
    return FALSE;
}

void ReleaseActor(Actor *actor, s32 flags) {
    if (actor->inst) {
        JallocFree(&actor->staticMtx);
    }
    if (flags & 1) {
        JallocFree(&actor->phys);
    }
    if (flags & 2) {
        JallocFree(&actor->obj);
    }
    if (flags & 4) {
        JallocFree(&actor->anim);
    }
    if (flags & 8) {
        JallocFree(&actor->shadow);
    }
    if (flags & 0x10) {
        JallocFree(&actor->state);
    }
}

void FreeOFTC(Object *obj) {
    if (obj->child != NULL) {
        FreeOFTC(obj->child);
    }
    if (obj->sibling != NULL) {
        FreeOFTC(obj->sibling);
    }
    JallocFree(&obj->mesh->origUV);
}

inline void FreeObjectSprites(Object *obj) {
    s32 i;

    if (obj->sprites) {
        for (i = 0; i < obj->numSprites; i++) {
            JallocFree(&obj->sprites[i].sprite);
            PRINTF("Freeing sprite %d from %s\n", i, obj->name);
        }
    }
    if (obj->child) {
        FreeObjectSprites(obj->child);
    }
    if (obj->sibling) {
        FreeObjectSprites(obj->sibling);
    }
}

void DestroyAndFreeActor(Actor *actor) {
    SubActor(actor);
    if (actor->inst) {
        JallocFree(&actor->staticMtx);
    }
    JallocFree(&actor->phys);
    JallocFree(&actor->obj);
    JallocFree(&actor->anim);
    JallocFree(&actor->shadow);
    JallocFree(&actor->state);
    if (actor->inst) {
        if (actor->inst->obj) {
            FreeObjectSprites(actor->inst->obj);
        }
        JallocFree(&actor->faces);
    }
    JallocFree(&actor);
}

void AccelerateActorAcrossPlane(Actor *actor, Vec3 *plane, f32 accel, f32 maxSpeed, f32 rot, s32 is3D) {
    Quat tempQ;
    Quat tempRot;
    Quat qAim;

    if (plane->y == -1.0f) {
        vertQ.w = Aabs(6.283185308 - rot);
        qAim.y = 0.0f;
        qAim.z = 0.0f;
        qAim.w = 3.1415927f;
        qAim.x = 1.0f;
    } else {
        vertQ.w = rot;
        CrossProductVertical((Vec3 *)&qAim, plane);
        MakeUnit2D((Vec3 *)&qAim);
        qAim.w = acos(plane->y);
    }
    GetQuaternionFromRotation(&tempQ, &vertQ);
    GetQuaternionFromRotation(&tempRot, &qAim);
    QuaternionMultiply(&tempQ, &tempRot, &tempQ);
    AccelerateActorFromQuaternion(actor, accel, maxSpeed, &tempQ, is3D);
}

void AccelerateActorAcrossPlane2(Actor *actor, Vec3 *normal, f32 accel, f32 max, Quat *rot, s32 is3D) {
    Quat q;
    Quat tilt;
    Quat axis;

    q = *rot;
    if (normal->y == -1.0f) {
        axis.y = 0.0f;
        axis.z = 0.0f;
        axis.w = 3.1415927f;
        axis.x = 1.0f;
    } else {
        CrossProductVertical((Vec3 *)&axis, normal);
        MakeUnit2D((Vec3 *)&axis);
        axis.w = acos(normal->y);
    }
    GetQuaternionFromRotation(&tilt, &axis);
    QuaternionMultiply(&q, &tilt, &q);
    AccelerateActorFromQuaternion(actor, accel, max, &q, is3D);
}

void GiveActorRandomQRotVel(Actor *actor) {
    actor->spinAxis.x = Random(21) - 10;
    actor->spinAxis.y = Random(21) - 10;
    actor->spinAxis.z = Random(21) - 10;
    MakeUnit(&actor->spinAxis);
}

INCLUDE_ASM("asm/nonmatchings/actor", pad_80162BD4);

INCLUDE_RODATA("asm/nonmatchings/actor", pad_80107498);
