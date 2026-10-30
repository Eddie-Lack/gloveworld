#include "common.h"
#include "cheat.h"
#include "ident.h"
#include "controller.h"
#include "game.h"
#include "ball.h"
#include "hand.h"
#include "sound.h"
#include "text.h"
#include "macros.h"
#include "collect.h"
#include "maths.h"
#include "actor.h"
#include "banks.h"
#include "debug.h"
#include "overlay.h"
#include "bonus.h"
#include "decompress.h"
#include "spell.h"

extern u8 ActiveController;
extern u8 cheatTempString[];
extern s32 controlMethod;
extern u8 gamePaused;
extern u8 cheatMsgCount;
extern s32 cheatCount;
extern s32 cheatTextPause;
extern s32 frameCount;
void SubText(struct Text *text);
extern s32 cheatAlphaVal;
extern struct Text *cheatText;

typedef struct CheatString {
    char c[8];
} CheatString;

extern char cheatMsg[];
extern CheatString cheatButtonPressed;
void SetFont(u8 type);
void MusHandleStop(s32 handle, s32 speed);

extern s32 cheatSfx;
extern s32 cheatSfxCount;
struct Text *NEW_TEXT(s16 x, s16 y, u8 r, u8 g, u8 b, u8 a, f32 sx, f32 sy, u8 oneShot, char *str, u8 centre);

u32 UpdateCRC(u8 *ptr);

extern s32 cheatActivated;
extern u8 ballEnemyType;
extern char *nmeObjectName[];
extern Actor plaster[4];
extern SpriteAnim garibAnimation[];
extern Vec3 cameraUpVectAim;
extern f32 gravity;
extern s16 fieldOfViewAim;
void InitSpriteAnimation(GameSprite *sprite, SpriteAnim *temp, s16 lifespan);
void FlashScreen(s16 r, s16 g, s16 b, s16 fadeVal, s16 fadeStep);

s32 CheckForCheats(void) {
    s8 cheat;

    if (cheatSfxCount && --cheatSfxCount == 0) {
        MusHandleStop(cheatSfx, 0);
    }
    if (pad[ActiveController].pressed[BUTTON_C_LEFT] || pad[ActiveController].pressed[BUTTON_C_RIGHT] || pad[ActiveController].pressed[BUTTON_C_UP] ||
        pad[ActiveController].pressed[BUTTON_C_DOWN]) {
        cheatTempString[cheatMsgCount++] = FindCheatButton();
        cheatSfx = PlaySampleNot3D(cheatSampleData[cheatTempString[cheatMsgCount - 1]].id, 100, 128,
                                   cheatSampleData[cheatTempString[cheatMsgCount - 1]].pitch);
        cheatSfxCount = 12;
        cheat = CompareCheatString();
        if (cheat != -1) {
            ActivateCheat(cheat, 1);
            return TRUE;
        }
    }
    if (cheatMsgCount == 8 || pad[ActiveController].pressed[BUTTON_Z]) {
        cheatMsgCount = 0;
        memset(cheatTempString, 0xFF, 8);
    }
    return FALSE;
}

s32 FindCheatButton(void) {
    if (pad[ActiveController].pressed[BUTTON_C_LEFT]) {
        return 2;
    } else if (pad[ActiveController].pressed[BUTTON_C_RIGHT]) {
        return 3;
    } else if (pad[ActiveController].pressed[BUTTON_C_UP]) {
        return 1;
    } else if (pad[ActiveController].pressed[BUTTON_C_DOWN]) { // keeps the last test a branch instead of sltu
        return 0;
    } else {
        return 0;
    }
}

s8 CompareCheatString(void) {
    s32 i;
    s32 j;
    u8 matches;

    for (i = 0; i < 26; i++) {
        matches = 0;
        for (j = 0; j < 8; j++) {
            if (cheatTempString[j] == cheatInfo[i].code[j]) {
                matches++;
            }
        }
        if (matches == 8) {
            return i;
        }
    }
    return -1;
}

