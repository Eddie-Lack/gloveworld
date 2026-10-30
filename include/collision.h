#ifndef COLLISION_H
#define COLLISION_H

#include "common.h"
#include "actor.h"

void CollideActorInsideSphere(Actor *actor, Vec3 *centre, f32 radius);
void CheckActorCollisions(Actor *actor);
f32 InitPlanes(Mesh *mesh, Vec3 *pos, CollFace *planes, Vec3 *scale, Quat *q, s32 collision);
void CheckForInnerEdges(u32 numFaces, CollFace *planes, s32 index);
void CheckCollisions(Actor *actor);
f32 NearEdges(Vec3 *point, CollFace *plane, f32 r, Vec3 *nearestPoint);
s32 CollideActorWithPlane(Actor *actor, CollFace *plane, Platform *platform);
f32 AltitudeCheckPlane(Vec3 *pos, CollFace *plane, Vec3 *point, f32 minHeight);
void CalculatePlaneInfoWithEdgeLens(CollFace *plane);
void CalculatePlaneInfo(CollFace *plane);
void CalcCollisionSpheres(Actor *actor);

#endif
