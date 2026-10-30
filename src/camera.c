#include "common.h"
#include "ident.h"
#include "camera.h"
#include "maths.h"
#include "game.h"
#include "hand.h"
#include "libc.h"
#include "controller.h"
#include "ball.h"
#include "actor.h"
#include "jalloc.h"
#include "object.h"

extern s32 levelCamHeight;
extern s32 levelCamDistance;
extern s16 cameraZoom;
extern f32 cameraZoomVal[];

void DisableControl(s16 time);
extern u8 icl;
extern u8 icr;
extern s32 controlMethod;
extern u8 ActiveController;
f32 Altitude(Vec3 *pos, Vec3 **floor, s32 *plat, ActorObject *coll);

extern f32 camBehindLimit;
extern f32 camBehindSpeed;
extern f32 camBehindAngle;
extern f32 camBehindScale;
extern f32 camVal;
extern u8 disableZoom;
extern u8 gamePaused;
extern f32 levelHandScale;

void ResetCamera(void) {
    memclear(&camera, sizeof(Actor));
    InitActorStructures(&camera, 0x13);
    camera.type = CAMERA;
    RestartCamera();
    memclear(&cameraInfo.actor, sizeof(Actor));
    InitActorStructures(&cameraInfo.actor, 1);
    cameraInfo.actor.pos.x = 33.0f;
    cameraInfo.actor.pos.y = 30.0f;
    cameraInfo.actor.pos.z = -48.0f;
    cameraInfo.actor.friction = 0.2f;
    cameraInfo.actor.phys->rotDrag = 0.8f;
    cameraInfo.actor.phys->maxSpeed = 30.0f;
    cameraInfo.actor.phys->friction = cameraInfo.actor.friction;
    cameraInfo.focus = &hand.pos;
    cameraInfo.targetHeight = levelCamHeight;
    cameraInfo.targetDistance = levelCamDistance * cameraZoomVal[cameraZoom];
}

void RestartCamera(void) {
    camera.obj->spheres[0].radius = 40.0f;
    camera.obj->spheres[0].targetRadius = 40.0f;
    levelCamHeight = -30;
    levelCamDistance = 100;
    camera.flags = 16;
    cameraInfo.sideOffsetEnabled = 1;
    cameraInfo.targetFrozen = 0;
    cameraInfo.destActive = 0;
    camera.obj->numSpheres = 1;
    camera.friction = 0.26666668f;
    camera.phys->bounce = -1.0f;
    camera.phys->rotDrag = 0.8f;
    camera.phys->friction = camera.friction;
    if (gameInfo.level == SPACEBOSS3) {
        camera.phys->maxSpeed = 200.0f;
    } else {
        camera.phys->maxSpeed = 20.0f;
    }
    camera.phys->turnSpeed = 0.05f;
    camera.phys->rotAim[0] = 1;
    camera.active = 1;
    cameraInfo.distance = 250.0f;
    cameraInfo.heightOffset = 0;
    cameraInfo.angleLock = 0;
    cameraInfo.followSpeed = 2.0f;
    cameraInfo.mode = 1;
    DeactivateCameraControlMode();
    cameraInfo.focusVel = &hand.vel;
}

void SetCameraOverride(f32 camX, f32 camY, f32 camZ, Vec3 *camTargetPos, Vec3 *camTargetVel, s32 timer, s32 freezeHandAndBall) {
    cameraInfo.overrideTimer = timer;
    cameraInfo.prevFocus = cameraInfo.focus;
    cameraInfo.prevFocusVel = cameraInfo.focusVel;
    cameraInfo.prevDestActive = cameraInfo.destActive;
    cameraInfo.prevCamPos = camera.pos;
    camera.pos.x = camX;
    camera.pos.y = camY;
    camera.pos.z = camZ;
    ZERO_VECTOR(&camera.vel);
    cameraInfo.focus = camTargetPos;
    cameraInfo.focusVel = camTargetVel;
    cameraInfo.prevTargetY = cameraInfo.targetY;
    cameraInfo.prevTargetPos = cameraInfo.actor.pos;
    cameraInfo.actor.pos = *camTargetPos;
    if (gameInfo.level != PRESENT) {
        DisableControl(timer + 10);
        cameraInfo.overrideFlags = freezeHandAndBall;
    }
}

