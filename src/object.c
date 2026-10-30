#include "common.h"
#include "ident.h"
#include "actor.h"
#include "jalloc.h"
#include "display.h"
#include "camera.h"
#include "ball.h"
#include "bullet.h"
#include "debris.h"
#include "game.h"
#include "overlay.h"
#include "platform.h"
#include "banks.h"
#include "blur.h"
#include "footprint.h"
#include "light.h"
#include "timer.h"
#include "object.h"
#include "anim.h"
#include "enemy.h"
#include "shadow.h"

extern s16 farPlaneDist;
extern Quat zeroQuat;
u32 UpdateCRC(char *ptr);
extern ObjectInstance *crystalBall;
extern ObjectInstance *crystalBallBit;
extern ObjectInstance *snowBallBit;
extern ObjectInstance *boulderBit;
extern ObjectInstance *icicleBit;
extern ObjectInstance *bombExplosion;
extern ObjectInstance *frogObj;
extern Actor extraBall[3];
extern Actor plaster[4];
extern s32 actbehave;
extern s32 actcoll;
extern ShadowVtx shadvtx;
extern u8 *tempObjectPtr;
extern Object *tempObjectPtr2;
extern f32 D_80284D74[4][4];
extern s32 texmode;
extern f32 texval1;
extern f32 texval2;
extern s32 frameCount;
f32 SineWave(f32 freq, f32 phase, s32 base);
extern Vec3 actorScale;
extern Mtx *staticObjectMtx;
extern Vec3 parentScaleStack[];
extern s32 parentScaleStackLevel;
extern s32 matrixStackCount;
extern u8 calcMtx;
extern u8 staticObj;
extern u8 noClip;
extern u8 gamePaused;
extern u8 animateActors;
extern Actor *currentDrawActor;
extern s16 xluOverride;

void PushMatrix(f32 mf[4][4]);
void PopMatrix(void);
void ComputeResultMatrix(void);
void QuaternionToMatrix(Quat *q, f32 mf[4][4]);
void QuatSlerp(Quat *src1, Quat *sp2, f32 t, Quat *dquat);
void MatrixToQuaternion(f32 mf[4][4], Quat *q);

void QuaternionMultiply(Quat *dest, Quat *src1, Quat *src2);

INCLUDE_ASM("asm/nonmatchings/object", pad_801425C8);

extern ActorList actorDistList;
extern Vtx objectsVtx[2][800];
extern Vtx *vtxPtr;
extern u8 myMicrocode;
extern u8 UseUCode;
extern u8 UseZMode;
extern u8 draw_buffer;
extern u8 aiSurf;
extern s8 mirrorTextures;
extern Actor *foregroundObjects[10];
extern Debris debris[120];
extern s32 numForegroundObjects;
f32 DistanceBetweenPointsSquared(Vec3 *v1, Vec3 *v2);
void InitDisplayLists(void);
void SetRenderMode(void);
void SetupViewing(void);
void ApplyGlobalTransformations(void);
void ProcessBackgroundLights(Actor *actor);
void ClearViewing(void);
void CleanupAndSendDisplayList(s32 flag, s32 flags);
extern u8 changeRenderMode;
extern u8 transparentSurf;
extern u8 xluSurf;
extern s16 xluFact;
struct Platform *TagToFirstPlatform(s32 tag);

s32 InitObject(Object **objp, s16 flags) {
    s32 size = *(s32 *)tempObjectPtr;

    *objp = tempObjectPtr2 = (Object *)(tempObjectPtr + 4);
    if ((*objp)->initialised) {
        return 0;
    }
    (*objp)->flags = flags;
    (*objp)->mesh = (Mesh *)(tempObjectPtr + (u32)(*objp)->mesh);
    (*objp)->scaleKeys = (PosKey *)(tempObjectPtr + (u32)(*objp)->scaleKeys);
    (*objp)->posKeys = (PosKey *)(tempObjectPtr + (u32)(*objp)->posKeys);
    (*objp)->rotKeys = (RotKey *)(tempObjectPtr + (u32)(*objp)->rotKeys);
    (*objp)->mesh->verts = (Vec3 *)(tempObjectPtr + (u32)(*objp)->mesh->verts);
    (*objp)->mesh->faces = (FaceIndices *)(tempObjectPtr + (u32)(*objp)->mesh->faces);
    tempObjectPtr += size;
    if ((*objp)->child) {
        InitObject(&(*objp)->child, flags);
    }
    if ((*objp)->sibling) {
        InitObject(&(*objp)->sibling, flags);
    }
    (*objp)->initialised = TRUE;
    return size;
}

