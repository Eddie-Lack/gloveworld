#include "common.h"
#include "ident.h"
#include "actor.h"
#include "game.h"
#include "sound.h"
#include "bullet.h"
#include "hand.h"
#include "maths.h"
#include "debris.h"
#include "macros.h"
#include "ball.h"
#include "quake.h"
#include "footprint.h"
#include "jalloc.h"
#include "camera.h"
#include "controller.h"
#include "cheat.h"
#include "cameo.h"
#include "banks.h"
#include "collect.h"
#include "nmeprocess.h"
#include "anim.h"
#include "enemy.h"
#include "nmeprocess2.h"
#include "object.h"
#include "parse.h"

extern s32 frameCount;

extern f32 spbval;
extern s32 levelCamHeight;
extern f32 handInertia;
extern s32 numBubbles;
extern f32 swishSpeed;
void CreateWake(Vec3 *pos, Vec3 *vel, Platform *platform, f32 level, f32 size, Vec3 *normal);
extern s32 numSmallBubbles;
u32 UpdateCRC(char *ptr);
extern s32 hubChickenNum;
extern Object *hubChickenTempObj;
extern s32 downAgain;
extern s16 hcCluck[];
extern u8 ActiveController;
struct Bullet *FireBulletFromActor(Actor *actor, s32 type, s32 flash);
void HurtHand(s32 type, s32 damage);
extern f32 cannonballRot[];
extern f32 cannonballMinSpeed;
extern Quat vertQ;

void NMEGetNewInstruction(Enemy *enemy, s16 instructionNum, s32 overrideConditions);
void AnimateActor(Actor *actor, s32 animNum, s32 loop, s32 queue, f32 speed);
void SquashActor(Actor *actor, Vec3 *dir, f32 power, f32 maxSquash);
extern Texture *blastTex;
extern f32 gravity;
extern f32 sb3x;
extern Vec3 zero;
extern f32 sb3y;
void FlashScreen(u8 r, u8 g, u8 b, s16 fadeVal, s16 fadeStep);

extern s32 numInstThisFrame;
void DropHand(s16 timer);

void NMEInitInstruction(u8 **data, NMEInstruction **instruction) {
    char string[256];
    s16 token;
    s16 time;

    CopyFromData(&token, data, 2);
    CopyFromData(&time, data, 2);
    *instruction = JallocAlloc(sizeof(NMEInstruction), 1, "Instruct");
    (*instruction)->time = time;
    (*instruction)->type = token;
    if (token == 0 || token == 22 || token == TOKEN_HAND_START || token == TOKEN_BALL_START || token == TOKEN_CONDITION_AT_POINT || token == 13) {
        CopyFromData(&(*instruction)->params.box.min, data, 12);
    } else if (token == TOKEN_CAMERA_START) {
        CopyFromData(&(*instruction)->params.box.min, data, 12);
        CopyFromData(&(*instruction)->params.box.size, data, 12);
    } else if (token == TOKEN_PUZZLE || token == TOKEN_PUZZLE_AND || token == TOKEN_PUZZLE_OR || token == TOKEN_PUZZLE_NUMTIMES || token == 12 || token == 15 || token == 14 ||
               token == 16 || token == 18 || token == 19 || token == 20 || token == 21 || token == 24) {
        CopyFromData(&(*instruction)->params.flags, data, 4);
    } else if (token == TOKEN_PUZZLE_ANY || token == 17) {
        CopyFromData(&(*instruction)->params.flags, data, 4);
    } else if (token == 10 || token == 11 || token == 23) {
        CopyFromData(&(*instruction)->params.box.min, data, 12);
    } else {
        goto error; // the ROM places the error block after the shared copies
    }
    CopyFromData(&(*instruction)->arg, data, 4);
    CopyFromData(&(*instruction)->condMin, data, 4);
    CopyFromData(&(*instruction)->condMax, data, 4);
    CopyFromData(&(*instruction)->flags, data, 4);
    CopyFromData(&(*instruction)->condition, data, 2);
    return;
error:
    PRINTF("NMEInitInstruction:error, unrecognised instruction %s\n", string);
}

void NMEAddNormalInstruction(u8 **data) {
    NMEBrain *script = &enemies[numberOfEnemies - 1].brain;

    NMEInitInstruction(data, &script->normalList[script->normalCount]);
    if (++script->normalCount > 30) {
        PRINTF("Error: too many instructions\n");
        Crash("NMEADDNORMAL");
    }
}

void NMEAddConditionalInstruction(u8 **data) {
    NMEBrain *script = &enemies[numberOfEnemies - 1].brain;

    NMEInitInstruction(data, &script->conditionalList[script->conditionalCount]);
    if (++script->conditionalCount > 10) {
        PRINTF("Error: too many instructions\n");
        Crash("NMEADDCONDITIONAL");
    }
}

void NMEAddAttackInstruction(u8 **data) {
    NMEBrain *script = &enemies[numberOfEnemies - 1].brain;

    NMEInitInstruction(data, &script->attackList[script->attackCount]);
    if (++script->attackCount > 30) {
        PRINTF("Error: too many instructions\n");
        Crash("NMEADDATTACK");
    }
}

inline void NMEGetNewInstruction(Enemy *enemy, s16 instructionNum, s32 overrideConditions) {
    NMEBrain *br = &enemy->brain;

    if (br->instr.type == NME_CIRCLE) {
        enemy->actor.rotAim.x = 0.0f;
    }
    if (!overrideConditions) {
        while (!NMEConditionsMet(enemy, br->curList[instructionNum])) {
            instructionNum++;
        }
    }
    br->instrIndex = instructionNum;
    br->instr = *br->curList[instructionNum];
    br->instrTime = 0;
    br->instrTimer = br->instr.time;
}