void MoveCam(void) {
    Vec3 v;
    Vec3 w;
    Vec3 a;
    Vec3 b;
    Actor *actor;
    Vec3 *plane = NULL;
    f32 val;
    f32 val2;
    f32 temp;
    f32 angle;
    f32 camSpeed;
    f32 maxSpeed;

    actor = &hand;
    if (gameInfo.level >= INTRO || cameraInfo.mode != 1) {
        return;
    }
    if (cameraInfo.overrideTimer != 0) {
        if (--cameraInfo.overrideTimer == 0) {
            cameraInfo.overrideFlags = 0;
            cameraInfo.focus = cameraInfo.prevFocus;
            cameraInfo.focusVel = cameraInfo.prevFocusVel;
            cameraInfo.destActive = cameraInfo.prevDestActive;
            camera.pos = cameraInfo.prevCamPos;
            cameraInfo.actor.pos = cameraInfo.prevTargetPos;
            cameraInfo.actor.pos.y = cameraInfo.prevTargetY;
            if (cameraInfo.destActive == 1) {
                cameraInfo.destPos = cameraInfo.prevDestPos;
            }
            if (cameraInfo.focus == &ball.pos && ball.link != &hand) {
                cameraInfo.focus = &hand.pos;
                cameraInfo.focusVel = &hand.vel;
            }
        }
    }
    if (pad[ActiveController].pressed[BUTTON_C_DOWN]) {
        if (!disableZoom) {
            cameraZoom++;
            PlaySampleNot3D(219, 100, 128, 100);
            if (cameraZoom >= 3) {
                cameraZoom = 0;
            }
        } else {
            PlaySampleNot3D(111, 100, 128, 100);
        }
    }
    if (cameraInfo.turnTimer) {
        cameraInfo.turnTimer--;
        ZERO_VECTOR(&w);
        w.z = FindShortestAngleSigned(cameraInfo.turnAngle, camera.rot.y);
        if (ABS(w.z) < 0.05) {
            cameraInfo.turnTimer = 0;
        }
        w.z *= 5.0;
        RotateVector2D(&w, &w, Aabs(camera.rot.y - 1.570796327));
        ADD_TO_VECTOR(&camera.vel, &w);
    }
    if (hand.state->mode == HAND_JOINED || hand.state->mode == HAND_PREWHACK || hand.state->mode == HAND_BALLWALK) {
        actor = &ball;
    }
    if (cameraInfo.destActive == 0 && cameraInfo.overrideTimer == 0 && cameraInfo.blocked == 0) {
        if (cameraInfo.slopeAdjust && !player.powerups[SUCTION]) {
            if (actor->obj->planes[0] != NULL && actor->shadow->floor != NULL) {
                if (actor->obj->contactCount[0] >= 2) {
                    plane = actor->shadow->floor;
                    b.x = b.z = 0.0f;
                    b.y = 1.0f;
                    ADD_VECTORS(&a, &actor->pos, &b);
                    val = DistanceBetweenPoints(&a, &camera.pos);
                    ADD_VECTORS(&a, plane, &actor->pos);
                    camVal = (DistanceBetweenPoints(&a, &camera.pos) - val) * 15.0f;
                    if (camVal >= 0.0f) {
                        camVal = (camVal > 9.0f) ? 9.0f : camVal;
                    } else {
                        camVal = MAX(camVal, -6.0f);
                    }
                }
            } else if (actor->obj->contactCount[0] >= 2) {
                camVal = 0.0f;
            }
        } else {
            camVal = 0.0f;
        }
        camSpeed = 5.0f;
        cameraInfo.heightOffset = (camVal + 3.0f) * 10.0f * levelHandScale / 0.05;
        cameraInfo.heightOffset += cameraInfo.targetHeight;
        if (gameInfo.level == SPACEBOSS3) {
            camSpeed = 50.0f;
        }
        if (cameraInfo.distance > cameraInfo.targetDistance + 1.0f) {
            cameraInfo.distance -= MIN(cameraInfo.distance - cameraInfo.targetDistance, camSpeed);
        } else if (cameraInfo.distance < cameraInfo.targetDistance - 1.0f) {
            cameraInfo.distance += MIN(cameraInfo.targetDistance - cameraInfo.distance, camSpeed);
        }
    }
    SUB_VECTORS(&v, &camera.pos, &cameraInfo.actor.pos);
    angle = atan2(v.x, v.z);
    cameraInfo.prevRotY = camera.rot.y;
    camera.rot.y = Aabs(angle + 3.141592654);
    if (cameraInfo.destActive == 1) {
        SUB_VECTORS(&v, &camera.pos, &cameraInfo.destPos);
        SCALE_VECTOR(&v, 0.25);
        v.y *= 2.0f;
        cameraInfo.heightOffset = 0.0f;
        if (DOT_PRODUCT(&v, &v) > 400.0f) {
            MakeUnit(&v);
            SCALE_VECTOR(&v, 20.0f);
        }
        SUB_FROM_VECTOR(&camera.pos, &v);
    } else if (cameraInfo.overrideTimer == 0) {
        if (cameraInfo.orbiting == 1) {
            w.x = hand.pos.x - camera.pos.x;
            w.y = hand.pos.y - camera.pos.y;
            cameraInfo.focusAngle = cameraInfo.orbitAngle;
            w.z = hand.pos.z - camera.pos.z;
            temp = FindShortestAngleSigned(Aabs(atan2(w.x, w.z)), cameraInfo.orbitAngle);
            if (temp > 0.01) {
                ZERO_VECTOR(&w);
                w.z = (temp * 3.0f > cameraInfo.orbitSpeed) ? cameraInfo.orbitSpeed * 10.0f : temp * 3.0f * 10.0f;
                RotateVector2D(&w, &w, Aabs(camera.rot.y + 1.570796327));
                camera.vel.x += w.x;
                camera.vel.y += w.y;
                camera.vel.z += w.z;
            } else if (temp < -0.01) {
                ZERO_VECTOR(&w);
                w.z = (-temp * 3.0f > cameraInfo.orbitSpeed) ? cameraInfo.orbitSpeed * 10.0f : -temp * 3.0f * 10.0f;
                RotateVector2D(&w, &w, Aabs(camera.rot.y - 1.570796327));
                camera.vel.x += w.x;
                camera.vel.y += w.y;
                camera.vel.z += w.z;
            }
        }
        val2 = LENGTH_2D(&v);
        maxSpeed = 10.0f;
        if (gameInfo.level == SPACEBOSS3 && !gameInfo.cameoActive) {
            maxSpeed = 100.0f;
        }
        camera.phys->maxSpeed = 2.0f * maxSpeed;
        if (val2 > cameraInfo.distance + levelHandScale * 10.0f / 0.05) {
            AccelerateActor2D(&camera, MIN((val2 - (cameraInfo.distance + levelHandScale * 10.0f / 0.05)) / 2.0, maxSpeed),
                              camera.phys->maxSpeed);
        }
        if (val2 < cameraInfo.distance - levelHandScale * 10.0f / 0.05) {
            AccelerateActor2D(&camera, -MIN((cameraInfo.distance - levelHandScale * 10.0f / 0.05 - val2) / 2.0, maxSpeed),
                              camera.phys->maxSpeed);
        }
        cameraInfo.blocked = DoCameraCollision();
        if (cameraInfo.angleLock == 0) {
            SUB_VECTORS(&w, cameraInfo.focus, &camera.pos);
            cameraInfo.focusAngle = Aabs(atan2(w.x, w.z));
            if (cameraInfo.orbiting == 0) {
                if (!cameraInfo.blocked && !gamePaused && LENGTH_2D(&hand.vel) > camBehindLimit
                    && hand.state->mode != HAND_PUSH) {
                    angle = FindShortestAngleSigned(camera.rot.y, Aabs(hand.rot.y));
                    if (ABS(angle) > camBehindAngle && ABS(angle) < 3.141592654 - camBehindAngle) {
                        if (ABS(angle) < 1.570796327) {
                            camBehindScale = ABS(angle / 1.570796327);
                        }
                        if (ABS(angle) > 1.570796327) {
                            camBehindScale = (3.141592654 - ABS(angle)) / 1.570796327;
                        }
                        camBehindScale = ABS(camBehindScale);
                        camBehindScale *= cameraZoomVal[cameraZoom];
                        if (angle > 0.0f) {
                            ZERO_VECTOR(&w);
                            w.z = camBehindSpeed * LENGTH_2D(&hand.vel) * camBehindScale;
                            RotateVector2D(&w, &w, Aabs(camera.rot.y + 1.570796327));
                        } else {
                            ZERO_VECTOR(&w);
                            w.z = camBehindSpeed * LENGTH_2D(&hand.vel) * camBehindScale;
                            RotateVector2D(&w, &w, Aabs(camera.rot.y - 1.570796327));
                        }
                        actor = &camera;
                        ADD_TO_VECTOR(&actor->vel, &w);
                    }
                }
                if (cameraInfo.orbiting == 0 && cameraInfo.alignBehind && !cameraInfo.blocked
                    && pad[ActiveController].mag == 0.0f) {
                    angle = FindShortestAngleSigned(camera.rot.y, Aabs(hand.rot.y));
                    if (angle > 0.0f) {
                        ZERO_VECTOR(&w);
                        if (angle > 0.2) {
                            w.z = 2.0f;
                        } else {
                            w.z = angle * 10.0;
                        }
                        RotateVector2D(&w, &w, Aabs(camera.rot.y + 1.570796327));
                    } else {
                        ZERO_VECTOR(&w);
                        if (-angle > 0.2) {
                            w.z = 2.0f;
                        } else {
                            w.z = -angle * 10.0;
                        }
                        RotateVector2D(&w, &w, Aabs(camera.rot.y - 1.570796327));
                    }
                    actor = &camera;
                    ADD_TO_VECTOR(&actor->vel, &w);
                }
            }
        }
    }
    if (cameraInfo.overrideTimer == 0 && cameraInfo.blocked == 0 && cameraInfo.destActive == 0) {
        v.y -= cameraInfo.distance / 3.0f + cameraInfo.heightOffset;
        camera.vel.y -= v.y / 8.0f;
    }
    if (cameraInfo.fallTime) {
        if (hand.shadow->altitude == -1.0f) {
            cameraInfo.targetDistance = (cameraInfo.targetDistance / 2.0 < 10.0) ? 10.0f : cameraInfo.targetDistance / 2.0;
            if (cameraInfo.targetHeight < 100.0f) {
                cameraInfo.targetHeight = 100.0f;
            } else {
                cameraInfo.targetHeight = MIN(cameraInfo.targetHeight * 1.3, 500.0f);
            }
            if (camera.vel.y < 0.0f) {
                camera.vel.y = 0.0f;
            }
        } else if (cameraInfo.targetHeight < 10.0f) {
            cameraInfo.targetHeight = 10.0f;
        } else {
            cameraInfo.targetHeight = MIN(cameraInfo.targetHeight * 1.05, 500.0f);
        }
        cameraInfo.fallTime++;
    } else {
        cameraInfo.targetHeight = levelCamHeight;
        cameraInfo.targetDistance = levelCamDistance * cameraZoomVal[cameraZoom];
    }
    if (cameraInfo.turnTimer == 0) {
        cameraInfo.prevAngleLock = cameraInfo.angleLock;
        cameraInfo.angleLock = 0;
        cameraInfo.orbiting = 0;
        cameraInfo.destActive = 0;
        cameraInfo.slopeAdjust = 1;
    }
    cameraInfo.slopeAdjust = 1;
}

