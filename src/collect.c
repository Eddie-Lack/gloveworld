#include "common.h"
#include "ident.h"
#include "collect.h"
#include "jalloc.h"
#include "hand.h"
#include "save.h"
#include "ball.h"
#include "maths.h"
#include "camera.h"
#include "game.h"
#include "debris.h"
#include "controller.h"
#include "texture.h"
#include "overlay.h"
#include "actor.h"
#include "banks.h"
#include "enemy.h"
#include "object.h"

void DestroyAndFreeActor();

extern s32 numGaribs;
void DisableControl(s16 time);
extern s16 currentSaveSlot;
extern char *collectableFileName[];
extern Quat zeroQuat;
u32 UpdateCRC(char *ptr);
void AnimateActor(Actor *actor, s32 animNum, s32 loop, s32 queue, f32 speed);
extern SpriteAnim garibAnimation[];
void InitSpriteAnimation(GameSprite *sprite, SpriteAnim *temp, s16 lifespan);
f32 Altitude(Vec3 *pos, Vec3 **floor, s32 *plat, s32 coll);

extern s16 colldeb;
extern s32 frameCount;
extern Vec3 zero;

extern s16 garibTimer;
extern s16 garibCount;
extern f32 garibRadius[];
extern s16 gaingaribfx;
extern s16 garibcrystalfxnum;
extern f32 gravity;
extern Overlay garibsGotOL;
void GainGarib(Vec3 *pos, s16 pitch, s16 fx);
extern s32 cheatActivated;
extern u8 ActiveController;
extern Actor plaster[4];
extern Texture *ballTextureList[];
extern u32 *ballTextureData;
extern u32 *ballTexturePalette;
extern u32 *ballTextureData2;
extern u32 *ballTexturePalette2;
extern u32 *ballBitTextureData;
extern u32 *ballBitTexturePalette;
extern u32 *handShinTextureData[2];
extern u32 *handShinTexturePalette[2];
extern Texture *handShinTextures[2];
extern f32 handInertia;
extern void *snowBallBit;

inline void AddCollectable(Collectable *collectable) {
    if (collectable->next == NULL) {
        collectableList.count++;
        collectable->prev = &collectableList.head;
        collectable->next = collectableList.head.next;
        collectableList.head.next->prev = collectable;
        collectableList.head.next = collectable;
    }
}

inline void SubCollectable(Collectable *collectable) {
    if (collectable->next != NULL) {
        collectableList.count--;
        collectable->next->prev = collectable->prev;
        collectable->prev->next = collectable->next;
        DestroyAndFreeActor(&collectable->actor);
        JallocFree(&collectable);
    }
}

void InitCollectableList(void) {
    collectableList.count = 0;
    collectableList.head.next = collectableList.head.prev = &collectableList.head;
}

void InitGaribGroupList(void) {
    garibGroupList.count = 0;
    garibGroupList.head.prev = garibGroupList.head.next = &garibGroupList.head;
}

void AddGaribGroup(GaribGroup *garibGroup) {
    if (garibGroup->prev == NULL) {
        garibGroupList.count++;
        garibGroup->next = &garibGroupList.head;
        garibGroup->prev = garibGroupList.head.prev;
        garibGroupList.head.prev->next = garibGroup;
        garibGroupList.head.prev = garibGroup;
    }
}

void SubGaribGroup(GaribGroup *garibGroup) {
    garibGroup->next->prev = garibGroup->prev;
    garibGroup->prev->next = garibGroup->next;
    garibGroupList.count--;
}

void InitGaribList(GaribList *garibList) {
    garibList->count = 0;
    garibList->head.prev = garibList->head.next = &garibList->head;
}

void AddGarib(GaribGroup *garibGroup, Garib *garib) {
    GaribList *garibList = &garibGroup->garibs;

    if (garib->prev == NULL) {
        garibList->count++;
        garib->next = &garibList->head;
        garib->prev = garibList->head.prev;
        garibList->head.prev->next = garib;
        garibList->head.prev = garib;
    }
}

