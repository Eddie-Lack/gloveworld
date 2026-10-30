#ifndef BALL_H
#define BALL_H

#include "common.h"
#include "actor.h"
#include "sound.h"

typedef struct BallControl {
    /* 0x00 */ s32 loseTimer;
    /* 0x04 */ struct Actor *enemy;
    /* 0x08 */ Vec3 enemyOffset;
    /* 0x14 */ s16 enemySphere;
    /* 0x16 */ s16 releaseTimer;
    /* 0x18 */ s16 fallTime;
    /* 0x1A */ s16 returnTime;
    /* 0x1C */ s8 prevGround[0x2];
    /* 0x1E */ s8 state;
    /* 0x1F */ s8 type;
    /* 0x20 */ Actor orbiter;
    /* 0x104 */ s32 cancerTimer;
    /* 0x108 */ f32 cancerScale;
    /* 0x10C */ s32 hitFlags;
    /* 0x110 */ Vec3 enemyRot;
    /* 0x11C */ s32 changeTime;
    /* 0x120 */ s32 prevType;
    /* 0x124 */ ContinuousSample samples[2];
    /* 0x15C */ f32 snow;
    /* 0x160 */ s8 transformState;
    /* 0x161 */ u8 numPlasters;
    /* 0x162 */ u8 hurtTimer;
    /* 0x163 */ u8 maxPlasters;
} BallControl;

extern BallControl ballControl;
extern s32 ballStatic;

void ResetBall(void);
void GiveBallCancer(s32 time);
void HurtBall(void);
void MoveBall(void);
void BurstBall(void);
void LoseBall(s32 time);
void DropBall(s16 timer);
void ReleaseBall(void);
f32 BallBounceMaxSpeed(void);
f32 BallJoinedMaxSpeed(void);
s32 BallWalkAllowed(void);
void MoveHandToBall(void);
s32 GetHandPosAim(Vec3 *bestPos, Vec3 *lagPos);
void MoveHandToBallStatic(void);
void MoveBallToHand(void);
void RotateBallWithHand(void);
s32 TrackBall(void);
void ActorStartRoll(Actor *actor, f32 x, f32 z);
void RollActor(Actor *actor);
void FindFlattestPlane(Actor *actor, Plane **plane, s32 *planeNum);
void ShatterBall(void);
void ShatterActor(Actor *actor, void *bit, s32 numBits, f32 scaleMult);
void CalcBallTexture(s32 index);
void BlendBallTexture(s32 mode1, s32 mode2, f32 val);
void ChangeBallInto(s32 type);
void HandTransformBall(void);
void UpdatePlasters(void);
void BreakOffSnow(f32 scale, f32 radAim, f32 rad);

#endif
