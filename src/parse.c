#include "common.h"
#include "ident.h"
#include "actor.h"
#include "platform.h"
#include "cameo.h"
#include "jalloc.h"
#include "collect.h"
#include "debug.h"
#include "texture.h"
#include "maths.h"
#include "spike.h"
#include "vent.h"
#include "blur.h"
#include "buzzer.h"
#include "save.h"
#include "game.h"
#include "hand.h"
#include "camera.h"
#include "puzzle.h"
#include "tip.h"
#include "wind.h"
#include "water.h"
#include "light.h"
#include "ball.h"
#include "overlay.h"
#include "banks.h"
#include "debris.h"
#include "sound.h"
#include "parse.h"
#include "anim.h"
#include "collision.h"
#include "enemy.h"
#include "nmeprocess.h"
#include "object.h"
#include "personal.h"

extern char tempName[];
extern Quat zeroQuat;

extern Cameo *lastCameo;
extern GaribGroup *parseGaribGroup;
extern SpriteAnim garibAnimation[];
extern s32 cheatActivated;
extern s32 numGaribs;
void InitSpriteAnimation(GameSprite *sprite, SpriteAnim *temp, s16 lifespan);
struct Collectable *CreateNewPickup(s32 type, Vec3 *pos, s16 time, s16 duration);
Platform *TagToFirstPlatform(s32 tag);
extern s16 spikeOrPlatformOrVent;
extern Vent *lastVent;
extern Actor *lastLandscape;
extern s16 ropeTag;
extern s16 numRopeBridges;
void AnimateActor(Actor *actor, s32 animNum, s32 loop, s32 queue, f32 speed);
extern f32 offsetY;
extern Buzzer *buzzer;
extern s16 numRestartGates;
extern s16 currentSaveSlot;
extern Puzzle *parsePuzzle;
extern f32 gravity;
extern Vec3 handStart;
extern Vec3 ballStart;
extern s32 ballLeftBehind;
extern s32 ballWithHand;
extern Fog originalFog;
extern s16 farPlaneDist;
extern u8 hubBackdropColour[][4];
extern CollSphere *cameoExtraSphere;
extern CollSphere cameoCameraSphere;
extern CollSphere cameoCameraTargetSphere;
extern s16 numCamExtraSpheres;
extern Texture *tipTexture;
void SetupLight(GameLight *light, u8 initR, u8 initG, u8 initB, u8 r, u8 g, u8 b, f32 x, f32 y, f32 speed);
void AddBackgroundLight(GameLight *light);
void FindTexture(void *texPtr, u32 texID, s32 verbose);
extern PuzzleAction *parseAction;
extern PuzzleCond *parseCondition;

INCLUDE_RODATA("asm/nonmatchings/parse", D_80108BC4);

extern s32 ballStatic;
extern GaribGroup *NMEGaribGroup;
extern s32 numSmallBubbles;
extern s32 numBubbles;
extern Vec3 ballEnd;
extern s32 numberOfEnemies;
extern char outputMessageBuffer[];
u32 UpdateCRC(char *ptr);
void NMEGetNewInstruction(Enemy *enemy, s32 instructionNum, s32 overrideConditions);
void DestroyAndFreeActor(Actor *actor);
f32 Altitude(Vec3 *pos, Vec3 **floor, Platform **plat, s32 coll);

void ParseLandscapeFile(s32 num) {
    s16 val[4]; // an 8-byte buffer: it is given the first stack slot, as in the original
    u8 *data;
    u32 ival;
    s16 nme;
    f32 x;
    f32 y;
    f32 z;
    f32 w;
    u8 *name;
    Platform *platform;
    GaribGroup *garibGroup;
    s32 i;
    s16 token;
    Actor *actor;
    Actor *actor2;
    Collectable *collectable;
    Garib *garib;
    GaribGroup *garibGroup2;
    Tip *tip;
    f32 dist;
    f32 radius;

    ropeTag = 10000;
    numRopeBridges = 0;
    numRestartGates = 0;
    ballStatic = 0;
    NMEGaribGroup = NULL;
    handControl.fakeball = NULL;
    numSmallBubbles = 0;
    numBubbles = 0;
    if (num == 42 && ballLeftBehind >= 42) {
        ballLeftBehind = num;
    }
    if (ballLeftBehind == num) {
        ball.pos = ballEnd;
        ballWithHand = FALSE;
        ballStatic = 0;
    } else if (ballLeftBehind != -1) {
        ballStatic = 2;
        ball.pos.x = 0.0f;
        ball.pos.z = 0.0f;
        ballStart.x = 0.0f;
        ballStart.z = 0.0f;
        ball.link = NULL;
        ballWithHand = FALSE;
        ballStart.y = ball.pos.y = -5000.0f;
    } else {
        ballWithHand = TRUE;
    }
    if (num == 9) {
        ballLeftBehind = -1;
        ballWithHand = TRUE;
    }
    data = LoadLandscapeFile(num);
    for (i = 0; i < 10; i++) {
        memclear(&ropeBridge[i], sizeof(RopeBridge));
    }
    InitCollectableList();
    InitDebris();
    if (data == NULL) {
        return;
    }
    name = data;
    data += 4;
    PRINTF("\nParsing landscape file %s\n", data);
    data += strlen(data) + 1;
    while ((token = *(s16 *)data) != 0x7D00) {
        data += 2;
        if (gameInfo.mode == 13) {
            break;
        }
        if (ParseTokenGeneric(token, &data) || ParseTokenCameo(token, &data) || ParseTokenPuzzle(token, &data)
            || ParseTokenCollectable(token, &data) || ParseTokenPlatformInteraction(token, &data, platform)
            || ParseTokenPlatformFeatures(token, &data, platform) || ParseTokenPlatformOrbit(token, &data, platform)
            || ParseTokenPlatformSpin(token, &data, platform) || ParseTokenPlatformPath(token, &data, platform)
            || ParseTokenPlatformGeneric(token, &data, platform) || ParseTokenBridge(token, &data)) {
            continue;
        }
        if (token == TOKEN_VENT) {
            spikeOrPlatformOrVent = 0xA2;
            lastVent = JallocAlloc(sizeof(Vent), 1, "Vent");
            CopyFromData(&lastVent->type, &data, 2);
            CopyFromData(&lastVent->tag, &data, 2);
            CopyFromData(val, &data, 2);
            if (val[0]) {
                lastVent->parent = TagToFirstPlatform(val[0]);
            }
            CopyFromData(&lastVent->origin, &data, 12);
            CopyFromData(&lastVent->velocity, &data, 12);
            lastVent->active = TRUE;
            switch (lastVent->type) {
                case 9:
                    lastVent->bulletType = CANNONBALL_BULLET;
                    break;
                case 0:
                    lastVent->bulletType = FLAME_BULLET;
                    if (lastVent->parent != NULL) {
                        ADD_VECTORS(&lastVent->soundPos, &lastVent->origin, &lastVent->parent->actor.pos);
                    } else {
                        lastVent->soundPos = lastVent->origin;
                    }
                    PlayContinuousSample(&lastVent->sound, 36, 100, &lastVent->soundPos, 100);
                    break;
                case 1:
                case 2:
                    lastVent->bulletType = STEAM_BULLET;
                    break;
                case 3:
                    lastVent->bulletType = SMOKE_BULLET;
                    break;
                case 4:
                    lastVent->bulletType = SNOWBALL_BULLET;
                    break;
                case 5:
                    lastVent->bulletType = BOULDER_BULLET;
                    break;
                case 6:
                    lastVent->bulletType = ICICLE_BULLET;
                    break;
                case 7:
                    lastVent->bulletType = FLOATY_BULLET;
                    break;
                case 10:
                    lastVent->bulletType = WATER_BULLET;
                    break;
            }
            AddVent(lastVent);
        } else if (token == TOKEN_VENT_PATTERN) {
            CopyFromData(&lastVent->framesOff[lastVent->numCycles], &data, 2);
            CopyFromData(&lastVent->framesOn[lastVent->numCycles], &data, 2);
            if (lastVent->numCycles == 0) {
                if (lastVent->framesOn[0] == -1) {
                    lastVent->on = TRUE;
                }
                lastVent->timer = lastVent->framesOff[0];
            }
            lastVent->numCycles++;
        } else if (token == TOKEN_PLATFORM) {
            offsetY = 0.0f;
            spikeOrPlatformOrVent = 0x62;
            platform = JallocAlloc(sizeof(Platform), 1, "Platform");
            AddPlatform(platform);
            InitActorStructures(&platform->actor, 0);
            CopyFromData(&ival, &data, 4);
            CopyFromData(tempName, &data, 8);
            FindObject(&platform->actor.inst, ival, tempName, 1);
            if (ival == UpdateCRC("exitpost.ndo")) {
                platform->actor.radius = 20.0f;
                InitActorStructures(&platform->actor, 4);
                AnimateActor(&platform->actor, 0, 1, 0, 0);
                platform->actor.scale.x = platform->actor.scale.z = 1.5f;
                platform->actor.scale.y = 2.0f;
            }
            platform->actor.faces =
                JallocAlloc(platform->actor.inst->obj->mesh->numFaces * sizeof(CollFace), 1, "Plat_Plane");
            platform->actor.flags = 0x10004;
            platform->actor.type = PLAT_ACTOR;
            AddActor(&platform->actor);
            platform->actor.rotQuat = zeroQuat;
            platform->actor.friction = 1.0f;
        } else if (token == TOKEN_DUMMY) {
            platform = JallocAlloc(sizeof(Platform), 1, "Platform");
            platform->actor.rotQuat = zeroQuat;
            AddPlatform(platform);
            platform->actor.type = PLAT_ACTOR;
            platform->flags |= 0x80;
            InitActorStructures(&platform->actor, 0);
            platform->actor.friction = 1.0f;
        } else if (token == TOKEN_ENEMY) {
            CopyFromData(&nme, &data, 2);
            CopyFromData(val, &data, 2);
            CopyFromData(&x, &data, 4);
            CopyFromData(&y, &data, 4);
            CopyFromData(&z, &data, 4);
            CopyFromData(&w, &data, 4);
            LoadEnemy(nme, x, y, z, val[0], w);
            enemies[numberOfEnemies - 1].brain.area.size.x = -1.0f;
            if (enemyStats[nme].flags & 0x40) {
                numGaribs++;
            }
        } else if (token == TOKEN_CONFINE_ENEMY) {
            CopyFromData(&enemies[numberOfEnemies - 1].brain.area, &data, 0x18);
        } else if (token == TOKEN_DISABLE_REDIRECTION) {
            enemies[numberOfEnemies - 1].brain.flags |= 4;
        } else if (token == TOKEN_END_ENEMY) {
            NMEGetNewInstruction(&enemies[numberOfEnemies - 1], 0, 0);
        } else if (token == TOKEN_NORMAL_INSTRUCTION) {
            NMEAddNormalInstruction(&data);
        } else if (token == TOKEN_CONDITIONAL_INSTRUCTION) {
            NMEAddConditionalInstruction(&data);
        } else if (token == TOKEN_ATTACK_INSTRUCTION) {
            NMEAddAttackInstruction(&data);
        }
    }
    JallocFree(&name);
    if (gameInfo.mode == 13) {
        return;
    }
    for (actor = actorList.head.next; actor != &actorList.head; actor = actor2) {
        actor2 = actor->next;
        if (actor->type == LANDSCAPE_ACTOR || actor->type == WATER_ACTOR) {
            if (actor->inst->obj->mesh->numFaces > 0) {
                actor->faces = JallocAlloc(actor->inst->obj->mesh->numFaces * sizeof(CollFace), 1, "Obj_Planes");
                actor->radius = InitPlanes(actor->inst->obj->mesh, &actor->pos, actor->faces, &actor->scale,
                                           &actor->rotQuat, actor->type == LANDSCAPE_ACTOR && !(actor->flags & 4));
                if (actor->type == LANDSCAPE_ACTOR) {
                    SetActorTerrain(actor, actor->lifeTimer);
                } else {
                    JallocFree(&actor->faces);
                }
            } else {
                PRINTF("WARNING - zero poly object %s\n\n", actor->inst->obj->name);
                DestroyAndFreeActor(actor);
            }
        }
    }
    for (platform = platformList.head.next; platform != &platformList.head; platform = platform->next) {
        if (platform->flags & 0x20000) {
            platform->flags &= ~0x20000;
        } else if (!platform->path && !platform->motion && !platform->orbit && !platform->push && !platform->scale
                   && !platform->link && !platform->crumble && !platform->parent) {
            platform->flags |= 0x20000;
        }
    }
    for (collectable = collectableList.head.next; collectable != &collectableList.head; collectable = collectable->next) {
        collectable->actor.shadow->altitude = Altitude(&collectable->actor.pos, &collectable->actor.shadow->floor, &platform, 0);
    }
    for (garibGroup = garibGroupList.head.prev; garibGroup != &garibGroupList.head; garibGroup = garibGroup2) {
        garibGroup2 = garibGroup->prev;
        if (garibGroup->garibs.count == 0) {
            SubGaribGroup(garibGroup);
            JallocFree(&garibGroup);
        } else {
            ZERO_VECTOR(&garibGroup->pos);
            for (garib = garibGroup->garibs.head.prev; garib != &garibGroup->garibs.head; garib = garib->prev) {
                ADD_TO_VECTOR(&garibGroup->pos, &garib->sprite.pos);
                if (garib->active) {
                    garib->altitude = Altitude(&garib->sprite.pos, &garib->normal, &platform, 0);
                }
            }
            SCALE_VECTOR(&garibGroup->pos, 1.0f / garibGroup->garibs.count);
            radius = 1.0f;
            for (garib = garibGroup->garibs.head.prev; garib != &garibGroup->garibs.head; garib = garib->prev) {
                dist = DistanceBetweenPoints(&garibGroup->pos, &garib->sprite.pos);
                if (radius < dist) {
                    radius = dist;
                }
            }
            garibGroup->radius = radius + 30.0f;
        }
    }
    for (tip = tipList.head.next; tip != &tipList.head; tip = tip->next) {
        tip->altitude = Altitude(&tip->pos, &tip->normal, &platform, 0);
    }
}