void InitActorStructures(Actor *tempActor, s32 initFlags) {
    char name[24];
    s32 i;

    tempActor->scale.x = tempActor->scale.y = tempActor->scale.z = 1.0f;
    if (initFlags & 1) {
        sprintf(name, "ActBeh%d", actbehave++);
        tempActor->phys = JallocAlloc(sizeof(ActorPhysics), 1, name);
        tempActor->phys->bottomDiv = 2.5f;
    }
    if (initFlags & 2) {
        sprintf(name, "ActCol%d", actcoll++);
        tempActor->obj = JallocAlloc(sizeof(ActorObject), 1, name);
        tempActor->obj->numSpheres = 1;
        tempActor->obj->spheres[0].flags = 32;
        for (i = 0; i < 4; i++) {
            tempActor->obj->planes[i] = NULL;
            tempActor->obj->contactCount[i] = 0;
        }
    }
    if ((initFlags & 4) && tempActor->inst && tempActor->inst->anim) {
        tempActor->anim = JallocAlloc(sizeof(AnimState), 1, "Act_Anim");
        InitActorAnim(tempActor);
    }
    if (initFlags & 8) {
        tempActor->shadow = JallocAlloc(sizeof(ActorShadow), 1, "Act_Shad");
        tempActor->shadow->shadow = shadvtx;
        tempActor->shadow->shadowAlpha = tempActor->shadow->shadowTarget = 255;
        tempActor->shadow->shadowStep = 10;
    }
    if (initFlags & 16) {
        tempActor->state = JallocAlloc(sizeof(ActorState), 1, "Act_Stats");
        tempActor->state->index = -1;
        tempActor->state->squash = 0;
        tempActor->state->squashXZ = 1.0f;
        tempActor->state->squashY = 1.0f;
    }
    tempActor->active = 1;
    tempActor->alpha = 100;
}

inline void InitActor(Actor *tempActor, char *name, f32 x, f32 y, f32 z, s32 initFlags) {
    FindObject(&tempActor->inst, UpdateCRC(name), name, 1);
    if (tempActor->inst) {
        InitActorStructures(tempActor, initFlags);
        tempActor->pos.x = x;
        tempActor->pos.y = y;
        tempActor->pos.z = z;
        tempActor->scale.x = tempActor->scale.y = tempActor->scale.z = 1.0f;
        tempActor->rotQuat = zeroQuat;
    }
}

void InitObjects(void) {
    char name[16];
    Actor *actor;
    s32 i;

    actor = &hand;
    actor->next = NULL;
    actor->flags |= 4;
    if (gameInfo.level == SPACEBOSS3) {
        InitActor(actor, "robot.ndo", 0.0f, 0.0f, 0.0f, -1);
        actor->scale.x = actor->scale.y = actor->scale.z = 0.5f;
        MakeUniqueActor(actor, 38);
    } else {
        if (gameInfo.level == FLYTHRU || gameInfo.level == FLYTHRU2) {
            InitActor(actor, "hoop.ndo", 0.0f, 0.0f, 0.0f, -1);
        } else {
            InitActor(actor, "goodglov.ndo", 0.0f, 0.0f, 0.0f, -1);
        }
        actor->scale.x = actor->scale.y = actor->scale.z = 0.05f;
    }
    AddActor(actor);
    actor = &ball;
    actor->next = NULL;
    actor->flags |= 4;
    InitActor(actor, "gball.ndo", 0.0f, 0.0f, 0.0f, 0x1A);
    AddActor(actor);
    actor->scale.x = actor->scale.y = actor->scale.z = 0.05f;
    sprintf(name, "crystalball.ndo");
    FindObject(&crystalBall, UpdateCRC(name), name, 0);
    sprintf(name, "ballbit.ndo");
    FindObject(&crystalBallBit, UpdateCRC(name), name, 0);
    sprintf(name, "snowbit.ndo");
    FindObject(&snowBallBit, UpdateCRC(name), name, 0);
    sprintf(name, "boulderbit.ndo");
    FindObject(&boulderBit, UpdateCRC(name), name, 0);
    sprintf(name, "frag.ndo");
    FindObject(&icicleBit, UpdateCRC(name), name, 0);
    sprintf(name, "explosion.ndo");
    FindObject(&bombExplosion, UpdateCRC(name), name, 0);
    sprintf(name, "nmefrog.ndo");
    FindObject(&frogObj, UpdateCRC(name), name, 0);
    for (i = 0; i < 3; i++) {
        actor = &extraBall[i];
        actor->next = NULL;
        actor->flags |= 4;
        InitActor(actor, "gball.ndo", 0.0f, 0.0f, 0.0f, 0);
        actor->scale.x = actor->scale.y = actor->scale.z = 0.022222223f;
    }
    for (i = 0; i < 4; i++) {
        actor = &plaster[i];
        actor->next = NULL;
        actor->flags |= 4;
        InitActor(actor, "plaster.ndo", 0.0f, 0.0f, 0.0f, 0x10);
        actor->scale.x = actor->scale.y = actor->scale.z = 0.05f;
    }
    for (i = 0; i < 30; i++) {
        if (!standardBullets[i].numSprites) {
            FindObject(&standardBullets[i].inst, UpdateCRC(standardBullets[i].name), standardBullets[i].name, 0);
        }
    }
}