void NMEProcessInstruction(Enemy *enemy) {
    Vec3 tempVect;
    u8 cur = enemy->brain.instrIndex;
    NMEBrain *script = &enemy->brain;
    NMEInstruction *ins = &script->instr;
    s32 needNewInstruction = FALSE;
    u8 newInstructionNum;

    if (script->attackTime) {
        script->attackTime++;
    }
    if (script->bobPeriod) {
        enemy->actor.vel.y += SineWave(script->bobPeriod, 0.0f, frameCount) * script->bobAmount;
    }
    if (ins->flags & 0x100000) {
        SUB_VECTORS(&tempVect, &hand.pos, &enemy->actor.pos);
        enemy->actor.rotAim.y = Aabs(atan2(tempVect.x, tempVect.z));
    }
    if (ins->flags & 0x200000) {
        SUB_VECTORS(&tempVect, &ball.pos, &enemy->actor.pos);
        enemy->actor.rotAim.y = Aabs(atan2(tempVect.x, tempVect.z));
    }
    if (ins->flags & 0x400000) {
        if (DistanceBetweenPointsSquared(&hand.pos, &enemy->actor.pos)
            < DistanceBetweenPointsSquared(&ball.pos, &enemy->actor.pos)) {
            SUB_VECTORS(&tempVect, &hand.pos, &enemy->actor.pos);
        } else {
            tempVect.x = ball.pos.x - enemy->actor.pos.x;
            tempVect.y = ball.pos.y - enemy->actor.pos.y;
            tempVect.z = ball.pos.z - enemy->actor.pos.z;
        }
        enemy->actor.rotAim.y = Aabs(atan2(tempVect.x, tempVect.z));
    }
    switch (ins->type) {
        case NME_MOVE_VECTOR:
            AddOneScaledVector(&tempVect, &enemy->actor.pos, &ins->params.box.min, 10.0f);
            if (NMEActionFaceTo(enemy, &tempVect) == 1 || (ins->flags & 0x1000)) {
                NMEActionMoveTo(enemy, &tempVect, 0.0f, ins->flags);
            }
            break;
        case NME_MOVETO:
            if (NMEActionMoveTo(enemy, &ins->params.box.min, ins->arg.f, ins->flags) == 1) {
                needNewInstruction = TRUE;
                newInstructionNum = script->instrIndex + 1;
                if (!(ins->flags & 0x4000)) {
                    AnimateActor(&enemy->actor, 14, 0, 0, 0.0f);
                }
            }
            break;
        case NME_MOVETO2D:
            if (script->instrTime == 0) {
                if (ins->flags & 0x40000) {
                    AnimateActor(&enemy->actor, 13, 0, 0, 0.0f);
                    AnimateActor(&enemy->actor, 9, 1, 1, 0.0f);
                } else if (!(ins->flags & 0x4000)) {
                    AnimateActor(&enemy->actor, 13, 0, 0, 0.0f);
                    AnimateActor(&enemy->actor, 15, 1, 1, 0.0f);
                }
            }
            if (!(ins->flags & 0x80000) || enemy->actor.anim->current != 13) {
                if (NMEActionMoveTo2D(enemy, &ins->params.box.min, ins->arg.f, ins->flags) == 1) {
                    needNewInstruction = TRUE;
                    newInstructionNum = script->instrIndex + 1;
                }
            }
            break;
        case NME_FACETO:
            if (NMEActionFaceTo(enemy, &ins->params.box.min) == 1) {
                needNewInstruction = TRUE;
                newInstructionNum = script->instrIndex + 1;
            }
            break;
        case NME_RANDOMMOVE:
            NMEActionRandomMove(enemy);
            break;
        case NME_WAIT:
            NMEActionWait(enemy);
            if (script->instrTimer < 0 && enemy->actor.anim->finishedAnim
                && (enemy->actor.anim->finishedAnim == 6 || !(ins->params.flags & 4))) {
                needNewInstruction = TRUE;
                newInstructionNum = script->instrIndex + 1;
            }
            break;
        case NME_DIE:
            KillEnemy(enemy);
            return;
        case NME_FIRE1:
            NMEActionFire1(enemy);
            if (script->instrTimer == -1 && enemy->actor.anim->finishedAnim) {
                needNewInstruction = TRUE;
                newInstructionNum = script->instrIndex + 1;
            }
            break;
        case NME_SPECIAL:
            if (script->instrTime == 0) {
                if (script->instr.flags & 4) {
                    FlushQueue(&enemy->actor);
                }
                if (script->instr.params.flags >= 0) {
                    if (script->instr.flags & 2) {
                        enemy->actor.anim->loop = 0;
                    }
                    AnimateActor(&enemy->actor, ins->params.flags + 10, script->instr.flags & 1, ((u32)script->instr.flags >> 1) & 1, 0.0f);
                } else {
                    if (script->instr.flags & 2) {
                        enemy->actor.anim->loop = 0;
                    }
                    AnimateActor(&enemy->actor, ins->arg.i, script->instr.flags & 1, ((u32)script->instr.flags >> 1) & 1, 0.0f);
                }
            }
            if (script->instrTimer < 0 && enemy->actor.anim->finishedAnim
                && enemy->actor.anim->finishedAnim == enemy->actor.anim->current + 1) {
                if (ins->params.flags < 0) {
                    if (enemy->actor.anim->current == ins->arg.i) {
                        needNewInstruction = TRUE;
                        newInstructionNum = script->instrIndex + 1;
                    }
                } else if (enemy->actor.anim->current == ins->params.flags + 10) {
                    needNewInstruction = TRUE;
                    newInstructionNum = script->instrIndex + 1;
                }
            }
            break;
        case NME_FIRE2:
            NMEActionFire1(enemy);
            needNewInstruction = TRUE;
            newInstructionNum = script->instrIndex + 1;
            break;
        case NME_FACEPLAYER:
            if (NMEActionFacePlayer(enemy, script->instr.flags) == 1 && script->instrTimer < 0) {
                needNewInstruction = TRUE;
                newInstructionNum = script->instrIndex + 1;
            }
            break;
        case NME_FOLLOWPLAYER:
            if (NMEActionFollowPlayer(enemy) == 1) {
                needNewInstruction = TRUE;
                newInstructionNum = script->instrIndex + 1;
            }
            break;
        case NME_HOLDPLAYER:
            NMEActionHoldPlayer(&enemy->actor, &ins->params.box.min, ins->arg.i, ins->flags);
            needNewInstruction = TRUE;
            newInstructionNum = script->instrIndex + 1;
            break;
        case NME_DROPPLAYER:
            if (script->instrTime == 0) {
                if (ins->flags & 0x80) {
                    if (ball.link == &enemy->actor) {
                        DropBall(5);
                    }
                    RotateVector2D(&tempVect, &ins->params.box.min, enemy->actor.rot.y);
                    SCALE_VECTOR_2D(&tempVect, 0.5);
                    ball.vel = tempVect;
                }
                if (ins->flags & 0x40) {
                    if (hand.link == &enemy->actor) {
                        DropHand(5);
                    }
                    RotateVector2D(&tempVect, &ins->params.box.min, enemy->actor.rot.y);
                    hand.vel = tempVect;
                    hand.flags |= 0x01002000;
                    hand.rotAim.x = 1.5707964f;
                }
            }
            if (script->instrTimer != -1 || enemy->actor.anim->finishedAnim) {
                needNewInstruction = TRUE;
                newInstructionNum = script->instrIndex + 1;
            }
            break;
        case NME_FLEEPLAYER:
            NMEActionFleePlayer(enemy);
            break;
        case NME_CIRCLE:
            NMEActionCircle(enemy);
            break;
        case NME_JUMP:
            NMEActionJump(enemy);
            if (script->instrTime > 10) {
                if (enemy->actor.anim->current == 8 && enemy->actor.anim->finishedAnim) {
                    needNewInstruction = TRUE;
                    newInstructionNum = script->instrIndex + 1;
                } else if (enemy->actor.state->waterState || enemy->actor.obj->onGround) {
                    if (!(ins->arg.i & 0x4000)) {
                        AnimateActor(&enemy->actor, 8, 0, 0, 0.0f);
                    } else {
                        needNewInstruction = TRUE;
                        newInstructionNum = script->instrIndex + 1;
                    }
                }
            }
            break;
        case NME_ATTACK:
            NMEActionStartAttack(enemy);
            needNewInstruction = TRUE;
            newInstructionNum = 0;
            break;
        case NME_ENDATTACK:
            if (script->instrTime == 0 && !(ins->flags & 0x4000)) {
                AnimateActor(&enemy->actor, 3, 0, (u8)ins->arg.i, 0.0f);
            }
            if (script->instrTimer == 0 || (script->instrTimer < 0 && enemy->actor.anim->finishedAnim)) {
                NMEEndAttack(enemy);
                newInstructionNum = script->instrIndex;
                needNewInstruction = TRUE;
            }
            break;
        case NME_DONTATTACK:
            needNewInstruction = TRUE;
            newInstructionNum = script->instrIndex + 1;
            script->cooldownTimer = ins->params.flags;
            break;
        case NME_CHARGE:
            NMEActionCharge(enemy);
            break;
        case NME_BRANCH:
            newInstructionNum = ins->params.flags;
            needNewInstruction = TRUE;
            break;
        case NME_LOOP:
            if (script->loopCount) {
                script->loopCount--;
                needNewInstruction = TRUE;
                newInstructionNum = ins->params.flags;
            } else {
                needNewInstruction = TRUE;
                newInstructionNum = cur + 1;
            }
            break;
        case NME_LOADCOUNTER:
            needNewInstruction = TRUE;
            newInstructionNum = cur + 1;
            script->loopCount = ins->params.flags;
            break;
    }
    script->instrTime++;
    if (script->cooldownTimer > 0) {
        script->cooldownTimer--;
    } else if (script->cooldownTimer < 0) {
        script->cooldownTimer++;
    }
    if (script->instrTimer > 0 && !needNewInstruction) {
        if (--script->instrTimer == 0) {
            if (!(ins->flags & 0x8000) || !NMEConditionsMet(enemy, ins)) {
                needNewInstruction = TRUE;
                newInstructionNum = cur + 1;
                if (ins->type == NME_ENDATTACK) {
                    NMEEndAttack(enemy);
                    newInstructionNum = script->instrIndex;
                }
            } else {
                enemy->brain.instrTimer = script->instr.time;
            }
        }
    }
    if (needNewInstruction == TRUE) {
        if (script->inConditional == TRUE) {
            script->curList = script->normalList;
            script->instrIndex = script->returnIndex;
            script->inConditional = FALSE;
            if (ins->type != NME_BRANCH) {
                newInstructionNum = script->instrIndex;
            }
        }
        NMEGetNewInstruction(enemy, newInstructionNum, FALSE);
        if (numInstThisFrame < 4) {
            numInstThisFrame++;
            NMEProcessInstruction(enemy);
        }
    }
}