void ActivateCheat(u8 num, u8 show) {
    char buf[64];
    GaribGroup *group;
    Garib *garib;
    s32 j;
    u8 cheatOn = FALSE;

    if (!CheatAllowed(num)) {
        return;
    }
    if (cheatInfo[num].type == 0) {
        if (cheatActivated & (1 << num)) {
            cheatActivated &= ~(1 << num);
        } else {
            cheatActivated |= 1 << num;
            cheatOn = TRUE;
        }
    }
    cheatCount = 0;
    switch (num) {
        case 4:
            if (cheatOn) {
                ActivateCollectable(POWERBALL, -1);
            } else {
                DeactivateCollectable(POWERBALL);
            }
            break;
        case 5:
            if (cheatOn) {
                ballStatic = 0;
                ball.flags = defaultBehaviour[ballControl.type].flags;
                ball.pos = hand.pos;
            } else {
                SetControlToHand();
            }
            break;
        case 7:
            if (cheatOn) {
                switch (gameInfo.world) {
                    case ATLANTIS:
                        ballEnemyType = 10;
                        break;
                    case CARNIVAL:
                        ballEnemyType = 28;
                        break;
                    case PREHISTORIC:
                        ballEnemyType = 7;
                        break;
                    case FEAR:
                        ballEnemyType = 43;
                        break;
                    case SPACE:
                        ballEnemyType = 25;
                        break;
                    case HUB:
                    case PIRATES:
                        ballEnemyType = 8;
                        break;
                }
                DeactivateCollectable(CRYSTALBALL);
                DeactivateCollectable(SNOWBALL);
                FindObject(&ball.inst, UpdateCRC(nmeObjectName[ballEnemyType]), nmeObjectName[ballEnemyType], 1);
                AddObjectsSpritesToSpriteList(ball.inst->obj);
                ball.scale.x = ball.scale.y = ball.scale.z = ball.obj->spheres[0].radius *
                                                             enemyStats[ballEnemyType].scale /
                                                             defaultBehaviour[ballEnemyType].radius;
                ball.obj->spheres[0].targetRadius = defaultBehaviour[ballEnemyType].radius;
                for (j = 0; j < 4; j++) {
                    plaster[j].alpha = 0;
                }
            } else {
                RemoveObjectSprites(ball.inst->obj, 1);
                FindObject(&ball.inst, UpdateCRC("gball.ndo"), "gball.ndo", 1);
                ball.scale.x = ball.scale.y = ball.scale.z = ball.obj->spheres[0].radius * 0.05 / 9.0;
                ball.obj->spheres[0].targetRadius = defaultBehaviour[ballControl.type].radius;
                for (j = 0; j < 4; j++) {
                    plaster[j].alpha = 100;
                }
            }
            break;
        case 8:
            if (cheatOn) {
                gravity /= 2.0f;
            } else {
                gravity *= 2.0f;
            }
            break;
        case 9:
            if (!(cheatActivated & 0x80)) {
                if (cheatOn) {
                    ball.obj->spheres[0].targetRadius = 20.0f;
                } else {
                    ball.obj->spheres[0].targetRadius = defaultBehaviour[ballControl.type].radius;
                }
            }
            break;
        case 10:
            if (cheatOn) {
                fieldOfViewAim = 120;
            } else {
                fieldOfViewAim = 45;
            }
            break;
        case 11:
            RotateVector2DXYZ(&cameraUpVectAim, &cameraUpVectAim, -PI / 4, 2);
            break;
        case 12:
            RotateVector2DXYZ(&cameraUpVectAim, &cameraUpVectAim, PI / 4, 2);
            break;
        case 13:
            for (group = garibGroupList.head.prev; group != &garibGroupList.head; group = group->prev) {
                for (garib = group->garibs.head.prev; garib != &group->garibs.head; garib = garib->prev) {
                    if (garib->type == NORMAL_GARIB) {
                        if (cheatOn) {
                            InitSpriteAnimation(&garib->sprite, &garibAnimation[3], 0);
                        } else {
                            InitSpriteAnimation(&garib->sprite, &garibAnimation[0], 0);
                        }
                    }
                }
            }
            break;
        case 14:
            cheatOn = FALSE;
            if (player.powerups[DEATH]) {
                DeactivateCollectable(DEATH);
            } else {
                cheatOn = TRUE;
                ActivateCollectable(DEATH, 1000);
            }
            break;
        case 15:
            cheatOn = FALSE;
            if (player.powerups[FROGGY]) {
                DeactivateCollectable(FROGGY);
            } else {
                cheatOn = TRUE;
                ActivateCollectable(FROGGY, 1000);
            }
            break;
        case 16:
            cheatOn = FALSE;
            if (player.powerups[HERCULES]) {
                DeactivateCollectable(HERCULES);
            } else {
                cheatOn = TRUE;
                ActivateCollectable(HERCULES, 1000);
            }
            break;
        case 17:
            cheatOn = FALSE;
            if (player.powerups[SPEEDUP]) {
                player.powerups[SPEEDUP] = 1;
            } else {
                cheatOn = TRUE;
                ActivateCollectable(SPEEDUP, 1000);
            }
            break;
        case 18:
            cheatOn = FALSE;
            if (player.powerups[BOOMERANGBALL]) {
                DeactivateCollectable(BOOMERANGBALL);
            } else {
                cheatOn = TRUE;
                ActivateCollectable(BOOMERANGBALL, 1000);
            }
            break;
        case 19:
            cheatOn = FALSE;
            if (player.powerups[VANISHBALL]) {
                DeactivateCollectable(VANISHBALL);
            } else {
                cheatOn = TRUE;
                ActivateCollectable(VANISHBALL, 1000);
            }
            break;
        case 20:
            cheatOn = TRUE;
            if (handControl.spellTimer[2] > 50) {
                cheatOn = FALSE;
                handControl.spellTimer[2] = 50;
            } else {
                ActivateSpellHand(2);
                handControl.spellTimer[2] = 1000;
            }
            break;
        case 22:
            for (j = 0; j < 26; j++) {
                if ((cheatActivated & (1 << j)) && cheatInfo[j].type == 0) {
                    ActivateCheat(j, 0);
                }
            }
            cameraUpVectAim.x = 0.0f;
            cameraUpVectAim.z = 0.0f;
            cameraUpVectAim.y = 1.0f;
            break;
    }
    FlashScreen(255, 255, 255, 255, -30);
    if (show) {
        if (cheatText != NULL) {
            SubText(cheatText);
        }
        SetFont(0);
        if (cheatInfo[num].type == 1) {
            cheatText = NEW_TEXT(320, 120, 255, 255, 255, 255, 1.0f, 1.0f, 0, cheatInfo[num].msg, 0);
        } else {
            if (cheatOn) {
                sprintf(buf, "%s ON!", cheatInfo[num].msg);
            } else {
                sprintf(buf, "%s OFF!", cheatInfo[num].msg);
            }
            cheatText = NEW_TEXT(320, 120, 255, 255, 255, 255, 1.0f, 1.0f, 0, buf, 0);
        }
        PlaySampleNot3D(142, 100, 128, 128);
    }
}

