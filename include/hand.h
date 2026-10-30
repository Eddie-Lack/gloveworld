#ifndef HAND_H
#define HAND_H

#include "types.h"
#include "actor.h"
#include "sound.h"

struct Footprint;

typedef struct HandControl {
    /* 0x000 */ s32 savedFaceNum;
    /* 0x004 */ u8 unk04[0x4];
    /* 0x008 */ s16 actionLock;
    /* 0x00A */ s16 actionDelay;
    /* 0x00C */ s16 spellDelay;
    /* 0x00E */ u8 unk0E[0x2];
    /* 0x010 */ s32 bounceQueued;
    /* 0x014 */ s32 lateBounceQueued;
    /* 0x018 */ s16 prevAction;
    /* 0x01A */ s16 actionTime;
    /* 0x01C */ s8 gait;
    /* 0x01D */ s8 prevGait;
    /* 0x01E */ s16 status;
    /* 0x020 */ s32 prevOnGround;
    /* 0x024 */ s16 noControlTimer;
    /* 0x026 */ s16 deathTimer;
    /* 0x028 */ s16 idleTime;
    /* 0x02A */ u8 bounceCount;
    /* 0x02B */ u8 releaseTimer;
    /* 0x02C */ s16 platformBounceCount;
    /* 0x02E */ s16 walkTime;
    /* 0x030 */ s16 fallTime;
    /* 0x032 */ u8 unk32[0x2];
    /* 0x034 */ s32 doubleJumped;
    /* 0x038 */ Actor edgeProbe;
    /* 0x11C */ Actor frontProbe;
    /* 0x200 */ Actor orbiter;
    /* 0x2E4 */ Actor orbiter2;
    /* 0x3C8 */ Actor orbiter3;
    /* 0x4AC */ f32 edgeAltitude;
    /* 0x4B0 */ Vec3 inputAccel;
    /* 0x4BC */ s16 wallPushTime;
    /* 0x4BE */ u8 unk4BE[0x2];
    /* 0x4C0 */ struct Platform *catapult;
    /* 0x4C4 */ struct Platform *climbPlatform;
    /* 0x4C8 */ Vec3 lastGroundPos;
    /* 0x4D4 */ struct Platform *lastGroundPlatform;
    /* 0x4D8 */ struct Platform *edgePlatform;
    /* 0x4DC */ struct Plane *clingPlanes[4];
    /* 0x4EC */ Vec3 *suctionPlane;
    /* 0x4F0 */ s32 suctionTime;
    /* 0x4F4 */ s32 suctionLostTime;
    /* 0x4F8 */ struct Actor *attachedTo;
    /* 0x4FC */ Vec3 attachOffset;
    /* 0x508 */ s16 attachTag;
    /* 0x50A */ s16 dropTimer;
    /* 0x50C */ s16 joinTimer;
    /* 0x50E */ u8 unk50E[0x2];
    /* 0x510 */ struct Platform *checkpoint;
    /* 0x514 */ f32 rotorSpeed;
    /* 0x518 */ s32 morphStep;
    /* 0x51C */ s32 morphReverse;
    /* 0x520 */ struct Debris *fakeball;
    /* 0x524 */ s16 fakeballDelay;
    /* 0x526 */ u8 unk526[0x2];
    /* 0x528 */ Vec3 prevRot;
    /* 0x534 */ s32 groundStateTime;
    /* 0x538 */ s16 steepSlide;
    /* 0x53A */ s16 touchedActor;
    /* 0x53C */ u8 unk53C[0xC];
    /* 0x548 */ s16 spellTimer[3];
    /* 0x54E */ s16 frog;
    /* 0x550 */ Vec3 blastPos;
    /* 0x55C */ s16 blastSize;
    /* 0x55E */ s16 castSpellTime;
    /* 0x560 */ struct Footprint *footprint;
    /* 0x564 */ s32 flags;
    /* 0x568 */ s32 smokeTimer;
    /* 0x56C */ s32 locateGaribs;
    /* 0x570 */ struct GameSprite *locate[2];
    /* 0x578 */ s16 gentleSlide;
    /* 0x57A */ s16 glovePrintTimer;
    /* 0x57C */ ContinuousSample samples[2];
} HandControl; /* size = 0x5B4 */

typedef struct Player {
    /* 0x00 */ u8 unk00[0x8];
    /* 0x08 */ s32 score;
    /* 0x0C */ s32 garibs;
    /* 0x10 */ s32 lives;
    /* 0x14 */ s32 health;
    /* 0x18 */ s32 levelFrames;
    /* 0x1C */ s32 powerups[15];
    /* 0x58 */ u8 unk58[0x4];
    /* 0x5C */ s32 hurtTimer;
    /* 0x60 */ s32 spell;
    /* 0x64 */ s32 castSpell;
    /* 0x68 */ s32 castSpellType;
} Player; /* size = 0x6C */

extern HandControl handControl;
extern Player player;

s32 ConditionsMet(s32 requestedAction);
void ApplyAction(s32 action);
void MaintainAction(void);
void MoveHand(void);
void CheckHandCatapult(void);
s32 HandOverstepsEdge(Actor *probe, CollFace *plane);
void LoseLife(void);
void CheckHandPlatform(void);
s32 HandCatchAllowed(void);
void HandleSpellOrbiters(void);
void SetHandRadius(f32 *radius, f32 value);
void HandThrowActor(Actor *actor);
void HandWhackActor(Actor *actor);
void AddFakeball(void);
void CheckPlaneForSliding(CollFace *plane);
void ResetHand(s32 first);
void SetHandHopPosition(void);
void DismountEnemy(void);
void DismountBall(void);
void HandProcessFroggy(void);
void DefrogHand(void);
s32 HandOnTrampoline(void);
void CreateBlastCircle(void);
void UpdateBlastCircle(void);
s32 HandSnatchAllowed(void);
void CheckForFootsteps(void);
void AddTongueBit(Vec3 *pos);
void PlaySpellSample(void);
void ApplyActionIdle(void);

#endif
