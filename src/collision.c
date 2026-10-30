#include "common.h"
#include "ident.h"
#include "macros.h"
#include "maths.h"
#include "actor.h"
#include "platform.h"
#include "hand.h"
#include "ball.h"
#include "game.h"
#include "debug.h"
#include "camera.h"
#include "texture.h"
#include "bullet.h"
#include "sound.h"
#include "collision.h"
#include "enemy.h"

extern Vec3 globalPos[];
extern Vec3 globalRot[];

void QuaternionToMatrix(Quat *q, f32 mf[4][4]);
extern s32 frameCount;

extern f32 gravity;
void HurtHand(s32 type, s16 damage);
void SquashActor(Actor *actor, Vec3 *dir, f32 power, f32 maxSquash);
extern ActorList collisionList;
extern Actor camera;
extern u8 specialActor;
extern f32 numEdgeChecks;
extern u32 facenum;
extern u32 numBounces;
extern s32 sphere;
extern s32 numBounces2;
extern u8 inertia;
extern Plane *actorPlane[4];
extern Vec3 nearestEdgePoint;
extern CollFace *corner;
extern Vec3 origPos;
extern Vec3 totalVel;
extern CollFace *edgePlane;
extern f32 edge;
extern f32 origSpeed;
extern Vec3 conveyorVel;
extern Texture *blastTex;
extern s16 worldExitTimer;
extern CameraInfo cameraInfo;
void WaterDoSplashes(f32 yPos, Actor *aPtr, Vec3 *plane, s16 ripple);
void CreateWake(Vec3 *pos, Vec3 *vel, Platform *platform, f32 level, f32 size, Vec3 *normal);
void CreateWaterRipple(f32 xPos, f32 yPos, f32 zPos, f32 rippleSize, s32 numRings, Vec3 *plane2, Texture *texture, s16 size);
void AddConveyorSpeed(Vec3 *vector, Platform *platform, Vec3 *pos, s32 set);
s32 CheckActorForTeleport(Actor *actor, Platform *platform);

void CollideActorInsideSphere(Actor *actor, Vec3 *centre, f32 radius) {
    Vec3 d;
    f32 dist;
    f32 inv;
    f32 dot;

    SUB_VECTORS(&d, centre, &actor->pos);
    dist = LENGTH(&d);
    if (dist == 0.0f) {
        return;
    }
    inv = 1.0f / dist;
    SCALE_VECTOR(&d, inv);
    if (radius - actor->radius < dist) {
        if (actor->pos.y < centre->y) {
            actor->obj->onGround = 1;
        }
        AddToOneScaledVector(&actor->pos, &d, dist + actor->radius - radius);
        dot = DOT_PRODUCT(&actor->vel, &d);
        if (dot < 0.0f) {
            AddToOneScaledVector(&actor->vel, &d, -dot);
        }
        AddToOneScaledVector(&actor->vel, &d, dist + actor->radius - radius);
    }
}