s32 DoCameraCollision(void) {
    Vec3 n;
    Vec3 sum;
    s32 i;
    s32 tooClose;
    s32 res;
    f32 outRadius;
    f32 half;
    f32 range;
    f32 dist;
    f32 velAngle;
    f32 pushAngle;
    f32 camAngle;

    ZERO_VECTOR(&sum);
    cameraInfo.squeezeDistAvg += (cameraInfo.squeezeDist - cameraInfo.squeezeDistAvg) / 8.0f;
    outRadius = camera.obj->spheres[0].radius;
    half = outRadius / 2.0f;
    res = FALSE;
    tooClose = FALSE;
    range = outRadius - half;
    for (i = 0; i < 4; i++) {
        if (camera.obj->planes[i] == NULL) {
            break;
        }
        n = *(Vec3 *)camera.obj->planes[i];
        SCALE_VECTOR(&n, 2.0f - cameraInfo.planeDist[i] / range);
        ADD_TO_VECTOR(&sum, &n);
        if (cameraInfo.planeDist[i] < half / 2.0f) {
            tooClose = TRUE;
        }
    }
    cameraInfo.squeezeDist = 0.0f;
    if (i != 0) {
        velAngle = Aabs(atan2(camera.vel.z, camera.vel.x));
        pushAngle = Aabs(atan2(sum.z, sum.x));
        camAngle = Aabs(atan2(camera.pos.z - cameraInfo.actor.pos.z, camera.pos.x - cameraInfo.actor.pos.x));
        if (FindShortestAngle(velAngle, pushAngle) > PI * 2 / 3 && FindShortestAngle(camAngle, pushAngle) > PI / 2) {
            if (tooClose) {
                n.x = camera.pos.x - hand.pos.x;
                n.z = camera.pos.z - hand.pos.z;
                ZERO_VECTOR(&camera.vel);
                dist = LENGTH_2D(&n);
                cameraInfo.squeezeDist = MIN(cameraInfo.distance - dist, 120.0f);
                if (cameraInfo.squeezeDist < 0.0f) {
                    cameraInfo.squeezeDist = 0.0f;
                }
                res = TRUE;
            }
        } else if (FindShortestAngle(velAngle, pushAngle) < PI / 2 && PI / 2 < FindShortestAngle(camAngle, pushAngle)) {
            sum.x *= 1.0f / i;
            sum.y *= 1.0f / i;
            sum.z *= 1.0f / i;
            cameraInfo.squeezeDist = 0.0f;
            camera.vel.x += sum.x;
            camera.vel.y += sum.y;
            camera.vel.z += sum.z;
        } else {
            sum.x *= 1.0f / i;
            sum.y *= 1.0f / i;
            sum.z *= 1.0f / i;
            SCALE_VECTOR(&sum, 6.0f);
            cameraInfo.squeezeDist = 0.0f;
            camera.vel.x += sum.x;
            camera.vel.y += sum.y;
            camera.vel.z += sum.z;
        }
    }
    return res;
}