void NMEEndAttack(Enemy *enemy) {
    enemy->brain.attackTime = 0;
    enemy->brain.curList = enemy->brain.normalList;
    enemy->brain.cooldownTimer = enemy->brain.instr.params.flags;
    enemy->brain.instrIndex = enemy->brain.returnIndex;
}

s32 NMEConditionsMet(Enemy *enemy, NMEInstruction *instruction) {
    // Converting into a shared local (cases 9 and 22) gives the target's $f4 temporary.
    f32 temp;
    f32 ballDist;
    f32 handDist;
    f32 ball2D;
    f32 hand2D;
    f32 nearDist;
    f32 near2D;
    Actor *closestActor;
    Actor *actor = &enemy->actor;

    if (enemy->actor.type == NME_BRANCH && instruction->type == NME_ATTACK && (ballControl.type == BALL_MODE_CRYSTAL || ballControl.cancerTimer)) {
        return FALSE;
    }
    if (instruction->type == NME_HOLDPLAYER && ballControl.enemy == &enemy->actor && (instruction->flags & 0x80)) {
        return FALSE;
    }
    if (instruction->type == NME_HOLDPLAYER && handControl.attachedTo == &enemy->actor && (instruction->flags & 0x40)) {
        return FALSE;
    }
    if (instruction->type == NME_ATTACK && enemy->brain.cooldownTimer > 0) {
        return FALSE;
    }
    ballDist = DistanceBetweenPoints(&ball.pos, &actor->pos);
    handDist = DistanceBetweenPoints(&hand.pos, &actor->pos);
    closestActor = (ballDist < handDist) ? &ball : &hand;
    nearDist = MIN(ballDist, handDist);
    ball2D = DistanceBetweenPoints2D(&ball.pos, &actor->pos);
    hand2D = DistanceBetweenPoints2D(&hand.pos, &actor->pos);
    near2D = MIN(ball2D, hand2D);
    if (instruction->type == NME_FOLLOWPLAYER && enemy->brain.area.size.x > 0.0f) {
        if ((instruction->flags & 0x40) && !IsPointWithinBox(&enemy->brain.area, &hand.pos)) {
            return FALSE;
        }
        if ((instruction->flags & 0x80) && !IsPointWithinBox(&enemy->brain.area, &ball.pos)) {
            return FALSE;
        }
        if ((instruction->flags & 0x100) && !IsPointWithinBox(&enemy->brain.area, &closestActor->pos)) {
            return FALSE;
        }
    }
    if ((instruction->flags & 0x10000)
        && enemy->actor.anim->anims[enemy->actor.anim->current].start + 2.0f * enemy->actor.anim->speed
               < enemy->actor.anim->time) {
        return FALSE;
    }
    switch (instruction->condition) {
        case IF_HAND_IN_RANGE:
            if (!handControl.attachedTo && !handControl.deathTimer && instruction->condMin <= handDist && handDist < instruction->condMax) {
                return TRUE;
            }
            break;
        case IF_HAND_IN_RANGE_2D:
            if (!handControl.attachedTo && !handControl.deathTimer && instruction->condMin <= hand2D && hand2D < instruction->condMax) {
                return TRUE;
            }
            break;
        case IF_BALL_IN_RANGE:
            if (!ballControl.enemy && !ballControl.loseTimer && instruction->condMin <= ballDist && ballDist < instruction->condMax) {
                return TRUE;
            }
            break;
        case IF_BALL_IN_RANGE_2D:
            if (!ballControl.enemy && !ballControl.loseTimer && instruction->condMin <= ball2D && ball2D < instruction->condMax) {
                return TRUE;
            }
            break;
        case IF_CLOSEST_IN_RANGE:
            if (((closestActor == &hand && handControl.attachedTo != &enemy->actor && !handControl.deathTimer)
                 || (closestActor == &ball && ballControl.enemy != &enemy->actor && !ballControl.loseTimer))
                && instruction->condMin <= nearDist && nearDist < instruction->condMax) {
                return TRUE;
            }
            break;
        case IF_CLOSEST_IN_RANGE_2D:
            if (((closestActor == &hand && handControl.attachedTo != &enemy->actor && !handControl.deathTimer)
                 || (closestActor == &ball && ballControl.enemy != &enemy->actor && !ballControl.loseTimer))
                && instruction->condMin <= near2D && near2D < instruction->condMax) {
                return TRUE;
            }
            break;
        case IF_CANSEE_BALL:
            if (IsActorFacingActor(actor, &ball, instruction->condMin) == 1) {
                return TRUE;
            }
            break;
        case IF_CANSEE_HAND:
            if (IsActorFacingActor(actor, &hand, instruction->condMin) == 1) {
                return TRUE;
            }
            break;
        case IF_CANSEE_CLOSEST:
            if (IsActorFacingActor(actor, closestActor, instruction->condMin) == 1) {
                return FALSE;
            }
        case EVERY_NUM_FRAMES:
            temp = frameCount % (s32)instruction->condMin;
            if (temp < 0.01) {
                return TRUE;
            }
            break;
        case BETWEEN_SPIN_VALS:
            if (actor->rot.x > instruction->condMin && actor->rot.x < instruction->condMax
                && frameCount % instruction->params.flags == 0) {
                return TRUE;
            }
            break;
        case IF_HAND_HOLDING_BALL:
            return IsHandHoldingBall();
        case IF_HAND_NOT_HOLDING_BALL:
            return !IsHandHoldingBall();
        case IF_IM_HOLDING_BALL:
            return &enemy->actor == ball.link;
        case IF_IM_NOT_HOLDING_BALL:
            return &enemy->actor != ball.link;
        case IF_IM_HOLDING_HAND:
            return &enemy->actor == hand.link;
        case IF_IM_NOT_HOLDING_HAND:
            return &enemy->actor != hand.link;
        case IF_TOUCHING_BALL:
            if (ballDist == 0.0f) {
                return TRUE;
            }
            break;
        case IF_TOUCHING_HAND:
            if (handDist == 0.0f) {
                return TRUE;
            }
            break;
        case IF_OUT_OF_BOUNDS:
            if (!IsPointWithinBox(&enemy->brain.area, &enemy->actor.pos)) {
                return TRUE;
            }
            break;
        case ALWAYS:
            return TRUE;
        case NEVER:
            return FALSE;
        case SOMETIMES:
            temp = Random(1000);
            if (temp <= instruction->condMin) {
                return TRUE;
            }
            break;
        default:
            Crash("NMECONDITIONSMET");
            break;
    }
    return FALSE;
}