void CheckActorCollisions(Actor *actor) {
    Actor *actPtr;
    Vec3 dist;
    Vec3 dist2;
    Vec3 relVel;
    Vec3 dv1;
    Vec3 dv2;
    Vec3 pos;
    Vec3 pos2;
    Vec3 rot;
    Vec3 rot2;
    u32 j;
    u32 k;
    s32 check;
    s32 ignoreHurt;
    s32 index;
    s32 flags;
    f32 val;
    f32 sum;
    f32 mass;
    f32 f;
    s32 bounce = TRUE; // never changes, but the ROM still tests it

    if (actor->phys->mass == 0.0f) {
        return;
    }
    for (actPtr = actorList.head.next; actPtr != &actorList.head; actPtr = actPtr->next) {
        check = FALSE;
        if ((actPtr->flags & 8) && actPtr->phys->mass != 0.0f && actPtr->type != CAMERA && actPtr != actor) {
            check = TRUE;
            if (actPtr->state != NULL && actPtr->state->teleport != NULL) {
                if (((actor == &hand && actPtr == &ball) || (actor == &ball && actPtr == &hand))
                    && DistanceBetweenPoints(&hand.pos, &ball.pos) <= hand.radius + ball.obj->spheres[0].radius) {
                    handControl.touchedActor = 1;
                }
                continue;
            }
            if (actPtr->link == actor || actPtr == actor->link) {
                continue;
            }
        }
        if (!check) {
            continue;
        }
        SUB_VECTORS(&dist, &actor->pos, &actPtr->pos);
        if ((actor->radius + actPtr->radius) * (actor->radius + actPtr->radius) < DOT_PRODUCT(&dist, &dist)) {
            continue;
        }
        for (j = 0; j < actor->obj->numSpheres; j++) {
            if ((actPtr == &ball && (actor->obj->spheres[j].flags & 2))
                || (actPtr == &hand && (actor->obj->spheres[j].flags & 1))) {
                continue;
            }
            pos = globalPos[j];
            rot = globalRot[j];
            for (k = 0; k < actPtr->obj->numSpheres; k++) {
                ignoreHurt = FALSE;
                if ((actor == &ball && (actPtr->obj->spheres[k].flags & 2))
                    || (actor == &hand && (actPtr->obj->spheres[k].flags & 1))) {
                    continue;
                }
                if (actor->obj->spheres[j].flags & 8) {
                    ignoreHurt = TRUE;
                }
                flags = (u16)actPtr->obj->spheres[k].flags;
                if (flags & 8) {
                    ignoreHurt = TRUE;
                }
                if (flags & 0x10) {
                    pos2 = actPtr->obj->spheres[k].pos;
                    SUB_VECTORS(&rot2, &pos2, &actPtr->pos);
                } else if (flags & 0x20) {
                    ZERO_VECTOR(&rot2);
                    pos2 = actPtr->pos;
                } else {
                    RotateVectorByQuaternion(&rot2, &actPtr->obj->spheres[k].pos, &actPtr->rotQuat);
                    ADD_VECTORS(&pos2, &actPtr->pos, &rot2);
                }
                sum = actor->obj->spheres[j].radius;
                sum += actPtr->obj->spheres[k].radius;
                SUB_VECTORS(&dist, &pos, &pos2);
                val = LENGTH(&dist);
                if (!(val <= sum)) {
                    continue;
                }
                if (sum / 2.0f <= actor->speed + actPtr->speed) {
                    AddOneScaledVector(&actor->pos, &actor->prevPos, &actor->vel, 0.5f);
                    ADD_VECTORS(&pos, &actor->pos, &rot);
                    SUB_VECTORS(&dist, &pos, &pos2);
                    val = LENGTH(&dist);
                }
                actPtr->state->hits -= actor->state->damage;
                actor->obj->contactFlags |= 8;
                actPtr->obj->contactFlags |= 8;
                if (actor->flags & 0x80) {
                    actor->lifeTimer = 1;
                }
                if (actPtr->flags & 0x80) {
                    actPtr->lifeTimer = 1;
                }
                if (hand.state->mode == HAND_SLAM) {
                    index = -1;
                    if (actor == &hand) {
                        if (actPtr->state->index != -1) {
                            index = actPtr->state->index;
                        } else if (actPtr == &ball) {
                            ballControl.hitFlags |= 8;
                            if (ballControl.enemy != NULL && ballControl.enemy->state != NULL
                                && ballControl.enemy->state->index != -1) {
                                enemies[ballControl.enemy->state->index].brain.flags |= 8;
                            }
                        }
                    } else if (actPtr == &hand) {
                        if (actor->state->index != -1) {
                            index = actor->state->index;
                        } else if (actor == &ball) {
                            ballControl.hitFlags |= 8;
                            if (ballControl.enemy != NULL && ballControl.enemy->state != NULL
                                && ballControl.enemy->state->index != -1) {
                                enemies[ballControl.enemy->state->index].brain.flags |= 8;
                            }
                        }
                    }
                    if (index != -1 && enemies[index].actor.active != 2) {
                        enemies[index].brain.flags |= 8;
                        if (!(enemies[index].brain.flags & 0x20)) {
                            NMEHitByBlastRing(&enemies[index]);
                        }
                        if ((enemies[index].brain.flags & 1) || enemies[index].brain.spellTimer[2]) {
                            KillEnemy(&enemies[index]);
                            return;
                        }
                    }
                }
                if (!ignoreHurt) {
                    if (actor == &hand && (actPtr->flags & 0x80000)
                        && (actPtr->state == NULL || actPtr->state->index == -1
                            || (!(enemies[actPtr->state->index].brain.flags & 0x400)
                                && !enemies[actPtr->state->index].brain.spellTimer[0]))) {
                        if (actPtr->flags < 0 && hand.obj->onGround && actPtr->vel.y < -gravity) {
                            HurtHand(HURT_SQUASH, -1);
                        } else {
                            HurtHand(HURT_GO_BACKWARDS, -1);
                        }
                        return;
                    }
                    if (actPtr == &hand && (actor->flags & 0x80000)
                        && (actor->state == NULL || actor->state->index == -1
                            || (!(enemies[actor->state->index].brain.flags & 0x400)
                                && !enemies[actor->state->index].brain.spellTimer[0]))) {
                        if (actor->flags < 0 && hand.obj->onGround && actor->vel.y < -gravity) {
                            HurtHand(HURT_SQUASH, -1);
                        } else {
                            HurtHand(HURT_GO_BACKWARDS, -1);
                        }
                        return;
                    }
                    if (hand.state->mode != HAND_SLAM && (hand.state->prevMode != HAND_SLAM || handControl.actionTime >= 6)) {
                        if (actor == &hand && (actPtr->flags & 0x20000)
                            && (actPtr->state == NULL || actPtr->state->index == -1
                                || (!(enemies[actPtr->state->index].brain.flags & 0x400)
                                    && !enemies[actPtr->state->index].brain.spellTimer[0]))) {
                            HurtHand(HURT_GO_BACKWARDS, 1);
                            return;
                        }
                        if (actPtr == &hand && (actor->flags & 0x20000)
                            && (actor->state == NULL || actor->state->index == -1
                                || (!(enemies[actor->state->index].brain.flags & 0x400)
                                    && !enemies[actor->state->index].brain.spellTimer[0]))) {
                            HurtHand(HURT_GO_BACKWARDS, 1);
                            return;
                        }
                    }
                }
                if (actor->type == BALL) {
                    actPtr->flags |= 0x04000000;
                    if (!ignoreHurt) {
                        if (gameInfo.level == SPACEBOSS3 && actPtr->state->hits == 1) {
                            actor->lifeTimer = 1;
                            DestroyBulletMissile(actor);
                        }
                        if (actPtr->state == NULL || actPtr->state->index == -1
                            || !enemies[actPtr->state->index].brain.spellTimer[0]) {
                            if (actPtr->flags & 0x100000) {
                                BurstBall();
                                return;
                            }
                            if (actPtr->flags & 0x40000) {
                                HurtBall();
                            }
                            if (actPtr->flags & 0x080A0000) {
                                if (IsHandHoldingBall()) {
                                    if (actPtr->flags & 0x80000) {
                                        HurtHand(HURT_GO_BACKWARDS, -1);
                                    } else if (actPtr->flags & 0x20000) {
                                        HurtHand(HURT_GO_BACKWARDS, 1);
                                    } else {
                                        ApplyAction(HAND_RELEASE);
                                    }
                                }
                                if (actPtr->type == CANNONBALL && actPtr->speed > 6.0f) {
                                    enemies[actPtr->state->index].brain.spellTimer[0] = 50;
                                }
                            }
                        }
                    }
                } else if (actPtr->type == BALL) {
                    actor->flags |= 0x04000000;
                    if (!ignoreHurt) {
                        if (gameInfo.level == SPACEBOSS3 && actor->state->hits == 1) {
                            actPtr->lifeTimer = 1;
                            DestroyBulletMissile(actPtr);
                        }
                        if (actor->state == NULL || actor->state->index == -1
                            || !enemies[actor->state->index].brain.spellTimer[0]) {
                            if (actor->flags & 0x100000) {
                                BurstBall();
                                return;
                            }
                            if (actor->flags & 0x40000) {
                                HurtBall();
                            }
                            if (actor->flags & 0x080A0000) {
                                if (IsHandHoldingBall()) {
                                    if (actor->flags & 0x80000) {
                                        HurtHand(HURT_GO_BACKWARDS, -1);
                                    } else if (actor->flags & 0x20000) {
                                        HurtHand(HURT_GO_BACKWARDS, 1);
                                    } else {
                                        ApplyAction(HAND_RELEASE);
                                    }
                                }
                                if (actor->type == CANNONBALL && actor->speed > 6.0f) {
                                    enemies[actor->state->index].brain.spellTimer[0] = 50;
                                }
                            }
                        }
                    }
                }
                if (actor == &hand) {
                    actPtr->flags |= 0x40000000;
                } else if (actPtr == &hand) {
                    actor->flags |= 0x40000000;
                }
                if (bounce) {
                    if (actPtr->type == DENIS && (actor == &ball || actor == &hand) && actPtr->vel.y < -gravity * 1.5) {
                        ZERO_VECTOR(&actor->vel);
                        SUB_VECTORS(&actor->vel, &actor->pos, &actPtr->pos);
                        SCALE_VECTOR(&actor->vel, 5.0f);
                        actor->vel.y = 2.0f * -actPtr->vel.y;
                    }
                    if ((actor == &ball && actPtr == &hand) || (actor == &hand && actPtr == &ball)) {
                        handControl.touchedActor = 1;
                    }
                    mass = actor->phys->mass + actPtr->phys->mass;
                    if (val < 0.1) {
                        val = 0.1f;
                    }
                    SCALE_VECTOR(&dist, 1.0f / val);
                    SUB_VECTORS(&relVel, &actor->vel, &actPtr->vel);
                    dv2 = dist;
                    if (actor->phys->maxSquash != 0.0f) {
                        SquashActor(actor, &dist,
                                    LENGTH(&relVel) * actPtr->phys->mass / 250.0f,
                                    actor->phys->maxSquash);
                    }
                    if (actPtr->phys->maxSquash != 0.0f) {
                        dist2.x = -dist.x;
                        dist2.y = -dist.y;
                        dist2.z = -dist.z;
                        SquashActor(actPtr, &dist2,
                                    LENGTH(&relVel) * actor->phys->mass / 250.0f,
                                    actPtr->phys->maxSquash);
                    }
                    f = 2.0f * actor->phys->mass * DOT_PRODUCT(&dv2, &relVel) / mass;
                    SCALE_VECTOR(&dv2, f);
                    AddOneScaledVector(&dv1, &relVel, &dv2, -actPtr->phys->mass / actor->phys->mass);
                    if (actPtr->type == BULLET) {
                        SCALE_VECTOR(&dv1, 2.0f);
                        if (actPtr->flags & 0x80) {
                            actPtr->link = actor;
                        }
                    }
                    if (actor->type == BULLET) {
                        SCALE_VECTOR(&dv2, 2.0f);
                        if (actor->flags & 0x80) {
                            actor->link = actPtr;
                        }
                    }
                    ADD_VECTORS(&actor->vel, &dv1, &actPtr->vel);
                    ADD_VECTORS(&actPtr->vel, &dv2, &actPtr->vel);
                    if (actor->obj->onGround && actor->vel.y < 0.0f) {
                        actor->vel.y *= 0.25;
                    }
                    if (actPtr->obj->onGround && actPtr->vel.y < 0.0f) {
                        actPtr->vel.y *= 0.25;
                    }
                    if (!(actor->flags & 0x80) && !(actPtr->flags & 0x80)) {
                        if (actor->phys->mass != 999999.0f) {
                            f = (sum - val) * actPtr->phys->mass / mass;
                            if (f > 0.0f) {
                                f += 0.5;
                            } else {
                                f -= 0.5;
                            }
                            AddToOneScaledVector(&actor->pos, &dist, f);
                        }
                        if (actPtr->phys->mass != 999999.0f) {
                            f = (val - sum) * actor->phys->mass / mass;
                            if (f > 0.0f) {
                                f += 0.5;
                            } else {
                                f -= 0.5;
                            }
                            AddToOneScaledVector(&actPtr->pos, &dist, f);
                        }
                    }
                }
                if (actPtr->state->hits < 0) {
                    actPtr->flags |= 2;
                }
            }
        }
    }
}

