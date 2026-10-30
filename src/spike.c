#include "common.h"
#include "ident.h"
#include "spike.h"
#include "jalloc.h"
#include "maths.h"
#include "actor.h"
#include "ball.h"
#include "enemy.h"

extern s32 SPIKE_RADIUS;

void HurtHand(s32 type, s32 damage);

INCLUDE_ASM("asm/nonmatchings/spike", pad_8018E37C);

void UpdateSpikes(void) {
    Spike *spike;
    Actor *actPtr;
    Vec3 spikePos;

    for (spike = spikeList.head.next; spike != &spikeList.head; spike = spike->next) {
        spikePos = spike->platform->actor.pos;
        for (actPtr = actorList.head.next; actPtr != &actorList.head; actPtr = actPtr->next) {
            if (actPtr == &hand) {
                if ((LENGTH_2D(&hand.vel) > 1.6 || !hand.obj->onGround)
                    && DistanceBetweenPoints(&actPtr->pos, &spikePos) < hand.obj->spheres[0].radius + 1.0 + SPIKE_RADIUS) {
                    HurtHand(HURT_GO_BACKWARDS, 1);
                }
            } else if (actPtr == &ball) {
                if (ballControl.state != BALL_BURST && DistanceBetweenPoints(&actPtr->pos, &spikePos) < ball.radius + 1.0 + SPIKE_RADIUS) {
                    BurstBall();
                }
            } else if (actPtr->type == BUGLE && DistanceBetweenPoints(&actPtr->pos, &spikePos) < actPtr->obj->spheres[0].radius + 4.0 + SPIKE_RADIUS) {
                KillEnemy(&enemies[actPtr->state->index]);
            }
        }
    }
}

void InitSpikeList(void) {
    spikeList.count = 0;
    spikeList.head.next = spikeList.head.prev = &spikeList.head;
}

void AddSpike(Spike *spike) {
    if (spike->next == NULL) {
        spikeList.count++;
        spike->prev = &spikeList.head;
        spike->next = spikeList.head.next;
        spikeList.head.next->prev = spike;
        spikeList.head.next = spike;
    }
}

INCLUDE_ASM("asm/nonmatchings/spike", pad_8018E658);

INCLUDE_RODATA("asm/nonmatchings/spike", pad_8010980C);
