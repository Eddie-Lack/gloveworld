#include "common.h"
#include "game.h"
#include "maths.h"
#include "actor.h"
#include "camera.h"
#include "hand.h"
#include "quake.h"
#include "ball.h"
#include "enemy.h"

extern s32 autoPlayback;
extern s32 frameCount;
extern f32 eqf;
extern f32 eqs;
extern f32 camStr;
extern f32 ballStr;

void HurtHand(s32 type, s32 damage);
void KillHand(s32 type);

inline void StartEarthquake(s16 duration, s16 maxStrength, s16 minStrength, s16 numCycles, s32 flags) {
    if (autoPlayback == 0) {
        if (flags & 4) {
            earthQuake.time = duration / 2 + 1;
        } else {
            earthQuake.time = duration;
        }
        earthQuake.end = minStrength;
        earthQuake.start = maxStrength;
        earthQuake.flags = flags;
        earthQuake.speed = earthQuake.time / (f32)(numCycles * TWOPI);
        earthQuake.phase = earthQuake.speed * (3 * PI) / 2.0 - frameCount;
    }
}

void StartEarthquakeAtPoint(Vec3 *pos, s16 duration, f32 maxStrength, f32 minStrength, s16 numCycles, s32 flags) {
    f32 dist;
    f32 str;
    f32 str2;

    if (autoPlayback == 2) {
        return;
    }
    dist = DistanceBetweenPoints(cameraInfo.focus, pos);
    if (dist < maxStrength || gameInfo.level == SPACEBOSS1) {
        str = maxStrength - dist;
        str *= 0.2;
        str = MIN(str, 1000.0f);
        str2 = minStrength - dist;
        str2 *= 0.2;
        str2 = MIN(str2, 1000.0f);
        if (gameInfo.level == SPACEBOSS1) {
            str *= 5.0f;
            str2 *= 5.0f;
            str = MIN(str < 50.0f ? 50.0f : str, 800.0f);
            str2 = MIN(str2 < 25.0f ? 25.0f : str2, 400.0f);
        }
        if (earthQuake.time == 0 || earthQuake.start < dist) {
            StartEarthquake(duration, str, str2, numCycles, flags);
        }
    }
}

void DoEarthquake(void) {
    s32 pad[4]; // forces the original stack frame size
    f32 temp;
    f32 str;
    s32 index;

    if (earthQuake.time > 0) {
        earthQuake.time--;
        temp = (SineWave(earthQuake.speed, earthQuake.phase, frameCount) + 1.0) / 2.0;
        str = temp * earthQuake.start + (1.0f - temp) * earthQuake.end;
        if (earthQuake.flags & 2) {
            RotateVector2DXYZ(&cameraUpVect, &cameraUpVectAim,
                              SineWave(eqf, frameCount, 0) * (eqs * (str < 500.0f ? str : 500.0f)) / 200.0f, 2);
            if (earthQuake.flags & 8) {
                if (earthQuake.time & 1) {
                    camera.pos.y += str / (2.0f * camStr);
                    cameraInfo.actor.pos.y += str / (2.0f * camStr);
                } else {
                    camera.pos.y -= str / (2.0f * camStr);
                    cameraInfo.actor.pos.y -= str / (2.0f * camStr);
                }
            } else {
                if (Random(10) < 5) {
                    (&camera.vel.x)[Random(3)] += str / (2.0f * camStr);
                } else {
                    (&camera.vel.x)[Random(3)] -= str / (2.0f * camStr);
                }
                if (Random(10) < 5) {
                    (&cameraInfo.actor.pos.x)[Random(3)] += str / (camStr * 4.0f);
                } else {
                    (&cameraInfo.actor.pos.x)[Random(3)] -= str / (camStr * 4.0f);
                }
            }
        }
        str = (Random(str) - str / 2.0f) / ballStr;
        index = Random(2);
        if (index == 1) {
            index = 2;
        }
        if (earthQuake.flags & 1) {
            if (hand.obj->onGround != 0) {
                (&hand.vel.x)[index] += str * 10.0f / hand.phys->mass;
            }
            if (ball.obj->onGround != 0) {
                (&ball.vel.x)[index] += str * 10.0f / ball.phys->mass;
            }
        }
    }
}

inline s32 ActorInExplosionRadius(Actor *actor, Vec3 *centre, f32 radius, Vec3 *sep, f32 *dist) {
    SUB_VECTORS(sep, &actor->pos, centre);
    *dist = LENGTH(sep);
    if (*dist <= radius) {
        return TRUE;
    }
    return FALSE;
}

void DoExplosion(Vec3 *centre, f32 radius1, f32 radius2, Enemy *actor, f32 maxSpeed, f32 scale) {
    Vec3 diff;
    f32 dist;
    s32 i;
    s32 j;
    s16 mode;

    for (i = 0; i < numberOfEnemies; i++) {
        if (enemies[i].actor.active != 0 && &enemies[i] != actor) {
            if (ActorInExplosionRadius(&enemies[i].actor, centre, radius2, &diff, &dist)) {
                KillEnemy(&enemies[i]);
            }
        }
    }
    j = ActorInExplosionRadius(&hand, centre, radius1, &diff, &dist);
    if (dist < radius2) {
        mode = hand.state->mode;
        if ((mode == HAND_DUCK | mode == HAND_CRAWL) == FALSE && handControl.deathTimer == 0) {
            ApplyAction(HAND_JUMP);
        }
        ZERO_VECTOR(&hand.vel);
        AddToOneScaledVector(&hand.vel, &diff, (radius2 - dist) * scale);
        ActorMaxOutVel(&hand, maxSpeed);
        if (hand.vel.y < 12.0f) {
            hand.vel.y = hand.vel.y + 8.0f < 12.0f ? hand.vel.y + 8.0f : 12.0f;
            ActorMaxOutVel(&hand, maxSpeed);
        }
        if (ball.link == &hand) {
            ReleaseBall();
        }
        mode = hand.state->mode;
        if ((mode == HAND_DUCK | mode == HAND_CRAWL) == FALSE) {
            hand.flags |= 0x01002000;
            hand.rotAim.x = PI / 2;
            hand.rotAim.y = Aabs(atan2(hand.vel.x, hand.vel.z));
            hand.obj->onGround = 0;
        }
    }
    if (j) {
        if (hand.state->mode == HAND_DUCK || hand.state->mode == HAND_CRAWL) {
            HurtHand(HURT_GO_BACKWARDS, 1);
        } else {
            KillHand(HURT_GO_BACKWARDS);
        }
    }
    j = ActorInExplosionRadius(&ball, centre, radius1, &diff, &dist);
    if (dist < radius2) {
        AddToOneScaledVector(&ball.vel, &diff, (radius2 - dist) * scale);
        ActorMaxOutVel(&ball, maxSpeed);
        if (ball.vel.y < 12.0f) {
            ball.vel.y = ball.vel.y + 8.0f < 12.0f ? ball.vel.y + 8.0f : 12.0f;
            ActorMaxOutVel(&ball, maxSpeed);
        }
        ball.obj->onGround = 0;
    }
    if (j) {
        BurstBall();
    }
}