f32 InitPlanes(Mesh *mesh, Vec3 *pos, CollFace *planes, Vec3 *scale, Quat *q, s32 collision) {
    Vec3 a;
    Vec3 b;
    f32 matrix[4][4];
    f32 x;
    f32 y;
    f32 z;
    f32 len;
    f32 max;
    s32 i;
    s32 j;
    CollFace *face;

    QuaternionToMatrix(q, matrix);
    max = 0.0f;
    for (i = 0; i < mesh->numFaces; i++) {
        face = &planes[i];
        for (j = 0; j < 3; j++) {
            x = scale->x * mesh->verts[mesh->faces[i].v[j]].x;
            y = scale->y * mesh->verts[mesh->faces[i].v[j]].y;
            z = scale->z * mesh->verts[mesh->faces[i].v[j]].z;
            face->edgeStart[j].x = matrix[0][0] * x + matrix[1][0] * y + matrix[2][0] * z;
            face->edgeStart[j].y = matrix[0][1] * x + matrix[1][1] * y + matrix[2][1] * z;
            face->edgeStart[j].z = matrix[0][2] * x + matrix[1][2] * y + matrix[2][2] * z;
            len = LENGTH(&face->edgeStart[j]);
            if (len > max) {
                max = len;
            }
        }
        if (collision) {
            SUB_VECTORS(&a, &face->edgeStart[1], &face->edgeStart[0]);
            SUB_VECTORS(&b, &face->edgeStart[2], &face->edgeStart[0]);
            CrossProduct((Vec3 *)&face->plane, &a, &b);
            MakeUnit((Vec3 *)&face->plane);
            for (j = 0; j < 3; j++) {
                ADD_TO_VECTOR(&face->edgeStart[j], pos);
            }
            CalculatePlaneInfoWithEdgeLens(face);
        }
    }
    if (collision) {
        for (i = 0; i < mesh->numFaces; i++) {
            CheckForInnerEdges(mesh->numFaces, planes, i);
        }
    }
    return max;
}

void CheckForInnerEdges(u32 numFaces, CollFace *planes, s32 index) {
    u8 conv[3] = { 0, 2, 1 };
    CollFace *plane;
    f32 t;
    s32 skip;
    u32 i;
    u32 j;
    u32 k;
    u32 l;
    u32 n;

    plane = &planes[index];
    if (DistanceBetweenPoints(&plane->edgeStart[0], &plane->edgeStart[1]) < 0.001
        || DistanceBetweenPoints(&plane->edgeStart[0], &plane->edgeStart[2]) < 0.001
        || DistanceBetweenPoints(&plane->edgeStart[2], &plane->edgeStart[1]) < 0.001) {
        plane->edgeLen[0] = plane->edgeLen[1] = plane->edgeLen[2] = -1.0f;
        return;
    }
    for (i = index + 1; i < numFaces; i++) {
        skip = 0;
        for (j = 0; j < 3; j++) {
            t = DOT_PRODUCT(&planes[i].edgeStart[j], &plane->plane) + plane->plane.d;
            if (ABS(t) > 0.01) {
                skip = 1;
            }
        }
        if (skip != 1) {
            for (j = 0; j < 3; j++) {
                for (k = 0; k < 3; k++) {
                    if (DistanceBetweenPoints(&plane->edgeStart[j], &planes[i].edgeStart[k]) < 0.01) {
                        n = (j + 1) % 3;
                        for (l = 0; l < 3; l++) {
                            if (DistanceBetweenPoints(&plane->edgeStart[n], &planes[i].edgeStart[l]) < 0.01) {
                                plane->edgeLen[j] = -1.0f;
                                planes[i].edgeLen[conv[k + l - 1]] = -1.0f;
                            }
                        }
                    }
                }
            }
        }
    }
}