void PlayerSpinCamera(void) {
    Vec3 d;
    Vec3 reflexVector;
    Vec3 unused; // unused, but takes the original stack slot
    Vec3 camVel;
    Vec3 spin;
    Vec3 vect;
    Vec3 *plane;
    s32 plat;
    s32 numPlanes = 0;
    s32 i;
    f32 diff;
    f32 angle; // reused for the camVel length, as the original register use needs
    f32 minHeight;

    if (cameraInfo.overrideTimer && (cameraInfo.overrideFlags & 1)) {
        ZERO_VECTOR(&hand.vel);
        ZERO_VECTOR(&ball.vel);
    }
    if (cameraInfo.orbiting == 1 || cameraInfo.destActive == 1 || controlMethod == CONTROL_CAMERA || cameraInfo.overrideTimer) {
        return;
    }
    ZERO_VECTOR(&camVel);
    ZERO_VECTOR(&vect);
    if (icl) {
        icl--;
        pad[ActiveController].latch[BUTTON_C_LEFT] = 1;
    }
    if (icr) {
        icr--;
        pad[ActiveController].latch[BUTTON_C_RIGHT] = 1;
    }
    if (cameraInfo.angleLock == 1) {
        if (pad[ActiveController].button[BUTTON_C_RIGHT] || pad[ActiveController].button[BUTTON_C_LEFT]) {
            vect.x = cameraInfo.focus->x - camera.pos.x;
            vect.y = cameraInfo.focus->y - camera.pos.y;
            vect.z = cameraInfo.focus->z - camera.pos.z;
            cameraInfo.focusAngle = Aabs(atan2(vect.x, vect.z));
        }
        if (cameraInfo.orbiting == 0) {
            vect.x = cameraInfo.focus->x - camera.pos.x;
            vect.y = cameraInfo.focus->y - camera.pos.y;
            vect.z = cameraInfo.focus->z - camera.pos.z;
            diff = FindShortestAngleSigned(Aabs(atan2(vect.x, vect.z)), cameraInfo.focusAngle);
            ZERO_VECTOR(&vect);
            if (diff > 0.0f) {
                vect.z = diff * 10.0f * 4.0f;
                RotateVector2D(&vect, &vect, Aabs(camera.rot.y + 1.570796327));
            } else {
                vect.z = -diff * 10.0f * 4.0f;
                RotateVector2D(&vect, &vect, Aabs(camera.rot.y - 1.570796327));
            }
            spin = vect;
        }
    }
    if (cameraInfo.fallTime == 0) {
        if (pad[ActiveController].button[BUTTON_C_RIGHT]) {
            d.x = __sinf(camera.rot.y - 1.570796327) * 4.0 * 4.0;
            d.y = 0.0f;
            d.z = __cosf(camera.rot.y - 1.570796327) * 4.0 * 4.0;
            camVel = d;
        } else if (pad[ActiveController].button[BUTTON_C_LEFT]) {
            d.x = __sinf(camera.rot.y + 1.570796327) * 4.0 * 4.0;
            d.y = 0.0f;
            d.z = __cosf(camera.rot.y + 1.570796327) * 4.0 * 4.0;
            camVel = d;
        }
    }
    if (cameraInfo.angleLock == 1) {
        ADD_TO_VECTOR(&camVel, &spin);
    }
    ZERO_VECTOR(&reflexVector);
    for (i = 0; i < 4 && camera.obj->planes[i] != NULL; i++) {
        ADD_TO_VECTOR(&reflexVector, camera.obj->planes[i]);
        numPlanes++;
    }
    if (numPlanes) {
        MakeUnit(&reflexVector);
        FlipVector(&reflexVector);
        d = camVel;
        MakeUnit(&d);
        angle = Aabs(acos(DOT_PRODUCT(&d, &reflexVector)));
        if (angle > 1.0f) {
            angle = LENGTH(&camVel);
            SCALE_VECTOR(&reflexVector, angle * 1.3);
            SUB_FROM_VECTOR(&camVel, &reflexVector);
            MakeUnit(&camVel);
            SCALE_VECTOR(&camVel, angle);
        } else {
            ZERO_VECTOR(&camera.vel);
            if (angle > 0.0f) {
                if (pad[ActiveController].button[BUTTON_C_LEFT]) {
                    icl = 20;
                    pad[ActiveController].latch[BUTTON_C_LEFT] = 1;
                }
                if (pad[ActiveController].button[BUTTON_C_RIGHT]) {
                    icr = 20;
                    pad[ActiveController].latch[BUTTON_C_RIGHT] = 1;
                }
                return;
            }
        }
    }
    ADD_TO_VECTOR(&camera.vel, &camVel);
    if (numPlanes) {
        minHeight = Altitude(&camera.pos, &plane, &plat, camera.obj);
        if (minHeight < 30.0f && minHeight >= 0.0f) {
            camera.pos.y += 30.0f - minHeight;
            camera.vel.y /= 2.0f;
        }
    }
}

void CamCirclePoint(Vec3 *pos) {
    Vec3 tempVect2;
    Vec3 dir;

    cameraInfo.destActive = TRUE;
    tempVect2.x = pos->x;
    tempVect2.z = pos->z;
    tempVect2.y = hand.pos.y;
    SUB_VECTORS(&dir, &hand.pos, &tempVect2);
    MakeUnit(&dir);
    SCALE_VECTOR_2D(&dir, cameraInfo.targetDistance);
    AddVector2D(&tempVect2, &hand.pos, &dir);
    tempVect2.y += levelCamHeight;
    cameraInfo.destPos = tempVect2;
}

void CalcCameraRot(void) {
    Vec3 d;
    f32 val;

    SUB_VECTORS(&d, &camera.pos, &cameraInfo.actor.pos);
    val = atan2(d.x, d.z);
    cameraInfo.prevRotY = camera.rot.y;
    camera.rot.y = Aabs(val + 3.141592654);
    val = atan2(LENGTH_2D(&d), d.y);
    cameraInfo.prevRotX = camera.rot.x;
    camera.rot.x = Aabs(val - 1.570796327);
}

INCLUDE_RODATA("asm/nonmatchings/camera", pad_8010770C);