void CalculateStaticMatrices(void) {
    f32 rotmat[4][4];
    f32 scalemat[4][4];
    f32 transmat[4][4];
    Actor *ptr;

    for (ptr = actorList.head.next; ptr != &actorList.head; ptr = ptr->next) {
        if (ptr->type == LANDSCAPE_ACTOR) {
            if (ptr->inst) {
                JallocFree(&ptr->staticMtx);
            }
            ptr->staticMtx = JallocAlloc(sizeof(Mtx), 1, "statMtx");
        } else {
            ptr->staticMtx = NULL;
            continue;
        }
        guTranslateF(transmat, ptr->pos.x, ptr->pos.y, ptr->pos.z);
        PushMatrix(transmat);
        QuaternionToMatrix(&ptr->rotQuat, rotmat);
        PushMatrix(rotmat);
        guScaleF(scalemat, ptr->scale.x, ptr->scale.y, ptr->scale.z);
        PushMatrix(scalemat);
        ComputeResultMatrix();
        guMtxF2L(D_80284D74, ptr->staticMtx);
        PopMatrix();
        PopMatrix();
        PopMatrix();
    }
}

void CreateObjects(void) {
    Actor *ptr;
    struct Bullet *bullet;
    GameSprite *sprite;
    s32 i;

    StartTimer(3, "NEAR");
    if (ballControl.numPlasters) {
        UpdatePlasters();
    }
    actorDistList.count = 0;
    actorDistList.head.distNext = actorDistList.head.distPrev = &actorDistList.head;
    for (ptr = actorList.head.next; ptr != &actorList.head; ptr = ptr->next) {
        ptr->distSq = DistanceBetweenPointsSquared(&ptr->pos, &camera.pos);
        AddActorToDistList(ptr);
    }
    EndTimer(3);
    objectMatrix = 0;
    UseZMode = 1;
    noClip = 0;
    vtxPtr = objectsVtx[draw_buffer];
    StartTimer(3, "NEAR");
    UseUCode = myMicrocode;
    InitDisplayLists();
    SetRenderMode();
    SetupViewing();
    ApplyGlobalTransformations();
    if (fog.on) {
        gDPSetFogColor(glistp++, fog.r, fog.g, fog.b, 255);
        gSPFogPosition(glistp++, fog.min, fog.max);
    }
    for (ptr = actorDistList.head.distNext; ptr != &actorDistList.head; ptr = ptr->distNext) {
        if (!(ptr->inst->obj->flags & 2) && !(ptr->inst->obj->flags & 0x80)) {
            if (!(ptr->inst->obj->flags & 8)) {
                ProcessBackgroundLights(ptr);
            }
            DrawActor(ptr);
        }
    }
    for (i = 0; i < 120; i++) {
        if (debris[i].active && debris[i].inst) {
            ProcessBackgroundLights((Actor *)&debris[i]);
            DrawActor((Actor *)&debris[i]);
        }
    }
    for (bullet = bulletList.head.next; bullet != &bulletList.head; bullet = bullet->next) {
        if (bullet->actor.inst != NULL && !(bullet->actor.inst->obj->flags & 2)) {
            ProcessBackgroundLights(&bullet->actor);
            DrawActor(&bullet->actor);
        }
    }
    if (spriteList.count) {
        sprite = PrintSpritesOpaque();
        UseUCode = myMicrocode;
        gDPPipeSync(glistp++);
        InitDisplayLists();
        SetRenderMode();
        SetupViewing();
        ApplyGlobalTransformations();
    }
    for (ptr = actorDistList.head.distPrev; ptr != &actorDistList.head; ptr = ptr->distPrev) {
        if ((ptr->inst->obj->flags & 2) && !(ptr->inst->obj->flags & 0x80)) {
            if (!(ptr->inst->obj->flags & 8)) {
                ProcessBackgroundLights(ptr);
            }
            DrawActor(ptr);
        }
    }
    for (bullet = bulletList.head.next; bullet != &bulletList.head; bullet = bullet->next) {
        if (bullet->actor.inst != NULL && (bullet->actor.inst->obj->flags & 2)) {
            ProcessBackgroundLights(&bullet->actor);
            DrawActor(&bullet->actor);
        }
    }
    if (spriteList.count) {
        PrintSpritesTranslucent(sprite);
    }
    if (gameInfo.mode == 4 || gameInfo.mode == 5) {
        DrawHelper();
        aiSurf = 1;
        for (ptr = actorDistList.head.distNext; ptr != &actorDistList.head; ptr = ptr->distNext) {
            if (ptr->inst->obj->flags & 0x80) {
                if (!(ptr->inst->obj->flags & 8)) {
                    ProcessBackgroundLights(ptr);
                }
                SetAIRenderMode(ptr->alpha);
                DrawActor(ptr);
            }
        }
        aiSurf = 0;
        mirrorTextures = 1;
        ProcessShadows();
        DrawFootprints();
        UpdateLightning();
        DrawBlurs();
        mirrorTextures = 0;
    }
    ClearViewing();
    noClip = 1;
    SetRenderMode();
    for (i = 0; i < numForegroundObjects; i++) {
        foregroundObjects[i]->distSq = 0;
        if (!(foregroundObjects[i]->inst->obj->flags & 8)) {
            ProcessBackgroundLights(foregroundObjects[i]);
        }
        DrawActor(foregroundObjects[i]);
    }
    CleanupAndSendDisplayList(0, 0);
    EndTimer(3);
    gDPPipeSync(glistp++);
    gDPSetCycleType(glistp++, G_CYC_1CYCLE);
}