void DropGaribs(s16 groupTag, f32 Yval) {
    GaribGroup *garibGroup;
    Garib *garib;
    s32 platform;

    for (garibGroup = garibGroupList.head.prev; garibGroup != &garibGroupList.head; garibGroup = garibGroup->prev) {
        if (garibGroup->tag == groupTag && (Yval != 0.0f || garibGroup->suspended == 1)) {
            if (Yval != 0.0f) {
                garibGroup->pos.y = Yval;
            }
            garibGroup->suspended = 0;
            for (garib = garibGroup->garibs.head.prev; garib != &garibGroup->garibs.head; garib = garib->prev) {
                if (!garib->active) {
                    garib->scaleTarget = garib->scale = 1.0f;
                    garib->velY = 0;
                    InitSpriteAnimation(&garib->sprite, &garibAnimation[garib->type], 0);
                    AddSprite(&garib->sprite, NULL);
                    if (Yval != 0.0f) {
                        garib->sprite.pos.y = Yval;
                    }
                    garib->active = 1;
                    if (!garib->mode) {
                        garib->altitude = Altitude(&garib->sprite.pos, &garib->normal, &platform, 0);
                    }
                }
            }
        }
    }
}

void UpdateGaribs(void) {
    Vec3 tempVect;
    GaribGroup *garibGroup;
    Garib *garib;
    Debris *d;
    Debris *scoreDebris;
    f32 radius;
    f32 distance;
    f32 ballDistance;
    f32 handDist;
    f32 handRadius;
    s32 first;
    s32 collectedBy;
    s32 mul;
    s32 pitch;

    if (garibTimer == 0 || --garibTimer == 0) {
        garibCount = 0;
    }
    for (garibGroup = garibGroupList.head.prev; garibGroup != &garibGroupList.head; garibGroup = garibGroup->prev) {
        first = TRUE;
        for (garib = garibGroup->garibs.head.prev; garib != &garibGroup->garibs.head; garib = garib->prev) {
            if (garib->active) {
                if (garib->active >= 2) {
                    garib->active--;
                }
                garib->distSq = DistanceBetweenPointsSquared(&garib->sprite.pos, &camera.pos);
                radius = garibRadius[garib->type];
                if (garibGroup->suspended == 0 && garib->active != -1) {
                    garib->velY = MAX(garib->velY - gravity, -15.0);
                    garib->sprite.pos.y += garib->velY;
                    if (first) {
                        garibGroup->pos.y += garib->velY;
                    }
                    if (garib->mode == 0) {
                        garib->altitude += garib->velY;
                    }
                    if (garib->altitude < radius && garib->velY <= 0.0f) {
                        garib->velY = -garib->velY * 0.7;
                        garib->sprite.pos.y += garib->velY;
                        if (first) {
                            garibGroup->pos.y += garib->velY;
                        }
                        if (garib->mode == 0) {
                            garib->altitude += garib->velY;
                        }
                        if (ABS(garib->velY) < 0.5) {
                            garib->active = -1;
                        }
                    }
                    first = FALSE;
                }
                garib->scale -= (garib->scale - garib->scaleTarget) / 2.0f;
                garib->sprite.scaleY = garib->sprite.scaleX = garib->scale * 64.0f;
                if (garib->type == EXTRA_LIFE) {
                    tempVect.x = 0.0f;
                    tempVect.y = SineWave(5.0f, 0.0f, frameCount) * 5.0f;
                    tempVect.z = 10.0f;
                    RotateVector2D(&tempVect, &tempVect, (f32)frameCount / 3.0);
                    ADD_TO_VECTOR(&tempVect, &garib->sprite.pos);
                    scoreDebris = AddDebris(colldeb, &tempVect, &zero, NULL, NULL);
                    if (scoreDebris != NULL) {
                        scoreDebris->sprite.anim.lifeTimer = 10;
                        scoreDebris->sprite.anim.def.life = 10;
                        scoreDebris->life = 10;
                        scoreDebris->lifeTimer = 10;
                    }
                }
            }
        }
        if (!(garibGroup->radius < DistanceBetweenPoints(&garibGroup->pos, &hand.pos)) ||
            !(garibGroup->radius < DistanceBetweenPoints(&garibGroup->pos, &ball.pos))) {
            for (garib = garibGroup->garibs.head.prev; garib != &garibGroup->garibs.head; garib = garib->prev) {
                if (abs(garib->active) != 1) {
                    continue;
                }
                distance = 100000.0f;
                if (ballControl.loseTimer == 0) {
                    SUB_VECTORS(&tempVect, &ball.pos, &garib->sprite.pos);
                    collectedBy = 2;
                    mul = 1;
                    radius = ball.radius * 1.5;
                    distance = ballDistance = DOT_PRODUCT(&tempVect, &tempVect);
                    if (ballControl.type == BALL_MODE_CRYSTAL) {
                        mul = 2;
                    }
                }
                if (hand.state->mode != HAND_JOINED) {
                    SUB_VECTORS(&tempVect, &hand.pos, &garib->sprite.pos);
                    handDist = DOT_PRODUCT(&tempVect, &tempVect);
                    handRadius = hand.radius; // read before the test: the ROM loads it ahead of the branch
                    if (handDist < ballDistance) {
                        radius = handRadius;
                        distance = handDist;
                        collectedBy = 1;
                        mul = 1;
                    }
                }
                if (distance < (radius + 15.0) * (radius + 15.0)) {
                    garib->active = 0;
                    SubSprite(&garib->sprite);
                    AddDebris(TRAIL, &garib->sprite.pos, &zero, NULL, NULL);
                    if (collectedBy == 1) {
                        if (garibCount < 5) {
                            garibCount++;
                        }
                    } else if (garibCount < 6) {
                        garibCount += 2;
                    }
                    garibTimer = 20;
                    if (garib->type != NORMAL_GARIB) {
                        if (player.health < 3) {
                            player.health++;
                            stats[HEALTH_STATS].scale = 0x800;
                        }
                        d = AddDebris(SCORE + 4, &garib->sprite.pos, &zero, NULL, NULL);
                        AddToScore(mul * 500);
                        stats[GARIB_STATS].scale = 0x800;
                        AddStarBurst(garibsGotOL.x, garibsGotOL.y, 6, SDT_STAR);
                        if (garib->type == EXTRA_LIFE) {
                            GainExtraLife();
                        } else {
                            GainGarib(&garib->sprite.pos, 128, 140);
                        }
                    } else {
                        AddStarBurst(garibsGotOL.x, garibsGotOL.y, 6, SDT_STAR);
                        pitch = (garibCount - 1) * gaingaribfx + 128;
                        switch (garibCount) {
                            case 1:
                                d = AddDebris(SCORE, &garib->sprite.pos, &zero, NULL, NULL);
                                AddToScore(mul * 10);
                                break;
                            case 2:
                                d = AddDebris(SCORE + 1, &garib->sprite.pos, &zero, NULL, NULL);
                                AddToScore(mul * 20);
                                break;
                            case 3:
                                d = AddDebris(SCORE + 2, &garib->sprite.pos, &zero, NULL, NULL);
                                AddToScore(mul * 50);
                                break;
                            case 4:
                                d = AddDebris(SCORE + 3, &garib->sprite.pos, &zero, NULL, NULL);
                                AddToScore(mul * 100);
                                break;
                            default:
                                if (distance == handDist) {
                                    pitch -= gaingaribfx;
                                    d = AddDebris(SCORE + 3, &garib->sprite.pos, &zero, NULL, NULL);
                                    AddToScore(mul * 100);
                                } else {
                                    d = AddDebris(SCORE + 4, &garib->sprite.pos, &zero, NULL, NULL);
                                    AddToScore(mul * 500);
                                }
                                break;
                        }
                        if (mul == 1) {
                            GainGarib(&garib->sprite.pos, pitch, 140);
                        } else {
                            GainGarib(&garib->sprite.pos, pitch, garibcrystalfxnum);
                        }
                    }
                    if (d != NULL) {
                        d->pos.y += 6.0f;
                    }
                    if (mul == 2) {
                        d = AddDebris(SCORE + 5, &garib->sprite.pos, &zero, NULL, NULL);
                        if (d != NULL) {
                            d->pos.y -= 2.0f;
                        }
                    }
                }
            }
        }
    }
}