inline void SetPlatformTerrain(Platform *platform, s8 terrainType) {
    s32 k;
    CollFace *face;

    for (k = 0; k < platform->actor.inst->obj->mesh->numFaces; k++) {
        face = &platform->actor.faces[k];
        face->terrain = terrainType;
    }
}

void SetActorTerrain(Actor *actor, u8 terrainType) {
    s32 k;
    CollFace *face;

    for (k = 0; k < actor->inst->obj->mesh->numFaces; k++) {
        face = &actor->faces[k];
        face->terrain = terrainType;
    }
}

void PlatformAddPoint(Platform *platform, f32 x, f32 y, f32 z, s32 counter) {
    if (!platform->path) {
        platform->path = JallocAlloc(sizeof(PlatformPath), 1, "Plat_Path");
    }
    if (platform->path->numPoints < 20) {
        platform->path->dir = 1;
        if (!platform->orbit) {
            platform->path->points[platform->path->numPoints] = JallocAlloc(sizeof(PathPoint), 1, "Path_Point");
            if (!counter) {
                platform->path->points[platform->path->numPoints]->time = 1;
            } else {
                platform->path->points[platform->path->numPoints]->time = counter;
            }
            platform->path->points[platform->path->numPoints]->pos.x = x;
            platform->path->points[platform->path->numPoints]->pos.y = y;
            platform->path->points[platform->path->numPoints]->pos.z = z;
            platform->path->numPoints++;
            if (platform->path->numPoints == 1) {
                SetPlatformPos(platform, &platform->path->points[0]->pos, 0.0f);
                platform->path->moving = counter;
                if (!platform->path->moving) {
                    platform->path->moving++;
                }
            }
        } else {
            PRINTF("WARNING!! Platform %d cannot follow path - it is an orbiter\n", platformList.count);
        }
    } else {
        PRINTF("WARNING!! Platform %d has too many POINTS!\n", platformList.count);
    }
}

inline void SetPlatformPos(Platform *platform, Vec3 *pos, f32 offsetY) {
    platform->actor.pos = *pos;
    if (platform->parent) {
        ADD_TO_VECTOR(&platform->actor.pos, &platform->parent->actor.pos);
    }
    platform->actor.prevPos = platform->actor.pos;
    InitPlatformInfo(platform, offsetY);
}

inline void CopyFromData(void *dst, u8 **src, s32 size) {
    memcpy(dst, *src, size);
    *src += size;
}

inline void InitActor2(Actor *tempActor, u32 obj, f32 x, f32 y, f32 z, s32 initFlags) {
    FindObject(&tempActor->inst, obj, tempName, 1);
    if (tempActor->inst) {
        InitActorStructures(tempActor, initFlags);
        tempActor->pos.x = x;
        tempActor->pos.y = y;
        tempActor->pos.z = z;
        tempActor->scale.x = tempActor->scale.y = tempActor->scale.z = 1.0f;
        tempActor->rotQuat = zeroQuat;
        if (!(tempActor->flags & 4) && tempActor->inst->obj->mesh->numFaces == 0) {
            tempActor->flags |= 4;
        }
    }
}

s32 ParseTokenCameo(s16 token, u8 **data) {
    CameoItem *inst;

    if (token == TOKEN_CAMEO) {
        lastCameo = JallocAlloc(sizeof(Cameo), 1, "Cameo");
        AddCameo(lastCameo);
        return TRUE;
    }
    if (token == TOKEN_CAMEO_INSTRUCTION) {
        inst = JallocAlloc(sizeof(CameoItem), 1, "CameoInst");
        CopyFromData(&inst->type, data, 2);
        switch (inst->type) {
            case 0:
                CopyFromData(&inst->target, data, 2);
                CopyFromData(&inst->u.args[0], data, 2);
                CopyFromData(&inst->u.args[1], data, 2);
                CopyFromData(&inst->speed, data, 4);
                break;
            case 1:
                CopyFromData(&inst->target, data, 2);
                CopyFromData(&inst->u.pos, data, 12);
                break;
            case 5:
                CopyFromData(&inst->target, data, 2);
                break;
            case 2:
                CopyFromData(&inst->target, data, 2);
                CopyFromData(&inst->u.pos, data, 12);
                CopyFromData(&inst->speed, data, 4);
                break;
            case 3:
                CopyFromData(&inst->target, data, 2);
                CopyFromData(&inst->u.pos.x, data, 4);
                break;
            case 4:
                CopyFromData(&inst->target, data, 2);
                CopyFromData(&inst->u.args[0], data, 2);
                CopyFromData(&inst->u.args[1], data, 2);
                break;
            case 6:
                CopyFromData(&inst->u.args[0], data, 2);
                CopyFromData(&inst->u.args[1], data, 2);
                break;
        }
        CopyFromData(&inst->duration, data, 2);
        CopyFromData(&inst->trigger, data, 2);
        lastCameo->items[lastCameo->count] = inst;
        lastCameo->count++;
        return TRUE;
    }
    return FALSE;
}