void DrawActor(Actor *ptr) {
    f32 rotmat[4][4];
    f32 scalemat[4][4];
    f32 transmat[4][4];
    Quat tempQ;
    f32 animTime = 0.0f;

    staticObjectMtx = ptr->staticMtx;
    if (staticObjectMtx != NULL && !calcMtx) {
        staticObj = TRUE;
    } else {
        staticObj = FALSE;
    }
    if (!noClip && !IsActorVisible(ptr)) {
        return;
    }
    if (!gamePaused && animateActors
        && (ptr->state == NULL || ptr->state->index == -1 || enemies[ptr->state->index].brain.spellTimer[0] == 0)) {
        UpdateAnimations(ptr);
    }
    if (!ptr->alpha) {
        return;
    }
    matrixStackCount = 0;
    if (calcMtx == TRUE || !staticObj) {
        guTranslateF(transmat, ptr->pos.x, ptr->pos.y, ptr->pos.z);
        PushMatrix(transmat);
        if (ptr->state != NULL && ptr->state->squash != 0.0f) {
            guScaleF(scalemat, ptr->state->squashXZ, ptr->state->squashY, ptr->state->squashXZ);
            QuaternionToMatrix(&ptr->state->squashQ, rotmat);
            guMtxCatF(scalemat, rotmat, scalemat);
            PushMatrix(scalemat);
            tempQ = ptr->state->squashQ;
            tempQ.w = -tempQ.w;
            QuaternionToMatrix(&tempQ, transmat);
            PushMatrix(transmat);
        }
        QuaternionToMatrix(&ptr->rotQuat, rotmat);
        PushMatrix(rotmat);
        actorScale.x = ptr->scale.x;
        actorScale.y = ptr->scale.y;
        actorScale.z = ptr->scale.z;
        parentScaleStack[0].x = parentScaleStack[0].y = parentScaleStack[0].z = 1.0f;
        parentScaleStackLevel = 0;
        animTime = 0.0f;
        if (ptr->anim != NULL && ptr->anim->anims != NULL) {
            animTime = ptr->anim->time;
        }
    }
    currentDrawActor = ptr;
    xluOverride = ptr->alpha;
    if (ptr->inst->obj != NULL) {
        TransformAndDrawObject(ptr->inst->obj, animTime, 0, 0);
    }
    if (calcMtx == TRUE || !staticObj) {
        PopMatrix();
        PopMatrix();
        if (ptr->state != NULL && ptr->state->squash != 0.0f) {
            PopMatrix();
            PopMatrix();
        }
    }
}

