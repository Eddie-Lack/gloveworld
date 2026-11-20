#ifndef CAMERA_H
#define CAMERA_H

#include "common.h"
#include "actor.h"

typedef struct CameraInfo {
    /* 0x000 */ f32 distance;
    /* 0x004 */ Actor actor;
    /* 0x0E8 */ f32 targetOffsetY;
    /* 0x0EC */ f32 targetY;
    /* 0x0F0 */ f32 prevTargetY;
    /* 0x0F4 */ u8 unk0F4[0x4];
    /* 0x0F8 */ f32 heightOffset;
    /* 0x0FC */ u8 unk0FC[0x4];
    /* 0x100 */ Vec3 *focus;
    /* 0x104 */ Vec3 *prevFocus;
    /* 0x108 */ Vec3 *focusVel;
    /* 0x10C */ Vec3 *prevFocusVel;
    /* 0x110 */ Vec3 prevTargetPos;
    /* 0x11C */ s32 prevDestActive;
    /* 0x120 */ Vec3 prevDestPos;
    /* 0x12C */ s32 overrideTimer;
    /* 0x130 */ s32 destActive;
    /* 0x134 */ Vec3 destPos;
    /* 0x140 */ Vec3 prevCamPos;
    /* 0x14C */ s32 orbiting;
    /* 0x150 */ f32 orbitAngle;
    /* 0x154 */ f32 orbitSpeed;
    /* 0x158 */ f32 focusAngle;
    /* 0x15C */ f32 targetDistance;
    /* 0x160 */ f32 targetHeight;
    /* 0x164 */ s32 alignBehind;
    /* 0x168 */ s32 slopeAdjust;
    /* 0x16C */ s16 turnTimer;
    /* 0x16E */ u8 unk16E[0x2];
    /* 0x170 */ f32 turnAngle;
    /* 0x174 */ f32 followSpeed;
    /* 0x178 */ s16 followTimer;
    /* 0x17A */ u8 unk17A[0x2];
    /* 0x17C */ f32 sideOffset;
    /* 0x180 */ f32 sideOffsetAim;
    /* 0x184 */ f32 sideOffsetVel;
    /* 0x188 */ f32 planeDist[4];
    /* 0x198 */ f32 squeezeDistAvg;
    /* 0x19C */ f32 squeezeDist;
    /* 0x1A0 */ s32 noCameraAction;
    /* 0x1A4 */ s32 mode;
    /* 0x1A8 */ s32 blocked;
    /* 0x1AC */ s32 angleLock;
    /* 0x1B0 */ s32 prevAngleLock;
    /* 0x1B4 */ s32 sideOffsetEnabled;
    /* 0x1B8 */ s32 targetFrozen;
    /* 0x1BC */ f32 prevRotX;
    /* 0x1C0 */ f32 prevRotY;
    /* 0x1C4 */ u8 unk1C4[0x4];
    /* 0x1C8 */ s16 fallTime;
    /* 0x1CA */ u8 unk1CA[0x2];
    /* 0x1CC */ s32 overrideFlags;
    /* 0x1D0 */ s32 exitingControlMode;
} CameraInfo;

extern Actor camera;
extern CameraInfo cameraInfo;
extern Vec3 cameraUpVect;
extern Vec3 cameraUpVectAim;

void ResetCamera(void);
void RestartCamera(void);
void SetCameraOverride(f32 camX, f32 camY, f32 camZ, Vec3 *camTargetPos, Vec3 *camTargetVel, s32 timer, s32 freezeHandAndBall);
void MoveCam(void);
s32 DoCameraCollision(void);
void PlayerSpinCamera(void);
void CamCirclePoint(Vec3 *pos);
void CalcCameraRot(void);

#endif