s32 ParseTokenGeneric(s16 token, u8 **data) {
    s16 sval[8];
    Vec3 rot;
    f32 x;
    f32 y;
    f32 z;
    u32 ival;
    s32 crc;
    Tip *tip;
    GameLight *light;
    Object *tempObj;
    Actor *tempActor;
    Wind *wind;
    Water *water;
    Backdrop *backdrop;

    if (token == TOKEN_AMBIENT_SOUND) {
        CopyFromData(sval, data, 6);
        if (gameInfo.mode != 15) {
            AddAmbientSfx(sval[0], sval[1], sval[2]);
        }
        return TRUE;
    }
    if (token == TOKEN_SOUND) {
        CopyFromData(sval, data, 16);
        CopyFromData(&rot, data, 12);
        CopyFromData(&x, data, 4);
        if (gameInfo.mode != 15) {
            AddAmbientSfxAtPoint(sval[0], sval[1], &rot, sval[2], sval[3], sval[4], sval[5], sval[6], sval[7], x);
        }
        return TRUE;
    }
    if (token == TOKEN_LEVEL_GRAVITY) {
        CopyFromData(&gravity, data, 4);
        return TRUE;
    }
    if (token == TOKEN_TIP) {
        CopyFromData(&x, data, 4);
        CopyFromData(&y, data, 4);
        CopyFromData(&z, data, 4);
        CopyFromData(sval, data, 2);
        tip = JallocAlloc(sizeof(Tip), 1, "tip");
        tip->state = 0;
        tip->tag = sval[0];
        if (saveSlot[currentSaveSlot].difficultyLevel == DIFFICULTY_EASY && tip->tag == 14) {
            tip->tag = 32;
        }
        tip->pos.x = x;
        tip->pos.y = y;
        tip->pos.z = z;
        tip->timer = Random(100);
        tip->shadowAlpha = 255;
        tip->sprite.flags |= 1;
        if (saveSlot[currentSaveSlot].difficultyLevel != DIFFICULTY_EASY || tip->tag != 31) {
            AddTip(tip);
            tip->sprite.tex = tipTexture;
            tip->sprite.pos = tip->pos;
            tip->sprite.scaleX = tip->sprite.scaleY = 44;
            tip->sprite.r = tip->sprite.g = tip->sprite.b = tip->sprite.alpha = 255;
            tip->sprite.offsetX = -tip->sprite.tex->width / 2;
            tip->sprite.offsetY = -tip->sprite.tex->height / 2;
            AddSprite(&tip->sprite, NULL);
        }
        return TRUE;
    }
    if (token == TOKEN_LIGHT) {
        CopyFromData(sval, data, 6);
        CopyFromData(&x, data, 4);
        CopyFromData(&y, data, 4);
        light = JallocAlloc(sizeof(GameLight), 1, "Light");
        SetupLight(light, sval[0], sval[1], sval[2], sval[0], sval[1], sval[2], x, y, 1.0f);
        AddBackgroundLight(light);
        light->timer = -1;
        light->type = 0;
        light->turnSpeed = 0.2f;
        return TRUE;
    }
    if (token == TOKEN_AMBIENT_LIGHT) {
        CopyFromData(sval, data, 6);
        SetupAmbientLight(&bgLighting.ambient, sval[0], sval[1], sval[2]);
        return TRUE;
    }
    if (token == TOKEN_HAND_START) {
        CopyFromData(&handStart, data, 12);
        hand.pos = handStart;
        return TRUE;
    }
    if (token == TOKEN_BALL_START) {
        CopyFromData(sval, data, 2);
        if (ballLeftBehind != gameInfo.level
            && (sval[0] != 2 || saveSlot[currentSaveSlot].hubTarget == saveSlot[currentSaveSlot].hubLevel)) {
            CopyFromData(&ballStart, data, 12);
            if (sval[0] == 3 || (ballLeftBehind != -1 && ballLeftBehind != gameInfo.level)) {
                ballStatic = 2;
            } else {
                ballStatic = sval[0];
            }
            ball.pos = ballStart;
            ballWithHand = FALSE;
            ball.link = NULL;
        }
        if (gameInfo.level == WAYROOM && ballStatic == 0) {
            ballWithHand = TRUE;
        }
        return TRUE;
    }
    if (token == TOKEN_CAMERA_START) {
        CopyFromData(&camera.pos, data, 12);
        CopyFromData(&camera.rot, data, 8);
        cameraInfo.actor.pos = hand.pos;
        return TRUE;
    }
    if (token == TOKEN_FOG) {
        CopyFromData(&fog.on, data, 1);
        CopyFromData(&fog.r, data, 1);
        CopyFromData(&fog.g, data, 1);
        CopyFromData(&fog.b, data, 1);
        CopyFromData(&fog.min, data, 2);
        CopyFromData(&farPlaneDist, data, 2);
        AdjustColours(&fog.r, &fog.g, &fog.b);
        originalFog = fog;
        return TRUE;
    }
    if (token == TOKEN_CAMEO_CAM) {
        CopyFromData(sval, data, 2);
        CopyFromData(&ival, data, 4);
        tempObj = FindSubObjectFromObject(lastLandscape->inst->obj, ival);
        if (sval[0] == 0) {
            cameoExtraSphere = JallocAlloc(sizeof(CollSphere) * 10, 1, "CamColSph");
            numCamExtraSpheres = 0;
            tempObj->sphere = &cameoCameraSphere;
            ZERO_VECTOR(&cameoCameraSphere.pos);
        } else if (sval[0] == 1) {
            tempObj->sphere = &cameoCameraTargetSphere;
            tempObj->sphere->pos.x = 0.0f;
            tempObj->sphere->pos.y = 0.0f;
            tempObj->sphere->pos.z = 100.0f;
        } else {
            tempObj->sphere = &cameoExtraSphere[numCamExtraSpheres++];
            tempObj->sphere->pos.x = 0.0f;
            tempObj->sphere->pos.y = 0.0f;
            tempObj->sphere->pos.z = 0.0f;
        }
        tempObj->sphere->flags = 0x10;
        tempObj->sphere->radius = 0.0f;
    }
    if (token == TOKEN_BACKGROUND || token == TOKEN_LANDSCAPE || token == TOKEN_CAMEO_OBJ) {
        if (token == TOKEN_BACKGROUND || token == TOKEN_CAMEO_OBJ) {
            tempActor = JallocAlloc(sizeof(Actor), 1, "Bkg_Actor");
            tempActor->flags = 0x10004;
            spikeOrPlatformOrVent = token;
            lastLandscape = tempActor;
        } else if (token == TOKEN_LANDSCAPE) {
            tempActor = JallocAlloc(sizeof(Actor), 1, "Land_Actor");
            tempActor->flags = 0x10000;
            spikeOrPlatformOrVent = 0x92;
            lastLandscape = tempActor;
        }
        CopyFromData(&crc, data, 4);
        CopyFromData(tempName, data, 8);
        CopyFromData(&x, data, 4);
        CopyFromData(&y, data, 4);
        CopyFromData(&z, data, 4);
        if (token == TOKEN_CAMEO_OBJ) {
            tempActor->type = CAMEO_ACTOR;
            InitActor2(tempActor, crc, x, y, z, 4);
            if (gameInfo.level < INTRO && gameInfo.level != CARNIVALBOSS) {
                AnimateActor(tempActor, 0, 1, 0, 1.0f);
            } else {
                AnimateActor(tempActor, 0, 0, 0, 1.0f);
            }
        } else {
            tempActor->type = LANDSCAPE_ACTOR;
            InitActor2(tempActor, crc, x, y, z, 0);
        }
        AddActor(tempActor);
        return TRUE;
    }
    if (token == TOKEN_LANDROT) {
        CopyFromData(&rot, data, 12);
        GetQuaternionFromXYZRot(&lastLandscape->rotQuat, &rot);
        return TRUE;
    }
    if (token == TOKEN_LANDSCALE) {
        CopyFromData(&lastLandscape->scale, data, 12);
        return TRUE;
    }
    if (token == TOKEN_WIND) {
        wind = JallocAlloc(sizeof(Wind), 1, "Wind");
        CopyFromData(wind->pos, data, 12);
        CopyFromData(wind->size, data, 12);
        CopyFromData(wind->dir, data, 12);
        CopyFromData(&wind->strength, data, 4);
        CopyFromData(&wind->radius, data, 4);
        CopyFromData(&wind->type, data, 4);
        CopyFromData(&wind->tag, data, 4);
        AddWind(wind);
        return TRUE;
    }
    if (token == TOKEN_WATER) {
        water = JallocAlloc(sizeof(Water), 1, "Water");
        CopyFromData(&water->box.pos, data, 12);
        CopyFromData(&water->box.size, data, 12);
        CopyFromData(&water->height, data, 4);
        CopyFromData(&water->currentX, data, 4);
        CopyFromData(&water->currentZ, data, 4);
        CopyFromData(&water->tag, data, 2);
        AddWater(water);
        water->targetHeight = water->height;
        if (water->height - 25.0f < water->box.pos.y) {
            water->box.pos.y = water->height - 25.0f;
        }
        if (water->box.size.y < water->height + 25.0f - water->box.pos.y) {
            water->box.size.y = water->height + 25.0f - water->box.pos.y;
        }
        tempActor = JallocAlloc(sizeof(Actor), 1, "Wtr_Actor");
        tempActor->type = WATER_ACTOR;
        tempActor->flags |= 0x10004;
        CopyFromData(&crc, data, 4);
        CopyFromData(tempName, data, 8);
        CopyFromData(&x, data, 4);
        CopyFromData(&y, data, 4);
        CopyFromData(&z, data, 4);
        InitActor2(tempActor, crc, x, y, z, 0);
        AddActor(tempActor);
        water->actor = tempActor;
        tempActor->inst->obj->flags |= 0x20;
        return TRUE;
    }
    if (token == TOKEN_BACKDROP) {
        backdrop = JallocAlloc(sizeof(Backdrop), 1, "Backdrop");
        AddBackdrop(backdrop);
        CopyFromData(&ival, data, 4);
        FindTexture(&backdrop->tex, ival, 1);
        backdrop->tex->height--;
        CopyFromData(sval, data, 6);
        backdrop->offsetX = sval[0];
        backdrop->offsetY = sval[1];
        backdrop->depth = sval[2];
        CopyFromData(&backdrop->yBias, data, 2);
        CopyFromData(&backdrop->scaleX, data, 2);
        CopyFromData(&backdrop->scaleY, data, 2);
        CopyFromData(&backdrop->flipX, data, 2);
        CopyFromData(&backdrop->flipY, data, 2);
        CopyFromData(&backdrop->wrap, data, 2);
        CopyFromData(&backdrop->unk64, data, 2);
        CopyFromData(sval, data, 2);
        if (gameInfo.level >= 0 && gameInfo.level < 8) {
            backdrop->r = hubBackdropColour[gameInfo.level][0];
            backdrop->g = hubBackdropColour[gameInfo.level][1];
            backdrop->b = hubBackdropColour[gameInfo.level][2];
            backdrop->alpha = 255;
        } else {
            backdrop->r = 255;
            backdrop->g = 255;
            backdrop->b = 255;
            backdrop->alpha = 255;
        }
        if (sval[0] != 0) {
            backdrop->parent = &backdropList.head;
            for (crc = 0; crc < sval[0]; crc++) {
                backdrop->parent = backdrop->parent->next;
            }
        }
        return TRUE;
    }
    return FALSE;
}