void TransformAndDrawObject(Object *obj, f32 time, s16 doKeys, s16 arg3) {
    Quat tempQuat;
    f32 rotmat[4][4];
    f32 scalemat[4][4];
    f32 transmat[4][4];
    Vec3 tempVect;
    f32 XTranslateAmount;
    f32 YTranslateAmount;
    f32 ZTranslateAmount;
    f32 XScaleAmount;
    f32 YScaleAmount;
    f32 ZScaleAmount;
    f32 tempFloat;
    s16 low;
    s16 high;
    s16 fromKey;
    s16 toKey;
    s32 i;
    s32 a;
    GameSprite *sprite;

    XScaleAmount = 1.0f;
    YScaleAmount = 1.0f;
    ZScaleAmount = 1.0f;
    toKey = -1;
    if (calcMtx == 1 || !staticObj) {
        fromKey = -1;
        if (obj->numPosKeys >= 2) {
            low = 0;
            high = obj->numPosKeys - 1;
            while (low <= high) {
                i = (low + high) >> 1;
                if (obj->posKeys[i].time == time) {
                    break;
                }
                if (time < obj->posKeys[i].time) {
                    high = i - 1;
                } else {
                    low = i + 1;
                }
            }
            if (low > 0) {
                low--;
            }
            for (i = low; i < obj->numPosKeys; i++) {
                if (time < obj->posKeys[i].time) {
                    toKey = i;
                    fromKey = i - 1;
                    break;
                }
            }
            if (fromKey != -1) {
                tempFloat = abs(obj->posKeys[toKey].time - obj->posKeys[fromKey].time);
                tempFloat = (time - obj->posKeys[fromKey].time) / tempFloat;
            } else {
                tempFloat = 1.0f;
                toKey = fromKey = obj->numPosKeys - 1;
            }
            tempVect.x = obj->posKeys[fromKey].pos.x + (obj->posKeys[toKey].pos.x - obj->posKeys[fromKey].pos.x) * tempFloat;
            tempVect.y = obj->posKeys[fromKey].pos.y + (obj->posKeys[toKey].pos.y - obj->posKeys[fromKey].pos.y) * tempFloat;
            tempVect.z = obj->posKeys[fromKey].pos.z + (obj->posKeys[toKey].pos.z - obj->posKeys[fromKey].pos.z) * tempFloat;
            XTranslateAmount = tempVect.x;
            YTranslateAmount = tempVect.y;
            ZTranslateAmount = tempVect.z;
        } else {
            XTranslateAmount = obj->posKeys[0].pos.x;
            YTranslateAmount = obj->posKeys[0].pos.y;
            ZTranslateAmount = obj->posKeys[0].pos.z;
        }
        toKey = -1;
        if (obj->numScaleKeys >= 2) {
            fromKey = -1;
            low = 0;
            high = obj->numScaleKeys - 1;
            while (low <= high) {
                i = (low + high) >> 1;
                if (obj->scaleKeys[i].time == time) {
                    break;
                }
                if (time < obj->scaleKeys[i].time) {
                    high = i - 1;
                } else {
                    low = i + 1;
                }
            }
            if (low > 0) {
                low--;
            }
            for (i = low; i < obj->numScaleKeys; i++) {
                if (time < obj->scaleKeys[i].time) {
                    toKey = i;
                    fromKey = i - 1;
                    break;
                }
            }
            if (fromKey != -1) {
                tempFloat = abs(obj->scaleKeys[toKey].time - obj->scaleKeys[fromKey].time);
                tempFloat = (time - obj->scaleKeys[fromKey].time) / tempFloat;
            } else {
                tempFloat = 1.0f;
                toKey = fromKey = obj->numScaleKeys - 1;
            }
            tempVect.x = obj->scaleKeys[fromKey].pos.x + (obj->scaleKeys[toKey].pos.x - obj->scaleKeys[fromKey].pos.x) * tempFloat;
            tempVect.y = obj->scaleKeys[fromKey].pos.y + (obj->scaleKeys[toKey].pos.y - obj->scaleKeys[fromKey].pos.y) * tempFloat;
            tempVect.z = obj->scaleKeys[fromKey].pos.z + (obj->scaleKeys[toKey].pos.z - obj->scaleKeys[fromKey].pos.z) * tempFloat;
            XScaleAmount = tempVect.x;
            YScaleAmount = tempVect.y;
            ZScaleAmount = tempVect.z;
        } else {
            XScaleAmount = obj->scaleKeys[0].pos.x;
            YScaleAmount = obj->scaleKeys[0].pos.y;
            ZScaleAmount = obj->scaleKeys[0].pos.z;
        }
        if (obj->numRotKeys >= 2) {
            if (obj->flags & 0x40) {
                time = frameCount % 40 + 1;
            }
            toKey = -1;
            fromKey = -1;
            low = 0;
            high = obj->numRotKeys - 1;
            while (low <= high) {
                i = (low + high) >> 1;
                if (obj->rotKeys[i].time == time) {
                    break;
                }
                if (time < obj->rotKeys[i].time) {
                    high = i - 1;
                } else {
                    low = i + 1;
                }
            }
            if (low > 0) {
                low--;
            }
            for (i = low; i < obj->numRotKeys; i++) {
                if (time < obj->rotKeys[i].time) {
                    toKey = i;
                    fromKey = i - 1;
                    break;
                }
            }
            if (fromKey != -1) {
                tempFloat = abs(obj->rotKeys[toKey].time - obj->rotKeys[fromKey].time);
                tempFloat = (time - obj->rotKeys[fromKey].time) / tempFloat;
            } else {
                tempFloat = 1.0f;
                toKey = fromKey = obj->numRotKeys - 1;
            }
            QuatSlerp(&obj->rotKeys[fromKey].q, &obj->rotKeys[toKey].q, tempFloat, &tempQuat);
            QuaternionToMatrix(&tempQuat, rotmat);
            if (obj->sphere != NULL && obj->sphere->platTag[1]) {
                QuaternionMultiply(&TagToFirstPlatform(obj->sphere->platTag[1])->actor.rotQuat, &currentDrawActor->rotQuat, &tempQuat);
            }
        } else {
            QuaternionToMatrix(&obj->rotKeys[0].q, rotmat);
            if (obj->sphere != NULL && obj->sphere->platTag[1]) {
                QuaternionMultiply(&TagToFirstPlatform(obj->sphere->platTag[1])->actor.rotQuat, &currentDrawActor->rotQuat,
                                   &obj->rotKeys[0].q);
            }
        }
        guTranslateF(transmat, XTranslateAmount * actorScale.x * parentScaleStack[parentScaleStackLevel].x,
                     YTranslateAmount * actorScale.y * parentScaleStack[parentScaleStackLevel].y,
                     ZTranslateAmount * actorScale.z * parentScaleStack[parentScaleStackLevel].z);
        rotmat[3][0] = transmat[3][0];
        rotmat[3][1] = transmat[3][1];
        rotmat[3][2] = transmat[3][2];
        rotmat[3][3] = transmat[3][3];
        PushMatrix(rotmat);
        guScaleF(scalemat, XScaleAmount * actorScale.x, YScaleAmount * actorScale.y, ZScaleAmount * actorScale.z);
        PushMatrix(scalemat);
    }
    changeRenderMode = FALSE;
    if (!aiSurf) {
        if (obj->flags & 4) {
            if (!transparentSurf) {
                transparentSurf = TRUE;
                changeRenderMode = TRUE;
            }
        } else if (transparentSurf == TRUE) {
            transparentSurf = FALSE;
            changeRenderMode = TRUE;
        }
        if (obj->flags & 2) {
            if (!xluSurf) {
                xluSurf = TRUE;
                changeRenderMode = TRUE;
            }
            if (obj->displayList == NULL) {
                xluSurf = TRUE;
            }
            tempFloat = xluOverride / 100.0f;
            tempFloat *= obj->alpha[0];
            if (tempFloat > 255.0f) {
                tempFloat = 255.0f;
            }
            if ((s32)tempFloat != xluFact) {
                changeRenderMode = TRUE;
            }
            xluFact = tempFloat;
        } else if (xluSurf == TRUE) {
            xluSurf = FALSE;
            changeRenderMode = TRUE;
        }
        if (changeRenderMode) {
            SetRenderMode();
        }
    } else {
        gDPSetPrimColor(glistp++, 0, 0, 255, 255, 255, obj->alpha[0]);
    }
    if (calcMtx == 1 || !staticObj) {
        ComputeResultMatrix();
        guMtxF2L(D_80284D74, &dynamicp->objectMtx[objectMatrix]);
        if (calcMtx == 1 && staticObjectMtx != NULL) {
            guMtxF2L(D_80284D74, staticObjectMtx);
        }
        gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&dynamicp->objectMtx[objectMatrix++]), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
        matrixStackCount++;
    } else {
        dynamicp->objectMtx[objectMatrix] = *staticObjectMtx;
        gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&dynamicp->objectMtx[objectMatrix++]), G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_PUSH);
        matrixStackCount++;
    }
    if (obj->displayList != NULL) {
        gSPDisplayList(glistp++, obj->displayList);
    } else if (obj->mesh->numFaces != 0) {
        if (obj->flags & 1) {
            if (!(obj->flags & 8)) {
                gSPSetGeometryMode(glistp++, G_LIGHTING);
            } else {
                gSPClearGeometryMode(glistp++, G_LIGHTING);
            }
            WriteObjectDisplayListGouraud(obj);
        } else {
            if (!(obj->flags & 8)) {
                gSPSetGeometryMode(glistp++, G_LIGHTING);
            } else {
                gSPClearGeometryMode(glistp++, G_LIGHTING);
            }
            WriteObjectDisplayListFlat(obj);
        }
    }
    if (obj->numSprites > 0) {
        for (i = 0; i < obj->numSprites; i++) {
            sprite = obj->sprites[i].sprite;
            if (sprite != NULL) {
                obj->sprites[i].sprite->scaleX = obj->sprites[i].width * actorScale.x * XScaleAmount;
                obj->sprites[i].sprite->scaleY = obj->sprites[i].height * actorScale.y * YScaleAmount;
                a = obj->alpha[0] * currentDrawActor->alpha / 100;
                obj->sprites[i].sprite->alpha = MIN(a, 255);
                if (a >= 255) {
                    obj->sprites[i].sprite->flags &= ~1;
                } else {
                    obj->sprites[i].sprite->flags |= 1;
                }
                guMtxXFMF(D_80284D74, obj->sprites->x, obj->sprites->y, obj->sprites->z, &tempVect.x, &tempVect.y, &tempVect.z);
                sprite->pos = tempVect;
            }
        }
    }
    if (obj->sphere != NULL) {
        guMtxXFMF(D_80284D74, 0.0f, 0.0f, 0.0f, &tempVect.x, &tempVect.y, &tempVect.z);
        obj->sphere->prevPos = obj->sphere->pos;
        obj->sphere->pos = tempVect;
        if (obj->sphere->platTag[0]) {
            TagToFirstPlatform(obj->sphere->platTag[0])->actor.pos = tempVect;
        }
    }
    gSPPopMatrix(glistp++, G_MTX_MODELVIEW);
    matrixStackCount--;
    if (calcMtx == 1 || !staticObj) {
        PopMatrix();
        if (obj->child != NULL) {
            parentScaleStack[parentScaleStackLevel + 1].x = XScaleAmount;
            parentScaleStack[parentScaleStackLevel + 1].y = YScaleAmount;
            parentScaleStack[parentScaleStackLevel + 1].z = ZScaleAmount;
            parentScaleStackLevel++;
            TransformAndDrawObject(obj->child, time, doKeys, arg3);
            parentScaleStackLevel--;
        }
        PopMatrix();
        if (obj->sibling != NULL) {
            TransformAndDrawObject(obj->sibling, time, doKeys, arg3);
        }
    }
}