void CheckCollisions(Actor *actor) {
    Actor *aptr;
    Platform *platform;
    Vec3 tempPoint;
    Vec3 e;
    Vec3 d;
    Vec3 tempPos;
    Vec3 centroid;
    u32 j;
    u32 k;
    s32 coll;
    s32 skip;
    f32 initVelY;
    f32 vol;
    f32 speed;
    f32 rad;

    initVelY = actor->vel.y;
    facenum = numBounces = numBounces2 = 0;
    corner = NULL;
    totalVel.x = totalVel.y = totalVel.z = conveyorVel.x = conveyorVel.y = conveyorVel.z = edge = 0.0f;
    origPos = actor->pos;
    origSpeed = LENGTH_2D(&actor->vel);
    for (k = 0; k < 4; k++) {
        actorPlane[k] = actor->obj->planes[k];
        actor->obj->planes[k] = NULL;
    }
    if (actor == &camera || actor == &handControl.frontProbe) {
        specialActor = TRUE;
    } else {
        specialActor = FALSE;
    }
    for (aptr = collisionList.head.collNext; aptr != &collisionList.head; aptr = aptr->collNext) {
        if (aptr == actor) {
            continue;
        }
        rad = actor->radius + aptr->radius;
        rad *= rad;
        if (actor == &camera && (aptr->flags & 0x20000000)) {
            continue;
        }
        if (rad < DistanceBetweenPointsSquared(&actor->pos, &aptr->pos)) {
            continue;
        }
        for (j = 0; j < aptr->inst->obj->mesh->numFaces; j++) {
            coll = CollideActorWithPlane(actor, &aptr->faces[j], NULL);
            if (coll & 1) {
                return;
            }
            if (coll & 2) {
                goto bounce;
            }
            if (coll & 4) {
                aptr->alpha = 210;
            }
        }
    }
    for (platform = platformList.head.next; platform != &platformList.head; platform = platform->next) {
        if (platform->flags & 0x84) {
            continue;
        }
        if (actor == &camera && (platform->actor.flags & 0x20000000)) {
            continue;
        }
        SUB_VECTORS(&tempPos, &actor->pos, &platform->actor.pos);
        if ((actor->radius + platform->actor.radius) * (actor->radius + platform->actor.radius) < DOT_PRODUCT(&tempPos, &tempPos)) {
            continue;
        }
        if (platform->lastUpdate < frameCount) {
            RecalculatePlatformInfo(platform);
        }
        for (j = 0; j < platform->actor.inst->obj->mesh->numFaces; j++) {
            coll = CollideActorWithPlane(actor, &platform->actor.faces[j], platform);
            if (coll & 1) {
                return;
            }
            if (coll & 2) {
                goto bounce;
            }
            if (coll & 4) {
                platform->actor.alpha = 210;
            }
        }
    }
    if (specialActor) {
        return;
    }
bounce:
    if ((actor->type == BALL || gameInfo.level == CARNIVALBONUS)
        && (numBounces2 || (actor->obj->planes[0] != NULL && actorPlane[0] == NULL))) {
        vol = 0.0f;
        for (k = 0; k < facenum; k++) {
            if (vol < MIN(ABS((actor->vel.x * actor->obj->planes[k]->x + actor->vel.y * actor->obj->planes[k]->y
                               + actor->vel.z * actor->obj->planes[k]->z))
                              * 15.0f,
                          100.0f)) {
                vol = MIN(ABS((actor->vel.x * actor->obj->planes[k]->x + actor->vel.y * actor->obj->planes[k]->y
                               + actor->vel.z * actor->obj->planes[k]->z))
                              * 15.0f,
                          100.0f);
            }
        }
        if (actor == &ball) {
            if (ballControl.snow != 0.0f) {
                PlaySample(136, &actor->pos, vol, (Random(56) + 100));
            } else {
                switch (ballControl.type) {
                    case BALL_MODE_NORMAL:
                        PlaySample(131, &ball.pos, vol, (Random(56) + 100));
                        break;
                    case BALL_MODE_BOWLING:
                        PlaySample(132, &ball.pos, vol, (Random(40) + 20));
                        break;
                    case BALL_MODE_POWER:
                        PlaySample(133, &ball.pos, vol, (Random(56) + 100));
                        break;
                    case BALL_MODE_BEARING:
                        PlaySample(134, &ball.pos, vol, (Random(36) + 110));
                        break;
                    case BALL_MODE_BEACH:
                        PlaySample(135, &ball.pos, vol, (Random(56) + 100));
                        break;
                    case BALL_MODE_CRYSTAL:
                        PlaySample(137, &ball.pos, vol, (Random(36) + 110));
                        break;
                }
            }
        } else {
            PlaySample(131, &actor->pos, vol, 128);
        }
    }
    if (numBounces) {
        if (actor->vel.y < -3.0) {
            actor->friction = actor->phys->friction;
        }
        SCALE_VECTOR(&totalVel, actor->speed * actor->phys->bounce / numBounces);
        actor->vel = totalVel;
    } else {
        if (actor->flags & 1) {
            SUB_VECTORS(&actor->vel, &actor->pos, &actor->prevPos);
        } else {
            SubVector2D(&actor->vel, &actor->pos, &actor->prevPos);
        }
        if (inertia && (actor->flags & 1) && ABS(actor->vel.y) < 0.1) {
            actor->vel.y -= gravity / 7.0f;
        }
        speed = LENGTH_2D(&actor->vel);
        if (speed != 0.0f && speed > origSpeed) {
            SCALE_VECTOR_2D(&actor->vel, origSpeed / speed);
        }
        if (actor->state != NULL && actor->state->flying) {
            MakeUnit(&actor->vel);
            SCALE_VECTOR(&actor->vel, actor->speed);
        }
    }
    SUB_VECTORS(&e, &actor->pos, &actor->prevPos);
    if (LENGTH_2D(&e) < actor->phys->minSpeed) {
        actor->vel.x = actor->vel.z = 0.0f;
    }
    AddConveyorSpeedToVel(&actor->vel, &conveyorVel);
    if (edge != 0.0f) {
        skip = FALSE;
        if (corner != NULL) {
            skip = TRUE;
            if ((facenum == 0 || !actor->obj->onGround) && actor->vel.y < -3.75
                && (facenum == 0 || (facenum == 1 && actor->obj->planes[0] == (Plane *)corner))) {
                for (j = 0; j <= numEdgeChecks; j++) {
                    AddScaledVector(&tempPos, &actor->pos, j / numEdgeChecks, &actor->prevPos, 1.0 - j / numEdgeChecks);
                    ADD_TO_VECTOR(&tempPos, &globalRot[sphere]);
                    if (NearEdges(&tempPos, corner, actor->obj->spheres[sphere].radius, &tempPoint) != 0.0f) {
                        skip = FALSE;
                        nearestEdgePoint = tempPoint;
                        actor->pos = tempPos;
                        break;
                    }
                }
            }
        }
        for (k = 0; k < 4; k++) {
            if (actor->obj->planes[k] != NULL && (Plane *)edgePlane != actor->obj->planes[k]
                && DOT_PRODUCT(&edgePlane->plane, actor->obj->planes[k]) > 0.95
                && ABS(edgePlane->plane.d - actor->obj->planes[k]->d) <= 0.5) {
                skip = TRUE;
            }
        }
        if (!skip) {
            tempPoint = actor->pos;
            SUB_VECTORS(&d, &actor->pos, &nearestEdgePoint);
            if (actor == &ball && facenum == 0 && d.y < 0.0f && ball.vel.y < 0.0f) {
                centroid = edgePlane->edgeStart[0];
                ADD_TO_VECTOR(&centroid, &edgePlane->edgeStart[1]);
                ADD_TO_VECTOR(&centroid, &edgePlane->edgeStart[2]);
                SCALE_VECTOR(&centroid, 1.0 / 3.0);
                SUB_FROM_VECTOR(&centroid, &actor->pos);
                if (DOT_PRODUCT(&d, &centroid) > 0.0f && DOT_PRODUCT(&ball.vel, &centroid) > 0.0f) {
                    PRINTF("COLLISION PROB FOUND!!!!!!!!!!!!!!!!!!!!\n");
                    FlipVector(&d);
                }
            }
            if ((facenum == 0 || !actor->obj->onGround) && actor->vel.y < -3.75) {
                actor->prevPos = actor->pos;
            }
            ADD_TO_VECTOR(&d, &globalRot[sphere]);
            MakeUnit(&d);
            AddOneScaledVector(&actor->pos, &nearestEdgePoint, &d, actor->obj->spheres[sphere].radius);
            SUB_FROM_VECTOR(&actor->pos, &globalRot[sphere]);
            if (corner != NULL) {
                SUB_VECTORS(&actor->vel, &tempPos, &nearestEdgePoint);
                MakeUnit(&actor->vel);
                SCALE_VECTOR(&actor->vel, actor->speed * actor->phys->bounce);
            } else {
                SUB_VECTORS(&actor->vel, &actor->pos, &actor->prevPos);
                if ((facenum == 0 || !actor->obj->onGround) && initVelY < -3.75) {
                    MakeUnit(&actor->vel);
                    SCALE_VECTOR(&actor->vel, actor->speed * actor->phys->bounce);
                }
                speed = LENGTH_2D(&actor->vel);
                if (speed != 0.0f && origSpeed < speed) {
                    SCALE_VECTOR_2D(&actor->vel, origSpeed / speed);
                }
            }
        }
    }
    if (facenum == 0) {
        if (actorPlane[0] != NULL) {
            actor->obj->contactCount[0] = 0;
        } else {
            actor->obj->contactCount[0]++;
        }
    }
    for (j = 0; j < facenum; j++) {
        for (k = 0; k < 4; k++) {
            if (actor->obj->planes[j] == actorPlane[k] && actor->obj->planes[j] != NULL) {
                actor->obj->contactCount[j]++;
                break;
            }
        }
        if (k == 4) {
            actor->obj->contactCount[j] = 0;
        }
    }
    skip = TRUE;
    if (facenum >= 2) {
        for (j = 0; j < facenum; j++) {
            speed = DOT_PRODUCT(&globalPos[actor->obj->contactSphere[j]], actor->obj->planes[j]) + actor->obj->planes[j]->d;
            if (speed < actor->obj->spheres[actor->obj->contactSphere[j]].radius - 0.1) {
                skip = FALSE;
                speed = actor->obj->spheres[actor->obj->contactSphere[j]].radius - speed - 0.1 + 1.0;
                break;
            }
        }
        if (!skip) {
            ZERO_VECTOR(&tempPos);
            for (j = 0; j < facenum; j++) {
                ADD_TO_VECTOR(&tempPos, actor->obj->planes[j]);
            }
            MakeUnit(&tempPos);
            AddToOneScaledVector(&actor->pos, &tempPos, speed);
            AddToOneScaledVector(&actor->vel, &tempPos, speed / 2.5);
        }
    }
}