s32 ParseTokenPuzzle(s16 token, u8 **data) {
    s16 sval[12]; // larger than needed to match the original stack frame
    s32 type;
    s32 id;

    if (token == TOKEN_PUZZLE) {
        parsePuzzle = NewPuzzle();
        return TRUE;
    }
    if (token == TOKEN_PUZZLE_AND) {
        if (parsePuzzle != NULL) {
            parsePuzzle->conds.requiredCount = 0;
        } else {
            PRINTF("WARNING!! Cannot have an AND without a PUZZLE\n");
        }
        return TRUE;
    }
    if (token == TOKEN_PUZZLE_OR) {
        if (parsePuzzle != NULL) {
            parsePuzzle->conds.requiredCount = 1;
        } else {
            PRINTF("WARNING!! Cannot have an OR without a PUZZLE\n");
        }
        return TRUE;
    }
    if (token == TOKEN_PUZZLE_NUMTIMES) {
        if (parsePuzzle != NULL) {
            CopyFromData(&parsePuzzle->conds.timesLeft, data, 2);
        } else {
            PRINTF("WARNING!! Cannot have a NUMTIMES without a PUZZLE\n");
        }
        return TRUE;
    }
    if (token == TOKEN_PUZZLE_ANY) {
        if (parsePuzzle != NULL) {
            CopyFromData(&parsePuzzle->conds.requiredCount, data, 2);
        } else {
            PRINTF("WARNING!! Cannot have an ANY without a PUZZLE\n");
        }
        return TRUE;
    }
    if (token == TOKEN_CONDITION) {
        if (parsePuzzle != NULL) {
            CopyFromData(sval, data, 2);
            id = sval[0];
            parseCondition = JallocAlloc(sizeof(PuzzleCond), 1, "Condition");
            type = id - 9;
            AddConditionToList(parseCondition, &parsePuzzle->conds);
            parseCondition->type = type;
            if (IsCameraCondition(type)) {
                parseCondition->cam = JallocAlloc(sizeof(PuzzleCamCond), 1, "CamCond");
                switch (id) {
                    case 34:
                        CopyFromData(&parseCondition->cam->pos, data, 12);
                        CopyFromData(&parseCondition->cam->size, data, 12);
                        CopyFromData(&parseCondition->cam->radius, data, 4);
                        break;
                    case 35:
                    case 37:
                    case 39:
                        CopyFromData(&parseCondition->cam->pos, data, 12);
                        CopyFromData(&parseCondition->cam->size, data, 12);
                        break;
                    case 36:
                    case 38:
                    case 40:
                        CopyFromData(&parseCondition->cam->pos, data, 12);
                        CopyFromData(&parseCondition->cam->radius, data, 4);
                        break;
                    case 41:
                        CopyFromData(&parseCondition->cam->pos, data, 12);
                        CopyFromData(&parseCondition->cam->size, data, 8);
                        break;
                }
            } else {
                CopyFromData(sval, data, 2);
                parseCondition->tag = sval[0];
                CopyFromData(&parseCondition->value, data, 2);
            }
        } else {
            PRINTF("WARNING!! Cannot have a CONDITION without a PUZZLE\n");
        }
        return TRUE;
    }
    if (token == TOKEN_ACTION) {
        if (parsePuzzle != NULL) {
            parseAction = JallocAlloc(sizeof(PuzzleAction), 1, "Action");
            CopyFromData(sval, data, 2);
            id = sval[0];
            type = id - 45;
            parseAction->type = type;
            parseAction->timer = -1;
            AddActionToList(parseAction, &parsePuzzle->actions);
            if (IsCameraAction(type)) {
                parseAction->cam = JallocAlloc(sizeof(PuzzleCamAction), 1, "CamAction");
                switch (id) {
                    case 0x56:
                        CopyFromData(parseAction->data, data, 10);
                        break;
                    case 0x55:
                        CopyFromData(&parseAction->cam->pos, data, 4);
                        break;
                    case 0x54:
                        CopyFromData(sval, data, 16);
                        CopyFromData(&parseAction->cam->pos, data, 12);
                        parseAction->data[0] = sval[0];
                        parseAction->data[1] = sval[1];
                        parseAction->data[2] = sval[2];
                        parseAction->data[3] = sval[3];
                        parseAction->data[4] = sval[4];
                        parseAction->data[5] = sval[5];
                        parseAction->value = sval[6];
                        parseAction->tag = sval[7];
                        break;
                    case 0x46:
                    case 0x47:
                    case 0x48:
                        CopyFromData(&parseAction->cam->pos, data, 4);
                        break;
                    case 0x4A:
                        CopyFromData(&parseAction->cam->pos, data, 4);
                        CopyFromData(&parseAction->cam->speed, data, 4);
                        break;
                    case 0x49:
                    case 0x4D:
                        CopyFromData(&parseAction->cam->pos, data, 12);
                        break;
                }
                CopyFromData(&parseAction->delay, data, 2);
            } else {
                if (type == 17 || type == 18 || type == 19 || type == 14 || type == 15 || type == 16 || type == 8) {
                    CopyFromData(parseAction->data, data, 12);
                } else if (type == 34) {
                    CopyFromData(parseAction->data, data, 8);
                }
                CopyFromData(&parseAction->value, data, 4);
                CopyFromData(&parseAction->tag, data, 2);
                CopyFromData(&parseAction->delay, data, 2);
                CopyFromData(&parseAction->flags, data, 4);
            }
        } else {
            PRINTF("WARNING!! Cannot have an ACTION without a PUZZLE\n");
        }
        return TRUE;
    }
    return FALSE;
}

s32 ParseTokenCollectable(s16 token, u8 **data) {
    u16 shortval[2];
    Vec3 tempVect;
    char mess[16];
    s16 sval;
    s16 sval2;
    s16 sval3;
    Garib *garib;

    if (token == TOKEN_GARIBGROUP) {
        sprintf(mess, "GaribGp%d", garibGroupList.count);
        parseGaribGroup = JallocAlloc(sizeof(GaribGroup), 1, mess);
        AddGaribGroup(parseGaribGroup);
        InitGaribList(&parseGaribGroup->garibs);
        parseGaribGroup->suspended = 1;
        CopyFromData(&parseGaribGroup->tag, data, 2);
        CopyFromData(&sval, data, 2);
        parseGaribGroup->startActive = sval;
        return TRUE;
    }
    if (token == TOKEN_GARIB) {
        CopyFromData(&tempVect, data, 12);
        CopyFromData(shortval, data, 4);
        garib = JallocAlloc(sizeof(Garib), 1, "Garib");
        AddGarib(parseGaribGroup, garib);
        garib->sprite.pos = tempVect;
        garib->type = shortval[0];
        garib->mode = shortval[1];
        garib->altitude = -1.0f;
        garib->active = parseGaribGroup->startActive;
        garib->scale = 1.0f;
        garib->scaleTarget = 1.0f;
        if (garib->type == NORMAL_GARIB) {
            if (cheatActivated & 0x2000) {
                InitSpriteAnimation(&garib->sprite, &garibAnimation[3], 0);
            } else {
                InitSpriteAnimation(&garib->sprite, &garibAnimation[0], 0);
            }
        } else {
            InitSpriteAnimation(&garib->sprite, &garibAnimation[garib->type], 0);
        }
        garib->sprite.r = 255;
        garib->sprite.g = 255;
        garib->sprite.b = 255;
        garib->sprite.offsetX = -garib->sprite.tex->width / 2;
        garib->sprite.offsetY = -garib->sprite.tex->height / 2;
        if (garib->active) {
            AddSprite(&garib->sprite, NULL);
        }
        if (garib->type != EXTRA_LIFE) {
            numGaribs++;
        }
        return TRUE;
    }
    if (token == TOKEN_PICKUP) {
        CopyFromData(&sval, data, 2);
        CopyFromData(&sval2, data, 2);
        CopyFromData(&sval3, data, 2);
        CopyFromData(&tempVect, data, 12);
        if (sval != 7 && sval != 9) {
            CreateNewPickup(sval, &tempVect, sval2, sval3);
        }
        return TRUE;
    }
    return FALSE;
}

s32 ParseTokenPlatformInteraction(s16 token, u8 **data, Platform *platform) {
    f32 d[2];
    s16 buf[8]; // larger than needed to match the original stack frame
    Platform *plat;
    s32 axis;
    s32 k;
    s32 n;
    f32 dist;
    f32 ang;

    if (token == TOKEN_SPINTOMOVE) {
        if (platform->link == NULL) {
            platform->link = JallocAlloc(sizeof(PlatformLink), 1, "Plat_MvSpn");
        }
        CopyFromData(&platform->link->spinToMoveTag, data, 2);
        CopyFromData(&platform->link->spinToMoveScale, data, 4);
        return TRUE;
    }
    if (token == TOKEN_MOVETOSPIN) {
        if (platform->link == NULL) {
            platform->link = JallocAlloc(sizeof(PlatformLink), 1, "Plat_MvSpn");
        }
        CopyFromData(&platform->link->moveToSpinTag, data, 2);
        CopyFromData(&platform->link->moveToSpinScale, data, 4);
        CopyFromData(&platform->link->moveToSpinDir, data, 12);
        return TRUE;
    }
    if (token == TOKEN_SPINTOSPIN) {
        if (platform->link == NULL) {
            platform->link = JallocAlloc(sizeof(PlatformLink), 1, "Plat_MvSpn");
        }
        CopyFromData(&platform->link->spinToSpinTag, data, 2);
        CopyFromData(&platform->link->spinToSpinScale, data, 4);
        return TRUE;
    }
    if (token == TOKEN_PARENTTAG) {
        CopyFromData(buf, data, 2);
        if (buf[0]) {
            if (platform->path == NULL || platform->path->numPoints == 0) {
                platform->parent = TagToFirstPlatform(buf[0]);
                if (platform->parent == NULL) {
                    PRINTF("WARNING!! No such parent tag %d for platform %d\n", buf[0], platformList.count);
                }
            } else {
                PRINTF("WARNING!! Platform %d has parenttag after first point\n", platformList.count);
            }
        }
        return TRUE;
    }
    if (token == TOKEN_MOVETARGET) {
        if (platform->link == NULL) {
            platform->link = JallocAlloc(sizeof(PlatformLink), 1, "Plat_MvSpn");
        }
        CopyFromData(&platform->link->tag, data, 2);
        CopyFromData(&platform->link->vel, data, 12);
        return TRUE;
    }
    if (token == TOKEN_MOVEBYSPEED) {
        if (platform->link == NULL) {
            platform->link = JallocAlloc(sizeof(PlatformLink), 1, "Plat_MvSpn");
        }
        CopyFromData(&platform->link->velOffset, data, 12);
        return TRUE;
    }
    if (token == TOKEN_TAKESPIN) {
        platform->flags |= 0x40;
        if (platform->motion == NULL) {
            platform->motion = JallocAlloc(0x98, 1, "Plat_Spin");
        }
        plat = platform->parent;
        if (plat != NULL) {
            axis = 10;
            if (plat->motion != NULL) {
                *(s32 *)platform->motion->active = *(s32 *)plat->motion->active;
                for (k = 0, n = 0; k < 3; k++) {
                    (&platform->motion->rot.x)[k] += (&plat->motion->rot.x)[k];
                    platform->motion->targetRot[k] += (&plat->motion->rot.x)[k];
                    if (!plat->motion->active[k]) {
                        d[n++] = (&platform->actor.pos.x)[k] - (&plat->actor.pos.x)[k];
                    } else {
                        axis = k;
                    }
                }
                if (axis != 10) {
                    dist = sqrtf(d[0] * d[0] + d[1] * d[1]);
                    ang = atan2(d[1], d[0]);
                    if (axis == 1) {
                        ang -= plat->motion->rot.y;
                    } else {
                        ang += (&plat->motion->rot.x)[axis];
                    }
                    ang = Aabs(ang);
                    d[0] = dist * __cosf(ang);
                    d[1] = dist * __sinf(ang);
                    for (k = 0, n = 0; k < 3; k++) {
                        if (!plat->motion->active[k]) {
                            (&platform->actor.pos.x)[k] = d[n++] + (&plat->actor.pos.x)[k];
                        }
                    }
                }
            }
        }
        return TRUE;
    }
    return FALSE;
}