void UpdatePickups(void) {
    Vec3 tempVect;
    Collectable *collectable;
    Collectable *collectable2;
    Debris *d;
    f32 scale;
    s32 x;
    s32 diff;

    for (collectable = collectableList.head.next; collectable != &collectableList.head; collectable = collectable2) {
        collectable2 = collectable->next;
        collectable->actor.rot.y = Aabs(collectable->actor.rot.y - 0.2);
        GetQuaternionFromXYZRot(&collectable->actor.rotQuat, &collectable->actor.rot);
        if (collectable->timer > 0) {
            collectable->actor.shadow->shadowTarget = 0;
            collectable->timer--;
            scale = (f32)collectable->timer * 0.7 / 10.0;
            collectable->actor.scale.z = collectable->actor.scale.y = collectable->actor.scale.x = scale;
            if (collectable->timer == 0) {
                CreateStarCircle(&collectable->actor.pos, 6, 19);
                AddToScore(300);
                if (collectable->respawnTime == 0) {
                    SubCollectable(collectable);
                } else {
                    collectable->timer = -collectable->respawnTime;
                }
            }
        } else if (collectable->timer < 0) {
            collectable->timer++;
            if (-collectable->timer < 10) {
                if (collectable->scaleTarget == 0.0f) {
                    collectable->timer--;
                } else {
                    scale = (f32)(collectable->timer + 10) * 0.7 / 10.0;
                    collectable->actor.scale.z = collectable->actor.scale.y = collectable->actor.scale.x = scale;
                }
            }
            if (collectable->timer == 0) {
                collectable->active = 1;
                collectable->actor.shadow->shadowTarget = 255;
            }
        }
        if (collectable->active) {
            collectable->actor.scale.z = collectable->actor.scale.y = collectable->actor.scale.x = collectable->actor.scale.x - (collectable->actor.scale.x - collectable->scaleTarget) / 2.0f;
            diff = (collectable->actor.alpha - collectable->alphaTarget) / 2;
            if (diff < -15) {
                diff = -15;
            }
            if (diff > 15) {
                diff = 15;
            }
            scale = diff;
            collectable->actor.alpha -= scale;
            if (collectable->alphaTarget > 0) {
                tempVect.x = 0.0f;
                tempVect.y = SineWave(5.0f, 0.0f, frameCount) * 5.0f + Random(5) - 2.0f;
                tempVect.z = 14.0f;
                RotateVector2D(&tempVect, &tempVect, (f32)frameCount / 3.0);
                ADD_TO_VECTOR(&tempVect, &collectable->actor.pos);
                d = AddDebris(colldeb, &tempVect, &zero, NULL, NULL);
                if (d != NULL) {
                    d->sprite.anim.lifeTimer = 10;
                    d->sprite.anim.def.life = 10;
                    d->life = 10;
                    d->lifeTimer = 10;
                }
            }
        }
    }
    for (collectable = collectableList.head.next; collectable != &collectableList.head; collectable = collectable->next) {
        if (collectable->active == 1 && collectable->scaleTarget != 0.0f && collectable->alphaTarget > 0) {
            SUB_VECTORS(&tempVect, &hand.pos, &collectable->actor.pos);
            if (DOT_PRODUCT(&tempVect, &tempVect) < collectable->radiusSq) {
                collectable->active = 0;
                collectable->timer = 10;
                collectable->actor.shadow->shadowTarget = 0;
                PlaySample(147, &collectable->actor.pos, 100, 128);
                player.castSpell = collectable->type;
                player.castSpellType = collectable->type;
                if (handControl.frog || hand.state->mode == HAND_ROTOR || ball.link == &hand || gameInfo.level == SPACEBONUS
                    || collectable->type == DEATH || collectable->type == FROGGY || collectable->type == STUN) {
                    ActivateCollectable(collectable->type, collectable->duration);
                } else {
                    ApplyAction(HAND_CASTSPELL);
                    handControl.castSpellTime = collectable->duration;
                }
            }
        }
    }
    for (x = 0; x < 15; x++) {
        if (player.powerups[x] > 0) {
            if (player.powerups[x] >= 2) {
                player.powerups[x]--;
            } else {
                DeactivateCollectable(x);
            }
        }
    }
}

