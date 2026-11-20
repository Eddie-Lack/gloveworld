#ifndef ACTOR_H
#define ACTOR_H

#include "common.h"
#include "sound.h"

typedef struct ActorState {
    /* 0x00 */ s8 hits;
    /* 0x01 */ u8 damage;
    /* 0x02 */ s8 index;
    /* 0x03 */ s8 teleportTimer;
    /* 0x04 */ s16 mode;
    /* 0x06 */ s16 prevMode;
    /* 0x08 */ u8 flying;
    /* 0x09 */ u8 waterState;
    /* 0x0A */ u8 enteredWater;
    /* 0x0B */ u8 onHurtPlatform;
    /* 0x0C */ struct PlatformTeleport *teleport;
    /* 0x10 */ f32 squashXZ;
    /* 0x14 */ f32 squashY;
    /* 0x18 */ f32 squash;
    /* 0x1C */ s16 squashTimer;
    /* 0x1E */ u8 unk1E[0x2];
    /* 0x20 */ Quat squashQ;
    /* 0x30 */ s16 hitFadeTimer;
    /* 0x32 */ u8 unk32[0x2];
} ActorState; /* size = 0x34 */

typedef struct Plane {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ f32 d;
} Plane;

typedef struct CollSphere {
    /* 0x00 */ Vec3 pos;
    /* 0x0C */ Vec3 prevPos;
    /* 0x18 */ f32 radius;
    /* 0x1C */ f32 targetRadius;
    /* 0x20 */ u8 platTag[0x2];
    /* 0x22 */ s16 flags;
} CollSphere; /* size = 0x24 */

typedef struct ActorObject {
    /* 0x000 */ struct Plane *planes[4];
    /* 0x010 */ Vec3 contactPos[4];
    /* 0x040 */ u8 contactSphere[4];
    /* 0x044 */ s32 contactCount[4];
    /* 0x054 */ u8 resting[4];
    /* 0x058 */ CollSphere spheres[8];
    /* 0x178 */ struct Platform *platform;
    /* 0x17C */ struct Platform *prevPlatform;
    /* 0x180 */ struct Platform *contactPlatform;
    /* 0x184 */ struct Platform *prevContactPlatform;
    /* 0x188 */ u8 onGround;
    /* 0x189 */ u8 wasOnGround;
    /* 0x18A */ u8 contactFlags;
    /* 0x18B */ u8 numSpheres;
} ActorObject;

typedef struct AnimInfo {
    /* 0x00 */ s16 start;
    /* 0x02 */ s16 end;
    /* 0x04 */ f32 speed;
    /* 0x08 */ s32 unk08;
} AnimInfo; /* size = 0xC */

typedef struct AnimState {
    /* 0x00 */ s16 numAnims;
    /* 0x02 */ s16 current;
    /* 0x04 */ s32 finishedAnim;
    /* 0x08 */ s32 loop;
    /* 0x0C */ f32 speed;
    /* 0x10 */ s16 queue[5];
    /* 0x1A */ u8 unk1A[0x2];
    /* 0x1C */ s32 queueLoop[5];
    /* 0x30 */ f32 queueSpeed[5];
    /* 0x44 */ s16 queueLen;
    /* 0x46 */ u8 unk46[0x2];
    /* 0x48 */ struct AnimInfo *anims;
    /* 0x4C */ f32 time;
} AnimState;

typedef struct FaceIndices {
    /* 0x00 */ s16 v[3];
} FaceIndices; /* size = 0x6 */

typedef struct MeshColour {
    /* 0x00 */ u8 r;
    /* 0x01 */ u8 g;
    /* 0x02 */ u8 b;
    /* 0x03 */ u8 a;
} MeshColour; /* size = 0x4 */

typedef struct Mesh {
    /* 0x00 */ s16 numFaces;
    /* 0x02 */ s16 numVerts;
    /* 0x04 */ Vec3 *verts;
    /* 0x08 */ FaceIndices *faces;
    /* 0x0C */ void *faceColours;
    /* 0x10 */ u16 (*uv)[2];
    /* 0x14 */ u16 (*origUV)[2];
    /* 0x18 */ u8 *colours;
    /* 0x1C */ u8 *faceFlags;
    /* 0x20 */ struct Texture **textures;
} Mesh;