void SlideTextureCoords(Actor *actor, f32 x, f32 y) {
    Object *obj = actor->inst->obj;
    u16 (*texCoord)[2] = obj->mesh->uv;
    u16 (*f)[2] = texCoord;
    u16 (*p)[2] = texCoord;
    Vec3 *vtx[3];
    u16 xtexsize;
    u16 ytexsize;
    s32 xtex;
    s32 ytex;
    s32 xsize;
    s32 ysize;
    s32 tempx;
    s32 tempy;
    s32 wrapU;
    s32 wrapV;
    s32 i;
    s32 j;

    if (texCoord == NULL) {
        return;
    }
    for (i = 0; i < obj->mesh->numFaces; i++) {
        vtx[0] = &obj->mesh->verts[obj->mesh->faces[i].v[0]];
        vtx[1] = &obj->mesh->verts[obj->mesh->faces[i].v[1]];
        vtx[2] = &obj->mesh->verts[obj->mesh->faces[i].v[2]];
        xtexsize = abs(f[0][0] - f[1][0]);
        xtex = abs(f[0][0] - f[2][0]);
        if ((u16)xtex > xtexsize) {
            xtexsize = xtex;
        }
        ytexsize = abs(f[0][1] - f[1][1]);
        ytex = abs(f[0][1] - f[2][1]);
        if ((u16)ytex > ytexsize) {
            ytexsize = ytex;
        }
        f += 3;
        xsize = ABS(vtx[0]->x - vtx[1]->x);
        tempx = ABS(vtx[0]->x - vtx[2]->x);
        if (xsize < tempx) {
            xsize = tempx;
        }
        ysize = ABS(vtx[0]->z - vtx[1]->z);
        tempy = ABS(vtx[0]->z - vtx[2]->z);
        if (ysize < tempy) {
            ysize = tempy;
        }
        if (actor->scale.x != 0.0f) {
            xsize = xsize * actor->scale.x;
        }
        if (actor->scale.z != 0.0f) {
            ysize = ysize * actor->scale.z;
        }
        if (ysize != 0) {
            tempx = x * xtexsize / ysize;
        }
        if (xsize != 0) {
            tempy = y * ytexsize / xsize;
        }
        wrapU = FALSE;
        wrapV = FALSE;
        for (j = 0; j < 3; j++) {
            p[0][0] += tempx;
            if (p[0][0] > 0x7FFF) {
                wrapU = TRUE;
            }
            p[0][1] += tempy;
            if (p[0][1] > 0x7FFF) {
                wrapV = TRUE;
            }
            p++;
        }
        if (wrapU) {
            if (tempx > 0) {
                for (j = 1; j < 4; j++) {
                    p[-j][0] -= xtexsize;
                }
            } else {
                for (j = 1; j < 4; j++) {
                    p[-j][0] += xtexsize;
                }
            }
        }
        if (wrapV) {
            if (tempy > 0) {
                for (j = 1; j < 4; j++) {
                    p[-j][1] -= ytexsize;
                }
            } else {
                for (j = 1; j < 4; j++) {
                    p[-j][1] += ytexsize;
                }
            }
        }
    }
}

