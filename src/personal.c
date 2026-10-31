#include "common.h"
#include "ident.h"
#include "actor.h"
#include "camera.h"
#include "maths.h"
#include "debug.h"
#include "macros.h"
#include "controller.h"
#include "platform.h"
#include "ball.h"
#include "game.h"
#include "sound.h"
#include "save.h"
#include "timer.h"
#include "jalloc.h"
#include "libc.h"
#include "light.h"
#include "banks.h"
#include "cheat.h"
#include "debris.h"
#include "personal.h"
#include "bonus.h"
#include "enemy.h"

extern s32 cheatActivated;
extern u8 gamePaused;
extern s32 incval;
extern u8 dontClearScreen;
extern u16 cfb_16_a[];
extern s32 screenshotCount;
s32 PCcreat(char *name, s32 mode);
s32 PCwrite(s32 fd, void *buf, s32 size);
s32 PCclose(s32 fd);

extern Platform *personalPlatform;
extern Vec3 zero;
extern f32 gravity;
extern s16 currentSaveSlot;
extern u8 inertia;
extern s32 autoPlayback;
extern s32 *recordKeys;
extern s16 *rkp;
extern s32 pcFile;
extern char message[];
extern s32 numCyclingTextures;
extern u8 ActiveController;
void PrepareSong(u8 song);
void RotateLight(void *set, s32 index, f32 x, f32 y);

void ProcessPersonalLoop(void) {
    Vec3 vect;

    ProcessInputPersonal();
    UpdatePause();
    PrintCheatString();
    if (cheatActivated & 0x20) {
        SetControlToBall();
    }
    if (!cameraInfo.mode && !gamePaused) {
        vect.x = 0.0f;
        vect.y = 0.0f;
        vect.z = 100.0f;
        RotateVector2DXYZ(&vect, &vect, -camera.rot.x, 0);
        RotateVector2D(&vect, &vect, camera.rot.y);
        ADD_VECTORS(&cameraInfo.actor.pos, &camera.pos, &vect);
    }
}

static inline void PutActorOnPlatform(Actor *actor) {
    f32 scaleMin;

    actor->pos = personalPlatform->actor.pos;
    if (personalPlatform->scale != NULL) {
        scaleMin = MAX(personalPlatform->scale->min[1], 1.0f);
        actor->pos.y += actor->radius + personalPlatform->extent.y * personalPlatform->actor.scale.y / scaleMin;
    } else {
        actor->pos.y += actor->radius + personalPlatform->extent.y;
    }
    actor->prevPos = actor->pos;
    actor->vel = zero;
    actor->vel.y -= gravity;
    if (!(personalPlatform->flags & 4)) {
        actor->obj->platform = actor->obj->prevPlatform = personalPlatform;
    }
}