void ProcessCollectables(void) {
    UpdateGaribs();
    UpdatePickups();
}

inline void CreateOrbiter(Actor *orbiter, s32 type, f32 scale) {
    if (!orbiter->inst) {
        FindObject(&orbiter->inst, UpdateCRC(collectableFileName[type]), collectableFileName[type], 1);
        InitActorStructures(orbiter, 0);
        orbiter->flags = 0x10004;
        orbiter->type = type + 1000;
        orbiter->rotQuat = zeroQuat;
        orbiter->scale.x = scale;
        orbiter->scale.y = scale;
        orbiter->scale.z = scale;
        if (orbiter == &handControl.orbiter && ballControl.orbiter.inst) {
            orbiter->rot.y = Aabs(ballControl.orbiter.rot.y + 3.141592654);
        } else if (orbiter == &ballControl.orbiter && handControl.orbiter.inst) {
            orbiter->rot.y = Aabs(handControl.orbiter.rot.y + 3.141592654);
        }
        AddActor(orbiter);
    }
}

void ActivateCollectable(s32 type, s32 timeVal) {
    s32 i;

    if (type == 11 && ballControl.snow == 0.0f) {
        CalcBallTexture(5);
        *ballTextureData = (u32)ballTextureList[5]->data;
        *ballTexturePalette = (u32)ballTextureList[5]->palette;
        *ballTextureData2 = (u32)ballTextureList[5]->data;
        *ballTexturePalette2 = (u32)ballTextureList[5]->palette;
        *ballBitTextureData = (u32)ballTextureList[5]->data;
        *ballBitTexturePalette = (u32)ballTextureList[5]->palette;
        ballControl.snow = 1.0f;
        ball.phys = &defaultBehaviour[5];
        for (i = 0; i < 4; i++) {
            plaster[i].alpha = 0;
        }
        return;
    }
    if (type > 0) {
        if (timeVal < 0) {
            player.powerups[type] = timeVal;
        } else if (player.powerups[type]) {
            player.powerups[type] += timeVal;
            return;
        } else {
            player.powerups[type] = timeVal;
        }
    }
    switch (type) {
        case -1:
            ball.shadow->shadowTarget = 255;
            for (i = 7; i < 13; i++) {
                DeactivateCollectable(i);
            }
            break;
        case 3:
            handControl.suctionTime = 0;
            hand.flags |= 0xC000;
            *handShinTextureData[0] = (u32)handShinTextures[1]->data;
            *handShinTextureData[1] = (u32)handShinTextures[1]->data;
            if (handShinTexturePalette[0]) {
                *handShinTexturePalette[0] = (u32)handShinTextures[1]->palette;
            }
            if (handShinTexturePalette[1]) {
                *handShinTexturePalette[1] = (u32)handShinTextures[1]->palette;
            }
            CreatePuffCircle(&hand.pos);
            PlaySample(146, &hand.pos, 100, 200);
            break;
        case 4:
            handControl.morphStep = 1;
            handControl.morphReverse = 0;
            handControl.actionLock = 10;
            hand.phys->mass = 18.0f;
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 13.0f);
            hand.obj->spheres[0].radius = hand.obj->spheres[0].targetRadius;
            CreatePuffCircle(&hand.pos);
            PlaySample(146, &hand.pos, 100, 200);
            break;
        case 5:
            hand.phys->accel = 10.0f;
            hand.phys->moveSpeed = 10.0f;
            CreateOrbiter(&handControl.orbiter2, type, 0.2f);
            CreatePuffCircle(&hand.pos);
            PlaySample(146, &hand.pos, 100, 200);
            break;
        case 6:
            DeactivateCollectable(SUCTION);
            DeactivateCollectable(HERCULES);
            DeactivateCollectable(SPEEDUP);
            ApplyAction(HAND_ROTOR);
            handControl.rotorSpeed = 0.0f;
            hand.phys->accel *= 0.2;
            CreateOrbiter(&handControl.orbiter3, 0, 0.2f);
            handControl.orbiter3.link = &hand;
            handControl.orbiter3.scale.x = 0.15f;
            handControl.orbiter3.scale.y = 0.05f;
            handControl.orbiter3.scale.z = 0.15f;
            CreatePuffCircle(&hand.pos);
            PlaySample(146, &hand.pos, 100, 200);
            break;
        case 12:
            if (ballControl.snow != 0.0f) {
                ShatterActor(&ball, snowBallBit, 8, 1.0f);
            }
            cheatActivated &= ~0x80;
        case 7:
        case 8:
        case 9:
        case 10:
            DeactivateCollectable(SNOWBALL);
        case 11:
            if (ballControl.cancerTimer == 0) {
                CalcBallTexture(type - 6);
                *ballTextureData = (u32)ballTextureList[type - 6]->data;
                *ballTexturePalette = (u32)ballTextureList[type - 6]->palette;
                *ballTextureData2 = (u32)ballTextureList[type - 6]->data;
                *ballTexturePalette2 = (u32)ballTextureList[type - 6]->palette;
                *ballBitTextureData = (u32)ballTextureList[type - 6]->data;
                *ballBitTexturePalette = (u32)ballTextureList[type - 6]->palette;
            }
            ball.flags &= ~ball.phys->flags;
            ball.phys = &defaultBehaviour[type - 6];
            ball.flags |= ball.phys->flags;
            ball.obj->spheres[0].targetRadius = ball.phys->radius;
            ball.shadow->shadowTarget = 255;
            ChangeBallInto(type - 6);
            if (gameInfo.level == PREHISTORICBOSS) {
                ball.friction = 1.0f;
            } else {
                ball.friction = ball.phys->friction;
            }
            if (type == 12) {
                for (i = 0; i < 4; i++) {
                    plaster[i].alpha = 0;
                }
            } else {
                for (i = 0; i < 4; i++) {
                    plaster[i].alpha = 100;
                }
            }
            break;
        case 13:
            EnemyDropBall();
            ballStatic = 0;
            ball.flags = defaultBehaviour[ballControl.type].flags;
            DeactivateCollectable(VANISHBALL);
            if (!IsHandHoldingBall()) {
                ballControl.returnTime = 50;
            }
            CreateOrbiter(&ballControl.orbiter, type, 0.2f);
            break;
        case 14:
            EnemyDropBall();
            ballStatic = 0;
            ball.flags = defaultBehaviour[ballControl.type].flags;
            DeactivateCollectable(BOOMERANGBALL);
            if (!IsHandHoldingBall()) {
                ballControl.returnTime = 50;
            }
            CreateOrbiter(&ballControl.orbiter, type, 0.2f);
            break;
        case 0:
        case 1:
        case 2:
        default:
            for (i = 0; i < 3; i++) {
                if (i != type) {
                    DeactivateCollectable(i);
                }
            }
            if (type != 0) {
                CreateOrbiter(&handControl.orbiter, type, 0.2f);
            }
            player.spell = type;
            break;
    }
    if (ball.phys->maxSquash == 0.0f) {
        ball.state->squash = 0.0f;
    }
}

