#ifndef COLLECT_H
#define COLLECT_H

#include "types.h"
#include "overlay.h"
#include "actor.h"

typedef struct Garib {
    /* 0x00 */ struct Garib *next;
    /* 0x04 */ struct Garib *prev;
    /* 0x08 */ f32 velY;
    /* 0x0C */ u8 unk0C;
    /* 0x0D */ s8 active;
    /* 0x0E */ u8 type;
    /* 0x0F */ u8 mode;
    /* 0x10 */ f32 scale;
    /* 0x14 */ f32 scaleTarget;
    /* 0x18 */ GameSprite sprite;
    /* 0x50 */ f32 altitude;
    /* 0x54 */ Vec3 *normal;
    /* 0x58 */ u8 unk58[0x4];
    /* 0x5C */ u8 shadowAlpha;
    /* 0x5D */ u8 shadowTarget;
    /* 0x5E */ u8 unk5E[0x4A];
    /* 0xA8 */ f32 distSq;
    /* 0xAC */ u8 unkAC[0x4];
} Garib; /* size = 0xB0 */

typedef struct GaribList {
    /* 0x00 */ Garib head;
    /* 0xB0 */ s32 count;
} GaribList;

typedef struct GaribGroup {
    /* 0x00 */ struct GaribGroup *next;
    /* 0x04 */ struct GaribGroup *prev;
    /* 0x08 */ GaribList garibs;
    /* 0xBC */ u8 unkBC[0x4];
    /* 0xC0 */ Vec3 pos;
    /* 0xCC */ f32 radius;
    /* 0xD0 */ u8 suspended;
    /* 0xD1 */ u8 startActive;
    /* 0xD2 */ s16 tag;
    /* 0xD4 */ u8 unkD4[0x4];
} GaribGroup; /* size = 0xD8 */

typedef struct GaribGroupList {
    /* 0x00 */ GaribGroup head;
    /* 0xD8 */ s32 count;
} GaribGroupList;

typedef struct Collectable {
    /* 0x00 */ struct Collectable *next;
    /* 0x04 */ struct Collectable *prev;
    /* 0x08 */ f32 radiusSq;
    /* 0x0C */ u8 type;
    /* 0x0D */ u8 active;
    /* 0x0E */ s16 duration;
    /* 0x10 */ s16 timer;
    /* 0x12 */ s16 respawnTime;
    /* 0x14 */ Actor actor;
    /* 0xF8 */ f32 scaleTarget;
    /* 0xFC */ s16 alphaTarget;
    /* 0xFE */ s16 tag;
} Collectable; /* size = 0x100 */

typedef struct CollectableList {
    /* 0x000 */ Collectable head;
    /* 0x100 */ s32 count;
} CollectableList;

extern GaribGroupList garibGroupList;
extern CollectableList collectableList;

void InitCollectableList(void);
void InitGaribGroupList(void);
void AddGaribGroup(GaribGroup *garibGroup);
void SubGaribGroup(GaribGroup *garibGroup);
void InitGaribList(GaribList *garibList);
void AddGarib(GaribGroup *garibGroup, Garib *garib);
void UpdateGaribs(void);
void UpdatePickups(void);
void ProcessCollectables(void);
void CreateOrbiter(Actor *orbiter, s32 type, f32 scale);
void ActivateCollectable(s32 type, s32 timeVal);
void DeactivateCollectable(s32 type);
void DeactivateAllCollectables(void);
void GainExtraLife(void);
void AddToScore(s32 scoreAdd);

#endif
