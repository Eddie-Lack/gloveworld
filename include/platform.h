#ifndef PLATFORM_H
#define PLATFORM_H

#include "types.h"
#include "actor.h"
#include "sound.h"

typedef struct PlatformMotion {
    /* 0x00 */ f32 speed;
    /* 0x04 */ s8 active[3];
    /* 0x07 */ u8 unk07;
    /* 0x08 */ u8 snapCount;
    /* 0x09 */ u8 unk09;
    /* 0x0A */ s16 pause;
    /* 0x0C */ u8 pauseType;
    /* 0x0D */ u8 pauseIndex;
    /* 0x0E */ s16 timer;
    /* 0x10 */ u16 limitFlags;
    /* 0x12 */ u8 unk12[0x2];
    /* 0x14 */ f32 flip;
    /* 0x18 */ f32 pendulumLength;
    /* 0x1C */ f32 accel;
    /* 0x20 */ s32 idleCount;
    /* 0x24 */ f32 tipAngle;
    /* 0x28 */ f32 rotLimit;
    /* 0x2C */ f32 maxRotVel;
    /* 0x30 */ f32 baseRot;
    /* 0x34 */ f32 bankScale;
    /* 0x38 */ Vec3 rot;
    /* 0x44 */ Vec3 oldRot;
    /* 0x50 */ f32 rotVel[3];
    /* 0x5C */ f32 targetRot[3];
    /* 0x68 */ s8 seekTarget[3];
    /* 0x6B */ u8 unk6B;
    /* 0x6C */ f32 damping;
    /* 0x70 */ f32 inertia;
    /* 0x74 */ ContinuousSample sample;
    /* 0x90 */ s32 stopSampleVolume;
    /* 0x94 */ s16 stopSamplePitch;
    /* 0x96 */ s16 stopSampleId;
} PlatformMotion; /* size = 0x98 */

typedef struct Fan {
    /* 0x00 */ Vec3 dir;
    /* 0x0C */ s16 on;
    /* 0x10 */ f32 strength;
    /* 0x14 */ f32 spread;
    /* 0x18 */ f32 blend;
    /* 0x1C */ f32 length;
} Fan;

typedef struct PathPoint {
    /* 0x00 */ Vec3 pos;
    /* 0x0C */ s16 time;
    /* 0x0E */ u8 unk0E[0x2];
    /* 0x10 */ s32 sampleVolume;
    /* 0x14 */ s16 samplePitch;
    /* 0x16 */ s16 sampleId;
} PathPoint; /* size = 0x18 */

typedef struct PlatformPath {
    /* 0x00 */ PathPoint *points[20];
    /* 0x50 */ s16 moving;
    /* 0x52 */ u8 unk52[0x2];
    /* 0x54 */ s8 dir;
    /* 0x55 */ u8 numPoints;
    /* 0x56 */ u8 target;
    /* 0x57 */ u8 current;
    /* 0x58 */ f32 accel;
} PlatformPath; /* size = 0x5C */

typedef struct PlatformRestartGate {
    /* 0x00 */ s16 unk00;
    /* 0x02 */ s16 restartTimer;
    /* 0x04 */ Vec3 cameraPos;
    /* 0x10 */ f32 shadowAlt;
    /* 0x14 */ s16 untouched;
    /* 0x16 */ u8 unk16;
    /* 0x17 */ u8 groundShadow;
    /* 0x18 */ f32 targetScale;
    /* 0x1C */ f32 yaw;
} PlatformRestartGate; /* size = 0x20 */

typedef struct PlatformPush {
    /* 0x00 */ s8 axis[3];
    /* 0x03 */ u8 unk03;
    /* 0x04 */ f32 mass;
} PlatformPush;

typedef struct PlatformTeleport {
    /* 0x00 */ Vec3 pos;
    /* 0x0C */ s16 flags;
    /* 0x0E */ s16 active;
    /* 0x10 */ s16 shrinkTime;
    /* 0x12 */ s16 growTime;
    /* 0x14 */ struct Platform *target;
} PlatformTeleport; /* size = 0x18 */

typedef struct PlatformCatapult {
    /* 0x00 */ Vec3 holdPos;
    /* 0x0C */ f32 launchSpeed;
    /* 0x10 */ f32 launchVelY;
    /* 0x14 */ s16 active;
    /* 0x16 */ s16 unk16;
    /* 0x18 */ s16 catchTimer;
    /* 0x1A */ s16 launchTimer;
    /* 0x1C */ f32 armRotVel;
    /* 0x20 */ s32 launchDelay;
} PlatformCatapult; /* size = 0x24 */

typedef struct PlatformScale {
    /* 0x00 */ f32 min[3];
    /* 0x0C */ f32 max[3];
    /* 0x18 */ s16 time;
    /* 0x1A */ s16 count;
    /* 0x1C */ s16 repeat;
} PlatformScale;

typedef struct PlatformCrumble {
    /* 0x00 */ s16 timer;
    /* 0x02 */ s16 shakeTime;
    /* 0x04 */ s16 fallTime;
    /* 0x06 */ u8 unk06[0x2];
    /* 0x08 */ f32 gravity;
    /* 0x0C */ Vec3 origPos;
    /* 0x18 */ ContinuousSample sample;
} PlatformCrumble;