void ProcessInputPersonal(void) {
    Platform *plat;
    Actor *actor;
    Vec3 oldPos;
    s32 count;
    s32 found;
    s32 i;

    if (pad[4].pressed[BUTTON_C_LEFT] || pad[4].pressed[BUTTON_C_RIGHT]) {
        count = 0;
        found = FALSE;
        for (plat = platformList.head.next; plat != &platformList.head; plat = plat->next) {
            if (!(plat->flags & 0x80)) {
                count++;
            }
            if (plat == personalPlatform) {
                found = TRUE;
            }
        }
        if (count == 0) {
            return;
        }
        oldPos = hand.pos;
        if (pad[4].pressed[BUTTON_C_RIGHT]) {
            if (found) {
                for (plat = platformList.head.next; plat != personalPlatform; plat = plat->next) {
                }
            }
            while (plat == &platformList.head || (plat->flags & 0x80) || (plat == personalPlatform && count >= 2)) {
                plat = plat->next;
            }
        } else if (pad[4].pressed[BUTTON_C_LEFT]) {
            if (found) {
                for (plat = platformList.head.next; plat != personalPlatform; plat = plat->prev) {
                }
            }
            while (plat == &platformList.head || (plat->flags & 0x80) || (plat == personalPlatform && count >= 2)) {
                plat = plat->prev;
            }
        }
        personalPlatform = plat;
        if (hand.state->mode == HAND_JOINED || hand.state->mode == HAND_BOUNCE) {
            actor = &ball;
            PutActorOnPlatform(actor);
        }
        actor = &hand;
        PutActorOnPlatform(actor);
        camera.prevPos = camera.pos;
        cameraInfo.actor.prevPos = cameraInfo.actor.pos;
        SUB_FROM_VECTOR(&camera.pos, &oldPos);
        SUB_FROM_VECTOR(&cameraInfo.actor.pos, &oldPos);
        ADD_TO_VECTOR(&camera.pos, &hand.pos);
        ADD_TO_VECTOR(&cameraInfo.actor.pos, &hand.pos);
    }
    if (pad[4].pressed[BUTTON_SHOULDER_LEFT]) {
        saveSlot[currentSaveSlot].levels = saveSlot[currentSaveSlot].allGaribs = -1;
    }
    if (pad[4].pressed[BUTTON_Z]) {
        timerMode = (timerMode + 1) % 3;
    }
    if (pad[4].button[BUTTON_Z]) {
        if (pad[4].stickX > 15) {
            helper.scale.x += 10.0;
        }
        if (pad[4].stickX < -15) {
            helper.scale.x -= 10.0;
        }
        if (pad[4].stickY > 15) {
            helper.scale.z += 10.0;
        }
        if (pad[4].stickY < -15) {
            helper.scale.z -= 10.0;
        }
        if (pad[4].button[BUTTON_B]) {
            helper.scale.y += 10.0;
        }
        if (pad[4].button[BUTTON_A]) {
            helper.scale.y -= 10.0;
        }
    } else {
        if (pad[4].stickX > 0) {
            helper.pos.x += 10.0;
        }
        if (pad[4].stickX < 0) {
            helper.pos.x -= 10.0;
        }
        if (pad[4].stickY > 0) {
            helper.pos.z -= 10.0;
        }
        if (pad[4].stickY < 0) {
            helper.pos.z += 10.0;
        }
        if (pad[4].button[BUTTON_B]) {
            helper.pos.y += 10.0;
        }
        if (pad[4].button[BUTTON_A]) {
            helper.pos.y -= 10.0;
        }
    }
    helper.scale.x = (helper.scale.x < 10.0f) ? 10.0f : helper.scale.x;
    helper.scale.y = (helper.scale.y < 10.0f) ? 10.0f : helper.scale.y;
    helper.scale.z = (helper.scale.z < 10.0f) ? 10.0f : helper.scale.z;
    if (pad[4].pressed[BUTTON_DPAD_LEFT]) {
        PrepareSong((audioCtrl.current - 1) % 61);
    }
    if (pad[4].button[BUTTON_DPAD_RIGHT]) {
        RotateLight(&backgroundLightList.ambient, 0, 0.0f, 0.2f);
        RotateLight(&backgroundLightList.ambient, 1, 0.0f, 0.2f);
    }
    if (pad[4].pressed[BUTTON_DPAD_RIGHT]) {
        PrepareSong((audioCtrl.current + 1) % 61);
    }
    if (pad[4].button[BUTTON_DPAD_UP]) {
        CreatePuffCircle(&hand.pos);
    }
    if (pad[4].button[BUTTON_DPAD_DOWN]) {
        RotateLight(&backgroundLightList.ambient, 0, -0.2f, 0.0f);
        RotateLight(&backgroundLightList.ambient, 1, -0.2f, 0.0f);
    }
    if ((cheatActivated & 2) && pad[ActiveController].pressed[BUTTON_SHOULDER_LEFT] && !IsHandHoldingBall() && ball.link != &hand) {
        EnemyDropBall();
        ball.pos = hand.pos;
        ZERO_VECTOR(&ball.vel);
        ball.pos.y += ball.radius + 20.0f;
        ball.prevPos = ball.pos;
        ballStatic = 0;
        ball.flags = defaultBehaviour[ballControl.type].flags;
    }
    if (pad[4].pressed[BUTTON_SHOULDER_RIGHT]) {
        if (!inertia) {
            inertia = 1;
            hand.phys->accel /= 3.0f;
        } else {
            inertia = 0;
            hand.phys->accel *= 3.0f;
        }
    }
    actor = &hand;
    if (hand.state->mode == HAND_JOINED) {
        actor = &ball;
    }
    if (pad[4].button[BUTTON_A] && pad[4].pressed[BUTTON_B]) {
        if (autoPlayback == 1) {
            *rkp = -1;
            rkp++;
            sprintf(message, "c:\\n64\\oddballs\\incbins\\records\\%s.moo", levelInfo[gameInfo.level].name);
            for (i = 0; i < strlen(message); i++) {
                if (message[i] == ' ') {
                    message[i] = '_';
                }
            }
            pcFile = PCcreat(message, 0);
            i = (u8 *)rkp - (u8 *)recordKeys;
            if (i % 16) {
                i += 16 - i % 16;
            }
            *recordKeys = i;
            PCwrite(pcFile, recordKeys, i);
            PCclose(pcFile);
            JallocFree(&recordKeys);
            autoPlayback = 0;
        }
        gameInfo.mode = 1;
        numCyclingTextures = 0;
        FreeLevel();
        return;
    }
    if (pad[4].button[BUTTON_C_UP]) {
        actor->vel.y += 2.0f;
        if (actor == &hand) {
            hand.state->mode = HAND_WALK;
        }
    }
    if (pad[4].button[BUTTON_C_DOWN]) {
        actor->vel.y -= 2.0f;
    }
    if (pad[4].pressed[BUTTON_START]) {
        HurtBall();
    }
}