typedef struct ObjectSprite {
    /* 0x00 */ struct Texture *tex;
    /* 0x04 */ struct GameSprite *sprite;
    /* 0x08 */ s16 x;
    /* 0x0A */ s16 y;
    /* 0x0C */ s16 z;
    /* 0x0E */ s16 width;
    /* 0x10 */ s16 height;
    /* 0x12 */ u8 unk12[0x4];
    /* 0x16 */ u16 flags;
} ObjectSprite; /* size = 0x18 */

typedef struct PosKey {
    /* 0x00 */ Vec3 pos;
    /* 0x0C */ f32 unk0C;
    /* 0x10 */ s32 time;
} PosKey; /* size = 0x14 */

typedef struct RotKey {
    /* 0x00 */ Quat q;
    /* 0x10 */ s32 time;
} RotKey; /* size = 0x14 */

typedef struct Object {
    /* 0x00 */ u32 crc;
    /* 0x04 */ char name[0x8];
    /* 0x0C */ u8 initialised;
    /* 0x0D */ u8 alpha[0x1];
    /* 0x0E */ s16 numScaleKeys;
    /* 0x10 */ s16 numPosKeys;
    /* 0x12 */ s16 numRotKeys;
    /* 0x14 */ Mesh *mesh;
    /* 0x18 */ u8 *displayList;
    /* 0x1C */ struct PosKey *scaleKeys;
    /* 0x20 */ struct PosKey *posKeys;
    /* 0x24 */ struct RotKey *rotKeys;
    /* 0x28 */ s32 numSprites;
    /* 0x2C */ ObjectSprite *sprites;
    /* 0x30 */ u8 unk30[0x2];
    /* 0x32 */ u16 flags;
    /* 0x34 */ struct Object *child;
    /* 0x38 */ struct Object *sibling;
    /* 0x3C */ struct CollSphere *sphere;
} Object; /* size = 0x40 */

typedef struct ObjectInstance {
    /* 0x00 */ u8 unk00[0xC];
    /* 0x0C */ Object *obj;
    /* 0x10 */ u8 unk10[0x8];
    /* 0x18 */ AnimState *anim;
} ObjectInstance;

typedef struct CollFace {
    /* 0x00 */ Plane plane;
    /* 0x10 */ Vec3 edgeStart[3];
    /* 0x34 */ Vec3 edgeDir[3];
    /* 0x58 */ f32 edgeLen[3];
    /* 0x64 */ f32 proj[4];
    /* 0x74 */ u8 terrain;
    /* 0x75 */ u8 axis[2];
    /* 0x77 */ u8 unk77;
} CollFace; /* size = 0x78 */

typedef struct Terrain {
    /* 0x00 */ f32 slip;
    /* 0x04 */ f32 bounce;
    /* 0x08 */ f32 slideSpeedScale;
    /* 0x0C */ f32 slideSlopeScale;
    /* 0x10 */ f32 gravityScale;
    /* 0x14 */ f32 shatterSpeed;
    /* 0x18 */ f32 handSlideScale;
} Terrain; /* size = 0x1C */

extern Terrain terrain[];

typedef struct ActorPhysics {
    /* 0x00 */ f32 mass;
    /* 0x04 */ f32 accel;
    /* 0x08 */ f32 maxSpeed;
    /* 0x0C */ f32 moveSpeed;
    /* 0x10 */ f32 minSpeed;
    /* 0x14 */ f32 slideLimit;
    /* 0x18 */ f32 friction;
    /* 0x1C */ f32 rotDrag;
    /* 0x20 */ f32 bounce;
    /* 0x24 */ s8 rotAim[3];
    /* 0x27 */ u8 unk27;
    /* 0x28 */ f32 turnSpeed;
    /* 0x2C */ f32 bottomDiv;
    /* 0x30 */ f32 maxSquash;
    /* 0x34 */ f32 squashScale;
    /* 0x38 */ f32 squashMin;
    /* 0x3C */ f32 squashPeriod;
    /* 0x40 */ f32 squashDecay;
    /* 0x44 */ s32 flags;
    /* 0x48 */ f32 radius;
    /* 0x4C */ f32 radiusRate;
} ActorPhysics; /* size = 0x50 */