void DeactivateCollectable(s32 type) {
    Vec3 tempVect;
    Collectable *collectable;
    s32 f;

    if (type == 11 && ballControl.snow != 0.0f) {
        ballControl.snow = 0.0f;
        *ballTextureData = (u32)ballTextureList[ballControl.type]->data;
        *ballTexturePalette = (u32)ballTextureList[ballControl.type]->palette;
        *ballTextureData2 = (u32)ballTextureList[ballControl.type]->data;
        *ballTexturePalette2 = (u32)ballTextureList[ballControl.type]->palette;
        *ballBitTextureData = (u32)ballTextureList[ballControl.type]->data;
        *ballBitTexturePalette = (u32)ballTextureList[ballControl.type]->palette;
        ball.obj->spheres[0].radius = ball.obj->spheres[0].targetRadius = defaultBehaviour[ballControl.type].radius;
        ball.phys = &defaultBehaviour[ballControl.type];
        if (ball.phys->maxSquash == 0.0f) {
            ball.state->squash = 0.0f;
        }
        for (f = 0; f < 4; f++) {
            plaster[f].alpha = 100;
        }
        return;
    }
    if (!player.powerups[type]) {
        return;
    }
    for (collectable = collectableList.head.next; collectable != &collectableList.head; collectable = collectable->next) {
        if (collectable->type == type && collectable->timer < 0) {
            collectable->timer = -11;
        }
    }
    player.powerups[type] = 0;
    switch (type) {
        case 1:
            cheatActivated &= ~0x4000;
            player.spell = 15;
            handControl.orbiter.inst = NULL;
            SubActor(&handControl.orbiter);
            break;
        case 2:
            cheatActivated &= ~0x8000;
            player.spell = 15;
            handControl.orbiter.inst = NULL;
            SubActor(&handControl.orbiter);
            break;
        case 3:
            *handShinTextureData[0] = (u32)handShinTextures[0]->data;
            *handShinTextureData[1] = (u32)handShinTextures[0]->data;
            if (handShinTexturePalette[0]) {
                *handShinTexturePalette[0] = (u32)handShinTextures[0]->palette;
            }
            if (handShinTexturePalette[1]) {
                *handShinTexturePalette[1] = (u32)handShinTextures[0]->palette;
            }
            handControl.suctionPlane = NULL;
            hand.flags &= ~0xC000;
            hand.rotAim.z = hand.rot.z = 0.0f;
            CreatePuffCircle(&hand.pos);
            break;
        case 4:
            handControl.morphReverse = handControl.morphStep = 1;
            handControl.actionLock = 10;
            hand.phys->mass = 10.0f;
            hand.phys->accel = 5.0f;
            SetHandRadius(&hand.obj->spheres[0].targetRadius, 13.0f);
            hand.obj->spheres[0].radius = hand.obj->spheres[0].targetRadius;
            hand.phys->moveSpeed = 7.0f;
            handInertia = 0.6f;
            cheatActivated &= ~0x10000;
            hand.friction = 0.6f;
            CreatePuffCircle(&hand.pos);
            break;
        case 5:
            hand.phys->moveSpeed = 7.0f;
            cheatActivated &= ~0x20000;
            hand.phys->accel = 5.0f;
            handControl.orbiter2.inst = NULL;
            SubActor(&handControl.orbiter2);
            CreatePuffCircle(&hand.pos);
            break;
        case 6:
            if (handControl.deathTimer == 0) {
                if (ball.link == &hand) {
                    ApplyAction(HAND_JOINED);
                } else {
                    ApplyActionIdle();
                }
            }
            hand.phys->accel /= 0.2;
            handControl.orbiter3.inst = NULL;
            SubActor(&handControl.orbiter3);
            hand.flags |= 1;
            ball.flags |= 1;
            hand.obj->spheres[0].flags |= 0x20;
            hand.obj->spheres[0].pos.y = 0.0f;
            hand.rot.x = hand.rotAim.x = 0.0f;
            GetQuaternionFromXYZRot(&hand.rotQuat, &hand.rot);
            tempVect = hand.pos;
            tempVect.y += 10.0f;
            CreatePuffCircle(&tempVect);
            pad[ActiveController].latch[BUTTON_A] = 1;
            pad[ActiveController].latch[BUTTON_B] = 1;
            break;
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            if (ballControl.cancerTimer == 0) {
                CalcBallTexture(0);
                *ballTextureData = (u32)ballTextureList[0]->data;
                *ballTexturePalette = (u32)ballTextureList[0]->palette;
                *ballTextureData2 = (u32)ballTextureList[6]->data;
                *ballTexturePalette2 = (u32)ballTextureList[6]->palette;
                *ballBitTextureData = (u32)ballTextureList[0]->data;
                *ballBitTexturePalette = (u32)ballTextureList[0]->palette;
            }
            ChangeBallInto(0);
            ball.flags &= ~ball.phys->flags;
            ball.phys = &defaultBehaviour[0];
            ball.flags |= ball.phys->flags;
            ball.obj->spheres[0].targetRadius = 9.0f;
            ball.friction = ball.phys->friction;
            for (f = 0; f < 4; f++) {
                plaster[f].alpha = 100;
            }
            break;
        case 13:
            ballControl.orbiter.inst = NULL;
            cheatActivated &= ~0x40000;
            SubActor(&ballControl.orbiter);
            ballControl.returnTime = 0;
            ball.flags |= 1;
            break;
        case 14:
            ballControl.orbiter.inst = NULL;
            cheatActivated &= ~0x80000;
            SubActor(&ballControl.orbiter);
            if (ball.radius <= 0.1 && handControl.deathTimer == 0) {
                DeactivateCollectable(POWERBALL);
                DeactivateCollectable(BOWLINGBALL);
                DeactivateCollectable(BEACHBALL);
                DeactivateCollectable(SNOWBALL);
                DeactivateCollectable(CRYSTALBALL);
                DeactivateCollectable(BALLBEARING);
                ball.obj->spheres[0].targetRadius = 9.0f;
                ball.vel = zero;
                ball.prevVel = zero;
                if (player.powerups[ROTORBLADES]) {
                    ball.link = &hand;
                    handControl.releaseTimer = 0;
                    ballControl.releaseTimer = 0;
                    return;
                }
                ball.pos = hand.pos;
                ball.prevPos = hand.pos;
                ball.platPrevPos = hand.pos;
                ApplyAction(HAND_JOINED);
            }
            break;
    }
}