void updateLava(u8 *buf, u32 width, u32 height) {
    u32 size;
    u32 k = 0;
    u32 x;
    u32 y;

    for (y = 0; y < width; y++) {
        size = width * height;
        for (x = 0; x < width; x++, k++) {
            buf[k] = ((u32)(buf[(k + 1) % size] + buf[(k - 1 + size) % size] + buf[(k + height) % size] +
                            buf[(k - height + size) % size]) >> 2) +
                     incval;
        }
    }
}

void ScreenShot(void) {
    u16 *fb = cfb_16_a;
    char name[16];
    u8 line[0x500];
    u8 header[0x36] = {
        0x42, 0x4D, 0x36, 0x84, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00,
        0x40, 0x01, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x84,
        0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    s32 fd;
    s32 x;
    s32 y;
    u8 *p;
    u16 c;
    u8 byte;
    u8 *lp;

    StopDrawing("scchot");
    dontClearScreen = TRUE;
    PRINTF("==================\n");
    PRINTF("Taking screen shot\n");
    sprintf(name, "c:\\n64%04d.bmp", screenshotCount++);
    fd = PCcreat(name, 0);
    if (fd == -1) {
        StartDrawing("scsho");
        PRINTF("FILEERROR:could not open file:\n");
        return;
    }
    PCwrite(fd, header, sizeof(header));
    for (y = 239, lp = line; y >= 0;) {
        for (x = 0, p = lp; x < 320; x++) {
            c = fb[y * 320 + x];
            byte = (c >> 1) << 3;
            *p++ = byte;
            byte = (c >> 6) << 3;
            *p++ = byte;
            byte = (c >> 11) << 3;
            *p++ = byte;
        }
        y--;
        PCwrite(fd, line, 960);
    }
    PCclose(fd);
    dontClearScreen = FALSE;
    StartDrawing("bum");
}

void AdjustColours(u8 *r, u8 *g, u8 *b) {
    u8 rv = *r;
    u8 gv = *g;
    u8 bv = *b;
    f32 tempFloat;
    f32 saturation;

    // building saturation in steps gives the ROM's float registers; the (f32) casts
    // convert through float before double, as the ROM does
    saturation = 1.0f / 255.0f;
    saturation *= rv;
    saturation = 1.0f - saturation;
    tempFloat = (f32)rv / (saturation * 0.086 + 1.0);
    if (tempFloat > 255.0f) {
        tempFloat = 255.0f;
    }
    rv = tempFloat;
    saturation = 1.0f / 255.0f;
    saturation *= gv;
    saturation = 1.0f - saturation;
    tempFloat = saturation * 0.04 * (f32)gv + (f32)gv;
    if (tempFloat > 255.0f) {
        tempFloat = 255.0f;
    }
    gv = tempFloat;
    saturation = 1.0f / 255.0f;
    saturation *= bv;
    saturation = 1.0f - saturation;
    tempFloat = (f32)bv / (saturation * 0.06 + 1.0);
    if (tempFloat > 255.0f) {
        tempFloat = 255.0f;
    }
    bv = tempFloat;
    *r = rv;
    *g = gv;
    *b = bv;
}

inline void SetObjectXlu(Object *obj) {
    obj->flags |= 2;
    if (obj->child != NULL) {
        SetObjectXlu(obj->child);
    }
    if (obj->sibling != NULL) {
        SetObjectXlu(obj->sibling);
    }
}

void SetActorXlu(Actor *actor) {
    SubActor(actor);
    if (actor->inst) {
        SetObjectXlu(actor->inst->obj);
    }
    AddActor(actor);
}

INCLUDE_RODATA("asm/nonmatchings/personal", pad_80108B9C);
