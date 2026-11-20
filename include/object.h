#ifndef OBJECT_H
#define OBJECT_H

#include "common.h"
#include "actor.h"

void InitActorStructures(Actor *tempActor, s32 initFlags);
void InitActor(Actor *tempActor, char *name, f32 x, f32 y, f32 z, s32 initFlags);
void InitObjects(void);
void CalculateStaticMatrices(void);
void CreateObjects(void);
void DrawActor(Actor *ptr);
void TransformAndDrawObject(Object *obj, f32 time, s16 doKeys, s16 arg3);
void SlideTextureCoords(Actor *actor, f32 x, f32 y);
void DeformTextureCoords(Object *obj);
s32 IsActorVisible(Actor *actor);
void HandSwish(Object *obj, f32 time, s16 animStart, s16 animEnd, f32 speed);

#endif