s32 NMEAnalyseConditionalList(Enemy *enemy) {
    NMEBrain *br = &enemy->brain;
    s16 x;

    for (x = 0; x < br->conditionalCount; x++) {
        if (br->curList[br->instrIndex] != br->conditionalList[x]) {
            if (NMEConditionsMet(enemy, br->conditionalList[x]) == TRUE) {
                if (br->conditionalList[x]->flags & 0x800) {
                    switch (br->conditionalList[x]->type) {
                        case 5:
                            NMEActionFire1(enemy);
                            break;
                        case 6:
                            NMEActionFire1(enemy);
                            break;
                        case 8:
                            NMEActionFacePlayer(enemy, br->conditionalList[x]->flags);
                            break;
                    }
                } else {
                    if (!br->inConditional) {
                        br->returnIndex = br->instrIndex;
                    }
                    br->inConditional = TRUE;
                    br->curList = br->conditionalList;
                    br->instrIndex = x;
                    NMEGetNewInstruction(enemy, x, TRUE);
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

void NMEProcessActorScript(Enemy *enemy) {
    if (enemy->brain.normalCount == 0) {
        return;
    }
    if (enemy->brain.delayTimer > 0) {
        enemy->brain.delayTimer--;
    } else if (enemy->brain.delayTimer < 0) {
        if (enemy->actor.anim->finishedAnim) {
            enemy->brain.delayTimer = 0;
        }
    } else {
        numInstThisFrame = 0;
        if (!enemy->brain.attackTime && enemy->brain.cooldownTimer >= 0) {
            NMEAnalyseConditionalList(enemy);
        }
        NMEProcessInstruction(enemy);
    }
    if (enemy->actor.active == 1) {
        switch (enemy->actor.type) {
            case 31:
                NMEProcessHubChicken(enemy);
                break;
            case 53:
                NMEProcessBat(enemy);
                break;
            case 42:
                NMEProcessEvilGlove(enemy);
                break;
            case 15:
                NMEProcessReggie(enemy);
                break;
            case 40:
                NMEProcessSpank(enemy);
                break;
            case 41:
                NMEProcessBabySpank(enemy);
                break;
            case 34:
                NMEProcessBigRaptor(enemy);
                break;
            case 35:
                NMEProcessSelwynWhale(enemy);
                break;
            case 36:
                NMEProcessSelwynCrab(enemy);
                break;
            case 37:
                NMEProcessSelwynFish(enemy);
                break;
            case 38:
                NMEProcessRobot(enemy);
                break;
            case 39:
                NMEProcessEvilRobot(enemy);
                break;
            case 19:
                NMEProcessFumble(enemy);
                break;
            case 23:
                NMEProcessTracey(enemy);
                break;
            case 33:
                NMEProcessKloset(enemy);
                break;
            case 7:
                NMEProcessBovva(enemy);
                break;
            case 54:
                NMEProcessFrog(enemy);
                break;
            case 30:
                NMEProcessChuck(enemy);
                break;
            case 21:
                NMEProcessRaptor(enemy);
                break;
            case 25:
                NMEProcessDroid(enemy);
                break;
            case 27:
                NMEProcessSucker(enemy);
                break;
            case 26:
                NMEProcessCylon(enemy);
                break;
            case 24:
                NMEProcessYoofow(enemy);
                break;
            case 12:
                NMEProcessLionFish(enemy);
                break;
            case 18:
                NMEProcessRobes(enemy);
                break;
            case 11:
                NMEProcessGeneralWu(enemy);
                break;
            case 28:
                NMEProcessBugle(enemy);
                break;
            case 29:
                NMEProcessDenis(enemy);
                break;
            case 14:
                NMEProcessKeg(enemy);
                break;
            case 16:
                NMEProcessSwish(enemy);
                break;
            case 22:
                NMEProcessCrumpet(enemy);
                break;
            case 8:
                NMEProcessCannonball(enemy);
                break;
            case 9:
                NMEProcessSamtex(enemy);
                break;
            case 17:
                NMEProcessKnight(enemy);
                break;
            case 45:
                NMEProcessSnail(enemy);
                break;
            case 50:
                NMEProcessCaterpillar(enemy);
                break;
            case 13:
                NMEProcessChester(enemy);
                break;
            case 20:
                NMEProcessMike(enemy);
                break;
            case 10:
                NMEProcessMallet(enemy);
                break;
        }
    }
}

void NMEProcessMallet(Enemy *enemy) {
    if (enemy->brain.instr.type == NME_MOVETO2D && enemy->brain.instrTime == 8) {
        PlayContinuousSample(&enemy->samples[0], 6, 60, &enemy->actor.pos, 128);
    }
    if (enemy->brain.instr.type != NME_MOVETO2D && enemy->samples[0].handle) {
        StopContinuousSample(&enemy->samples[0]);
    }
    if (TriggerAtFrame(88.0f, &enemy->actor)) {
        PlaySample(7, &enemy->actor.pos, 100, 100);
    }
}

void NMEProcessYoofow(Enemy *enemy) {
    Bullet *bullet;
    f32 temp;

    if (enemy->actor.anim->current == 0 && enemy->brain.instrTime > 25 && frameCount % 10 == 0) {
        PlaySample(84, &enemy->actor.pos, 60, 128);
        bullet = FireBulletFromActor(&enemy->actor, YOOFOW_BULLET, 0);
        temp = (f32)(enemy->brain.instrTime - 25) / 2.5;
        bullet->actor.vel.y += temp;
    }
}

void NMEProcessMike(Enemy *enemy) {
    Actor *actor = &enemy->actor;

    if (enemy->actor.anim->current == 15 && !enemy->actor.anim->loop) {
        AccelerateActor2D(&enemy->actor, enemy->actor.phys->accel, enemy->actor.phys->moveSpeed);
    }
    if (enemy->actor.anim->current == 6) {
        AccelerateActor2D(&enemy->actor, enemy->actor.phys->accel * 2, enemy->actor.phys->moveSpeed);
    }
    if (enemy->actor.anim->current == 10) {
        enemy->actor.vel.x *= 0.5;
        enemy->actor.vel.z *= 0.5;
    } else if (enemy->actor.anim->current == 5) {
        actor->rotAim.y = actor->rot.y;
    }
    if (enemy->actor.obj->onGround == 1 && enemy->actor.obj->wasOnGround == 0) {
        CreateDustCircleScaled(&enemy->actor.pos, 3, 0, 2.0f);
        StartEarthquakeAtPoint(&enemy->actor.pos, 6, 2000.0f, 500.0f, 1, 2);
        PlaySample(54, &enemy->actor.pos, 100, 128);
    }
    if (enemy->actor.anim->current == 15) {
        if (QueryAnimTime(&enemy->actor, 42.0f)) {
            PlaySample(52, &enemy->actor.pos, 100, 128);
        }
        if (QueryAnimTime(&enemy->actor, 60.0f)) {
            PlaySample(53, &enemy->actor.pos, 100, 128);
        }
    }
    if (enemy->actor.anim->current == 5) {
        if (QueryAnimTime(&enemy->actor, 25.0f)) {
            PlaySample(52, &enemy->actor.pos, 50, 128);
        }
        if (QueryAnimTime(&enemy->actor, 35.0f)) {
            PlaySample(53, &enemy->actor.pos, 50, 128);
        }
    }
}

void NMEProcessChester(Enemy *enemy) {
    Enemy *e = enemy;

    enemy->actor.flags &= 0x1FFFF;
    if (!enemy->actor.obj->onGround) {
        enemy->actor.flags |= 0x60000;
    }
    if (enemy->actor.anim->current == 12) {
        enemy->actor.flags |= 0x60000;
    } else if (enemy->actor.anim->current == 6 && enemy->brain.instrTime == 2) {
        PlaySample(77, &enemy->actor.pos, 100, 60);
    } else if (enemy->actor.anim->current == 15) {
        if (TriggerAtFrame(127.0f, &e->actor) || TriggerAtFrame(133.0f, &e->actor) || TriggerAtFrame(139.0f, &e->actor) ||
            TriggerAtFrame(145.0f, &e->actor) || TriggerAtFrame(151.0f, &e->actor) || TriggerAtFrame(157.0f, &e->actor)) {
            PlaySample(76, &e->actor.pos, 50, 128);
        }
    }
    if (e->brain.instr.type == NME_JUMP) {
        if (e->brain.instrTime == 2) {
            PlaySample(118, &e->actor.pos, 50, 128);
        }
        enemy->actor.rotAim.x += 0.08;
    }
    if (enemy->actor.obj->onGround) {
        enemy->actor.rotAim.x = 0.0f;
        if (!enemy->actor.obj->wasOnGround) {
            if (e->actor.anim->current == 8) {
                PlaySample(75, &e->actor.pos, 100, 128);
            }
            CreateDustCircleScaled(&enemy->actor.pos, 3, 0, 2.0f);
            enemy->actor.rot.x -= 1.0f;
        }
    }
}

void NMEProcessKnight(Enemy *enemy) {
    NMEBrain *br = &enemy->brain;
    s32 hit = FALSE;
    Vec3 tempVect;
    Vec3 tempVect2;

    if (enemy->brain.instr.type == NME_WAIT && (enemy->brain.instrTime == 18 || enemy->brain.instrTime == 27)) {
        PlaySample(51, &enemy->actor.pos, 100, 128);
    }
    if (br->instr.type == NME_SPECIAL && br->instr.params.flags == 0 && br->instrTime == 54) {
        hit = TRUE;
    }
    if (br->instr.type == NME_SPECIAL && br->instr.params.flags == 1 && br->instrTime == 9) {
        hit = TRUE;
    }
    if (hit) {
        tempVect = enemy->actor.obj->spheres[1].pos;
        tempVect.y -= enemy->actor.obj->spheres[1].radius - 2.0f;
        CreateDustCircleScaled(&tempVect, 6, 0.0f, 0.5f);
        tempVect2.x = 5.0f;
        tempVect2.y = Random(5) + 10;
        tempVect2.z = 0.0f;
        RotateVector2D(&tempVect2, &tempVect2, Random(6));
        AddDebris(PLATBIT, &tempVect, &tempVect2, 0, 0);
        tempVect2.x = 5.0f;
        tempVect2.y = Random(5) + 10;
        tempVect2.z = 0.0f;
        RotateVector2D(&tempVect2, &tempVect2, Random(6));
        AddDebris(PLATBIT, &tempVect, &tempVect2, 0, 0);
        StartEarthquakeAtPoint(&enemy->actor.pos, 6, 800.0f, 600.0f, 1, 10);
        PlaySample(50, &enemy->actor.pos, 100, 128);
        CreateEnemyBlastring(&tempVect, 10, 0.0f, 0.2f);
    }
}

void NMEProcessKeg(Enemy *enemy) {
    if (enemy->actor.anim->current == 5 && TriggerAtFrame(157.0f, &enemy->actor)) {
        PlaySample(73, &enemy->actor.pos, 100, 128);
    }
    if (enemy->actor.anim->current == 10 && TriggerAtFrame(191.0f, &enemy->actor)) {
        PlaySample(73, &enemy->actor.pos, 100, 128);
    }
    if (enemy->actor.anim->current == 11 && !enemy->samples[0].handle) {
        PlayContinuousSample(&enemy->samples[0], 74, 100, &enemy->actor.pos, 128);
    }
    if (enemy->samples[0].volume) {
        if (enemy->samples[0].handle) {
            if (!enemy->actor.obj->onGround) {
                StopContinuousSample(&enemy->samples[0]);
            }
        } else if (enemy->actor.obj->onGround) {
            PlayContinuousSample(&enemy->samples[0], 74, 100, &enemy->actor.pos, 128);
        }
    }
    if (enemy->brain.instr.type == NME_SPECIAL && enemy->brain.instr.params.flags == 1) {
        AccelerateActor2D(&enemy->actor, 0.85f, enemy->actor.phys->maxSpeed);
        enemy->actor.anim->speed = LENGTH(&enemy->actor.vel) * 0.3;
        if (enemy->brain.instrTime > 200) {
            KillEnemy(enemy);
        }
    }
}

void NMEProcessDenis(Enemy *enemy) {
    Vec3 tempVect;

    if (enemy->actor.obj->onGround == 1 && enemy->actor.obj->wasOnGround == 0) {
        if (enemy->actor.prevVel.y < -5.0f) {
            PlaySample(20, &enemy->actor.pos, 100, 128);
        }
        AnimateActor(&enemy->actor, 5, 1, 0, 0.0f);
        if (enemy->actor.prevVel.y < -5.0f) {
            CreateDustCircleScaled(&enemy->actor.pos, 4, 0, 2.0f);
        }
        tempVect = enemy->actor.vel;
        MakeUnit(&tempVect);
        SquashActor(&enemy->actor, &tempVect, ABS(enemy->actor.vel.y * 0.1), enemy->actor.phys->maxSquash);
    }
    if (hand.link == &enemy->actor) {
        enemy->actor.flags |= 0x1000000;
    } else {
        enemy->actor.flags &= ~0x1000000;
    }
}

void NMEProcessBugle(Enemy *enemy) {
    Vec3 tempVect;
    Quat tempQ;
    Quat qAim;
    f32 tempFloat;

    if ((frameCount & 0x3F) == 0x3F && Random(20) < 5) {
        PlaySample(25, &enemy->actor.pos, 100, 128);
    }
    if (ball.link == &enemy->actor && ballControl.type == BALL_MODE_BOWLING) {
        enemy->actor.vel.y -= gravity / 3.0f;
    }
    if (enemy->brain.attackTime) {
        if (enemy->brain.instrIndex == 2 || enemy->brain.instrIndex == 3) {
            if (enemy->samples[0].handle == 0) {
                PlayContinuousSample(&enemy->samples[0], 26, 100, &enemy->actor.pos, 128);
            }
            SCALE_VECTOR_2D(&ball.vel, 0.5);
        } else if (enemy->samples[0].handle) {
            enemy->samples[0].volume -= 5;
            if (enemy->samples[0].volume <= 0) {
                StopContinuousSample(&enemy->samples[0]);
            }
        }
        if (enemy->brain.instrIndex != 0 && enemy->brain.instrIndex < 4) {
            tempVect.x = ball.pos.x - enemy->actor.pos.x;
            tempVect.y = ball.pos.y - enemy->actor.pos.y;
            tempVect.z = ball.pos.z - enemy->actor.pos.z;
            enemy->actor.rot.y = Aabs(atan2(tempVect.x, tempVect.z));
            enemy->actor.rot.x = Aabs(-atan2(tempVect.y, LENGTH_2D(&tempVect)) - 0.3);
            enemy->actor.rot.z = 0.0f;
            GetQuaternionFromXYZRot(&tempQ, &enemy->actor.rot);
            if (enemy->brain.instr.type == NME_SPECIAL) {
                SCALE_VECTOR(&enemy->actor.vel, 0.6);
                MakeUnit(&tempVect);
                tempFloat = 2.0 * (f32)enemy->brain.instrTime / 19.0 + 1.0;
                tempFloat = (tempFloat > 3.0f) ? 3.0f : tempFloat;
                SquashActor(&ball, &tempVect, -tempFloat, 1.0f - tempFloat / 6.0f);
                QuatSlerpAtFixedSpeed(&enemy->actor.rotQuat, &tempQ, 0.3f, &enemy->actor.rotQuat);
            } else {
                QuatSlerpAtFixedSpeed(&enemy->actor.rotQuat, &tempQ, 0.1f, &enemy->actor.rotQuat);
            }
            enemy->actor.spinSpeed = 0.0f;
            return;
        }
        if (enemy->brain.instrIndex == 4) {
            enemy->actor.spinSpeed = 0.0f;
            if (enemy->brain.instrTime >= 5) {
                tempVect.x = 0.0f;
                tempVect.y = 0.0f;
                tempVect.z = 1.0f;
                RotateVectorByQuaternion(&tempVect, &tempVect, &enemy->actor.rotQuat);
                qAim.x = 0.0f;
                qAim.y = 1.0f;
                qAim.z = 0.0f;
                qAim.w = atan2(tempVect.x, tempVect.z);
                GetQuaternionFromRotation(&tempQ, &qAim);
                QuatSlerpAtFixedSpeed(&enemy->actor.rotQuat, &tempQ, 0.5f, &enemy->actor.rotQuat);
            }
        }
    } else {
        if (enemy->samples[0].handle) {
            enemy->samples[0].volume -= 5;
            if (enemy->samples[0].volume <= 0) {
                StopContinuousSample(&enemy->samples[0]);
            }
        }
        tempVect.x = Random(21) - 10;
        tempVect.y = Random(21) - 10;
        tempVect.z = Random(21) - 10;
        MakeUnit(&tempVect);
        SCALE_VECTOR(&tempVect, 0.05);
        ADD_TO_VECTOR(&enemy->actor.spinAxis, &tempVect);
        MakeUnit(&enemy->actor.spinAxis);
        enemy->actor.spinSpeed = 0.035f;
        if (ball.link == &enemy->actor && LENGTH_2D(&ball.vel) < 0.3) {
            tempVect.x = 0.0f;
            tempVect.y = enemy->actor.radius;
            tempVect.z = 0.0f;
            RotateVectorByRotation(&tempVect, &tempVect, (Quat *)&enemy->actor.spinAxis);
            ActorStartRoll(&ball, tempVect.x, tempVect.z);
        }
    }
}

void NMEProcessGeneralWu(Enemy *enemy) {
    NMEBrain *br = &enemy->brain;
    Vec3 tempVect;
    Vec3 tempVect2;

    enemy->actor.obj->spheres[1].flags |= 8;
    if (enemy->actor.anim->current == 15 && frameCount % 3 == 0) {
        PlaySample(Random(2) == 0 ? 9 : 8, &enemy->actor.pos, 70, 128);
    }
    if (br->instr.type == NME_SPECIAL && br->instr.params.flags == 0) {
        if (enemy->actor.anim->time > 10.0f && enemy->actor.anim->time < 30.0f) {
            helper.scale.x = helper.scale.y = helper.scale.z = 30.0f;
            enemy->actor.obj->spheres[1].flags &= ~8;
        }
        if (TriggerAtFrame(46.0f, &enemy->actor)) {
            PlaySample(123, &enemy->actor.pos, 100, 128);
        }
        if (enemy->actor.anim->time >= 24.0f && enemy->actor.anim->time < enemy->actor.anim->speed + 24.0f) {
            PlaySample(11, &enemy->actor.pos, 100, 128);
            tempVect = enemy->actor.obj->spheres[1].pos;
            tempVect.y -= enemy->actor.obj->spheres[1].radius;
            CreateDustCircleScaled(&tempVect, 4, 2.0f, 2.0f);
            tempVect2.x = 5.0f;
            tempVect2.y = Random(5) + 10;
            tempVect2.z = 0.0f;
            RotateVector2D(&tempVect2, &tempVect2, Random(6));
            AddDebris(PLATBIT, &tempVect, &tempVect2, NULL, NULL);
            tempVect2.x = 5.0f;
            tempVect2.y = Random(5) + 10;
            tempVect2.z = 0.0f;
            RotateVector2D(&tempVect2, &tempVect2, Random(6));
            AddDebris(PLATBIT, &tempVect, &tempVect2, NULL, NULL);
        }
    }
}

void NMEProcessLionFish(Enemy *enemy) {
    NMEBrain *br = &enemy->brain;
    NMEInstruction *ins = &enemy->brain.instr;
    Actor *actor = &enemy->actor;

    if (br->instr.type != NME_JUMP) {
        return;
    }
    if (br->instrTime == 1) {
        actor->vel.x *= 0.3;
        actor->vel.y *= 0.3;
        actor->vel.z *= 0.3;
    }
    if (br->instrTime == 9) {
        actor->vel.y = ins->params.box.min.y;
    }
    if (br->instrTime < 9) {
        return;
    }
    if (br->instrTime < 14) {
        actor->flags &= ~0x200000;
    } else {
        actor->flags |= 0x200000;
    }
    if (br->instrTime < 18) {
        actor->vel.y += gravity / 3.0f;
        AccelerateActor2D(&enemy->actor, ins->params.box.min.z, 20.0f);
        return;
    }
    if (br->instrTime == 35) {
        AnimateActor(actor, 8, 0, 0, 0.0f);
    }
    if (br->instrTime < 35) {
        actor->vel.x *= 0.2;
        actor->vel.y = 0.0f;
        actor->vel.z *= 0.2;
    }
    if (br->instrTime >= 35) {
        actor->vel.y -= 4.0f;
    }
}

void NMEProcessFrogged(Enemy *enemy) {
    if (enemy->actor.anim->current == 5 && TriggerAtFrame(35.0f, &enemy->actor)) {
        PlaySample(218, &enemy->actor.pos, 50, 118);
    }
    if (enemy->actor.obj->onGround) {
        enemy->actor.vel.x *= 0.5;
        enemy->actor.vel.z *= 0.5;
    }
    if (enemy->actor.anim->finishedAnim) {
        switch (enemy->actor.anim->current) {
            case 5:
                if (enemy->brain.scratch) {
                    enemy->brain.scratch--;
                    AnimateActor(&enemy->actor, 5, 0, 0, 0.0f);
                } else {
                    AnimateActor(&enemy->actor, 10, 0, 0, 0.0f);
                }
                break;
            case 10:
                AnimateActor(&enemy->actor, 7, 0, 0, 0.0f);
                enemy->actor.vel.y = 8.0f;
                PlaySample(104, &enemy->actor.pos, 50, 110);
                AccelerateActor2D(&enemy->actor, 4.0f, 50.0f);
                break;
            case 7:
                if (enemy->actor.obj->onGround) {
                    AnimateActor(&enemy->actor, 8, 0, 0, 0.0f);
                    AnimateActor(&enemy->actor, 5, 0, 1, 0.0f);
                    enemy->brain.scratch = 3;
                    enemy->actor.rotAim.y = Aabs(enemy->actor.rotAim.y + 1.2);
                }
                break;
        }
    }
}

void NMEProcessBigRaptor(Enemy *enemy) {
    Vec3 tempVect;

    switch (enemy->actor.anim->current) {
        case 15:
            if (TriggerAtFrame(1.0f, &enemy->actor)) {
                PlaySample(46, &enemy->actor.pos, 10, 128);
            } else if (TriggerAtFrame(14.0f, &enemy->actor)) {
                PlaySample(47, &enemy->actor.pos, 10, 128);
            }
            break;
        case 9:
            if (TriggerAtFrame(28.0f, &enemy->actor) || TriggerAtFrame(36.0f, &enemy->actor)) {
                PlaySample(46, &enemy->actor.pos, 10, 128);
            } else if (TriggerAtFrame(32.0f, &enemy->actor) || TriggerAtFrame(40.0f, &enemy->actor)) {
                PlaySample(47, &enemy->actor.pos, 10, 128);
            }
            break;
        case 5:
            if (TriggerAtFrame(84.0f, &enemy->actor)) {
                PlaySample(44, &enemy->actor.pos, 255, 128);
            }
            break;
        case 12:
            if (TriggerAtFrame(56.0f, &enemy->actor)) {
                PlaySample(45, &enemy->actor.pos, 100, 128);
            }
            break;
    }
    if (enemy->actor.anim->current == 9) {
        enemy->actor.phys->maxSpeed = 160.0f;
        enemy->actor.phys->moveSpeed = 8.0f;
    } else {
        enemy->actor.phys->moveSpeed = 2.0f;
        enemy->actor.phys->maxSpeed = 40.0f;
    }
    if (enemy->actor.anim->current == 12) {
        if (enemy->actor.anim->time > 48.0f) {
            tempVect.x = enemy->actor.pos.x + Random(5) - 2.0f;
            tempVect.y = enemy->actor.pos.y - enemy->actor.radius + 3.0f;
            tempVect.z = enemy->actor.pos.z + Random(5) - 2.0f;
            AddDebris(DUST, &tempVect, &zero, NULL, NULL);
        }
        enemy->actor.friction = 0.96f;
    } else {
        enemy->actor.friction = enemy->actor.phys->friction;
    }
    if (enemy->actor.anim->current == 5 && enemy->actor.anim->time > 83.0f && enemy->actor.anim->time < 90.0f
        && earthQuake.time == 0) {
        StartEarthquakeAtPoint(&hand.pos, 50, 700.0f, 350.0f, 1, 2);
    }
}

INCLUDE_ASM("asm/nonmatchings/nmeprocess", pad_8019A988);

INCLUDE_RODATA("asm/nonmatchings/nmeprocess", D_8010A82C);