void DeformTextureCoords(Object *obj) {
    u16 (*texCoord)[2] = obj->mesh->uv;
    u16 (*oTexCoord)[2] = obj->mesh->origUV;
    Vec3 *v;
    s32 plus;
    s32 j;
    s32 i;

    if (texmode == 0) {
        for (i = 0; i < obj->mesh->numFaces; i++) {
            for (j = 0; j < 3; j++) {
                v = &obj->mesh->verts[obj->mesh->faces[i].v[j]];
                plus = SineWave(20.0f, (s16)(v->x + v->y + v->z), frameCount) * 8.0f;
                (*texCoord)[0] += plus;
                plus = SineWave(20.0f, (s16)(v->x + v->y + v->z) / 2, frameCount) * 8.0f;
                (*texCoord)[1] += plus;
                texCoord++;
            }
        }
    } else {
        for (i = 0; i < obj->mesh->numFaces; i++) {
            for (j = 0; j < 3; j++) {
                v = &obj->mesh->verts[obj->mesh->faces[i].v[j]];
                (*texCoord)[0] = (*oTexCoord)[0] + SineWave(20.0f, (v->x + v->z) / texval1, frameCount) * 300.0f;
                (*texCoord)[1] = (*oTexCoord)[1] + SineWave(25.0f, (v->x + v->z) / texval2 + 37.699111848, frameCount) * 300.0f;
                texCoord++;
                oTexCoord++;
            }
        }
    }
}