inline void CalcBounce(Vec3 *vel, Vec3 *normal) {
    f32 NdotL;

    MakeUnit(vel);
    NdotL = DOT_PRODUCT(vel, normal);
    AddToOneScaledVector(vel, normal, -NdotL * 2.0f);
    MakeUnit(vel);
}

f32 NearEdges(Vec3 *point, CollFace *plane, f32 r, Vec3 *nearestPoint) {
    Vec3 d;
    f32 t;
    s16 j;

    for (j = 0; j < 3; j++) {
        if (plane->edgeLen[j] == -1.0f) {
            continue;
        }
        t = DOT_PRODUCT(&plane->edgeDir[j], point) - DOT_PRODUCT(&plane->edgeStart[j], &plane->edgeDir[j]);
        if (t < 0.0f || t > plane->edgeLen[j]) {
            continue;
        }
        AddOneScaledVector(nearestPoint, &plane->edgeStart[j], &plane->edgeDir[j], t);
        SUB_VECTORS(&d, point, nearestPoint);
        if (DOT_PRODUCT(&d, &d) < r * r) {
            return LENGTH(&d);
        }
    }
    return 0.0f;
}

s32 CollideActorWithPlane(Actor *actor, CollFace *plane, Platform *platform) {
    Vec3 pos;
    Vec3 q;
    Vec3 nearestPoint;
    Vec3 v;
    Vec3 bvel[10];
    Vec3 dir;
    Vec3 rotvel;
    Vec3 rel;
    Vec3 diff;
    Vec3 newvel;
    Vec3 move;
    s32 others[3];
    Quat quat;
    Quat quat2;
    Quat quat3;
    u32 k;
    s32 bounce;
    s32 retval;
    s32 loop;
    u32 j;
    s32 onTop;
    s32 n;
    u32 m;
    Vec3 *normal;
    f32 *vel;
    f32 dist;
    f32 radius;
    f32 u0;
    f32 v0;
    f32 alpha;
    f32 beta;
    f32 val;
    f32 thisEdge;
    f32 dot;
    f32 len;
    f32 r;
    f32 ang;

    bounce = FALSE;
    retval = 0;
    if ((plane->proj[2] == 0.0f && plane->proj[3] == 0.0f) || (plane->proj[0] == 0.0f && plane->proj[1] == 0.0f)) {
        return 0;
    }
    normal = (Vec3 *)&plane->plane;
    for (loop = actor->obj->numSpheres - 1; loop >= 0; loop--) {
        if (actor->obj->spheres[loop].flags & 4) {
            continue;
        }
        if (retval & 2) {
            return retval;
        }
        if (bounce) {
            actor->vel = newvel;
        }
        pos = globalPos[loop];
        dist = -(pos.x * normal->x + pos.y * normal->y + pos.z * normal->z + plane->plane.d);
        radius = actor->obj->spheres[loop].radius;
        if (dist > 0.0f) {
            if (-(DOT_PRODUCT(&actor->prevPos, normal)
                  + plane->plane.d)
                <= 0.0f) {
                for (k = 0; dist > 0.0f;) {
                    AddScaledVector(&pos, &pos, 0.9f, &actor->prevPos, 0.1f);
                    k++;
                    dist = -(pos.x * normal->x + pos.y * normal->y + pos.z * normal->z + plane->plane.d);
                    if (k == 20) {
                        pos = actor->prevPos;
                        dist = -(pos.x * normal->x + pos.y * normal->y + pos.z * normal->z + plane->plane.d);
                        break;
                    }
                }
            } else {
                continue;
            }
        }
        if (radius < -dist) {
            continue;
        }
        AddOneScaledVector(&q, &pos, normal, dist);
        u0 = (&q.x)[plane->axis[0]] - (&plane->edgeStart[0].x)[plane->axis[0]];
        v0 = (&q.x)[plane->axis[1]] - (&plane->edgeStart[0].x)[plane->axis[1]];
        alpha = -1.0f;
        if (plane->proj[0] == 0.0f) {
            beta = u0 / plane->proj[1];
            if (beta >= -0.001 && beta <= 1.001) {
                alpha = (v0 - beta * plane->proj[3]) / plane->proj[2];
            }
        } else {
            beta = (v0 * plane->proj[0] - u0 * plane->proj[2]) / (plane->proj[3] * plane->proj[0] - plane->proj[1] * plane->proj[2]);
            if (beta >= -0.001 && beta <= 1.001) {
                alpha = (u0 - beta * plane->proj[1]) / plane->proj[0];
            }
        }
        if (alpha < -0.001 || alpha + beta > 1.001) {
            thisEdge = NearEdges(&pos, plane, radius, &nearestPoint);
            if (thisEdge == 0.0f) {
                continue;
            }
            if (normal->y > 0.2 && actor->vel.y < 0.0f) {
                actor->obj->contactFlags |= 2;
            } else {
                actor->obj->contactFlags |= 4;
            }
            nearestEdgePoint = nearestPoint;
            edge = thisEdge;
            edgePlane = plane;
            corner = NULL;
            sphere = loop;
            if (platform != NULL && platform->path != NULL && platform->path->moving == -3 && actor->type == BALL) {
                if (gameInfo.level == CARNIVALBONUS) {
                    PlaySample(110, &actor->pos, 100, 128);
                } else if (gameInfo.level == SPACEBOSS3) {
                    DestroyBulletMissile(actor);
                    actor->lifeTimer = 1;
                }
                if (actor != &ball) {
                    actor->type = BULLET;
                }
                platform->path->moving = 1;
                platform->pathStopFlags = 2;
            }
            if (actor->flags & 0x80) {
                actor->lifeTimer = 1;
                retval |= 1;
            }
            bounce = FALSE;
        } else {
            thisEdge = NearEdges(&origPos, plane, radius, &nearestPoint);
            if (thisEdge != 0.0f) {
                nearestEdgePoint = nearestPoint;
                edge = thisEdge;
                edgePlane = plane;
                sphere = loop;
                if (corner == NULL || corner->plane.y > plane->plane.y) {
                    corner = plane;
                }
            }
            actor->obj->planes[facenum] = (Plane *)plane;
            actor->obj->contactFlags |= 1;
            actor->obj->contactPos[facenum] = q;
            actor->obj->contactSphere[facenum] = loop;
            if (normal->y > 0.0f) {
                actor->obj->resting[facenum] = 1;
            }
            facenum++;
            if (facenum == 4) {
                retval |= 2;
            }
            if (specialActor) {
                if (actor == &camera) {
                    cameraInfo.planeDist[facenum - 1] = -dist;
                }
                return retval;
            }
            AddOneScaledVector(&actor->pos, &q, normal, radius);
            SUB_FROM_VECTOR(&actor->pos, &globalRot[loop]);
            if (actor->flags & 0x80) {
                actor->lifeTimer = 1;
                bounce = FALSE;
                retval |= 1;
            } else {
                bvel[numBounces] = actor->vel;
                if (platform != NULL) {
                    SUB_FROM_VECTOR(bvel + numBounces, &platform->actor.vel);
                }
                CalcBounce(&bvel[numBounces], normal);
                if (edge == 0.0f && ABS(actor->vel.y) < 2.0 && actor->vel.y < 0.0f) {
                    bvel[numBounces].y = 0.0f;
                }
                bounce = TRUE;
                if ((actor->flags & 1) && ABS(actor->vel.y) < gravity && actor->speed < 2.0f * gravity) {
                    bounce = FALSE;
                } else {
                    if (actorPlane[0] != NULL) {
                        for (k = 0; k < 4; k++) {
                            if (actorPlane[k] == (Plane *)plane) {
                                if (actor->obj->contactCount[k] >= 1) {
                                    bounce = FALSE;
                                }
                            }
                        }
                    }
                    dir = actor->vel;
                    MakeUnit(&dir);
                    len = DOT_PRODUCT(&dir, normal);
                    if (len > -0.3 || (len > -0.8 && (actor->phys->bounce <= 0.0f || actor->obj->onGround))) {
                        bounce = FALSE;
                    }
                }
                if (bounce == TRUE) {
                    if (plane->terrain == 5) {
                        WaterDoSplashes(pos.y + dist + 2.0f, actor, (Vec3 *)plane, 1);
                    }
                    if (plane->terrain == 12) {
                        CreateWaterRipple(q.x + plane->plane.x, q.y + plane->plane.y, q.z + plane->plane.z,
                                          MIN(MAX(ABS(DOT_PRODUCT(&actor->vel, &plane->plane)) / 4.0f, 1.0f), 5.0f), 2,
                                          (Vec3 *)plane, blastTex, 0x800);
                        retval |= 4;
                    }
                    if (actor->phys->maxSquash != 0.0f && !(actor->flags & 0x02000000)) {
                        SquashActor(actor, normal, -DOT_PRODUCT(&actor->vel, normal) / 25.0f, actor->phys->maxSquash);
                    }
                    if (platform != NULL) {
                        if (platform->flags & 0x200) {
                            AddConveyorSpeed(&conveyorVel, platform, &actor->pos, 0);
                        }
                        for (j = 0; j < 3; j++) {
                            if (platform->bounceAxis[j]) {
                                PlaySample(18, &actor->pos, 100, (Random(50) + 100));
                                vel = &bvel[numBounces].x;
                                vel[j] *= platform->bounce;
                            }
                        }
                    }
                    if (actor->phys->bounce >= 0.0f) {
                        newvel = bvel[numBounces];
                        SCALE_VECTOR(&newvel, actor->speed * terrain[plane->terrain].bounce);
                        actor->obj->resting[facenum - 1] = 0;
                        AddToOneScaledVector(&totalVel, &bvel[numBounces], terrain[plane->terrain].bounce);
                        numBounces++;
                        numBounces2++;
                    } else {
                        bounce = FALSE;
                        numBounces2++;
                    }
                }
                if (!bounce || actor->phys->bounce <= 0.0f) {
                    if (plane->terrain == 5) {
                        CreateWake(&pos, &actor->vel, platform, pos.y + dist + 2.0f, origSpeed, (Vec3 *)plane);
                    }
                    if (plane->terrain == 12) {
                        dot = DOT_PRODUCT(&actor->vel, &plane->plane);
                        if (dot < -0.5) {
                            CreateWaterRipple(q.x + plane->plane.x, q.y + plane->plane.y, q.z + plane->plane.z,
                                              MIN(MAX(ABS(dot) / 4.0f, 1.0f), 5.0f), 1, (Vec3 *)plane, blastTex, 0x800);
                            retval |= 4;
                        }
                    }
                    if (plane->plane.y > 0.3 || (actor->flags & 0x8000)) {
                        actor->obj->platform = platform;
                    }
                }
            }
        }
        if (actor == &ball && ballControl.type == BALL_MODE_CRYSTAL && hand.anim->current != HANDANIM_SPIN2CUP && worldExitTimer == 0) {
            SUB_VECTORS(&move, &ball.pos, &ball.prevPos);
            len = ABS(DOT_PRODUCT(&move, &plane->plane));
            len = MAX(len, ABS(DOT_PRODUCT(&ball.vel, &plane->plane)));
            if (len > terrain[plane->terrain].shatterSpeed
                || (terrain[plane->terrain].shatterSpeed < ball.speed && len > terrain[plane->terrain].shatterSpeed / 1.5)) {
                ShatterBall();
                retval |= 1;
                break;
            }
        }
        CalcCollisionSpheres(actor);
        if (plane->terrain == 6 && (actor->flags & 0x400000)) {
            actor->lifeTimer = 1;
            if (actor == &ball) {
                BurstBall();
                retval |= 1;
                break;
            }
            if (actor == &hand) {
                HurtHand(HURT_GO_BACKWARDS, -1);
                break;
            }
            if (actor->state->index != -1) {
                KillEnemy(&enemies[actor->state->index]);
                break;
            }
        } else if (plane->terrain == 9
                   && (actor == &hand || (actor == &ball && ball.link == &hand && !handControl.releaseTimer))) {
            HurtHand(HURT_GO_BACKWARDS, 1);
            break;
        }
        if (platform == NULL) {
            continue;
        }
        if (platform->flags & 0x100) {
            if ((actor->flags & 0x200) && (actor->speed > 1.0f || (actor->type == SWISH && loop != 0))) {
                platform->flags |= 2;
            }
        }
        if ((platform->flags & 0x40000) && actor == &ball && ballControl.type == BALL_MODE_BOWLING && ball.speed > 6.0f
            && (ball.link == NULL || (ball.link == &hand && handControl.releaseTimer))) {
            platform->flags |= 2;
        }
        if ((actor->flags & 0x400) && (platform->actor.flags & 0x400)) {
            actor->obj->platform = platform;
        }
        actor->obj->contactPlatform = platform;
        if (platform->flags & 8) {
            actor->state->onHurtPlatform = 1;
        }
        if (platform->flags & 0x4000) {
            if (actor == &hand) {
                HurtHand(HURT_GO_TO_SAFE, 1);
                retval |= 1;
                break;
            }
            if (actor == &ball) {
                BurstBall();
                retval |= 1;
                break;
            }
        }
        if (CheckActorForTeleport(actor, platform)) {
            bounce = FALSE;
            retval |= 1;
            break;
        }
        if (platform->crumble != NULL && platform->crumble->shakeTime > 0 && platform->crumble->timer == 0
            && !(actor->flags & 0x40000000)) {
            platform->crumble->timer = 1;
        }
        if (platform != NULL && platform->path != NULL && platform->path->moving == -3 && actor->type == BALL) {
            if (gameInfo.level == CARNIVALBONUS) {
                PlaySample(110, &actor->pos, 100, 128);
            } else if (gameInfo.level == SPACEBOSS3) {
                DestroyBulletMissile(actor);
                actor->lifeTimer = 1;
            }
            if (actor != &ball) {
                actor->type = BULLET;
            }
            platform->path->moving = 1;
            platform->pathStopFlags = 2;
        }
        if (platform->motion != NULL) {
            rotvel = *(Vec3 *)platform->motion->rotVel;
            k = 0;
            n = 0;
            for (m = 0; m < 3; m++) {
                if (!platform->motion->active[m]) {
                    others[n++] = m;
                } else {
                    k = m;
                }
            }
        }
        if (actor->phys->mass != 0.0f) {
            if (platform->push != NULL && platform->push->mass != 0.0f && !(actor->flags & 0x40000000)
                && (actor != &hand
                    || ((hand.state->mode == HAND_JOINED || hand.state->mode == HAND_PREWHACK) == 0 && hand.state->mode != HAND_THROWAIM))) {
                SUB_VECTORS(&rel, &actor->vel, &platform->actor.vel);
                onTop = FALSE;
                if (actor->obj->platform == platform || actor->obj->prevPlatform == platform) {
                    onTop = TRUE;
                    if (platform->actor.vel.y <= 0.0f) {
                        rel.y = -gravity;
                    } else {
                        rel.y = -platform->actor.vel.y - gravity;
                    }
                    if ((platform->actor.flags & 0x200000) && (actor->flags & 0x200000)) {
                        onTop = FALSE;
                        if (actor->state->waterState) {
                            rel.y /= 2.0f;
                        } else {
                            platform->actor.vel.y *= 0.9;
                            rel.y /= 1.2;
                        }
                    }
                }
                SCALE_VECTOR(&rel, actor->phys->mass / platform->push->mass);
                if (!onTop) {
                    val = -DOT_PRODUCT(normal, &rel) - 0.2;
                } else {
                    val = -rel.y;
                }
                if (val > 0.0f) {
                    v = *normal;
                    SCALE_VECTOR(&v, -val);
                    for (m = 0; m < 3; m++) {
                        if (!platform->push->axis[m]) {
                            (&v.x)[m] = 0.0f;
                        }
                    }
                    ADD_TO_VECTOR(&platform->actor.vel, &v);
                    platform->actor.speed = LENGTH(&platform->actor.vel);
                    if (platform->actor.speed > platform->maxSpeed) {
                        SCALE_VECTOR(&platform->actor.vel, platform->maxSpeed / platform->actor.speed);
                        platform->actor.speed = platform->maxSpeed;
                    }
                }
            }
            if (platform->flags & 0x10000) {
                SubVector2D(&diff, &actor->pos, &platform->actor.pos);
                len = LENGTH_2D(&diff);
                if (len != 0.0f) {
                    quat.y = 0.0f;
                    quat.z = diff.x / len;
                    quat.x = -diff.z / len;
                    quat.w = actor->phys->mass * actor->vel.y * len / platform->motion->inertia;
                    GetQuaternionFromRotation(&quat3, &quat);
                    GetQuaternionFromRotation(&quat2, (Quat *)&platform->actor.spinAxis);
                    QuaternionMultiply(&quat, &quat2, &quat3);
                    GetRotationFromQuaternion((Quat *)&platform->actor.spinAxis, &quat);
                }
            }
            if ((platform->flags & 0x10) && !(actor->flags & 0x40000000)) {
                SUB_VECTORS(&v, &actor->vel, &platform->actor.vel);
                if (actor->flags & 1) {
                    v.y -= gravity;
                }
                len = sqrtf((&v.x)[others[0]] * (&v.x)[others[0]]
                            + (&v.x)[others[1]] * (&v.x)[others[1]]);
                if (len != 0.0f) {
                    (&v.x)[others[0]] /= len;
                    (&v.x)[others[1]] /= len;
                }
                val = (&v.x)[others[0]] * (&plane->plane.x)[others[0]]
                    + (&v.x)[others[1]] * (&plane->plane.x)[others[1]];
                if (!(val >= 0.0f)) {
                    SUB_VECTORS(&dir, &q, &platform->actor.pos);
                    if (platform->motion->rot.y != 0.0f) {
                        RotateVector2D(&dir, &dir, -platform->motion->rot.y);
                    }
                    r = sqrtf((&dir.x)[others[0]] * (&dir.x)[others[0]]
                              + (&dir.x)[others[1]] * (&dir.x)[others[1]])
                        * (-val * len);
                    val = (&v.x)[others[0]] * (&dir.x)[others[1]]
                            - (&v.x)[others[1]] * (&dir.x)[others[0]];
                    if (!(ABS(val) < 0.25)) {
                        platform->motion->idleCount = 0;
                        r *= val;
                        r *= actor->phys->mass;
                        r /= platform->motion->inertia * 100000.0f;
                        if (k == 1) {
                            platform->motion->rotVel[1] += r;
                        } else {
                            platform->motion->rotVel[k] -= r;
                        }
                    }
                }
            }
        }
        if (platform->motion != NULL && !(platform->flags & 0x10000)) {
            if (others[0] < 0 || others[0] > 2 || others[1] < 0 || others[1] > 2) {
                if (retval & 1) {
                    break;
                }
                continue;
            }
            if ((&rotvel.x)[k] != 0.0f && actor->obj->prevContactPlatform != platform) {
                SUB_VECTORS(&nearestPoint, &q, &platform->actor.pos);
                (&nearestPoint.x)[k] = 0.0f;
                r = LENGTH(&nearestPoint);
                ang = atan2((&nearestPoint.x)[others[1]], (&nearestPoint.x)[others[0]]);
                if (k == 1) {
                    ang += rotvel.y;
                } else {
                    ang -= (&rotvel.x)[k];
                }
                (&dir.x)[others[0]] = (&nearestPoint.x)[others[0]] - r * cosf(ang);
                (&dir.x)[others[1]] = (&nearestPoint.x)[others[1]] - r * sinf(ang);
                (&dir.x)[k] = 0;
                MakeUnit(&dir);
                SUB_VECTORS(&nearestPoint, &actor->pos, &q);
                ADD_TO_VECTOR(&nearestPoint, &globalRot[loop]);
                if ((&rotvel.x)[k] != 0.0f && (platform->flags & 0x10)) {
                    for (j = 0; actor->obj->planes[j] != NULL; j++) {
                        if ((Plane *)plane != actor->obj->planes[j]
                            && DOT_PRODUCT(&dir, actor->obj->planes[j]) < -0.5) {
                            platform->motion->rotVel[k] = -platform->motion->rotVel[k] * actor->phys->bounce;
                        }
                    }
                }
                if (k != 1) {
                    SCALE_VECTOR(&dir, r * ABS((&rotvel.x)[k]));
                    ADD_TO_VECTOR(&actor->vel, &dir);
                    SUB_FROM_VECTOR(&actor->prevPos, &dir);
                    if (numBounces) {
                        ADD_TO_VECTOR(&newvel, &dir);
                        SUB_FROM_VECTOR(&totalVel, &bvel[numBounces - 1]);
                        AddToOneScaledVector(&dir, &bvel[numBounces - 1], actor->speed);
                        MakeUnit(&dir);
                        ADD_TO_VECTOR(&totalVel, &dir);
                    }
                    actor->speed = LENGTH(&actor->vel);
                }
            }
        }
    }
    if (bounce) {
        actor->vel = newvel;
    }
    return retval;
}