void UpdateCheatText(void) {
    s32 x;

    SetFont(0);
    x = cheatText->x + GetStringPixelLength(cheatText->str, cheatText->scaleX, cheatText->scaleY) / 2;
    if (x > 160) {
        cheatText->x -= MIN(MAX(1, (x - 160) / 2), 50);
    } else {
        if (cheatCount++ > cheatTextPause) {
            cheatText->x -= (cheatCount - cheatTextPause) * 3;
        }
        if (cheatText->x < -480) {
            SubText(cheatText);
            cheatText = NULL;
        }
    }
    SetFont(1);
    NEW_TEXT(112, 200 - cheatCount * 2, 255, 255, 255, MIN(510 - cheatCount * cheatAlphaVal, 255), 1.0f, 1.0f, 1,
             cheatButtonPressed.c, 0);
    if ((frameCount & 1) && cheatCount >= 3) {
        AddStarBurst(Random(60) + 130, 200 - cheatCount * 2, 2, SDT_STAR);
    }
}

void PrintCheatString(void) {
    s32 i = 0; // the early initialisation matches the original register use

    if (gamePaused || gameInfo.level == FLYTHRU2) {
        for (i = 0; i < 8; i++) {
            switch (cheatTempString[i]) {
                case 2:
                    cheatMsg[i] = 0xAA;
                    break;
                case 3:
                    cheatMsg[i] = 0xAB;
                    break;
                case 1:
                    cheatMsg[i] = 0xAC;
                    break;
                case 0:
                    cheatMsg[i] = 0xAD;
                    break;
                default:
                    cheatMsg[i] = ' ';
                    break;
            }
        }
        cheatMsg[i] = 0;
        if (gameInfo.level == FLYTHRU2) {
            return;
        }
    }
    if (gamePaused) {
        SetFont(1);
        NEW_TEXT(112, 200, 255, 255, 255, 255, 1.0f, 1.0f, 1, cheatMsg, 0);
    }
    if (!cheatCount && cheatMsgCount) {
        cheatMsgCount = 0;
        cheatButtonPressed = *(CheatString *)cheatMsg;
        memset(cheatTempString, 0xFF, 8);
    }
}

s32 CheatAllowed(s32 cheat) {
    switch (cheat) {
        case 1:
            if (gameInfo.level == CAVE) {
                return FALSE;
            }
            break;
        case 5:
            if (ballControl.enemy || handControl.attachedTo || gameInfo.level == CAVE || ballStatic) {
                return FALSE;
            }
            break;
        case 18:
        case 19:
            if (ballStatic || controlMethod == CONTROL_BALL) {
                return FALSE;
            }
            break;
        case 14:
        case 15:
            if (gameInfo.level == ATLANTISBOSS || gameInfo.level == CARNIVALBOSS || gameInfo.level == PIRATESBOSS
                || gameInfo.level == PREHISTORICBOSS || gameInfo.level == FEARBOSS || gameInfo.level == SPACEBOSS1
                || gameInfo.level == SPACEBOSS3) {
                return FALSE;
            }
            break;
    }
    return TRUE;
}

INCLUDE_ASM("asm/nonmatchings/cheat", pad_801BC70C);
