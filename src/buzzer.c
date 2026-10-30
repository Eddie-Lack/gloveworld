#include "common.h"
#include "buzzer.h"
#include "actor.h"
#include "maths.h"
#include "platform.h"
#include "light.h"
#include "sound.h"
#include "game.h"
#include "overlay.h"
#include "macros.h"
#include "ball.h"

extern s32 buzzOffset;
extern s32 frameCount;

s32 CheckActorBuzzed(Actor *actor, Vec3 *pos1, Vec3 *pos2);
void HurtHand(s32 type, s32 damage);

void InitBuzzerList(void) {
    buzzerList.head.next = buzzerList.head.prev = &buzzerList.head;
    buzzerList.count = 0;
}

void AddBuzzer(Buzzer *buzzer) {
    Buzzer *ptr = buzzerList.head.next;

    buzzer->prev = ptr;
    buzzer->next = ptr->next;
    ptr->next->prev = buzzer;
    ptr->next = buzzer;
    buzzerList.count++;
}

void UpdateBuzzers(void) {
    Buzzer *buzzer;

    buzzOffset = 0;
    for (buzzer = buzzerList.head.next; buzzer != &buzzerList.head; buzzer = buzzer->next) {
        if (buzzer->active) {
            UpdateBuzzer(buzzer);
        }
        buzzOffset += 60;
    }
}

void UpdateBuzzer(Buzzer *buzzer) {
    Vec3 tempVect;
    Vec3 tempVect2;
    Vec3 d;
    s16 r;
    s16 g;
    s16 b;
    s32 numSegs;
    s16 i;

    if (buzzer->timer > 0 && --buzzer->timer == 0) {
        if (buzzer->buzzing) {
            StopContinuousSample(&buzzer->sample);
            buzzer->buzzing = 0;
            buzzer->timer = buzzer->onTime[buzzer->index];
        } else {
            PlayContinuousSample(&buzzer->sample, 60, 60, &buzzer->mid, 128);
            buzzer->buzzing = 1;
            buzzer->timer = buzzer->offTime[buzzer->index];
            buzzer->index = (buzzer->index + 1) % buzzer->numTimes;
        }
    }
    if (buzzer->buzzing && frameCount) {
        if (buzzer->plat1 != NULL) {
            RotateVectorByQuaternion(&tempVect, &buzzer->p1, &buzzer->plat1->actor.rotQuat);
            ADD_TO_VECTOR(&tempVect, &buzzer->plat1->actor.pos);
        } else {
            tempVect = buzzer->p1;
        }
        if (buzzer->plat2 != NULL) {
            RotateVectorByQuaternion(&tempVect2, &buzzer->p2, &buzzer->plat2->actor.rotQuat);
            ADD_TO_VECTOR(&tempVect2, &buzzer->plat2->actor.pos);
        } else {
            tempVect2 = buzzer->p2;
        }
        SUB_VECTORS(&d, &tempVect, &tempVect2);
        numSegs = MIN(MAX(5, LENGTH(&d) / 10.0), 13);
        for (i = 0; i < 2; i++) {
            if ((buzzer->flags & 0x10) && Random(10) < 3) {
                r = g = b = 255;
            } else if (buzzer->a) {
                r = buzzer->r + Random(buzzer->a * 2 + 1) - buzzer->a;
                g = buzzer->g + Random(buzzer->a * 2 + 1) - buzzer->a;
                b = buzzer->b + Random(buzzer->a * 2 + 1) - buzzer->a;
                r = MIN(MAX(r, 0), 255);
                g = MIN(MAX(g, 0), 255);
                b = MIN(MAX(b, 0), 255);
            } else {
                r = buzzer->r;
                g = buzzer->g;
                b = buzzer->b;
            }
            AddLightning(&tempVect, &tempVect2, r, g, b, numSegs, 1, buzzer->flags, buzzer->jitter, buzzer->width);
        }
        if ((frameCount & 1) && (buzzer->flags & 0x40)) {
            gameInfo.fade = 235;
            if (r == 255 && g == 255 && b == 255) {
                SetFadeColour(0, 0, 0);
            } else {
                SetFadeColour(215, 215, 255);
            }
        }
        if (CheckActorBuzzed(&hand, &tempVect, &tempVect2)) {
            HurtHand(HURT_GO_BACKWARDS, 0);
        }
        if (CheckActorBuzzed(&ball, &tempVect, &tempVect2)) {
            BurstBall();
        }
        ADD_VECTORS(&buzzer->mid, &tempVect, &tempVect2);
        SCALE_VECTOR(&buzzer->mid, 0.5);
        UpdateContinuousSample(&buzzer->sample);
        buzzer->sample.pitch = SineWave(30.0f, frameCount, buzzOffset) * 10.0f + 128.0f;
    }
}

s32 CheckActorBuzzed(Actor *actor, Vec3 *pos1, Vec3 *pos2) {
    Vec3 d;
    Vec3 c;
    f32 len;
    f32 t;

    SUB_VECTORS(&d, pos2, pos1);
    len = LENGTH(&d);
    if (len == 0.0f) {
        return FALSE;
    }
    SCALE_VECTOR(&d, 1.0f / len);
    t = DOT_PRODUCT(&d, &actor->pos) - DOT_PRODUCT(pos1, &d);
    if (t < 0.0f || t > len) {
        return FALSE;
    }
    AddOneScaledVector(&c, pos1, &d, t);
    SUB_VECTORS(&d, &actor->pos, &c);
    return DOT_PRODUCT(&d, &d) < actor->radius * actor->radius;
}