s32 IsActorVisible(Actor *actor) {
    s32 pad[16]; // forces the original stack frame size
    f32 d = actor->radius + farPlaneDist;

    if (d * d < actor->distSq) {
        return FALSE;
    }
    return TRUE;
}

void HandSwish(Object *obj, f32 time, s16 animStart, s16 animEnd, f32 speed) {
    Quat tempQuat;
    f32 m[4][4];
    f32 rot[4][4];
    f32 trans[4][4];
    Vec3 v;
    f32 tempFloat;
    f32 x;
    f32 y;
    f32 z;
    s16 toKey;
    s16 prev;
    s32 i;

    prev = -1;
    time += speed;
    toKey = -1;
    if (animEnd < time) {
        time = animStart;
    }
    if (obj->numPosKeys >= 2) {
        for (i = 0; i < obj->numPosKeys; i++) {
            if (time < obj->posKeys[i].time) {
                toKey = i;
                prev = i - 1;
                break;
            }
        }
        if (prev != -1) {
            tempFloat = abs(obj->posKeys[toKey].time - obj->posKeys[prev].time);
            tempFloat = (time - obj->posKeys[prev].time) / tempFloat;
        } else {
            tempFloat = 1.0f;
            toKey = prev = obj->numPosKeys - 1;
        }
        v.x = obj->posKeys[prev].pos.x + (obj->posKeys[toKey].pos.x - obj->posKeys[prev].pos.x) * tempFloat;
        v.y = obj->posKeys[prev].pos.y + (obj->posKeys[toKey].pos.y - obj->posKeys[prev].pos.y) * tempFloat;
        v.z = obj->posKeys[prev].pos.z + (obj->posKeys[toKey].pos.z - obj->posKeys[prev].pos.z) * tempFloat;
        x = v.x;
        y = v.y;
        z = v.z;
    } else {
        x = obj->posKeys[0].pos.x;
        y = obj->posKeys[0].pos.y;
        z = obj->posKeys[0].pos.z;
    }
    toKey = -1;
    if (obj->numRotKeys >= 2) {
        prev = -1;
        for (i = 0; i < obj->numRotKeys; i++) {
            if (time < obj->rotKeys[i].time) {
                toKey = i;
                prev = i - 1;
                break;
            }
        }
        if (prev != -1) {
            tempFloat = abs(obj->rotKeys[toKey].time - obj->rotKeys[prev].time);
            tempFloat = (time - obj->rotKeys[prev].time) / tempFloat;
        } else {
            tempFloat = 1.0f;
            toKey = prev = obj->numRotKeys - 1;
        }
        QuatSlerp(&obj->rotKeys[prev].q, &obj->rotKeys[toKey].q, tempFloat, &tempQuat);
        QuaternionToMatrix(&tempQuat, m);
    } else {
        QuaternionToMatrix(&obj->rotKeys[0].q, m);
    }
    guTranslateF(trans, x * 0.1, y * 0.1, z * 0.1);
    guRotateF(rot, 180.0f, 0.0f, 1.0f, 0.0f);
    guMtxCatF(m, rot, m);
    guMtxCatF(rot, m, m);
    MatrixToQuaternion(m, &hand.rotQuat);
}

INCLUDE_ASM("asm/nonmatchings/object", pad_80145674);

// linker junk that follows the 0.1 constant emitted by HandSwish
const u32 D_801060E4[] = { 0x5604A001, 0x6F0DD80F, 0xA00FB10E };
