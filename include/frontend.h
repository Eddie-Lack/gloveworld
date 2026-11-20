#ifndef FRONTEND_H
#define FRONTEND_H

#include "common.h"
#include "actor.h"

void FadeScreenOutN(void);
void MoveGloveOL(void);
void FadeScreenInN(void);
void RunFadeN(void);
void KillTitle(void);
void initFGObject(char *NameString, f32 XPos, f32 YPos, f32 ZPos, f32 XRot, f32 YRot, f32 ZRot, f32 XSize, f32 YSize, f32 ZSize, s32 TL);
void initSpinFGObjects(s32 numFgObjects);
void initSpinOutFGObjects(s32 numFgObjects);
void spinOutFGObjects(s32 numFgObjects);
void spinInFGObjects(s32 numFgObjects);
void FreeFGObjects(void);
void DrawScreen(s32 *cond);
void ActContinueGame(void);
void CheatStartGame(s8 mode);
s32 addAnim(s32 *anim);
void DoMenu(void);
s32 IsSaveSlotEmpty(s32 slot);

#endif