f32 Altitude(Vec3 *pos, Vec3 **floor, Platform **plat, ActorObject *coll) {
    Vec3 d;
    Vec3 point;
    f32 minHeight = 1000000.0f;
    f32 height;
    Actor *actPtr;
    Platform *platform;
    u32 k;

    point = *pos;
    *floor = NULL;
    *plat = NULL;
    if (coll != NULL && coll->planes[0] != NULL && ((CollFace *)coll->planes[0])->plane.y >= 0.95
        && pos->y >= coll->contactPos[0].y) {
        *floor = (Vec3 *)coll->planes[0];
        *plat = coll->platform;
        return pos->y - coll->contactPos[0].y;
    }
    for (actPtr = collisionList.head.collNext; actPtr != &collisionList.head; actPtr = actPtr->collNext) {
        SubVector2D(&d, pos, &actPtr->pos);
        if (!(actPtr->radius * actPtr->radius < d.x * d.x + d.z * d.z) && !(minHeight < pos->y - actPtr->pos.y - actPtr->radius)) {
            for (k = 0; k < actPtr->inst->obj->mesh->numFaces; k++) {
                height = AltitudeCheckPlane(pos, &actPtr->faces[k], &point, minHeight);
                if (height != 1000000.0f) {
                    minHeight = height;
                    *floor = (Vec3 *)&actPtr->faces[k];
                }
            }
        }
    }
    for (platform = platformList.head.next; platform != &platformList.head; platform = platform->next) {
        if (!(platform->flags & 0x84) && !(minHeight < pos->y - platform->actor.pos.y - platform->actor.radius)) {
            SubVector2D(&d, pos, &platform->actor.pos);
            if (!(platform->actor.radius * platform->actor.radius < d.x * d.x + d.z * d.z)) {
                if (platform->lastUpdate < frameCount) {
                    RecalculatePlatformInfo(platform);
                }
                for (k = 0; k < platform->actor.inst->obj->mesh->numFaces; k++) {
                    height = AltitudeCheckPlane(pos, &platform->actor.faces[k], &point, minHeight);
                    if (height != 1000000.0f) {
                        minHeight = height;
                        *floor = (Vec3 *)&platform->actor.faces[k];
                        *plat = platform;
                    }
                }
            }
        }
    }
    return minHeight != 1000000.0f ? minHeight : -1.0f;
}