void DeactivateAllCollectables(void) {
    s32 j;

    for (j = 0; j < 15; j++) {
        DeactivateCollectable(j);
    }
}

Collectable *CreateNewPickup(u8 type, Vec3 *pos, s16 time, s16 duration) {
    Collectable *collectable = JallocAlloc(sizeof(Collectable), 1, "Collect");
    s32 platform;

    collectable->type = type;
    collectable->actor.pos = *pos;
    collectable->respawnTime = time;
    collectable->duration = duration;
    FindObject(&collectable->actor.inst, UpdateCRC(collectableFileName[collectable->type]), NULL, 1);
    InitActorStructures(&collectable->actor, 12);
    AnimateActor(&collectable->actor, 0, 1, 0, 0.0f);
    collectable->scaleTarget = 0.4f;
    collectable->actor.scale.x = collectable->scaleTarget;
    collectable->actor.scale.y = collectable->scaleTarget;
    collectable->actor.scale.z = collectable->scaleTarget;
    collectable->alphaTarget = 101;
    collectable->actor.type = COLLECTABLE_ACTOR;
    collectable->actor.rotQuat = zeroQuat;
    collectable->radiusSq = 400.0f;
    collectable->active = 1;
    collectable->actor.flags = 0x10104;
    collectable->actor.shadow->altitude = Altitude(&collectable->actor.pos, &collectable->actor.shadow->floor, &platform, 0);
    collectable->actor.shadow->shadowSize = 10.0f;
    collectable->actor.shadow->shadowTarget = 255;
    collectable->actor.shadow->shadowStep = 30;
    AddActor(&collectable->actor);
    AddCollectable(collectable);
    return collectable;
}