s32 ParseTokenBridge(s16 token, u8 **data) {
    f32 inFloat[10];
    s16 sval[8];
    s16 inChar;
    u32 ival;
    f32 x, y, z;
    f32 x2, y2, z2;
    f32 x3, y3, z3;
    Platform *platform;
    s32 k;
    s32 j;

    if (token == TOKEN_ROPEBRIDGE) {
        CopyFromData(sval, data, 4);
        CopyFromData(&inChar, data, 2);
        CopyFromData(&ival, data, 4);
        CopyFromData(tempName, data, 8);
        CopyFromData(inFloat, data, 28);
        CopyFromData(&x, data, 4);
        CopyFromData(&y, data, 4);
        CopyFromData(&z, data, 4);
        CopyFromData(&x2, data, 4);
        CopyFromData(&y2, data, 4);
        CopyFromData(&z2, data, 4);
        ropeBridge[numRopeBridges].first = ropeTag;
        ropeBridge[numRopeBridges].stiffness = inFloat[0];
        ropeBridge[numRopeBridges].last = ropeTag + sval[0] - 1;
        for (k = 0; k < sval[0]; k++) {
            platform = JallocAlloc(sizeof(Platform), 1, "Rope_Plat");
            AddPlatform(platform);
            platform->actor.type = PLAT_ACTOR;
            InitActorStructures(&platform->actor, 0);
            FindObject(&platform->actor.inst, ival, tempName, 1);
            platform->actor.faces = JallocAlloc(platform->actor.inst->obj->mesh->numFaces * sizeof(CollFace), 1, "Plat_Plane");
            platform->actor.flags = 4;
            AddActor(&platform->actor);
            platform->actor.rotQuat = zeroQuat;
            platform->actor.friction = 1.0f;
            PlatformAddPoint(platform, x + k * x2, y + k * y2, z + k * z2, 0);
            if (platform->motion == NULL) {
                platform->motion = JallocAlloc(0x98, 1, "Plat_Spin");
            } else {
                PRINTF("Plat already has spin
");
                memclear(platform->motion, 0x98);
            }
            platform->motion->limitFlags |= 3;
            platform->motion->active[inChar] = 1;
            (&platform->motion->rot.x)[inChar] = platform->motion->baseRot = platform->motion->targetRot[inChar] = inFloat[4];
            if (inFloat[2] > 0.0f && (k == 0 || k == sval[0] - 1)) {
                platform->motion->seekTarget[inChar] = 1;
            }
            platform->flags |= 0x10;
            platform->tag = ropeTag++;
            platform->motion->damping = inFloat[1];
            platform->motion->accel = inFloat[2];
            // the conditional is written this way. GCC distributes the multiply into both arms, as in the original
            platform->motion->rotLimit = inFloat[3] + ((ropeBridge[numRopeBridges].last - k - ropeBridge[numRopeBridges].first) >= k ? (f32)k : (f32)(ropeBridge[numRopeBridges].last - k - ropeBridge[numRopeBridges].first)) * inFloat[6];
            platform->motion->maxRotVel = inFloat[5];
            platform->motion->inertia = sval[1];
            InitPlatformInfo(platform, 0.0f);
        }
        if (inFloat[6] == 0.0f) {
            ropeBridge[numRopeBridges].dampEnds = 1;
        }
        numRopeBridges++;
        return TRUE;
    }
    if (token == TOKEN_SINEBRIDGE) {
        CopyFromData(sval, data, 4);
        CopyFromData(&ival, data, 4);
        CopyFromData(tempName, data, 8);
        CopyFromData(&x, data, 4);
        CopyFromData(&y, data, 4);
        CopyFromData(&z, data, 4);
        CopyFromData(&x2, data, 4);
        CopyFromData(&y2, data, 4);
        CopyFromData(&z2, data, 4);
        CopyFromData(&x3, data, 4);
        CopyFromData(&y3, data, 4);
        CopyFromData(&z3, data, 4);
        CopyFromData(&inFloat[0], data, 4);
        CopyFromData(&inFloat[1], data, 4);
        for (k = 0; k < sval[0]; k++) {
            platform = JallocAlloc(sizeof(Platform), 1, "Sine_Plat");
            AddPlatform(platform);
            platform->actor.type = PLAT_ACTOR;
            InitActorStructures(&platform->actor, 0);
            FindObject(&platform->actor.inst, ival, tempName, 1);
            platform->actor.faces = JallocAlloc(platform->actor.inst->obj->mesh->numFaces * sizeof(CollFace), 1, "Plat_Plane");
            platform->actor.flags = 4;
            AddActor(&platform->actor);
            platform->actor.rotQuat = zeroQuat;
            platform->actor.friction = 1.0f;
            PlatformAddPoint(platform, x + k * x2, y + k * y2, z + k * z2, 0);
            PlatformAddPoint(platform, x + k * x2 + x3, y + k * y2 + y3,
                             z + k * z2 + z3, 0);
            platform->maxSpeed = inFloat[0];
            platform->path->accel = inFloat[1];
            for (j = 0; j < k * sval[1]; j++) {
                UpdatePlatform(platform);
            }
        }
        return TRUE;
    }
    if (token == TOKEN_ORBITBRIDGE) {
        CopyFromData(&inChar, data, 2);
        CopyFromData(sval, data, 6);
        CopyFromData(&ival, data, 4);
        CopyFromData(tempName, data, 8);
        CopyFromData(&x, data, 4);
        CopyFromData(&y, data, 4);
        CopyFromData(&z, data, 4);
        CopyFromData(&x2, data, 4);
        CopyFromData(&y2, data, 4);
        CopyFromData(&z2, data, 4);
        CopyFromData(&x3, data, 4);
        CopyFromData(&y3, data, 4);
        CopyFromData(&z3, data, 4);
        CopyFromData(&inFloat[0], data, 4);
        for (k = 0; k < sval[0]; k++) {
            platform = JallocAlloc(sizeof(Platform), 1, "Orbit_Plat");
            AddPlatform(platform);
            platform->actor.type = PLAT_ACTOR;
            InitActorStructures(&platform->actor, 0);
            FindObject(&platform->actor.inst, ival, tempName, 1);
            platform->actor.faces = JallocAlloc(platform->actor.inst->obj->mesh->numFaces * sizeof(CollFace), 1, "Plat_Plane");
            platform->actor.flags = 4;
            AddActor(&platform->actor);
            platform->actor.rotQuat = zeroQuat;
            platform->actor.friction = 1.0f;
            platform->orbit = JallocAlloc(sizeof(PlatformOrbit), 1, "Plat_Orbit");
            platform->orbit->speed = inFloat[0];
            platform->orbit->center.x = x + k * x2;
            platform->orbit->center.y = y + k * x2;
            platform->orbit->center.z = z + k * x2;
            if (sval[2] == -1) {
                platform->actor.pos.x = x + k * x2 + x3;
                platform->actor.pos.y = y + k * y2 + y3;
                platform->actor.pos.z = z + k * z2 + z3;
            } else {
                j = (sval[0] - k - 1 >= k ? k : sval[0] - k - 1) + sval[2];
                platform->actor.pos.x = x + k * x2 + j * x3;
                platform->actor.pos.y = y + k * y2 + j * y3;
                platform->actor.pos.z = z + k * z2 + j * z3;
            }
            switch (inChar) {
                case 0x58:
                    platform->orbit->axis[0] = 1;
                    break;
                case 0x59:
                    platform->orbit->axis[1] = 1;
                    break;
                case 0x5A:
                    platform->orbit->axis[2] = 1;
                    break;
            }
            for (j = 0; j < k * sval[1]; j++) {
                UpdatePlatform(platform);
            }
        }
        return TRUE;
    }
    return FALSE;
}

s32 ParseTokenPlatformFeatures(s16 token, u8 **data, Platform *platform) {
    s16 sval[8];
    PlatformTeleport *teleport;
    Platform *plat;

    if (token == TOKEN_RECALC) {
        platform->flags |= 0x20000;
        return TRUE;
    }
    if (token == TOKEN_MODGE) {
        if (spikeOrPlatformOrVent == 0x62) {
            platform->actor.inst->obj->flags |= 0x20;
        } else {
            lastLandscape->inst->obj->flags |= 0x20;
        }
        return TRUE;
    }
    if (token == TOKEN_COLL_OFFSET) {
        CopyFromData(&offsetY, data, 4);
        return TRUE;
    }
    if (token == TOKEN_BUZZER) {
        buzzer = JallocAlloc(sizeof(Buzzer), 1, "Buzzer");
        CopyFromData(&buzzer->tag, data, 2);
        CopyFromData(sval, data, 4);
        if (sval[0]) {
            buzzer->plat1 = TagToFirstPlatform(sval[0]);
        }
        if (sval[1]) {
            buzzer->plat2 = TagToFirstPlatform(sval[1]);
        }
        CopyFromData(&buzzer->flags, data, 2);
        CopyFromData(sval, data, 8);
        buzzer->r = sval[0];
        buzzer->g = sval[1];
        buzzer->b = sval[2];
        buzzer->a = sval[3];
        CopyFromData(&buzzer->p1, data, 12);
        CopyFromData(&buzzer->p2, data, 12);
        CopyFromData(&buzzer->jitter, data, 4);
        CopyFromData(&buzzer->width, data, 4);
        if (!(buzzer->flags & 0x20)) {
            if (buzzer->plat1 != NULL) {
                ADD_VECTORS(&buzzer->mid, &buzzer->p1, &buzzer->plat1->actor.pos);
            } else {
                buzzer->mid = buzzer->p1;
            }
            if (buzzer->plat2 != NULL) {
                ADD_TO_VECTOR(&buzzer->mid, &buzzer->plat2->actor.pos);
            }
            ADD_TO_VECTOR(&buzzer->mid, &buzzer->p2);
            SCALE_VECTOR(&buzzer->mid, 0.5);
            PlayContinuousSample(&buzzer->sample, 60, 100, &buzzer->mid, 128);
            buzzer->active = 1;
        }
        buzzer->buzzing = 1;
        AddBuzzer(buzzer);
        return TRUE;
    }
    if (token == TOKEN_BUZZER_PATTERN) {
        CopyFromData(&buzzer->onTime[buzzer->numTimes], data, 2);
        CopyFromData(&buzzer->offTime[buzzer->numTimes], data, 2);
        if (buzzer->numTimes == 0) {
            if (buzzer->offTime[0] == -1) {
                buzzer->buzzing = 1;
            } else {
                buzzer->buzzing = 0;
                StopContinuousSample(&buzzer->sample);
            }
            buzzer->timer = buzzer->onTime[0];
        }
        buzzer->numTimes++;
        return TRUE;
    }
    if (token == TOKEN_SPARKLY) {
        CopyFromData(sval, data, 2);
        if (spikeOrPlatformOrVent == 0x92) {
            lastLandscape->sparkleRate = sval[0];
        } else if (spikeOrPlatformOrVent == 0x62) {
            platform->actor.sparkleRate = sval[0];
        }
        return TRUE;
    }
    if (token == TOKEN_NOCAMCOLLIDE) {
        if (spikeOrPlatformOrVent == 0x92) {
            lastLandscape->flags |= 0x20000000;
        } else if (spikeOrPlatformOrVent == 0x62) {
            platform->actor.flags |= 0x20000000;
        }
        return TRUE;
    }
    if (token == TOKEN_EXITDOOR) {
        CopyFromData(&sval[0], data, 2);
        CopyFromData(&sval[1], data, 2);
        platform->exitType = sval[0];
        if (sval[1] == 0) {
            platform->actor.alpha = 0;
            platform->flags |= 4;
            JallocFree(&platform->actor.faces);
        }
        return TRUE;
    }
    if (token == TOKEN_CATAPULT) {
        platform->catapult = JallocAlloc(sizeof(PlatformCatapult), 1, "Plat_Cat");
        if (platform->motion == NULL) {
            platform->motion = JallocAlloc(0x98, 1, "Plat_Spin");
            platform->motion->active[0] = 1;
        } else {
            PRINTF("Plat already has spin
");
            memclear(platform->motion, 0x98);
        }
        platform->motion->damping = 1.0f;
        CopyFromData(&platform->catapult->launchDelay, data, 2);
        CopyFromData(&platform->catapult->holdPos, data, 12);
        CopyFromData(&platform->catapult->launchSpeed, data, 4);
        CopyFromData(&platform->catapult->launchVelY, data, 4);
        CopyFromData(&platform->catapult->armRotVel, data, 4);
        platform->catapult->active = 1;
        return TRUE;
    }
    if (token == TOKEN_CONVEYOR) {
        platform->flags |= 0x200;
        CopyFromData(&platform->conveyorVel, data, 12);
        return TRUE;
    }
    if (token == TOKEN_CURRENT) {
        platform->flags |= 0x200;
        CopyFromData(&platform->conveyorVel, data, 12);
        CopyFromData(&platform->pullSpeed, data, 4);
        return TRUE;
    }
    if (token == TOKEN_TELEPORT) {
        CopyFromData(sval, data, 4);
        plat = TagToFirstPlatform(sval[1]);
        if (plat != NULL || sval[1] == 0) {
            teleport = JallocAlloc(sizeof(PlatformTeleport), 1, "Plat_Tele");
            if (sval[1]) {
                teleport->target = plat;
            }
            CopyFromData(&teleport->shrinkTime, data, 2);
            CopyFromData(&teleport->growTime, data, 2);
            CopyFromData(&teleport->pos, data, 12);
            teleport->active = 1;
            teleport->flags = sval[0];
            platform->teleport = teleport;
        } else {
            PRINTF("WARNING!! Platform %d cannot TELEPORT - no such target tag %d\n", platformList.count, sval[1]);
        }
        return TRUE;
    }
    if (token == TOKEN_FAN) {
        platform->fan = JallocAlloc(sizeof(Fan), 1, "Plat_Fan");
        CopyFromData(&platform->fan->on, data, 2);
        CopyFromData(&platform->fan->dir, data, 12);
        CopyFromData(&platform->fan->strength, data, 4);
        CopyFromData(&platform->fan->spread, data, 4);
        CopyFromData(&platform->fan->blend, data, 4);
        CopyFromData(&platform->fan->length, data, 4);
        return TRUE;
    }
    if (token == TOKEN_MAGNET) {
        platform->fan = JallocAlloc(sizeof(Fan), 1, "Plat_Magnet");
        CopyFromData(&platform->fan->on, data, 2);
        CopyFromData(&platform->fan->dir, data, 12);
        CopyFromData(&platform->fan->strength, data, 4);
        CopyFromData(&platform->fan->spread, data, 4);
        CopyFromData(&platform->fan->blend, data, 4);
        CopyFromData(&platform->fan->length, data, 4);
        if (platform->fan->on) {
            platform->actor.flags |= 0x400;
        }
        return TRUE;
    }
    if (token == TOKEN_RESTART) {
        platform->restartGate = JallocAlloc(sizeof(PlatformRestartGate), 1, "Plat_Rest");
        CopyFromData(sval, data, 2);
        platform->restartGate->groundShadow = sval[0];
        CopyFromData(&platform->restartGate->yaw, data, 4);
        if (numRestartGates == 0 || saveSlot[currentSaveSlot].difficultyLevel != DIFFICULTY_HARD) {
            platform->restartGate->untouched = 1;
            platform->restartGate->targetScale = 1.0f;
        }
        if (++numRestartGates == 1) {
            if (gameInfo.level >= 0 && gameInfo.level < 8) {
                switch ((s8)(gameInfo.prevWorld - 1)) {
                    case 0:
                        platform->actor.pos.x = 0.0f;
                        platform->actor.pos.y = 0.0f;
                        platform->actor.pos.z = 1253.0f;
                        platform->restartGate->yaw = Aabs(PI);
                        break;
                    case 1:
                        platform->actor.pos.x = -1086.0f;
                        platform->actor.pos.y = 0.0f;
                        platform->actor.pos.z = 630.0f;
                        platform->restartGate->yaw = Aabs(8.341593f);
                        break;
                    case 2:
                        platform->actor.pos.x = 1117.0f;
                        platform->actor.pos.y = 0.0f;
                        platform->actor.pos.z = 636.0f;
                        platform->restartGate->yaw = Aabs(4.2715926f);
                        break;
                    case 3:
                        platform->actor.pos.x = -1092.0f;
                        platform->actor.pos.y = 0.0f;
                        platform->actor.pos.z = -623.0f;
                        platform->restartGate->yaw = Aabs(7.341593f);
                        break;
                    case 4:
                        platform->actor.pos.x = 1110.0f;
                        platform->actor.pos.y = 0.0f;
                        platform->actor.pos.z = -631.0f;
                        platform->restartGate->yaw = Aabs(5.2315927f);
                        break;
                    case 5:
                        platform->actor.pos.x = -10.0f;
                        platform->actor.pos.y = 0.0f;
                        platform->actor.pos.z = -1250.0f;
                        platform->restartGate->yaw = Aabs(6.301593f);
                        break;
                }
            }
            handControl.checkpoint = platform;
            platform->restartGate->cameraPos.x = 0.0f;
            if (gameInfo.level == SPACEBONUS) {
                platform->restartGate->cameraPos.y = 0.0f;
            } else {
                platform->restartGate->cameraPos.y = 195.0f;
            }
            platform->restartGate->cameraPos.z = 169.0f;
            RotateVector2D(&platform->restartGate->cameraPos, &platform->restartGate->cameraPos, platform->restartGate->yaw);
            ADD_TO_VECTOR(&platform->restartGate->cameraPos, &platform->actor.pos);
            platform->restartGate->restartTimer = 65;
            camera.pos = platform->restartGate->cameraPos;
            platform->restartGate->untouched = 0;
        }
        return TRUE;
    }
    if (token == TOKEN_CRUMBLE) {
        platform->crumble = JallocAlloc(sizeof(PlatformCrumble), 1, "Plat_Crumb");
        CopyFromData(&platform->crumble->shakeTime, data, 2);
        CopyFromData(&platform->crumble->fallTime, data, 2);
        CopyFromData(&platform->crumble->gravity, data, 4);
        platform->crumble->origPos = platform->actor.pos;
        return TRUE;
    }
    if (token == TOKEN_CRUMBLE_SOUND) {
        CopyFromData(sval, data, 6);
        platform->crumble->sample.id = sval[0];
        platform->crumble->sample.volume = sval[1];
        platform->crumble->sample.baseVolume = sval[1];
        platform->crumble->sample.pitch = sval[2];
        return TRUE;
    }
    if (token == TOKEN_BOUNCE) {
        CopyFromData(sval, data, 2);
        if (sval[0] & 1) {
            platform->bounceAxis[0] = 1;
        }
        if (sval[0] & 2) {
            platform->bounceAxis[1] = 1;
        }
        if (sval[0] & 4) {
            platform->bounceAxis[2] = 1;
        }
        CopyFromData(&platform->bounce, data, 4);
        return TRUE;
    }
    if (token == TOKEN_ELECTRIC) {
        CopyFromData(sval, data, 2);
        if (sval[0]) {
            platform->flags |= 0x4000;
        }
        platform->flags |= 8;
        return TRUE;
    }
    if (token == TOKEN_PUSH) {
        platform->push = JallocAlloc(sizeof(PlatformPush), 1, "Plat_Push");
        CopyFromData(sval, data, 2);
        if (sval[0] & 1) {
            platform->push->axis[0] = 1;
        }
        if (sval[0] & 2) {
            platform->push->axis[1] = 1;
        }
        if (sval[0] & 4) {
            platform->push->axis[2] = 1;
        }
        CopyFromData(&platform->push->mass, data, 4);
        CopyFromData(&platform->actor.friction, data, 4);
        CopyFromData(&platform->maxSpeed, data, 4);
        return TRUE;
    }
    if (token == TOKEN_CONFINE) {
        platform->confine = JallocAlloc(sizeof(PlatformConfine), 1, "Plat_Conf");
        CopyFromData(platform->confine->min, data, 12);
        CopyFromData(platform->confine->size, data, 12);
        return TRUE;
    }
    return FALSE;
}

s32 ParseTokenPlatformOrbit(s16 token, u8 **data, Platform *platform) {
    s16 sval[8];
    f32 d[2];
    s32 i;
    s32 n;

    if (token == TOKEN_ORBIT_SOUND) {
        CopyFromData(sval, data, 6);
        platform->orbit->sample.id = sval[0];
        platform->orbit->sample.volume = sval[1];
        platform->orbit->sample.baseVolume = sval[1];
        platform->orbit->sample.pitch = sval[2];
        return TRUE;
    }
    if (token == TOKEN_ORBIT_STOP_SOUND) {
        CopyFromData(sval, data, 6);
        platform->orbit->stopSampleId = sval[0];
        platform->orbit->stopSampleVolume = sval[1];
        platform->orbit->stopSamplePitch = sval[2];
        return TRUE;
    }
    if (token == TOKEN_ORBIT) {
        platform->orbit = JallocAlloc(sizeof(PlatformOrbit), 1, "Plat_Orbit");
        CopyFromData(sval, data, 2);
        platform->orbit->axis[sval[0]] = 1;
        if (platform->path == NULL || platform->path->numPoints < 2) {
            CopyFromData(&platform->orbit->center, data, 12);
            CopyFromData(&platform->orbit->speed, data, 4);
        } else {
            PRINTF("WARNING!! Platform %d cannot orbit - more than one point specified\n", platformList.count);
        }
        for (i = 0, n = 0; i < 3; i++) {
            if (!platform->orbit->axis[i]) {
                d[n++] = (&platform->actor.pos.x)[i] - (&platform->orbit->center.x)[i];
            }
        }
        sqrtf(d[0] * d[0] + d[1] * d[1]); // result unused; the original still makes the call
        platform->orbit->angle = platform->orbit->startAngle = atan2(d[1], d[0]);
        return TRUE;
    }
    if (token == TOKEN_ORBITPAUSE) {
        if (platform->orbit->flip == 0.0f) {
            CopyFromData(&platform->orbit->pause, data, 2);
            CopyFromData(sval, data, 2);
            platform->orbit->pauseType = sval[0];
            platform->orbit->pauseIndex = (platform->orbit->pauseType - 1 >= 2) ? 1 : platform->orbit->pauseType - 1;
        } else {
            PRINTF("WARNING!! Platform %d cannot ORBITPAUSE - ORBITFLIP already specified\n", platformList.count);
        }
        return TRUE;
    }
    if (token == TOKEN_ORBITFLIP) {
        if (platform->orbit->pauseType == 0) {
            CopyFromData(&platform->orbit->pause, data, 2);
            CopyFromData(&platform->orbit->flip, data, 4);
        } else {
            PRINTF("WARNING!! Platform %d cannot ORBITFLIP - ORBITPAUSE already specified\n", platformList.count);
        }
        return TRUE;
    }
    return FALSE;
}

s32 ParseTokenPlatformSpin(s16 token, u8 **data, Platform *platform) {
    f32 inFloat[10];
    s16 sval[8];
    Vec3 tempVect;
    Vec3 tempVect2;
    f32 x;
    f32 y;
    f32 z;
    f32 x2;
    s32 i;
    s32 j;

    if (token == TOKEN_SPIN_SOUND) {
        CopyFromData(sval, data, 6);
        platform->motion->sample.id = sval[0];
        platform->motion->sample.volume = sval[1];
        platform->motion->sample.baseVolume = sval[1];
        platform->motion->sample.pitch = sval[2];
        return TRUE;
    }
    if (token == TOKEN_SPIN_STOP_SOUND) {
        CopyFromData(sval, data, 6);
        platform->motion->stopSampleId = sval[0];
        platform->motion->stopSampleVolume = sval[1];
        platform->motion->stopSamplePitch = sval[2];
        return TRUE;
    }
    if (token == TOKEN_TILTY) {
        if (platform->motion == NULL) {
            platform->motion = JallocAlloc(0x98, 1, "Plat_Spin");
        } else {
            PRINTF("Plat already has spin
");
            memclear(platform->motion, 0x98);
        }
        CopyFromData(&platform->motion->damping, data, 4);
        CopyFromData(&platform->motion->inertia, data, 4);
        CopyFromData(&platform->motion->accel, data, 4);
        CopyFromData(&platform->motion->rotLimit, data, 4);
        platform->actor.flags |= 0x4000;
        platform->flags |= 0x10000;
        return TRUE;
    }
    if (token == TOKEN_SPINPAUSE) {
        CopyFromData(sval, data, 4);
        if (platform->motion->flip == 0.0f) {
            platform->motion->pauseType = sval[0];
            platform->motion->pause = sval[1];
            platform->motion->pauseIndex = (platform->motion->pauseType - 1 >= 2) ? 1 : platform->motion->pauseType - 1;
        } else {
            PRINTF("WARNING!! Platform %d cannot SPINPAUSE - SPINFLIP already specified\n", platformList.count);
        }
        return TRUE;
    }
    if (token == TOKEN_SPINFLIP) {
        if (platform->motion->pauseType == 0) {
            CopyFromData(&platform->motion->pause, data, 2);
            CopyFromData(&platform->motion->flip, data, 4);
        } else {
            PRINTF("WARNING!! Platform %d cannot SPINFLIP - SPINPAUSE already specified\n", platformList.count);
        }
        return TRUE;
    }
    if (token == TOKEN_MAXROT) {
        CopyFromData(&platform->motion->rotLimit, data, 4);
        return TRUE;
    }
    if (token == TOKEN_SPIN) {
        if (platform->motion == NULL) {
            platform->motion = JallocAlloc(0x98, 1, "Plat_Spin");
        } else {
            PRINTF("Plat already has spin
");
            memclear(platform->motion, 0x98);
        }
        CopyFromData(sval, data, 2);
        platform->motion->active[sval[0]] = 1;
        CopyFromData(&(&platform->motion->rot.x)[sval[0]], data, 4);
        platform->motion->baseRot = (&platform->motion->rot.x)[sval[0]];
        CopyFromData(&platform->motion->speed, data, 4);
        platform->motion->damping = 1.0f;
        return TRUE;
    }
    if (token == TOKEN_SEESAW) {
        if (platform->motion == NULL) {
            platform->motion = JallocAlloc(0x98, 1, "Plat_Spin");
        } else {
            PRINTF("Plat already has spin
");
            memclear(platform->motion, 0x98);
        }
        CopyFromData(sval, data, 2);
        CopyFromData(&x, data, 4);
        CopyFromData(&y, data, 4);
        CopyFromData(&z, data, 4);
        CopyFromData(&x2, data, 4);
        CopyFromData(inFloat, data, 4);
        CopyFromData(&sval[1], data, 2);
        platform->motion->active[sval[0]] = 1;
        (&platform->motion->rot.x)[sval[0]] = platform->motion->baseRot = platform->motion->targetRot[sval[0]] = z;
        if (x > 0.0f) {
            platform->motion->seekTarget[sval[0]] = 1;
        }
        platform->flags |= 0x10;
        platform->motion->damping = inFloat[0];
        platform->motion->accel = x;
        platform->motion->rotLimit = y;
        platform->motion->maxRotVel = x2;
        platform->motion->inertia = sval[1];
        platform->motion->limitFlags |= 3;
        return TRUE;
    }
    if (token == TOKEN_TOPPLE) {
        if (platform->motion == NULL) {
            platform->motion = JallocAlloc(0x98, 1, "Plat_Spin");
        } else {
            PRINTF("Plat already has spin
");
            memclear(platform->motion, 0x98);
        }
        CopyFromData(sval, data, 2);
        CopyFromData(&x, data, 4);
        CopyFromData(&platform->motion->rotLimit, data, 4);
        CopyFromData(&platform->motion->tipAngle, data, 4);
        CopyFromData(&platform->motion->maxRotVel, data, 4);
        CopyFromData(&platform->motion->damping, data, 4);
        CopyFromData(&platform->motion->inertia, data, 4);
        CopyFromData(&platform->motion->limitFlags, data, 2);
        platform->motion->active[sval[0]] = 1;
        if (x > 0.0f) {
            platform->motion->seekTarget[sval[0]] = 1;
        }
        platform->flags |= 0x10;
        platform->motion->accel = x;
        return TRUE;
    }
    if (token == TOKEN_LOOKATHAND) {
        if (platform != NULL) {
            if (platform->motion == NULL) {
                platform->motion = JallocAlloc(0x98, 1, "Plat_Spin");
            } else {
                PRINTF("Plat already has spin
");
                memclear(platform->motion, 0x98);
            }
            platform->motion->active[1] = 1;
            CopyFromData(&platform->motion->damping, data, 4);
            platform->flags |= 0x400;
            CopyFromData(&platform->motion->accel, data, 4);
            platform->motion->seekTarget[1] = 1;
        } else {
            PRINTF("WARNING!! Cannot have LOOKATHAND without PLATFORM\n");
        }
        return TRUE;
    }
    if (token == TOKEN_LOOKATBALL) {
        if (platform != NULL) {
            if (platform->motion == NULL) {
                platform->motion = JallocAlloc(0x98, 1, "Plat_Spin");
            } else {
                PRINTF("Plat already has spin
");
                memclear(platform->motion, 0x98);
            }
            platform->motion->active[1] = 1;
            CopyFromData(&platform->motion->damping, data, 4);
            platform->flags |= 0x800;
            CopyFromData(&platform->motion->accel, data, 4);
            platform->motion->seekTarget[1] = 1;
        } else {
            PRINTF("WARNING!! Cannot have LOOKATBALL without PLATFORM\n");
        }
        return TRUE;
    }
    if (token == TOKEN_PENDULUM) {
        if (platform->motion == NULL) {
            platform->motion = JallocAlloc(0x98, 1, "Plat_Spin");
        } else {
            PRINTF("Plat already has spin
");
            memclear(platform->motion, 0x98);
        }
        CopyFromData(sval, data, 2);
        platform->motion->active[sval[0]] = 1;
        CopyFromData(&(&platform->motion->rot.x)[sval[0]], data, 4);
        platform->motion->baseRot = (&platform->motion->rot.x)[sval[0]];
        CopyFromData(&platform->motion->pendulumLength, data, 4);
        platform->motion->damping = 1.0f;
        CopyFromData(inFloat, data, 4);
        CopyFromData(&sval[1], data, 2);
        for (i = 0; i < sval[1]; i++) {
            for (j = 0; j < 3; j++) {
                platform->motion->rotVel[j] -= FindShortestAngleSigned((&platform->motion->rot.x)[j], 0.0f) / platform->motion->pendulumLength;
                (&platform->motion->rot.x)[j] += platform->motion->rotVel[j];
                (&platform->motion->rot.x)[j] = Aabs((&platform->motion->rot.x)[j]);
                if (ABS(platform->motion->rotVel[j]) <= 0.0005) {
                    platform->motion->rotVel[j] = 0.0f;
                }
            }
        }
        tempVect.x = 0.0f;
        tempVect.y = -platform->extent.y;
        tempVect.z = 0.0f;
        RotateVector2DXYZ(&tempVect, &tempVect, platform->motion->baseRot, sval[0]);
        tempVect2 = tempVect;
        SCALE_VECTOR(&tempVect2, 0.82);
        ADD_TO_VECTOR(&tempVect, &platform->actor.pos);
        ADD_TO_VECTOR(&tempVect2, &platform->actor.pos);
        if (inFloat[0] != 0.0f) {
            platform->blur = JallocAlloc(sizeof(PlatformBlur), 1, "Blur");
            platform->blur->innerScale = inFloat[0];
            AddBlur(&platform->blur->blur);
            StartBlurFX(&platform->blur->blur, &tempVect, &tempVect2, 255, 0, 20, 20);
            platform->blur->blur.idleCount = 255;
        }
        return TRUE;
    }
    if (token == TOKEN_SYMMETRY) {
        CopyFromData(&platform->motion->snapCount, data, 2);
        return TRUE;
    }
    return FALSE;
}

s32 ParseTokenPlatformPath(s16 token, u8 **data, Platform *platform) {
    s16 sval[8];
    f32 inFloat[5];

    if (token == TOKEN_MOVE_SOUND) {
        CopyFromData(sval, data, 6);
        platform->sample.id = sval[0];
        platform->sample.volume = sval[1];
        platform->sample.baseVolume = sval[1];
        platform->sample.pitch = sval[2];
        return TRUE;
    }
    if (token == TOKEN_STOP_SOUND) {
        CopyFromData(sval, data, 6);
        platform->path->points[platform->path->numPoints - 1]->sampleId = sval[0];
        platform->path->points[platform->path->numPoints - 1]->sampleVolume = sval[1];
        platform->path->points[platform->path->numPoints - 1]->samplePitch = sval[2];
        return TRUE;
    }
    if (token == TOKEN_LOOKATPOINT) {
        if (platform != NULL) {
            if (platform->motion == NULL) {
                platform->motion = JallocAlloc(0x98, 1, "Plat_Spin");
            } else {
                PRINTF("Plat already has spin
");
                memclear(platform->motion, 0x98);
            }
            CopyFromData(&platform->motion->damping, data, 4);
            CopyFromData(&platform->motion->accel, data, 4);
            platform->flags |= 0x1000;
            platform->motion->seekTarget[1] = 1;
            platform->motion->active[1] = 1;
            platform->motion->targetRot[1] = platform->motion->rot.y = Aabs(atan2(
                platform->path->points[0]->pos.x - platform->path->points[1]->pos.x,
                platform->path->points[0]->pos.z - platform->path->points[1]->pos.z));
        } else {
            PRINTF("WARNING!! Cannot have LOOKATPOINT without PLATFORM\n");
        }
        return TRUE;
    }
    if (token == TOKEN_GOFORWARDS) {
        if (platform != NULL) {
            if (platform->motion == NULL) {
                platform->motion = JallocAlloc(0x98, 1, "Plat_Spin");
            } else {
                PRINTF("Plat already has spin
");
                memclear(platform->motion, 0x98);
            }
            CopyFromData(&platform->motion->damping, data, 4);
            CopyFromData(&platform->motion->accel, data, 4);
            CopyFromData(&platform->actor.friction, data, 4);
            CopyFromData(&platform->motion->bankScale, data, 4);
            if (platform->motion->bankScale != 0.0f) {
                platform->motion->seekTarget[2] = 1;
            }
            platform->flags |= 0x3000;
            platform->motion->seekTarget[1] = 1;
            platform->motion->seekTarget[0] = 1;
            platform->motion->active[1] = 1;
            platform->motion->targetRot[1] = platform->motion->rot.y = Aabs(atan2(
                platform->path->points[0]->pos.x - platform->path->points[1]->pos.x,
                platform->path->points[0]->pos.z - platform->path->points[1]->pos.z));
        } else {
            PRINTF("WARNING!! Cannot have GOFORWARDS without PLATFORM\n");
        }
        return TRUE;
    }
    if (token == TOKEN_POS) {
        CopyFromData(&platform->actor.pos, data, 12);
        SetPlatformPos(platform, &platform->actor.pos, offsetY);
        return TRUE;
    }
    if (token == TOKEN_POINT) {
        CopyFromData(sval, data, 2);
        CopyFromData(inFloat, data, 12);
        PlatformAddPoint(platform, inFloat[0], inFloat[1], inFloat[2], sval[0]);
        return TRUE;
    }
    if (token == TOKEN_STARTPOINT) {
        CopyFromData(sval, data, 2);
        platform->path->current = sval[0];
        platform->path->target = sval[0];
        platform->actor.pos = platform->path->points[sval[0]]->pos;
        platform->actor.prevPos = platform->actor.pos;
        platform->path->moving = platform->path->points[sval[0]]->time;
        return TRUE;
    }
    if (token == TOKEN_SPEED) {
        CopyFromData(&platform->maxSpeed, data, 4);
        return TRUE;
    }
    if (token == TOKEN_ACCEL) {
        if (platform->path == NULL) {
            platform->path = JallocAlloc(sizeof(PlatformPath), 1, "Plat_Path");
        }
        CopyFromData(&platform->path->accel, data, 4);
        return TRUE;
    }
    return FALSE;
}

s32 ParseTokenPlatformGeneric(s16 token, u8 **data, Platform *platform) {
    s16 sval[8];
    Spike *spike;
    s32 i;

    if (token == TOKEN_ANIMATE_PLATFORM) {
        if (platform->actor.inst != NULL && platform->actor.inst->anim != NULL) {
            platform->actor.anim = JallocAlloc(0x50, 1, "PlatAnim");
            InitActorAnim(&platform->actor);
        }
        AnimateActor(&platform->actor, 0, 1, 0, 0.0f);
    }
    if (token == TOKEN_FLOATS) {
        platform->actor.flags |= 0x200000;
        return TRUE;
    }
    if (token == TOKEN_HEADSTART) {
        CopyFromData(sval, data, 2);
        if (spikeOrPlatformOrVent == 0x62) {
            for (i = 0; i < sval[0]; i++) {
                UpdatePlatform(platform);
            }
        } else if (spikeOrPlatformOrVent == 0xA2) {
            for (i = 0; i < sval[0]; i++) {
                UpdateVent(lastVent);
            }
        }
        return TRUE;
    }
    if (token == TOKEN_NOCOLLISION) {
        platform->flags |= 4;
        JallocFree(&platform->actor.faces);
        return TRUE;
    }
    if (token == TOKEN_DESTRUCTIBLE) {
        platform->destruct = JallocAlloc(sizeof(PlatformDestruct), 1, "Plat_Dest");
        CopyFromData(sval, data, 2);
        if (sval[0] & 1) {
            platform->flags |= 0x100;
        }
        if (sval[0] & 2) {
            platform->flags |= 0x40000;
        }
        if (sval[0] & 4) {
            platform->flags |= 0x80000;
        }
        CopyFromData(&platform->destruct->numDebris, data, 4);
        CopyFromData(&platform->destruct->inst, data, 4);
        CopyFromData(tempName, data, 8);
        FindObject(&platform->destruct->inst, (u32)platform->destruct->inst, tempName, 1);
        return TRUE;
    }
    if (token == TOKEN_DESTROY_SOUND) {
        CopyFromData(sval, data, 6);
        platform->destruct->sampleId = sval[0];
        platform->destruct->sampleVolume = sval[1];
        platform->destruct->samplePitch = sval[2];
        return TRUE;
    }
    if (token == TOKEN_GRAVITY) {
        platform->actor.flags |= 1;
        return TRUE;
    }
    if (token == TOKEN_REVERSING) {
        platform->flags |= 0x20;
        return TRUE;
    }
    if (token == TOKEN_TERRAIN) {
        CopyFromData(sval, data, 2);
        if (spikeOrPlatformOrVent == 0x62) {
            SetPlatformTerrain(platform, sval[0]);
        } else {
            lastLandscape->lifeTimer = sval[0];
        }
        return TRUE;
    }
    if (token == TOKEN_TAG) {
        CopyFromData(&platform->tag, data, 2);
        return TRUE;
    }
    if (token == TOKEN_SPIKE) {
        spike = JallocAlloc(sizeof(Spike), 1, "spike");
        spike->platform = platform;
        platform->flags |= 4;
        spike->next = NULL;
        JallocFree(&platform->actor.faces);
        AddSpike(spike);
    }
    if (token == TOKEN_SCALE) {
        if (platform->path == NULL || platform->path->numPoints == 0) {
            CopyFromData(&platform->actor.scale, data, 12);
        } else {
            PRINTF("WARNING!! Platform %d has scale after first point\n", platformList.count);
        }
        return TRUE;
    }
    if (token == TOKEN_STRETCH) {
        platform->scale = JallocAlloc(0x28, 1, "Plat_Str");
        CopyFromData(platform->scale->max, data, 12);
        CopyFromData(&platform->scale->time, data, 2);
        CopyFromData(&platform->scale->repeat, data, 2);
        platform->scale->count = 1;
        return TRUE;
    }
    return FALSE;
}

INCLUDE_ASM("asm/nonmatchings/parse", pad_80188F88);

INCLUDE_RODATA("asm/nonmatchings/parse", pad_801094E8);
