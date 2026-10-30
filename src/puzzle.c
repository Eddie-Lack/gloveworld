#include "common.h"
#include "ident.h"
#include "puzzle.h"
#include "maths.h"
#include "macros.h"
#include "actor.h"
#include "platform.h"
#include "collect.h"
#include "hand.h"
#include "camera.h"
#include "vent.h"
#include "buzzer.h"
#include "wind.h"
#include "water.h"
#include "ambient.h"
#include "overlay.h"
#include "game.h"
#include "cameo.h"
#include "sound.h"
#include "texture.h"
#include "debris.h"
#include "jalloc.h"
#include "quake.h"
#include "enemy.h"

extern s8 disableZoom;

// PuzzleEvaluateCondition's cases 22 and 23 index the vectors as arrays: the ROM's addresses are an ARRAY_REF's
// (base + idx * 4)
typedef struct {
    f32 v[3];
} Vec3Arr;

extern Vec3 puzzleCamVect;
extern s32 puzzleFrameNum;
extern s32 levelCamDistance;
extern s32 levelCamHeight;
extern s32 frameCount;
extern f32 gravity;
extern s32 controlMethod;
extern s32 ballStatic;
extern Fog originalFog;
extern s16 farPlaneDist;
extern Vec3 zero;
Platform *TagToFirstPlatform(s32 tag);
void DropGaribs(s32 groupTag, f32 Yval);
void FlashScreen(u8 r, u8 g, u8 b, s16 fadeVal, s16 fadeStep);
void FindTexture(void *texPtr, u32 texID, s32 verbose);
void NMEGetNewInstruction(Enemy *enemy, s32 instructionNum, s32 overrideConditions);
Collectable *CreateNewPickup(s32 type, Vec3 *pos, s16 time, s16 duration);

void AddConditionToList(PuzzleCond *condition, PuzzleCondList *conditionList) {
    if (condition->prev == NULL) {
        conditionList->count++;
        condition->next = &conditionList->head;
        condition->prev = conditionList->head.prev;
        conditionList->head.prev->next = condition;
        conditionList->head.prev = condition;
    }
}

void AddActionToList(PuzzleAction *action, PuzzleActionList *actionList) {
    if (action->prev == NULL) {
        actionList->count++;
        action->prev = &actionList->head;
        action->next = actionList->head.next;
        actionList->head.next->prev = action;
        actionList->head.next = action;
    }
}

void InitConditionList(PuzzleCondList *conditionList) {
    conditionList->count = 0;
    conditionList->head.next = &conditionList->head;
    conditionList->head.prev = &conditionList->head;
    conditionList->timesLeft = -1;
}

void InitActionList(PuzzleActionList *actionList) {
    actionList->count = 0;
    actionList->head.next = &actionList->head;
    actionList->head.prev = &actionList->head;
}

void InitPuzzleList(void) {
    s32 i;

    for (i = 0; i < 100; i++) {
        puzzleVal[i] = 0;
    }
    puzzleList.count = 0;
    puzzleList.head.prev = puzzleList.head.next = &puzzleList.head;
}

Puzzle *NewPuzzle(void) {
    Puzzle *puzzle = JallocAlloc(sizeof(Puzzle), 1, "Puzzle");

    AddPuzzle(puzzle);
    puzzle->conds.head.prev = puzzle->conds.head.next = &puzzle->conds.head;
    puzzle->conds.timesLeft = -1;
    puzzle->conds.count = 0;
    puzzle->actions.count = 0;
    puzzle->actions.head.prev = puzzle->actions.head.next = &puzzle->actions.head;
    return puzzle;
}

void AddPuzzle(Puzzle *puzzle) {
    if (puzzle->prev == NULL) {
        puzzleList.count++;
        puzzle->next = &puzzleList.head;
        puzzle->prev = puzzleList.head.prev;
        puzzleList.head.prev->next = puzzle;
        puzzleList.head.prev = puzzle;
    }
}

void CheckPuzzles(void) {
    Puzzle *puzzle;

    disableZoom = FALSE;
    for (puzzle = puzzleList.head.prev; puzzle != &puzzleList.head; puzzle = puzzle->prev) {
        if (puzzle->conds.timesLeft && PuzzleEvaluateConditionList(&puzzle->conds)) {
            puzzle->conds.timesLeft--;
            PuzzleStartActionList(&puzzle->actions);
        }
        if (puzzle->actions.active) {
            PuzzleCheckActionList(&puzzle->actions);
        }
    }
}

void PuzzleCheckActionList(PuzzleActionList *actionList) {
    PuzzleAction *action;
    s32 num = 0;

    for (action = actionList->head.prev; action != &actionList->head; action = action->prev) {
        switch (action->timer) {
            case -1:
                num++;
                break;
            case 0:
                PuzzleExecuteAction(action);
            default:
                action->timer--;
                break;
        }
    }
    if (num == actionList->count) {
        actionList->active = FALSE;
    }
}

