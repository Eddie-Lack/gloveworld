#ifndef PUZZLE_H
#define PUZZLE_H

#include "types.h"

typedef struct PuzzleCamCond {
    /* 0x00 */ Vec3 pos;
    /* 0x0C */ Vec3 size;
    /* 0x18 */ f32 radius;
    /* 0x1C */ u8 unk1C[0x8];
} PuzzleCamCond; /* size = 0x24 */

typedef struct PuzzleCamAction {
    /* 0x00 */ Vec3 pos;
    /* 0x0C */ f32 speed;
    /* 0x10 */ u8 unk10[0x8];
} PuzzleCamAction; /* size = 0x18 */

typedef struct PuzzleCond {
    /* 0x00 */ struct PuzzleCond *next;
    /* 0x04 */ struct PuzzleCond *prev;
    /* 0x08 */ u8 type;
    /* 0x09 */ u8 tag;
    /* 0x0A */ s16 value;
    /* 0x0C */ struct PuzzleCamCond *cam;
} PuzzleCond; /* size = 0x10 */

typedef struct PuzzleCondList {
    /* 0x00 */ PuzzleCond head;
    /* 0x10 */ s32 count;
    /* 0x14 */ s16 requiredCount;
    /* 0x16 */ s16 timesLeft;
} PuzzleCondList;

typedef struct PuzzleAction {
    /* 0x00 */ struct PuzzleAction *next;
    /* 0x04 */ struct PuzzleAction *prev;
    /* 0x08 */ s16 timer;
    /* 0x0A */ s16 delay;
    /* 0x0C */ s16 type;
    /* 0x0E */ s16 tag;
    /* 0x10 */ f32 value;
    /* 0x14 */ s16 data[6];
    /* 0x20 */ s32 flags;
    /* 0x24 */ struct PuzzleCamAction *cam;
} PuzzleAction; /* size = 0x28 */

typedef struct PuzzleActionList {
    /* 0x00 */ PuzzleAction head;
    /* 0x28 */ s32 count;
    /* 0x2C */ s32 active;
} PuzzleActionList;

typedef struct Puzzle {
    /* 0x00 */ struct Puzzle *next;
    /* 0x04 */ struct Puzzle *prev;
    /* 0x08 */ PuzzleCondList conds;
    /* 0x20 */ PuzzleActionList actions;
} Puzzle; /* size = 0x50 */

typedef struct PuzzleList {
    /* 0x00 */ Puzzle head;
    /* 0x50 */ s32 count;
} PuzzleList;

extern PuzzleList puzzleList;
extern s32 puzzleVal[100];

void AddConditionToList(PuzzleCond *condition, PuzzleCondList *conditionList);
void AddActionToList(PuzzleAction *action, PuzzleActionList *actionList);
void InitConditionList(PuzzleCondList *conditionList);
void InitActionList(PuzzleActionList *actionList);
Puzzle *NewPuzzle(void);
s32 IsCameraAction(s16 type);
s32 IsCameraCondition(s16 type);

void InitPuzzleList(void);
void AddPuzzle(Puzzle *puzzle);
void CheckPuzzles(void);
void PuzzleCheckActionList(PuzzleActionList *actionList);
void PuzzleStartActionList(PuzzleActionList *actionList);
void PuzzleExecuteAction(PuzzleAction *action);
s32 PuzzleEvaluateConditionList(PuzzleCondList *conditionList);
s32 PuzzleEvaluateCondition(PuzzleCond *condition);

#endif
