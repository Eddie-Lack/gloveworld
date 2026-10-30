#ifndef PARSE_H
#define PARSE_H

#include "common.h"
#include "actor.h"

void ParseLandscapeFile(s32 num);
void SetActorTerrain(Actor *actor, u8 terrainType);
void PlatformAddPoint(Platform *platform, f32 x, f32 y, f32 z, s32 counter);
void SetPlatformPos(Platform *platform, Vec3 *pos, f32 offsetY);
void CopyFromData(void *dst, u8 **src, s32 size);
s32 ParseTokenCameo(s16 token, u8 **data);
s32 ParseTokenGeneric(s16 token, u8 **data);
s32 ParseTokenPuzzle(s16 token, u8 **data);
s32 ParseTokenCollectable(s16 token, u8 **data);
s32 ParseTokenPlatformInteraction(s16 token, u8 **data, Platform *platform);
s32 ParseTokenBridge(s16 token, u8 **data);
s32 ParseTokenPlatformFeatures(s16 token, u8 **data, Platform *platform);
s32 ParseTokenPlatformOrbit(s16 token, u8 **data, Platform *platform);
s32 ParseTokenPlatformSpin(s16 token, u8 **data, Platform *platform);
s32 ParseTokenPlatformPath(s16 token, u8 **data, Platform *platform);
s32 ParseTokenPlatformGeneric(s16 token, u8 **data, Platform *platform);

#endif