typedef struct ShadowVtx {
    Vtx v[4];
} ShadowVtx;

typedef struct ActorShadow {
    /* 0x00 */ f32 altitude;
    /* 0x04 */ Vec3 *floor;
    /* 0x08 */ f32 shadowSize;
    /* 0x0C */ u8 shadowAlpha;
    /* 0x0D */ u8 shadowTarget;
    /* 0x0E */ u8 shadowStep;
    /* 0x0F */ u8 unk0F;
    /* 0x10 */ ShadowVtx shadow;
    /* 0x50 */ u8 sphere;
    /* 0x51 */ u8 unk51[0x7];
} ActorShadow; /* size = 0x58 */

typedef struct Actor {
    /* 0x00 */ struct Actor *next;
    /* 0x04 */ struct Actor *prev;
    /* 0x08 */ struct Actor *collNext;
    /* 0x0C */ struct Actor *collPrev;
    /* 0x10 */ struct Actor *distNext;
    /* 0x14 */ struct Actor *distPrev;
    /* 0x18 */ f32 distSq;
    /* 0x1C */ Mtx *staticMtx;
    /* 0x20 */ s16 type;
    /* 0x22 */ s8 active;
    /* 0x23 */ u8 sparkleRate;
    /* 0x24 */ s16 lifeTimer;
    /* 0x26 */ s16 alpha;
    /* 0x28 */ s32 flags;
    /* 0x2C */ struct Actor *link;
    /* 0x30 */ f32 radius;
    /* 0x34 */ Vec3 pos;
    /* 0x40 */ Vec3 prevPos;
    /* 0x4C */ Vec3 platPrevPos;
    /* 0x58 */ Vec3 vel;
    /* 0x64 */ Vec3 prevVel;
    /* 0x70 */ f32 friction;
    /* 0x74 */ f32 speed;
    /* 0x78 */ Vec3 rot;
    /* 0x84 */ Vec3 rotAim;
    /* 0x90 */ Vec3 rotVel;
    /* 0x9C */ Quat rotQuat;
    /* 0xAC */ Vec3 spinAxis;
    /* 0xB8 */ f32 spinSpeed;
    /* 0xBC */ Vec3 scale;
    /* 0xC8 */ ObjectInstance *inst;
    /* 0xCC */ struct ActorShadow *shadow;
    /* 0xD0 */ ActorPhysics *phys;
    /* 0xD4 */ ActorState *state;
    /* 0xD8 */ ActorObject *obj;
    /* 0xDC */ AnimState *anim;
    /* 0xE0 */ CollFace *faces;
} Actor; /* size = 0xE4 */

typedef struct ActorList {
    /* 0x00 */ Actor head;
    /* 0xE4 */ s32 count;
} ActorList;

typedef struct EnemyBox {
    /* 0x00 */ Vec3 min;
    /* 0x0C */ Vec3 size;
} EnemyBox;

typedef union EnemyParams {
    EnemyBox box;
    s32 flags;
} EnemyParams;

typedef union NMEArg {
    s32 i;
    f32 f;
} NMEArg;

typedef struct NMEInstruction {
    /* 0x00 */ s16 type;
    /* 0x02 */ s16 time;
    /* 0x04 */ EnemyParams params;
    /* 0x1C */ NMEArg arg;
    /* 0x20 */ u8 unk20[0x8];
    /* 0x28 */ s32 flags;
    /* 0x2C */ s16 condition;
    /* 0x2E */ u8 unk2E[0x2];
    /* 0x30 */ f32 condMin;
    /* 0x34 */ u8 unk34[0x8];
    /* 0x3C */ f32 condMax;
    /* 0x40 */ u8 unk40[0x8];
} NMEInstruction; /* size = 0x48 */