typedef struct PlatformLink {
    /* 0x00 */ Vec3 vel;
    /* 0x0C */ s16 tag;
    /* 0x0E */ u8 unk0E[0x2];
    /* 0x10 */ f32 spinToMoveScale;
    /* 0x14 */ s16 spinToMoveTag;
    /* 0x16 */ u8 unk16[0x2];
    /* 0x18 */ f32 spinToSpinScale;
    /* 0x1C */ s16 spinToSpinTag;
    /* 0x1E */ u8 unk1E[0x2];
    /* 0x20 */ f32 moveToSpinScale;
    /* 0x24 */ s16 moveToSpinTag;
    /* 0x26 */ u8 unk26[0x2];
    /* 0x28 */ Vec3 moveToSpinDir;
    /* 0x34 */ Vec3 velOffset;
} PlatformLink; /* size = 0x40 */

typedef struct PlatformOrbit {
    /* 0x00 */ f32 speed;
    /* 0x04 */ s8 axis[3];
    /* 0x07 */ u8 unk07;
    /* 0x08 */ s16 pause;
    /* 0x0A */ s16 timer;
    /* 0x0C */ u8 pauseType;
    /* 0x0D */ u8 pauseIndex;
    /* 0x0E */ u8 unk0E[0x2];
    /* 0x10 */ f32 flip;
    /* 0x14 */ f32 angle;
    /* 0x18 */ Vec3 center;
    /* 0x24 */ f32 startAngle;
    /* 0x28 */ ContinuousSample sample;
    /* 0x44 */ s32 stopSampleVolume;
    /* 0x48 */ s16 stopSamplePitch;
    /* 0x4A */ s16 stopSampleId;
} PlatformOrbit; /* size = 0x4C */

typedef struct PlatformDestruct {
    /* 0x00 */ s32 numDebris;
    /* 0x04 */ ObjectInstance *inst;
    /* 0x08 */ s32 sampleVolume;
    /* 0x0C */ s16 samplePitch;
    /* 0x0E */ s16 sampleId;
} PlatformDestruct; /* size = 0x10 */

typedef struct PlatformConfine {
    /* 0x00 */ f32 min[3];
    /* 0x0C */ f32 size[3];
} PlatformConfine;

typedef struct Platform {
    /* 0x00 */ struct Platform *next;
    /* 0x04 */ struct Platform *prev;
    /* 0x08 */ Vec3 extent;
    /* 0x14 */ s32 flags;
    /* 0x18 */ s32 exitType;
    /* 0x1C */ f32 maxSpeed;
    /* 0x20 */ struct Platform *parent;
    /* 0x24 */ PlatformDestruct *destruct;
    /* 0x28 */ PlatformOrbit *orbit;
    /* 0x2C */ PlatformMotion *motion;
    /* 0x30 */ struct PlatformScale *scale;
    /* 0x34 */ PlatformLink *link;
    /* 0x38 */ struct PlatformPath *path;
    /* 0x3C */ struct PlatformCrumble *crumble;
    /* 0x40 */ struct PlatformPush *push;
    /* 0x44 */ struct PlatformConfine *confine;
    /* 0x48 */ Fan *fan;
    /* 0x4C */ struct PlatformTeleport *teleport;
    /* 0x50 */ struct PlatformCatapult *catapult;
    /* 0x54 */ struct PlatformRestartGate *restartGate;
    /* 0x58 */ struct PlatformBlur *blur;
    /* 0x5C */ Vec3 conveyorVel;
    /* 0x68 */ f32 pullSpeed;
    /* 0x6C */ s16 tag;
    /* 0x6E */ u16 lastUpdate;
    /* 0x70 */ f32 bounce;
    /* 0x74 */ u8 bounceAxis[3];
    /* 0x77 */ u8 unk77;
    /* 0x78 */ u16 pathStopFlags;
    /* 0x7A */ u16 spinStopFlags;
    /* 0x7C */ u16 orbitStopFlags;
    /* 0x7E */ u8 unk7E[0x2];
    /* 0x80 */ Actor actor;
    /* 0x164 */ ContinuousSample sample;
} Platform;

typedef struct PlatformList {
    /* 0x00 */ s32 count;
    /* 0x04 */ Platform head;
} PlatformList;

extern PlatformList platformList;

typedef struct RopeBridge {
    /* 0x00 */ s32 first;
    /* 0x04 */ s32 last;
    /* 0x08 */ f32 stiffness;
    /* 0x0C */ s32 dampEnds;
} RopeBridge; /* size = 0x10 */
extern RopeBridge ropeBridge[];
Platform *FindTaggedPlatform(Platform *startPlatform, s32 tag);

void UpdatePlatforms(void);
void InitPlatformList(void);
void AddPlatform(Platform *platform);
void MovePlatform(Platform *platform, Vec3 *nextPoint);
void PlatformInherit(Platform *platform, Vec3 *nextPoint);
void SpinPlatform(Platform *platform);
void OrbitPlatform(Platform *platform);
void ConfinePlatform(Platform *platform);
void RotatePlatform(Platform *platform);
void UpdatePlatform(Platform *platform);
void FloatPlatform(Platform *platform);
void InitPlatformInfo(Platform *platform, f32 offsetY);
void CalculatePlatformInfo(Platform *platform, f32 offsetY);
void RecalculatePlatformInfo(Platform *platform);
void UpdateRopeBridges(void);
void CheckRestartGate(Platform *platform);
void RestartPlayer(void);

void KillPlatform(Platform *platform, s32 killKids);

#endif