f32 AltitudeCheckPlane(Vec3 *pos, CollFace *plane, Vec3 *point, f32 minHeight) {
    f32 t;
    f32 u0;
    f32 v0;
    f32 alpha;
    f32 beta;

    if (!(plane->plane.y <= 0.0f)) {
        t = DOT_PRODUCT(pos, &plane->plane) + plane->plane.d;
        t /= plane->plane.y;
        if (t > 0.0f && t < minHeight) {
            point->y = pos->y - t;
            u0 = (&point->x)[plane->axis[0]] - (&plane->edgeStart[0].x)[plane->axis[0]];
            v0 = (&point->x)[plane->axis[1]] - (&plane->edgeStart[0].x)[plane->axis[1]];
            beta = -1.0f;
            if (plane->proj[0] == 0.0f) {
                alpha = u0 / plane->proj[1];
                if (alpha >= -0.001 && alpha <= 1.001) {
                    beta = (v0 - alpha * plane->proj[3]) / plane->proj[2];
                }
            } else {
                alpha = (v0 * plane->proj[0] - u0 * plane->proj[2]) / (plane->proj[3] * plane->proj[0] - plane->proj[1] * plane->proj[2]);
                if (alpha >= -0.001 && alpha <= 1.001) {
                    beta = (u0 - alpha * plane->proj[1]) / plane->proj[0];
                }
            }
            if (!(beta < -0.001) && !(beta + alpha > 1.001)) {
                return t;
            }
        }
    }
    return 1000000.0f;
}

