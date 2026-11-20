#ifndef ENEMY_H
#define ENEMY_H

#include "common.h"
#include "actor.h"

void ClearUniqueEnemyList(void);
void MakeUniqueActor(Actor *actor, s32 type);
void RemoveUniqueActor(Actor *actor, s32 type);
void LoadEnemy(s32 enemyType, f32 xpos, f32 ypos, f32 zpos, s32 tag, f32 initYRot);
void RestartEnemy(Enemy *enemy, f32 xpos, f32 ypos, f32 zpos, s32 tag, f32 initYRot, s32 enemyType);
void InitEnemy(Enemy *enemy, s32 type);
void NMEActionStartAttack(Enemy *enemy);
s32 NMEActionMoveTo(Enemy *enemy, Vec3 *dest, f32 proximity, s32 flags);
s32 NMEActionMoveTo2D(Enemy *enemy, Vec3 *dest, f32 proximity, s32 flags);
void NMEActionWait(Enemy *enemy);
s32 NMEActionFaceTo(Enemy *enemy, Vec3 *dest);
void NMEActionRandomMove(Enemy *enemy);
s32 NMEActionFacePlayer(Enemy *enemy, s32 flags);
s32 NMEActionFollowPlayer(Enemy *enemy);
s32 NMEActionFleePlayer(Enemy *enemy);
s32 NMEActionCharge(Enemy *enemy);
void NMEActionJump(Enemy *enemy);
s32 NMEActionCircle(Enemy *enemy);
void NMEActionFire1(Enemy *enemy);
void NMEActionHoldPlayer(Actor *enemy, Vec3 *pos, s32 num, s32 flags);
void UpdateEnemies(void);
void UpdateEnemy(s32 x);
void NMEProcessFroggy(Enemy *enemy);
void TurnEnemyInto(Enemy *enemy, s32 type);
void KillEnemy(Enemy *enemy);
void EnemyDropBall(void);
void EnemyDropHand(void);
f32 HeightToCentre(Actor *actor);
void NMEHitByBlastRing(Enemy *enemy);
void NMEHandleDeathSpell(Enemy *enemy);
void DoDizzyStars(Vec3 *centre);

#endif