inline void GainExtraLife(void) {
    stats[LIVES_STATS].scale = 0x800;
    player.lives++;
    PlaySampleNot3D(143, 100, 128, 128);
    PlaySampleNot3D(269, 100, 128, 128);
}

void GainGarib(Vec3 *pos, s16 pitch, s16 fx) {
    PlaySample(fx, pos, 100, pitch);
    if (++player.garibs % 10 == 0 && player.health < 3) {
        player.health++;
        stats[HEALTH_STATS].scale = 0x800;
    }
    if (player.garibs % difficultyStats[saveSlot[currentSaveSlot].difficultyLevel].garibsPerLife == 0) {
        GainExtraLife();
    }
    stats[GARIB_STATS].scale = 0x800;
    if (player.garibs == numGaribs) {
        AddToScore(5000);
        PlaySample(272, &hand.pos, 100, 128);
        if (!handControl.frog && gameInfo.level != CARNIVALBONUS && hand.state->mode != HAND_BALLWALK && hand.state->mode != HAND_ROTOR
            && !IsHandHoldingBall() && hand.link == NULL) {
            AnimateActor(&hand, HANDANIM_OKAY, 0, 0, 0);
            hand.rotAim.y = camera.rot.y;
            hand.state->mode = HAND_NOTHING;
            handControl.actionLock = 1013;
            DisableControl(20);
            SCALE_VECTOR(&hand.vel, 0.5);
        }
    }
}

void AddToScore(s32 scoreAdd) {
    s32 score = player.score + scoreAdd;

    if (score / difficultyStats[saveSlot[currentSaveSlot].difficultyLevel].lifeScore >
        player.score / difficultyStats[saveSlot[currentSaveSlot].difficultyLevel].lifeScore) {
        GainExtraLife();
    }
    player.score = score;
}