void CalculatePlaneInfoWithEdgeLens(CollFace *plane) {
    f32 m;
    u32 i;

    plane->plane.d = -(plane->edgeStart[0].x * plane->plane.x + plane->edgeStart[0].y * plane->plane.y
                      + plane->edgeStart[0].z * plane->plane.z);
    m = MAX(ABS(plane->plane.x), MAX(ABS(plane->plane.y), ABS(plane->plane.z)));
    if (m == ABS(plane->plane.x)) {
        plane->axis[0] = 1;
        plane->axis[1] = 2;
    } else if (m == ABS(plane->plane.y)) {
        plane->axis[0] = 2;
        plane->axis[1] = 0;
    } else {
        plane->axis[0] = 0;
        plane->axis[1] = 1;
    }
    plane->proj[0] = (&plane->edgeStart[1].x)[plane->axis[0]] - (&plane->edgeStart[0].x)[plane->axis[0]];
    plane->proj[1] = (&plane->edgeStart[2].x)[plane->axis[0]] - (&plane->edgeStart[0].x)[plane->axis[0]];
    plane->proj[2] = (&plane->edgeStart[1].x)[plane->axis[1]] - (&plane->edgeStart[0].x)[plane->axis[1]];
    plane->proj[3] = (&plane->edgeStart[2].x)[plane->axis[1]] - (&plane->edgeStart[0].x)[plane->axis[1]];
    for (i = 0; i < 3; i++) {
        SUB_VECTORS(&plane->edgeDir[i], &plane->edgeStart[(i + 1) % 3], &plane->edgeStart[i]);
        plane->edgeLen[i] = LENGTH(&plane->edgeDir[i]);
        if (plane->edgeLen[i] != 0.0f) {
            plane->edgeDir[i].x /= plane->edgeLen[i];
            plane->edgeDir[i].y /= plane->edgeLen[i];
            plane->edgeDir[i].z /= plane->edgeLen[i];
        }
    }
}

void CalculatePlaneInfo(CollFace *plane) {
    f32 m;
    u32 i;

    plane->plane.d = -(plane->edgeStart[0].x * plane->plane.x + plane->edgeStart[0].y * plane->plane.y
                      + plane->edgeStart[0].z * plane->plane.z);
    m = MAX(ABS(plane->plane.x), MAX(ABS(plane->plane.y), ABS(plane->plane.z)));
    if (m == ABS(plane->plane.x)) {
        plane->axis[0] = 1;
        plane->axis[1] = 2;
    } else if (m == ABS(plane->plane.y)) {
        plane->axis[0] = 2;
        plane->axis[1] = 0;
    } else {
        plane->axis[0] = 0;
        plane->axis[1] = 1;
    }
    plane->proj[0] = (&plane->edgeStart[1].x)[plane->axis[0]] - (&plane->edgeStart[0].x)[plane->axis[0]];
    plane->proj[1] = (&plane->edgeStart[2].x)[plane->axis[0]] - (&plane->edgeStart[0].x)[plane->axis[0]];
    plane->proj[2] = (&plane->edgeStart[1].x)[plane->axis[1]] - (&plane->edgeStart[0].x)[plane->axis[1]];
    plane->proj[3] = (&plane->edgeStart[2].x)[plane->axis[1]] - (&plane->edgeStart[0].x)[plane->axis[1]];
    for (i = 0; i < 3; i++) {
        SUB_VECTORS(&plane->edgeDir[i], &plane->edgeStart[(i + 1) % 3], &plane->edgeStart[i]);
        if (plane->edgeLen[i] != 0.0f) {
            plane->edgeDir[i].x /= plane->edgeLen[i];
            plane->edgeDir[i].y /= plane->edgeLen[i];
            plane->edgeDir[i].z /= plane->edgeLen[i];
        }
    }
}

void CalcCollisionSpheres(Actor *actor) {
    u32 j;

    for (j = 0; j < actor->obj->numSpheres; j++) {
        if (actor->obj->spheres[j].flags & 0x20) {
            COPY_VECTOR(globalPos + j, &actor->pos);
            ZERO_VECTOR(globalRot + j);
        } else if (actor->obj->spheres[j].flags & 0x10) {
            COPY_VECTOR(globalPos + j, &actor->obj->spheres[j].pos);
            SUB_VECTORS(globalRot + j, globalPos + j, &actor->pos);
        } else {
            RotateVectorByQuaternion(globalRot + j, &actor->obj->spheres[j].pos, &actor->rotQuat);
            ADD_VECTORS(globalPos + j, globalRot + j, &actor->pos);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/collision", pad_8015C34C);

INCLUDE_RODATA("asm/nonmatchings/collision", pad_801071B4);