void PuzzleStartActionList(PuzzleActionList *actionList) {
    PuzzleAction *action;

    actionList->active = TRUE;
    for (action = actionList->head.prev; action != &actionList->head; action = action->prev) {
        if (action->timer == -1) {
            if (action->flags & 0x200) {
                action->timer = action->delay + Random(6) * 5;
            } else {
                action->timer = action->delay;
            }
        }
    }
}

void PuzzleExecuteAction(PuzzleAction *action) {
    Vec3 tempVect;
    Vec3 tempVect2;
    AmbientSound *sound;
    AmbientSound *tmp;
    Vent *vent;
    Buzzer *buzzer;
    Wind *wind;
    Platform *platPtr;
    Collectable *collectable;
    Platform *platform;
    Water *water;
    f32 f;
    s32 j;

    if (IsCameraAction(action->type)) {
        if (cameraInfo.turnTimer) {
            return;
        }
        cameraInfo.noCameraAction = 0;
    }
    if (action->type == ACTION_START_MOVE && action->tag < 0) {
        platform = TagToFirstPlatform(-action->tag);
        platform->actor.vel.y = action->value;
        platform->actor.pos.y += platform->actor.vel.y;
        return;
    }
    if (action->type == ACTION_SET_STATE) {
        if (action->flags & 0x100) {
            vent = &ventList.head;
            do {
                vent = FindTaggedVent(vent, action->tag);
                if (vent == NULL) {
                    break;
                }
                vent->active = action->value;
                if (!vent->active && vent->sound.handle) {
                    StopContinuousSample(&vent->sound);
                }
            } while (vent != NULL);
        }
        if (action->flags & 0x400) {
            buzzer = &buzzerList.head;
            do {
                buzzer = FindTaggedBuzzer(buzzer, action->tag);
                if (buzzer == NULL) {
                    break;
                }
                buzzer->active = action->value;
                if (!buzzer->active) {
                    if (buzzer->sample.handle) {
                        StopContinuousSample(&buzzer->sample);
                    }
                } else if (buzzer->buzzing) {
                    PlayContinuousSample(&buzzer->sample, 60, 100, &buzzer->mid, 128);
                }
            } while (buzzer != NULL);
        }
    }
    switch (action->type) {
        case ACTION_ADD_TO_SCORE:
            AddToScore(action->value);
            return;
        case ACTION_SET_FOG:
            fog.r = action->data[0];
            fog.g = action->data[1];
            fog.b = action->data[2];
            fog.min = action->data[3];
            farPlaneDist = action->data[4];
            originalFog = fog;
            return;
        case ACTION_SET_BACKDROP:
            FindTexture(&backdropList.head.next->tex, *(u32 *)action->cam, 1);
            backdropList.head.next->tex->height--;
            return;
        case ACTION_START_SOUND:
            if (action->data[1] > 0) {
                AddAmbientSfxAtPoint(action->data[0], action->data[1], (Vec3 *)action->cam, action->data[2], action->data[3],
                                     action->data[4], action->data[5], (s16)action->value, action->tag, 0);
                return;
            }
            if (action->data[1] < 0) {
                if (action->value != 0.0f) {
                    platform = TagToFirstPlatform(action->value);
                    RotateVectorByQuaternion(&tempVect, (Vec3 *)action->cam, &platform->actor.rotQuat);
                    ADD_TO_VECTOR(&tempVect, &platform->actor.pos);
                    PlaySample(action->data[0], &tempVect, -action->data[1], action->data[2]);
                } else {
                    PlaySample(action->data[0], (Vec3 *)action->cam, -action->data[1], action->data[2]);
                }
                return;
            }
            sound = &ambientSoundList.head;
            do {
                sound = FindTaggedAmbientSound(sound, action->tag);
                tmp = sound;
                if (tmp != NULL) {
                    if (tmp->sample.handle) {
                        StopContinuousSample(&tmp->sample);
                    }
                    SubAmbientSound(tmp);
                    JallocFree(&tmp);
                }
            } while (sound != NULL);
            return;
        case ACTION_UNFREEZE_BALL:
            ballStatic = 0;
            ball.flags |= 0x19;
            return;
        case ACTION_SET_CAM_DIST:
            levelCamDistance = action->value;
            if (cameoList.count && cameoList.head.next->active) {
                cameraInfo.distance = action->value;
            }
            return;
        case ACTION_SET_CAM_HEIGHT:
            levelCamHeight = action->value;
            return;
        case ACTION_DROP_PICKUP:
            collectable = CreateNewPickup(action->tag, (Vec3 *)action->data, 0, action->flags);
            collectable->tag = action->value;
            ZERO_VECTOR(&collectable->actor.scale);
            CreateStarCircle(&collectable->actor.pos, 6, 19);
            FlashScreen(0, 0, 255, 245, -30);
            return;
        case ACTION_START_CAMEO:
            StartCameo(action->tag);
            return;
        case ACTION_SET_LEVEL_GRAVITY:
            gravity = action->value;
            return;
        case ACTION_EARTHQUAKE:
            StartEarthquake(action->data[0], action->data[1], action->data[2], action->data[3], 3);
            return;
        case ACTION_SET_WATERLEVEL:
            water = FindTaggedWater(action->tag);
            water->targetHeight = action->value;
            return;
        case ACTION_SET_VAR:
            if (action->flags & 0x80) {
                puzzleVal[action->tag] = puzzleVal[(s32)action->value];
            } else {
                puzzleVal[action->tag] = action->value;
            }
            return;
        case ACTION_ADDTO_VAR:
            if (action->flags & 0x80) {
                puzzleVal[action->tag] += puzzleVal[(s32)action->value];
            } else {
                puzzleVal[action->tag] += action->value;
            }
            return;
        case ACTION_SUBFROM_VAR:
            if (action->flags & 0x80) {
                puzzleVal[action->tag] -= puzzleVal[(s32)action->value];
            } else {
                puzzleVal[action->tag] -= action->value;
            }
            return;
        case ACTION_SET_CAMERA_POS:
            if (hand.state->teleport == NULL && controlMethod && !handControl.checkpoint->restartGate->restartTimer) {
                puzzleCamVect = *(Vec3 *)action->data;
                disableZoom = 1;
                puzzleFrameNum = frameCount;
            }
            return;
        case ACTION_LOOK_AT_POINT:
            if (!cameraInfo.overrideTimer && hand.state->teleport == NULL && controlMethod
                && !handControl.checkpoint->restartGate->restartTimer) {
                if (frameCount == puzzleFrameNum) {
                    SetCameraOverride(puzzleCamVect.x, puzzleCamVect.y, puzzleCamVect.z, (Vec3 *)action->data, &zero,
                                      action->value, action->flags);
                } else {
                    SetCameraOverride(camera.pos.x, camera.pos.y, camera.pos.z, (Vec3 *)action->data, &zero,
                                      action->value, action->flags);
                }
                disableZoom = 1;
            }
            return;
        case ACTION_NME_GOTO_ACTION:
            for (j = 0; j < numberOfEnemies; j++) {
                if (enemies[j].brain.tag == action->tag) {
                    NMEGetNewInstruction(&enemies[j], (s16)action->value, 1);
                }
            }
            return;
        case ACTION_ADD_NME:
            for (j = 0; j < numberOfEnemies; j++) {
                if (enemies[j].brain.tag == action->tag && !enemies[j].actor.active) {
                    enemies[j].actor.active = 1;
                    InitEnemy(&enemies[j], enemies[j].actor.type);
                    enemies[j].actor.pos = *(Vec3 *)action->data;
                    enemies[j].actor.scale.x = enemies[j].actor.scale.y = enemies[j].actor.scale.z =
                        enemyStats[enemies[j].actor.type].scale;
                    AddActor(&enemies[j].actor);
                    CreateStarCircle(&enemies[j].actor.pos, 6, 19);
                    if (gameInfo.level == CARNIVAL1) {
                        FlashScreen(255, 0, 0, 245, -30);
                    }
                }
            }
            return;
        case ACTION_SET_WIND:
            wind = &windList.head;
            while ((wind = FindTaggedWind(wind, action->tag)) != NULL) {
                wind->type = action->value;
            }
            return;
        case ACTION_DROP_GARIBS:
            if (action->flags >= 0) {
                DropGaribs(action->tag, action->value);
                action->flags--;
            }
            return;
        case ACTION_CAM_HEIGHT:
            cameraInfo.targetHeight = action->cam->pos.x;
            return;
        case ACTION_CAM_DISTANCE:
            disableZoom = 1;
            cameraInfo.targetDistance = action->cam->pos.x;
            return;
        case ACTION_CAM_CIRCLE:
            cameraInfo.destActive = 1;
            disableZoom = 1;
            if (action->cam->pos.z != 0.0f) {
                tempVect.x = action->cam->pos.x;
                tempVect.z = action->cam->pos.y;
                tempVect.y = hand.pos.y;
                tempVect2.x = hand.pos.x - tempVect.x;
                tempVect2.y = hand.pos.y - tempVect.y;
                tempVect2.z = hand.pos.z - tempVect.z;
                MakeUnit(&tempVect2);
                SCALE_VECTOR_2D(&tempVect2, action->cam->pos.z);
                SubVector2D(&tempVect, &tempVect, &tempVect2);
            } else {
                tempVect.x = action->cam->pos.x;
                tempVect.z = action->cam->pos.y;
                tempVect.y = hand.pos.y;
                tempVect2.x = hand.pos.x - tempVect.x;
                tempVect2.y = hand.pos.y - tempVect.y;
                tempVect2.z = hand.pos.z - tempVect.z;
                MakeUnit(&tempVect2);
                SCALE_VECTOR_2D(&tempVect2, cameraInfo.targetDistance);
                AddVector2D(&tempVect, &hand.pos, &tempVect2);
            }
            tempVect.y += cameraInfo.targetHeight;
            cameraInfo.destPos = tempVect;
            return;
        case ACTION_CAM_PARALLEL:
            if (cameraInfo.prevAngleLock != 1) {
                SUB_VECTORS(&tempVect, cameraInfo.focus, &camera.pos);
                cameraInfo.focusAngle = Aabs(atan2(tempVect.x, tempVect.z));
                cameraInfo.orbiting = 0;
            }
            cameraInfo.angleLock = 1;
            return;
        case ACTION_CAM_FIXEDROT:
            f = action->cam->speed;
            if (f < 0.0f) {
                cameraInfo.sideOffsetEnabled = 0;
                cameraInfo.orbitSpeed = -f;
            } else {
                cameraInfo.orbitSpeed = f;
            }
            cameraInfo.orbiting = 1;
            cameraInfo.orbitAngle = action->cam->pos.x;
            return;
        case ACTION_CAM_FIXEDPOS:
            cameraInfo.destActive = 1;
            cameraInfo.destPos = action->cam->pos;
            disableZoom = 1;
            return;
        case ACTION_CAM_IGNORE_SLOPES:
            cameraInfo.slopeAdjust = 0;
            cameraInfo.destPos = action->cam->pos;
            return;
    }
    platPtr = &platformList.head;
    do {
        platPtr = FindTaggedPlatform(platPtr, action->tag);
        if (platPtr == NULL) {
            break;
        }
        platform = platPtr;
        if (action->type == ACTION_START_MOVE || action->type == ACTION_SET_NEXT_POINT) {
            platform->pathStopFlags = 0;
            if (action->flags & 2) {
                platform->pathStopFlags = 2;
            } else if (action->flags & 1) {
                platform->pathStopFlags = 1;
            }
        } else if (action->type == ACTION_START_ORBIT) {
            platform->orbitStopFlags = 0;
            if (action->flags & 2) {
                platform->orbitStopFlags = 2;
            } else if (action->flags & 1) {
                platform->orbitStopFlags = 1;
            }
        } else if (action->type == ACTION_START_SPIN || action->type == ACTION_SET_NEXT_SPIN) {
            platform->spinStopFlags = 0;
            if (action->flags & 2) {
                platform->spinStopFlags = 2;
            } else if (action->flags & 1) {
                platform->spinStopFlags = 1;
            }
        }
        switch (action->type) {
            case ACTION_SET_CONVEYOR:
                platform->conveyorVel = *(Vec3 *)action->data;
                if (LENGTH(&platform->conveyorVel) != 0.0f) {
                    platform->flags |= 0x200;
                } else {
                    platform->flags &= ~0x200;
                }
                break;
            case ACTION_MOVE_PLAT:
                platform->actor.vel = *(Vec3 *)action->data;
                break;
            case ACTION_SET_PLAT_DIR:
                platform->path->dir = action->value;
                break;
            case ACTION_START_MOVE:
                if (action->value != 0.0f || (action->flags & 4)) {
                    if ((action->flags & 4) && action->value != 0.0f && platform->path->moving) {
                        platform->path->moving = 1;
                    }
                    if (action->value != -1.0f) {
                        platform->maxSpeed = action->value;
                    }
                    if ((action->flags & 4) && platform->actor.speed > platform->maxSpeed) {
                        SCALE_VECTOR(&platform->actor.vel, platform->maxSpeed / platform->actor.speed);
                        platform->actor.speed = platform->maxSpeed;
                    }
                }
                break;
            case ACTION_START_SPIN:
                if (action->value != 0.0f || (action->flags & 4)) {
                    platform->motion->speed = action->value;
                    if (action->flags & 4) {
                        if (action->value != 0.0f && platform->motion->timer) {
                            platform->motion->timer = 1;
                        }
                        for (j = 0; j < 3; j++) {
                            if (platform->motion->active[j]) {
                                platform->motion->rotVel[j] = action->value;
                            }
                        }
                    }
                }
                break;
            case ACTION_SET_SPIN_WAIT:
                platform->motion->pause = action->value;
                if (action->value == 0.0f) {
                    platform->motion->timer = action->value;
                }
                break;
            case ACTION_START_ORBIT:
                if (action->value != 0.0f || (action->flags & 4)) {
                    platform->orbit->speed = action->value;
                    if (platform->orbit->speed == 0.0f) {
                        platform->actor.vel = zero;
                    }
                }
                break;
            case ACTION_SET_NEXT_POINT:
                platform->path->target = (platform->path->numPoints + (s32)action->value - 1) % platform->path->numPoints;
                platform->path->moving = 1;
                break;
            case ACTION_SET_SPIN_AXIS:
                platform->motion->active[0] = platform->motion->active[1] = platform->motion->active[2] = 0;
                platform->motion->active[(s32)action->value] = 1;
                break;
            case ACTION_SET_NEXT_SPIN:
                platform->motion->pauseIndex = action->value;
                break;
            case ACTION_SET_STATE:
                if (action->flags & 8) {
                    if (action->value == 2.0f) {
                        platform->fan->on = 1 - platform->fan->on;
                    } else {
                        platform->fan->on = action->value;
                    }
                    if (platform->fan->on && platform->fan->strength < 0.0f) {
                        platform->actor.flags |= 0x400;
                    }
                }
                if (action->flags & 0x10) {
                    platform->teleport->active = action->value;
                }
                if (action->flags & 0x20) {
                    platform->catapult->active = action->value;
                }
                if (action->flags & 0x40) {
                    if (action->value != 0.0f) {
                        platform->flags |= 0x4000;
                    } else {
                        platform->flags &= ~0x4000;
                    }
                }
                break;
            case ACTION_SET_EXISTANCE:
                if (action->value != 0.0f) {
                    platform->actor.alpha = 100;
                    platform->flags &= ~4;
                } else {
                    if (platform->sample.handle) {
                        StopContinuousSample(&platform->sample);
                    }
                    if (platform->destruct) {
                        platform->flags |= 2;
                    } else {
                        platform->actor.alpha = 0;
                        platform->flags |= 4;
                    }
                }
                break;
            case ACTION_SET_GRAVITY:
                if (action->value != 0.0f) {
                    platform->actor.flags |= 1;
                } else {
                    platform->actor.flags &= ~1;
                }
                break;
            case ACTION_LOOK_AT_PLAT:
                if (!cameraInfo.overrideTimer) {
                    if (((Vec3 *)action->data)->x == 0.0f && ((Vec3 *)action->data)->y == 0.0f
                        && ((Vec3 *)action->data)->z == 0.0f) {
                        SetCameraOverride(camera.pos.x, camera.pos.y, camera.pos.z, &platform->actor.pos, &platform->actor.vel,
                                          action->value, action->flags);
                    } else {
                        SetCameraOverride(((Vec3 *)action->data)->x, ((Vec3 *)action->data)->y,
                                          ((Vec3 *)action->data)->z, &platform->actor.pos, &platform->actor.vel, action->value,
                                          action->flags);
                    }
                }
                break;
        }
    } while (platPtr != NULL);
}