typedef struct NMEBrain {
    /* 0x000 */ NMEInstruction instr;
    /* 0x048 */ s32 inConditional;
    /* 0x04C */ NMEInstruction *normalList[30];
    /* 0x0C4 */ NMEInstruction *conditionalList[10];
    /* 0x0EC */ NMEInstruction *attackList[30];
    /* 0x164 */ s16 cooldownTimer;
    /* 0x166 */ s16 normalCount;
    /* 0x168 */ s16 conditionalCount;
    /* 0x16A */ s16 attackCount;
    /* 0x16C */ NMEInstruction **curList;
    /* 0x170 */ u8 instrIndex;
    /* 0x171 */ u8 returnIndex;
    /* 0x172 */ u8 unk172;
    /* 0x173 */ u8 loopCount;
    /* 0x174 */ s32 attackTime;
    /* 0x178 */ s32 instrTime;
    /* 0x17C */ Vec3 target;
    /* 0x188 */ s32 scratch;
    /* 0x18C */ f32 scratchF;
    /* 0x190 */ f32 tiltAngle;
    /* 0x194 */ s16 delayTimer;
    /* 0x196 */ s16 instrTimer;
    /* 0x198 */ s32 flags;
    /* 0x19C */ s16 tag;
    /* 0x19E */ s16 bobPeriod;
    /* 0x1A0 */ f32 bobAmount;
    /* 0x1A4 */ u8 fireState[0x4];
    /* 0x1A8 */ f32 bank;
    /* 0x1AC */ Box area;
    /* 0x1C4 */ f32 maxAlt;
    /* 0x1C8 */ f32 minAlt;
    /* 0x1CC */ f32 startRot;
    /* 0x1D0 */ s16 spellTimer[3];
    /* 0x1D6 */ u8 unk1D6[0x2];
    /* 0x1D8 */ Vec3 startPos;
} NMEBrain; /* size = 0x1E4 */

typedef struct Enemy {
    /* 0x000 */ Actor actor;
    /* 0x0E4 */ NMEBrain brain;
    /* 0x2C8 */ ContinuousSample samples[2];
} Enemy; /* size = 0x300 */

typedef struct EnemyStat {
    /* 0x00 */ s32 flags;
    /* 0x04 */ f32 scale;
    /* 0x08 */ s32 score;
} EnemyStat; /* size = 0xC */

extern ActorList actorList;
extern ActorList collisionList;
extern ActorList distList;
extern Actor hand;
extern Actor ball;
extern Enemy enemies[];
extern ActorPhysics defaultBehaviour[];
extern EnemyStat enemyStats[];
extern s32 numberOfEnemies;

void ActorMaxOutVel(Actor *actor, f32 maxSpeed);
void AccelerateActor2D(Actor *actor, f32 speed, f32 maxSpeed);
s32 IsActorFacingPoint(Actor *actor, Vec3 *target, f32 range);

void GetQuaternionFromXYZRot(Quat *quat, Vec3 *rot);
s32 IsHandHoldingBall(void);
void AddObjectsSpritesToSpriteList(Object *obj);
void InitActorList(void);
void AddActorToDistList(Actor *actor);
void AddActor(Actor *actor);
void CalcMaxActorRadius(Actor *actor);
void RemoveObjectSprites(Object *obj, s32 freeit);
void SubActor(Actor *actor);
void AddConveyorSpeedToVel(Vec3 *vel, Vec3 *add);
void UpdateActor(Actor *actor);
s32 UpdateActorCollisionAndGravity(Actor *actor);
void UpdateActorPosAndRot(Actor *actor);
s32 UpdateActorTeleport(Actor *actor);
void ActorHandleRotaim(Actor *actor, s32 i);
void AccelerateActor(Actor *actor, f32 speed, f32 maxSpeed);
void ActorMaxOutVel2D(Actor *actor, f32 maxSpeed);
void AccelerateActorFromQuaternion(Actor *actor, f32 speed, f32 maxSpeed, Quat *quat, s32 is3D);
void AccelerateActorFromActor(Actor *sourceActor, Actor *targetActor, f32 speed, f32 maxSpeed);
s32 IsActorFacingActor(Actor *actor, Actor *target, f32 range);
void UpdateActorPositionDependencies(void);
void FreeOFTC(Object *obj);
void GiveActorRandomQRotVel(Actor *actor);

#endif