s32 PuzzleEvaluateConditionList(PuzzleCondList *conditionList) {
    PuzzleCond *condition;
    s32 num;

    if (conditionList->requiredCount == 0) {
        for (condition = conditionList->head.prev; condition != &conditionList->head; condition = condition->prev) {
            if (!PuzzleEvaluateCondition(condition)) {
                return FALSE;
            }
        }
        return TRUE;
    }
    num = 0;
    for (condition = conditionList->head.prev; condition != &conditionList->head; condition = condition->prev) {
        if (PuzzleEvaluateCondition(condition) == TRUE) {
            num++;
            if (num == conditionList->requiredCount) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

s32 PuzzleEvaluateCondition(PuzzleCond *condition) {
    Collectable *collectable;
    GaribGroup *gGroup;
    Garib *garib;
    Platform *platPtr;
    Platform *next;
    Vec3 v;
    s32 j;
    s32 idx;

    switch (condition->type) {
        case CONDITION_PICKUP_ON:
            if (condition->value) {
                return player.powerups[condition->tag] > 0;
            }
            return player.powerups[condition->tag] == 0;
        case CONDITION_COLLECTED:
            switch (condition->value) {
                case 0:
                    for (collectable = collectableList.head.next; collectable != &collectableList.head; collectable = collectable->next) {
                        if (collectable->tag == condition->tag) {
                            return TRUE;
                        }
                    }
                    return FALSE;
                case 1:
                    for (collectable = collectableList.head.next; collectable != &collectableList.head; collectable = collectable->next) {
                        if (collectable->tag == condition->tag) {
                            return FALSE;
                        }
                    }
                    return TRUE;
                case 2:
                    for (gGroup = garibGroupList.head.prev; gGroup != &garibGroupList.head; gGroup = gGroup->prev) {
                        if (gGroup->tag == condition->tag) {
                            for (garib = gGroup->garibs.head.prev; garib != &gGroup->garibs.head; garib = garib->prev) {
                                if (garib->active) {
                                    return TRUE;
                                }
                            }
                        }
                    }
                    return FALSE;
                case 3:
                    for (gGroup = garibGroupList.head.prev; gGroup != &garibGroupList.head; gGroup = gGroup->prev) {
                        if (gGroup->tag == condition->tag) {
                            for (garib = gGroup->garibs.head.prev; garib != &gGroup->garibs.head; garib = garib->prev) {
                                if (garib->active) {
                                    return FALSE;
                                }
                            }
                        }
                    }
                    return TRUE;
            }
            // no break: any other unkA falls into case 18, as in the ROM
        case CONDITION_VAR_EQUAL:
            if (condition->tag >= 100) {
                return puzzleVal[condition->tag - 100] == puzzleVal[condition->value];
            }
            return puzzleVal[condition->tag] == condition->value;
        case CONDITION_VAR_NOT_EQUAL:
            if (condition->tag >= 100) {
                return puzzleVal[condition->tag - 100] != puzzleVal[condition->value];
            }
            return puzzleVal[condition->tag] != condition->value;
        case CONDITION_VAR_GREATER_THAN:
            if (condition->tag >= 100) {
                return puzzleVal[condition->tag - 100] > puzzleVal[condition->value];
            }
            return puzzleVal[condition->tag] > condition->value;
        case CONDITION_VAR_LESS_THAN:
            if (condition->tag >= 100) {
                return puzzleVal[condition->tag - 100] < puzzleVal[condition->value];
            }
            return puzzleVal[condition->tag] < condition->value;
        case CONDITION_NME_DEAD:
            if (condition->value) {
                for (j = 0; j < numberOfEnemies; j++) {
                    if (enemies[j].brain.tag == condition->tag && enemies[j].actor.active) {
                        return FALSE;
                    }
                }
                return TRUE;
            }
            for (j = 0; j < numberOfEnemies; j++) {
                if (enemies[j].brain.tag == condition->tag && enemies[j].actor.active) {
                    return TRUE;
                }
            }
            return FALSE;
        case CONDITION_HAND_IN_BOX:
            return IsPointWithinBox((Box *)condition->cam, &hand.pos);
        case CONDITION_HAND_IN_CYLINDER:
            v.x = condition->cam->pos.x;
            v.z = condition->cam->pos.y;
            if (DistanceBetweenPoints2D(&v, &hand.pos) < condition->cam->pos.z && hand.pos.y <= condition->cam->size.x
                && condition->cam->size.y <= hand.pos.y) {
                return TRUE;
            }
            return FALSE;
        case CONDITION_HAND_IN_RANGE:
            return DistanceBetweenPointsSquared(&hand.pos, &condition->cam->pos) < condition->cam->radius * condition->cam->radius;
        case CONDITION_BALL_IN_BOX:
            return IsPointWithinBox((Box *)condition->cam, &ball.pos);
        case CONDITION_BALL_IN_RANGE:
            return DistanceBetweenPointsSquared(&ball.pos, &condition->cam->pos) < condition->cam->radius * condition->cam->radius;
        case CONDITION_CAM_IN_BOX:
            return IsPointWithinBox((Box *)condition->cam, &camera.pos);
        case CONDITION_CAM_IN_RANGE:
            return DistanceBetweenPointsSquared(&camera.pos, &condition->cam->pos) < condition->cam->radius * condition->cam->radius;
        default:
            next = &platformList.head;
            do {
                next = FindTaggedPlatform(next, condition->tag);
                if (next == NULL) {
                    return TRUE;
                }
                platPtr = next;
                switch (condition->type) {
                    case CONDITION_AT_POINT:
                        if (platPtr->path->current != condition->value || platPtr->actor.speed != 0.0f) {
                            return FALSE;
                        }
                        break;
                    case CONDITION_JUST_AT_POINT:
                        if (condition->value == -1) {
                            if (platPtr->actor.speed == 0.0f) {
                                return FALSE;
                            }
                            SUB_VECTORS(&v, &platPtr->actor.pos, &platPtr->path->points[platPtr->path->current]->pos);
                            if (platPtr->actor.speed <= LENGTH(&v)) {
                                return TRUE;
                            }
                            return FALSE;
                        }
                        if (platPtr->path->current != condition->value
                            || platPtr->path->moving != platPtr->path->points[platPtr->path->current]->time) {
                            return FALSE;
                        }
                        break;
                    case CONDITION_AT_SPIN:
                        if (condition->value == -1) {
                            if (platPtr->motion->speed != 0.0f) {
                                return FALSE;
                            }
                            break;
                        }
                        if ((platPtr->motion->pauseIndex + platPtr->motion->pauseType - 1) % platPtr->motion->pauseType != condition->value
                            || platPtr->motion->speed != 0.0f) {
                            return FALSE;
                        }
                        break;
                    case CONDITION_JUST_AT_SPIN:
                        if ((platPtr->motion->pauseIndex + platPtr->motion->pauseType - 1) % platPtr->motion->pauseType != condition->value
                            || platPtr->motion->timer != platPtr->motion->pause) {
                            return FALSE;
                        }
                        break;
                    case CONDITION_AT_ORBIT:
                        if ((platPtr->orbit->pauseIndex + platPtr->orbit->pauseType - 1) % platPtr->orbit->pauseType != condition->value
                            || platPtr->orbit->timer == 0) {
                            return FALSE;
                        }
                        break;
                    case CONDITION_JUST_AT_ORBIT:
                        if ((platPtr->orbit->pauseIndex + platPtr->orbit->pauseType - 1) % platPtr->orbit->pauseType != condition->value
                            || platPtr->orbit->timer != platPtr->orbit->pause) {
                            return FALSE;
                        }
                        break;
                    case CONDITION_HAND_ON:
                        if ((hand.obj->platform == platPtr && condition->value) || (hand.obj->platform != platPtr && !condition->value)) {
                            return TRUE;
                        }
                        return FALSE;
                    case CONDITION_HAND_JUST_ON:
                        if ((condition->value == 0 && hand.obj->prevPlatform == platPtr && hand.obj->platform != platPtr)
                            || (hand.obj->platform == platPtr && hand.obj->prevPlatform != platPtr
                                && (condition->value == 1 || (condition->value == 2 && handControl.actionTime < 3 && hand.state->mode == HAND_SLAM2ST)))) {
                            return TRUE;
                        }
                        return FALSE;
                    case CONDITION_BALL_ON:
                        if ((ball.obj->platform == platPtr && condition->value) || (ball.obj->platform != platPtr && !condition->value)) {
                            return TRUE;
                        }
                        return FALSE;
                    case CONDITION_BALL_JUST_ON:
                        if ((condition->value && ball.obj->platform == platPtr && ball.obj->prevPlatform != platPtr)
                            || (!condition->value && ball.obj->prevPlatform == platPtr && ball.obj->platform != platPtr)) {
                            return TRUE;
                        }
                        return FALSE;
                    case CONDITION_NME_ON:
                        if (condition->value) {
                            for (j = 0; j < numberOfEnemies; j++) {
                                if (enemies[j].actor.obj->platform == platPtr) {
                                    return TRUE;
                                }
                            }
                            return FALSE;
                        }
                        for (j = 0; j < numberOfEnemies; j++) {
                            if (enemies[j].actor.obj->platform == platPtr) {
                                break;
                            }
                        }
                        if (j != numberOfEnemies) {
                            return FALSE;
                        }
                        break;
                    case CONDITION_NME_JUST_ON:
                        if (condition->value) {
                            for (j = 0; j < numberOfEnemies; j++) {
                                if (enemies[j].actor.obj->platform == platPtr && enemies[j].actor.obj->prevPlatform != platPtr) {
                                    return TRUE;
                                }
                            }
                            return FALSE;
                        }
                        for (j = 0; j < numberOfEnemies; j++) {
                            if (enemies[j].actor.obj->prevPlatform == platPtr && enemies[j].actor.obj->platform != platPtr) {
                                return TRUE;
                            }
                        }
                        return FALSE;
                    case CONDITION_HAND_TOUCHING:
                        if ((hand.obj->contactPlatform == platPtr && condition->value) || (hand.obj->contactPlatform != platPtr && !condition->value)) {
                            return TRUE;
                        }
                        return FALSE;
                    case CONDITION_HAND_JUST_TOUCHED:
                        if ((condition->value && hand.obj->contactPlatform == platPtr && hand.obj->prevContactPlatform != platPtr)
                            || (!condition->value && hand.obj->prevContactPlatform == platPtr && hand.obj->contactPlatform != platPtr)) {
                            return TRUE;
                        }
                        return FALSE;
                    case CONDITION_BALL_TOUCHING:
                        if ((ball.obj->contactPlatform == platPtr && condition->value) || (ball.obj->contactPlatform != platPtr && !condition->value)) {
                            return TRUE;
                        }
                        return FALSE;
                    case CONDITION_BALL_JUST_TOUCHED:
                        if ((condition->value && ball.obj->contactPlatform == platPtr && ball.obj->prevContactPlatform != platPtr)
                            || (!condition->value && ball.obj->prevContactPlatform == platPtr && ball.obj->contactPlatform != platPtr)) {
                            return TRUE;
                        }
                        return FALSE;
                    case CONDITION_NME_TOUCHING:
                        if (condition->value) {
                            for (j = 0; j < numberOfEnemies; j++) {
                                if (enemies[j].actor.obj->contactPlatform == platPtr) {
                                    return TRUE;
                                }
                            }
                            return FALSE;
                        }
                        for (j = 0; j < numberOfEnemies; j++) {
                            if (enemies[j].actor.obj->contactPlatform == platPtr) {
                                break;
                            }
                        }
                        if (j != numberOfEnemies) {
                            return FALSE;
                        }
                        break;
                    case CONDITION_NME_JUST_TOUCHED:
                        if (condition->value) {
                            for (j = 0; j < numberOfEnemies; j++) {
                                if (enemies[j].actor.obj->contactPlatform == platPtr && enemies[j].actor.obj->prevContactPlatform != platPtr) {
                                    return TRUE;
                                }
                            }
                            return FALSE;
                        }
                        for (j = 0; j < numberOfEnemies; j++) {
                            if (enemies[j].actor.obj->prevContactPlatform == platPtr && enemies[j].actor.obj->contactPlatform != platPtr) {
                                return TRUE;
                            }
                        }
                        return FALSE;
                    case CONDITION_CONFINED:
                        idx = condition->value;
                        if (idx < 3) {
                            if (platPtr->parent != NULL) {
                                if (((Vec3Arr *)&platPtr->actor.pos)->v[idx] - ((Vec3Arr *)&platPtr->parent->actor.pos)->v[idx]
                                    != platPtr->confine->min[idx]) {
                                    return FALSE;
                                }
                            } else if (((Vec3Arr *)&platPtr->actor.pos)->v[idx] != platPtr->confine->min[idx]) {
                                return FALSE;
                            }
                        } else if (platPtr->parent != NULL) {
                            if (((Vec3Arr *)&platPtr->actor.pos)->v[idx - 3] - ((Vec3Arr *)&platPtr->parent->actor.pos)->v[idx - 3]
                                != platPtr->confine->min[idx - 3] + platPtr->confine->size[idx - 3]) {
                                return FALSE;
                            }
                        } else if (((Vec3Arr *)&platPtr->actor.pos)->v[idx - 3] != platPtr->confine->min[idx - 3] + platPtr->confine->size[idx - 3]) {
                            return FALSE;
                        }
                        break;
                    case CONDITION_JUST_CONFINED:
                        idx = condition->value;
                        if (idx < 3) {
                            if (platPtr->parent != NULL) {
                                if (ABS(((Vec3Arr *)&platPtr->actor.pos)->v[idx] - ((Vec3Arr *)&platPtr->parent->actor.pos)->v[idx] - platPtr->confine->min[idx]) > 0.0001
                                    || ABS(((Vec3Arr *)&platPtr->actor.platPrevPos)->v[idx] - ((Vec3Arr *)&platPtr->parent->actor.platPrevPos)->v[idx] - platPtr->confine->min[idx]) <= 0.0001) {
                                    return FALSE;
                                }
                            } else {
                                if (ABS(((Vec3Arr *)&platPtr->actor.pos)->v[idx] - platPtr->confine->min[idx]) > 0.0001
                                    || ABS(((Vec3Arr *)&platPtr->actor.prevPos)->v[idx] - platPtr->confine->min[idx]) <= 0.0001) {
                                    return FALSE;
                                }
                            }
                        } else if (platPtr->parent != NULL) {
                            if (ABS(((Vec3Arr *)&platPtr->actor.pos)->v[idx - 3] - ((Vec3Arr *)&platPtr->parent->actor.pos)->v[idx - 3] - platPtr->confine->min[idx - 3] - platPtr->confine->size[idx - 3]) > 0.0001
                                || ABS(((Vec3Arr *)&platPtr->actor.platPrevPos)->v[idx - 3] - ((Vec3Arr *)&platPtr->parent->actor.platPrevPos)->v[idx - 3] - platPtr->confine->min[idx - 3] - platPtr->confine->size[idx - 3]) <= 0.0001) {
                                return FALSE;
                            }
                        } else {
                            if (ABS(((Vec3Arr *)&platPtr->actor.pos)->v[idx - 3] - platPtr->confine->min[idx - 3] - platPtr->confine->size[idx - 3]) > 0.0001
                                || ABS(((Vec3Arr *)&platPtr->actor.prevPos)->v[idx - 3] - platPtr->confine->min[idx - 3] - platPtr->confine->size[idx - 3]) <= 0.0001) {
                                return FALSE;
                            }
                        }
                        break;
                    case CONDITION_DESTROYED:
                        return FALSE;
                }
            } while (next != NULL);
            return TRUE;
        case CONDITION_HAND_CROSSES_LINE:
            return TRUE;
    }
}

s32 IsCameraAction(s16 type) {
    return type == 25 || type == 26 || type == 27 || type == 28 || type == 29 || type == 30 ||
           type == 31 || type == 32 || type == 41 || type == 39 || type == 40;
}

s32 IsCameraCondition(s16 type) {
    return type == 25 || type == 26 || type == 27 || type == 28 || type == 29 || type == 30 || type == 31 ||
           type == 32;
}

INCLUDE_ASM("asm/nonmatchings/puzzle", pad_80191114);

// linker junk that follows the last jump table emitted by PuzzleEvaluateCondition
const u32 D_80109A48[] = { 0x00000000, 0x00000000 };
