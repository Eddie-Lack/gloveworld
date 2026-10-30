#include "common.h"
#include "ident.h"
#include "debug.h"
#include "actor.h"
#include "game.h"
#include "texture.h"
#include "banks.h"
#include "display.h"
#include "jalloc.h"

extern s32 numCyclingTextures;
extern s32 numoftc;
extern u32 *ballTextureData;
extern u32 *ballTextureData2;
extern u32 *ballTexturePalette;
extern u32 *ballTexturePalette2;
extern u32 *ballBitTextureData;
extern u32 *ballBitTexturePalette;
extern u32 *handShinTextureData[2];
extern u32 *handShinTexturePalette[2];
extern Texture *cyclingTextures[20];
extern u8 codeDrawingRequest;
extern u8 gfxIsDrawing;
extern u8 disableGraphics;
extern u8 onlyDrawBackdrops;
extern u8 recordKeyFilesRomStart[];
extern u8 landscapeFilesRomStart[];
extern u8 landscapeFilesRomEnd[];
extern u8 recordKeyFilesRomEnd[];
extern OSIoMesg dmaIOMessageBuf;
extern OSMesgQueue dmaMessageQ;
extern char D_80106E78[]; // format string followed by linker junk
extern char D_80106864[]; // "Whoops, texture '%X' not found!\n" followed by linker junk

extern u8 titleTexBankRomStart[];
extern u8 genericTexBankRomStart[];
extern u8 atlantisTexBankRomStart[];
extern u8 carnivalTexBankRomStart[];
extern u8 piratesTexBankRomStart[];
extern u8 prehistoricTexBankRomStart[];
extern u8 fortressTexBankRomStart[];
extern u8 ootwTexBankRomStart[];
extern u8 hubTexBankRomStart[];
extern u8 levelEndTexBankRomStart[];
extern u8 fontTexBankRomStart[];
extern u8 cameoTexBankRomStart[];
extern u8 levelEnd2TexBankRomStart[];
extern u8 presentTexBankRomStart[];
extern u8 flythruTexBankRomStart[];
extern u8 ckBonusTexBankRomStart[];
extern u8 ckCheat1TexBankRomStart[];
extern u8 ckCheat2TexBankRomStart[];
extern u8 ckCheat2TexBankRomEnd[];
extern u8 rcDemoTexBankRomStart[];
extern u8 genericObjBankRomStart[];
extern u32 fontTexBankBuffer[];
extern char outputMessageBuffer[];
extern char message[];
extern u8 atlantisSharedObjBankRomStart[];
extern u8 atlantisL1ObjBankRomStart[];
extern u8 atlantisL2ObjBankRomStart[];
extern u8 atlantisL3aObjBankRomStart[];
extern u8 atlantisBossObjBankRomStart[];
extern u8 atlantisBonusObjBankRomStart[];
extern u8 carnivalSharedObjBankRomStart[];
extern u8 carnivalL1ObjBankRomStart[];
extern u8 carnivalL2aObjBankRomStart[];
extern u8 carnivalL3aObjBankRomStart[];
extern u8 carnivalBossObjBankRomStart[];
extern u8 carnivalBonusObjBankRomStart[];
extern u8 piratesSharedObjBankRomStart[];
extern u8 piratesL1ObjBankRomStart[];
extern u8 piratesL2aObjBankRomStart[];
extern u8 piratesBossObjBankRomStart[];
extern u8 piratesBonusObjBankRomStart[];
extern u8 prehistoricSharedObjBankRomStart[];
extern u8 prehistoricL1aObjBankRomStart[];
extern u8 prehistoricL2aObjBankRomStart[];
extern u8 prehistoricBossObjBankRomStart[];
extern u8 prehistoricBonusObjBankRomStart[];
extern u8 fortressSharedObjBankRomStart[];
extern u8 fortressL1aObjBankRomStart[];
extern u8 fortressL2aObjBankRomStart[];
extern u8 fortressL3bObjBankRomStart[];
extern u8 fortressBonusObjBankRomStart[];
extern u8 fortressBossObjBankRomStart[];
extern u8 hubSharedObjBankRomStart[];
extern u8 hubPart1ObjBankRomStart[];
extern u8 piratesL3bObjBankRomStart[];
extern u8 prehistoricL3bObjBankRomStart[];
extern u8 ootwL2aObjBankRomStart[];
extern u8 ootwL2bObjBankRomStart[];
extern u8 ootwL3ObjBankRomStart[];
extern u8 ootwBoss1ObjBankRomStart[];
extern u8 tweenObjBankRomStart[];
extern u8 ootwBonusObjBankRomStart[];
extern u8 ootwSharedObjBankRomStart[];
extern u8 caveObjBankRomStart[];
extern u8 introObjBankRomStart[];
extern u8 wayroomObjBankRomStart[];
extern u8 hubPart2ObjBankRomStart[];
extern u8 hubPart3ObjBankRomStart[];
extern u8 hubPart4ObjBankRomStart[];
extern u8 hubPart5ObjBankRomStart[];
extern u8 hubPart6ObjBankRomStart[];
extern u8 hubPart7ObjBankRomStart[];
extern u8 hubPart8ObjBankRomStart[];
extern u8 assaultCourseObjBankRomStart[];
extern u8 titleObjBankRomStart[];
extern u8 outroObjBankRomStart[];
extern u8 systemObjBankRomStart[];
extern u8 presentationObjBankRomStart[];
extern u8 flythruObjBankRomStart[];
extern Vtx *vtxPtr;
extern s16 xluFact;
extern u8 mirrorTextures;

void InitTextureBanks(void) {
    s32 i;

    for (i = 0; i < 5; i++) {
        textureBanks[i].data = 0;
        textureBanks[i].textures = 0;
        textureBanks[i].count = 0;
    }
    numCyclingTextures = 0;
    for (i = 0; i < 20; i++) {
        cyclingTextures[i] = NULL;
    }
}

void LoadTextureBank(u32 num) {
    u32 *ptr;
    u32 start;
    u32 end;
    s32 len;
    s16 slot = -1;
    s16 i = 0;
    Texture *tex;
    Texture *cur;

    switch (num) {
        case 18:
            start = (u32)rcDemoTexBankRomStart;
            end = (u32)genericObjBankRomStart;
            sprintf(message, "RCDEMO_TEX_BANK");
            break;
        case CKBONUS_TEX_BANK:
            start = (u32)ckBonusTexBankRomStart;
            end = (u32)ckCheat1TexBankRomStart;
            sprintf(message, "CKBONUS_TEX_BANK");
            break;
        case 16:
            start = (u32)ckCheat1TexBankRomStart;
            end = (u32)ckCheat2TexBankRomStart;
            sprintf(message, "CKCHEAT1_TEX_BANK");
            break;
        case 17:
            start = (u32)ckCheat2TexBankRomStart;
            end = (u32)ckCheat2TexBankRomEnd;
            sprintf(message, "CKCHEAT2_TEX_BANK");
            break;
        case FONT_TEX_BANK:
            start = (u32)fontTexBankRomStart;
            end = (u32)cameoTexBankRomStart;
            sprintf(message, "FONT_TEX_BANK");
            slot = 0;
            break;
        case HUB_TEX_BANK:
            start = (u32)hubTexBankRomStart;
            end = (u32)levelEndTexBankRomStart;
            sprintf(message, "HUB_TEX_BANK");
            break;
        case CAMEO_TEX_BANK:
            start = (u32)cameoTexBankRomStart;
            end = (u32)levelEnd2TexBankRomStart;
            sprintf(message, "CAMEO_TEX_BANK");
            break;
        case TITLE_TEX_BANK:
            start = (u32)titleTexBankRomStart;
            end = (u32)genericTexBankRomStart;
            sprintf(message, "TITLE_TEX_BANK");
            break;
        case LEVEND1_TEX_BANK:
            start = (u32)levelEndTexBankRomStart;
            end = (u32)fontTexBankRomStart;
            sprintf(message, "LEVELEND_TEX_BANK");
            break;
        case LEVEND2_TEX_BANK:
            start = (u32)levelEnd2TexBankRomStart;
            end = (u32)presentTexBankRomStart;
            sprintf(message, "LEVELEND_TEX_BANK");
            break;
        case GENERIC_TEX_BANK:
            start = (u32)genericTexBankRomStart;
            end = (u32)atlantisTexBankRomStart;
            sprintf(message, "GENERIC_TEX_BANK");
            break;
        case ATLANTIS_TEX_BANK:
            start = (u32)atlantisTexBankRomStart;
            end = (u32)carnivalTexBankRomStart;
            sprintf(message, "ATLANTIS_TEX_BANK");
            break;
        case CARNIVAL_TEX_BANK:
            start = (u32)carnivalTexBankRomStart;
            end = (u32)piratesTexBankRomStart;
            sprintf(message, "CARNIVAL_TEX_BANK");
            break;
        case PIRATES_TEX_BANK:
            start = (u32)piratesTexBankRomStart;
            end = (u32)prehistoricTexBankRomStart;
            sprintf(message, "PIRATES_TEX_BANK");
            break;
        case PREHISTORIC_TEX_BANK:
            start = (u32)prehistoricTexBankRomStart;
            end = (u32)fortressTexBankRomStart;
            sprintf(message, "PREHISTORIC_TEX_BANK");
            break;
        case FORTRESS_TEX_BANK:
            start = (u32)fortressTexBankRomStart;
            end = (u32)ootwTexBankRomStart;
            sprintf(message, "FORTRESS_TEX_BANK");
            break;
        case OOTW_TEX_BANK:
            start = (u32)ootwTexBankRomStart;
            end = (u32)hubTexBankRomStart;
            sprintf(message, "OOTW_TEX_BANK");
            break;
        case PRESENT_TEX_BANK:
            start = (u32)presentTexBankRomStart;
            end = (u32)flythruTexBankRomStart;
            sprintf(message, "PRESENT_TEX_BANK");
            break;
        case FLYTHRU_TEX_BANK:
            start = (u32)flythruTexBankRomStart;
            end = (u32)ckBonusTexBankRomStart;
            sprintf(message, "FLYTHRU_TEX_BANK");
            break;
        default:
            PRINTF("Invalid texture bank!\n");
            Crash("LOAD TEXTURE BANK");
            break;
    }
    len = end - start;
    if (num != FONT_TEX_BANK) {
        ptr = JallocAlloc(DMAGetSize(start, end), 0, "TexBank");
    } else {
        ptr = fontTexBankBuffer;
    }
    DMAMemory(ptr, start, end);
    if (ptr != NULL) {
        PRINTF("Loaded texture bank %s [%d]\n", message, len);
    } else {
        PRINTF("Unable to load texture bank %s\n", message);
        return;
    }
    if (num != FONT_TEX_BANK) {
        while (textureBanks[++slot].data) {
        }
        if (slot >= 5) {
            PRINTF("ERROR:too many texture banks downloaded\n");
            return;
        }
    }
    textureBanks[slot].data = (s32)ptr;
    textureBanks[slot].count = *ptr;
    ptr += 1;
    textureBanks[slot].textures = (s32)ptr;
    cur = (Texture *)textureBanks[slot].textures;
    for (i = 0; i < textureBanks[slot].count; i++) {
        tex = cur;
        tex->data = (u8 *)tex + sizeof(Texture);
        if (tex->palette) {
            tex->palette = (s32 *)((u32)tex->palette + (u32)tex);
        }
        cur = (Texture *)((u8 *)cur + tex->size);
        if (tex->flags & 4) {
            cyclingTextures[numCyclingTextures++] = tex;
        }
    }
}

void FindTexture(Texture **texPtr, u32 texID, s32 verbose) {
    s32 i;
    s32 j;
    Texture *tex;
    u8 *p;

    for (i = 4; i >= 0; i--) {
        if (textureBanks[i].data) {
            p = (u8 *)textureBanks[i].textures;
            for (j = 0; j < textureBanks[i].count; j++) {
                tex = (Texture *)p;
                if (tex->crc == texID) {
                    *texPtr = tex;
                    return;
                }
                p += tex->size;
            }
        }
    }
    *texPtr = NULL;
    if (verbose) {
        PRINTF(D_80106864, texID);
    }
    return; // forces the original jump to the epilogue
}

void CycleTextures(void) {
    s32 i;
    s32 j;
    Texture *tex;
    u16 *pal;
    u16 tmp;

    for (i = 0; i < numCyclingTextures; i++) {
        tex = cyclingTextures[i];
        pal = (u16 *)tex->palette;
        tex->cycleCount += (tex->cycleSpeed >= 0) ? tex->cycleSpeed : -tex->cycleSpeed;
        while (tex->cycleCount >= 64) {
            if (tex->cycleSpeed > 0) {
                tmp = pal[tex->cycleStart];
                for (j = tex->cycleStart; j < tex->cycleEnd; j++) {
                    pal[j] = pal[j + 1];
                }
                pal[tex->cycleEnd] = tmp;
                tex->cycleCount -= 64;
            } else {
                tmp = pal[tex->cycleEnd];
                for (j = tex->cycleEnd; j > tex->cycleStart; j--) {
                    pal[j] = pal[j - 1];
                }
                pal[tex->cycleStart] = tmp;
                tex->cycleCount -= 64;
            }
        }
    }
}

s32 Rom2Ram(s32 addr, s32 base) {
    s32 ram;

    ram = addr & 0xFFFFFF;
    ram += base;
    return ram;
}

void RestoreDrawList(u8 *list, s32 base, Object *obj) {
    u8 *dl = list;
    s32 addr;
    u32 *w;
    Texture *tex;

    while (*dl != 0xB8) {
        switch (*dl) {
            case 0x04:
                w = (u32 *)(dl + 4);
                addr = *w;
                addr = Rom2Ram(addr, base);
                *w = addr;
                dl += 8;
                break;
            case 0xFD:
                if (dl[8] == 0xF5) {
                    w = (u32 *)(dl + 4);
                    addr = *w;
                    FindTexture(&tex, addr, 1);
                    if (tex != NULL) {
                        *w = (u32)tex->data;
                    }
                    dl += 0x38;
                if (obj->name[0] == 'g' && obj->name[1] == 'b' && obj->name[2] == 'a' && obj->name[3] == 'l' && obj->name[4] == 'l' && obj->name[5] == 0) {
                    ballTextureData = w;
                } else if (obj->name[0] == 'c' && obj->name[1] == 'b' && obj->name[2] == 'a' && obj->name[3] == 'l' && obj->name[4] == 'l' && obj->name[5] == 0) {
                    ballTextureData2 = w;
                } else if (obj->name[0] == 'G' && obj->name[1] == 'S' && obj->name[2] == 'h' && obj->name[3] == 'i' && obj->name[4] == 'n' && obj->name[5] == 'L' && obj->name[6] == 0) {
                    handShinTextureData[0] = w;
                } else if (obj->name[0] == 'G' && obj->name[1] == 'S' && obj->name[2] == 'h' && obj->name[3] == 'i' && obj->name[4] == 'n' && obj->name[5] == 'R' && obj->name[6] == 0) {
                    handShinTextureData[1] = w;
                } else if (ballBitTextureData == NULL && obj->name[0] == 'g' && obj->name[1] == 'b' && obj->name[2] == 'l' && obj->name[3] == '_' && obj->name[4] == 'b' && obj->name[5] == 't' && obj->name[6] == 0) {
                    ballBitTextureData = w;
                }
                } else if (dl[8] == 0xE8) {
                    w = (u32 *)(dl + 4);
                    addr = *w;
                    FindTexture(&tex, addr, 1);
                    if (tex != NULL) {
                        *w = (u32)tex->palette;
                    }
                    dl += 0x30;
                if (obj->name[0] == 'g' && obj->name[1] == 'b' && obj->name[2] == 'a' && obj->name[3] == 'l' && obj->name[4] == 'l' && obj->name[5] == 0) {
                    ballTexturePalette = w;
                } else if (obj->name[0] == 'c' && obj->name[1] == 'b' && obj->name[2] == 'a' && obj->name[3] == 'l' && obj->name[4] == 'l' && obj->name[5] == 0) {
                    ballTexturePalette2 = w;
                } else if (obj->name[0] == 'G' && obj->name[1] == 'S' && obj->name[2] == 'h' && obj->name[3] == 'i' && obj->name[4] == 'n' && obj->name[5] == 'L' && obj->name[6] == 0) {
                    handShinTexturePalette[0] = w;
                } else if (obj->name[0] == 'G' && obj->name[1] == 'S' && obj->name[2] == 'h' && obj->name[3] == 'i' && obj->name[4] == 'n' && obj->name[5] == 'R' && obj->name[6] == 0) {
                    handShinTexturePalette[1] = w;
                } else if (ballBitTexturePalette == NULL && obj->name[0] == 'g' && obj->name[1] == 'b' && obj->name[2] == 'l' && obj->name[3] == '_' && obj->name[4] == 'b' && obj->name[5] == 't' && obj->name[6] == 0) {
                    ballBitTexturePalette = w;
                }
                } else {
                    dl += 0x10;
                }
                break;
            default:
                dl += 8;
                break;
        }
    }
}

INCLUDE_RODATA("asm/nonmatchings/banks", D_80106864);

INCLUDE_RODATA("asm/nonmatchings/banks", ident_80106890);

void RestoreObjectPointers(Object *obj, s32 base) {
    void *p;
    s32 i;
    char name[16];

    p = obj->mesh;
    if (p != NULL) {
        obj->mesh = (Mesh *)Rom2Ram((s32)obj->mesh, base);
        obj->mesh->verts = (Vec3 *)Rom2Ram((s32)obj->mesh->verts, base);
        if (obj->mesh->numFaces) {
            obj->mesh->faces = (FaceIndices *)Rom2Ram((s32)obj->mesh->faces, base);
            if (obj->mesh->faceColours) {
                obj->mesh->faceColours = (void *)Rom2Ram((s32)obj->mesh->faceColours, base);
            }
            if (obj->mesh->uv) {
                obj->mesh->uv = (u16(*)[2])Rom2Ram((s32)obj->mesh->uv, base);
                sprintf(name, "OFTC%d", numoftc++);
                obj->mesh->origUV = JallocAlloc(obj->mesh->numFaces * 12, 1, name);
                memcpy(obj->mesh->origUV, obj->mesh->uv, obj->mesh->numFaces * 12);
            }
            if (obj->mesh->faceFlags) {
                obj->mesh->faceFlags = (u8 *)Rom2Ram((s32)obj->mesh->faceFlags, base);
            }
            if (obj->mesh->textures) {
                obj->mesh->textures = (Texture **)Rom2Ram((s32)obj->mesh->textures, base);
                for (i = 0; i < obj->mesh->numFaces; i++) {
                    FindTexture(&obj->mesh->textures[i], (u32)obj->mesh->textures[i], 1);
                }
            }
        }
        if (obj->mesh->colours) {
            obj->mesh->colours = (u8 *)Rom2Ram((s32)obj->mesh->colours, base);
        }
    }
    p = obj->sprites;
    if (p != NULL) {
        obj->sprites = (ObjectSprite *)Rom2Ram((s32)obj->sprites, base);
        for (i = 0; i < obj->numSprites; i++) {
            FindTexture(&obj->sprites[i].tex, (u32)obj->sprites[i].tex, 1);
        }
    }
    p = obj->displayList;
    if (p != NULL) {
        obj->displayList = (void *)Rom2Ram((s32)obj->displayList, base);
    }
    p = obj->scaleKeys;
    if (p != NULL) {
        obj->scaleKeys = (void *)Rom2Ram((s32)obj->scaleKeys, base);
    }
    p = obj->posKeys;
    if (p != NULL) {
        obj->posKeys = (void *)Rom2Ram((s32)obj->posKeys, base);
    }
    p = obj->rotKeys;
    if (p != NULL) {
        obj->rotKeys = (void *)Rom2Ram((s32)obj->rotKeys, base);
    }
    p = obj->child;
    if (p != NULL) {
        obj->child = (Object *)Rom2Ram((s32)obj->child, base);
    }
    p = obj->sibling;
    if (p != NULL) {
        obj->sibling = (Object *)Rom2Ram((s32)obj->sibling, base);
    }
    if (obj->displayList) {
        RestoreDrawList(obj->displayList, base, obj);
    }
    if (obj->child) {
        RestoreObjectPointers(obj->child, base);
    }
    if (obj->sibling) {
        RestoreObjectPointers(obj->sibling, base);
    }
}

void LoadObjectBank(u32 num) {
    s32 ptr;
    u32 start;
    u32 end;
    s32 len;
    s16 slot = -1;
    s16 i = 0;
    s32 rom;
    ObjectInstance *inst;

    switch (num) {
        case 71:
            start = (u32)tweenObjBankRomStart;
            end = (u32)ootwBonusObjBankRomStart;
            sprintf(message, "TWEEN");
            break;
        case 0:
            start = (u32)systemObjBankRomStart;
            end = (u32)presentationObjBankRomStart;
            sprintf(message, "SYSTEM");
            break;
        case 67:
            start = (u32)wayroomObjBankRomStart;
            end = (u32)hubPart2ObjBankRomStart;
            sprintf(message, "WAYROOM");
            break;
        case 66:
            start = (u32)outroObjBankRomStart;
            end = (u32)systemObjBankRomStart;
            sprintf(message, "OUTRO");
            break;
        case 68:
            start = (u32)presentationObjBankRomStart;
            end = (u32)flythruObjBankRomStart;
            sprintf(message, "PRESENTATION");
            break;
        case 69:
            start = (u32)flythruObjBankRomStart;
            end = (u32)landscapeFilesRomStart;
            sprintf(message, "FLYTHRU");
            break;
        case 65:
            start = (u32)introObjBankRomStart;
            end = (u32)wayroomObjBankRomStart;
            sprintf(message, "INTRO");
            break;
        case 64:
            start = (u32)hubSharedObjBankRomStart;
            end = (u32)hubPart1ObjBankRomStart;
            sprintf(message, "HUB_SHARED");
            break;
        case 2:
            start = (u32)hubPart1ObjBankRomStart;
            end = (u32)piratesL3bObjBankRomStart;
            sprintf(message, "HUB_PART1");
            break;
        case 3:
            start = (u32)hubPart2ObjBankRomStart;
            end = (u32)hubPart3ObjBankRomStart;
            sprintf(message, "HUB_PART2");
            break;
        case 4:
            start = (u32)hubPart3ObjBankRomStart;
            end = (u32)hubPart4ObjBankRomStart;
            sprintf(message, "HUB_PART3");
            break;
        case 5:
            start = (u32)hubPart4ObjBankRomStart;
            end = (u32)hubPart5ObjBankRomStart;
            sprintf(message, "HUB_PART4");
            break;
        case 6:
            start = (u32)hubPart5ObjBankRomStart;
            end = (u32)hubPart6ObjBankRomStart;
            sprintf(message, "HUB_PART5");
            break;
        case 7:
            start = (u32)hubPart6ObjBankRomStart;
            end = (u32)hubPart7ObjBankRomStart;
            sprintf(message, "HUB_PART6");
            break;
        case 8:
            start = (u32)hubPart7ObjBankRomStart;
            end = (u32)hubPart8ObjBankRomStart;
            sprintf(message, "HUB_PART7");
            break;
        case 9:
            start = (u32)hubPart8ObjBankRomStart;
            end = (u32)assaultCourseObjBankRomStart;
            sprintf(message, "HUB_PART8");
            break;
        case 10:
            start = (u32)caveObjBankRomStart;
            end = (u32)introObjBankRomStart;
            sprintf(message, "CAVE");
            break;
        case 11:
            start = (u32)assaultCourseObjBankRomStart;
            end = (u32)titleObjBankRomStart;
            sprintf(message, "ASSAULT COURSE");
            break;
        case 1:
            start = (u32)genericObjBankRomStart;
            end = (u32)atlantisSharedObjBankRomStart;
            sprintf(message, "GENERIC");
            break;
        case 12:
            start = (u32)atlantisSharedObjBankRomStart;
            end = (u32)atlantisL1ObjBankRomStart;
            sprintf(message, "ATLANTIS_SHARED");
            break;
        case 13:
            start = (u32)atlantisL1ObjBankRomStart;
            end = (u32)atlantisL2ObjBankRomStart;
            sprintf(message, "ATLANTIS_L1");
            break;
        case 14:
            start = (u32)atlantisL2ObjBankRomStart;
            end = (u32)atlantisL3aObjBankRomStart;
            sprintf(message, "ATLANTIS_L2");
            break;
        case 15:
            start = (u32)atlantisL3aObjBankRomStart;
            end = (u32)atlantisBossObjBankRomStart;
            sprintf(message, "ATLANTIS_L3A");
            break;
        case 17:
            start = (u32)atlantisBossObjBankRomStart;
            end = (u32)atlantisBonusObjBankRomStart;
            sprintf(message, "ATLANTIS_BOSS");
            break;
        case 18:
            start = (u32)atlantisBonusObjBankRomStart;
            end = (u32)carnivalSharedObjBankRomStart;
            sprintf(message, "ATLANTIS_BONUS");
            break;
        case 19:
            start = (u32)carnivalSharedObjBankRomStart;
            end = (u32)carnivalL1ObjBankRomStart;
            sprintf(message, "CARNIVAL_SHARED");
            break;
        case 20:
            start = (u32)carnivalL1ObjBankRomStart;
            end = (u32)carnivalL2aObjBankRomStart;
            sprintf(message, "CARNIVAL_L1");
            break;
        case 21:
            start = (u32)carnivalL2aObjBankRomStart;
            end = (u32)carnivalL3aObjBankRomStart;
            sprintf(message, "CARNIVAL_L2A");
            break;
        case 23:
            start = (u32)carnivalL3aObjBankRomStart;
            end = (u32)carnivalBossObjBankRomStart;
            sprintf(message, "CARNIVAL_L3A");
            break;
        case 25:
            start = (u32)carnivalBossObjBankRomStart;
            end = (u32)carnivalBonusObjBankRomStart;
            sprintf(message, "CARNIVAL_BOSS");
            break;
        case 26:
            start = (u32)carnivalBonusObjBankRomStart;
            end = (u32)piratesSharedObjBankRomStart;
            sprintf(message, "CARNIVAL_BONUS");
            break;
        case 38:
            start = (u32)piratesSharedObjBankRomStart;
            end = (u32)piratesL1ObjBankRomStart;
            sprintf(message, "PIRATES_SHARED");
            break;
        case 39:
            start = (u32)piratesL1ObjBankRomStart;
            end = (u32)piratesL2aObjBankRomStart;
            sprintf(message, "PIRATES_L1");
            break;
        case 40:
            start = (u32)piratesL2aObjBankRomStart;
            end = (u32)piratesBossObjBankRomStart;
            sprintf(message, "PIRATES_L2A");
            break;
        case 43:
            start = (u32)piratesL3bObjBankRomStart;
            end = (u32)prehistoricL3bObjBankRomStart;
            sprintf(message, "PIRATES_L3B");
            break;
        case 44:
            start = (u32)piratesBossObjBankRomStart;
            end = (u32)piratesBonusObjBankRomStart;
            sprintf(message, "PIRATES_BOSS");
            break;
        case 45:
            start = (u32)piratesBonusObjBankRomStart;
            end = (u32)prehistoricSharedObjBankRomStart;
            sprintf(message, "PIRATES_BONUS");
            break;
        case 46:
            start = (u32)prehistoricSharedObjBankRomStart;
            end = (u32)prehistoricL1aObjBankRomStart;
            sprintf(message, "PREHISTORIC_SHARED");
            break;
        case 47:
            start = (u32)prehistoricL1aObjBankRomStart;
            end = (u32)prehistoricL2aObjBankRomStart;
            sprintf(message, "PREHISTORIC_L1A");
            break;
        case 49:
            start = (u32)prehistoricL2aObjBankRomStart;
            end = (u32)prehistoricBossObjBankRomStart;
            sprintf(message, "PREHISTORIC_L2A");
            break;
        case 52:
            start = (u32)prehistoricL3bObjBankRomStart;
            end = (u32)ootwL2aObjBankRomStart;
            sprintf(message, "PREHISTORIC_L3B");
            break;
        case 53:
            start = (u32)prehistoricBossObjBankRomStart;
            end = (u32)prehistoricBonusObjBankRomStart;
            sprintf(message, "PREHISTORIC_BOSS");
            break;
        case 54:
            start = (u32)prehistoricBonusObjBankRomStart;
            end = (u32)fortressSharedObjBankRomStart;
            sprintf(message, "PREHISTORIC_BONUS");
            break;
        case 55:
            start = (u32)fortressSharedObjBankRomStart;
            end = (u32)fortressL1aObjBankRomStart;
            sprintf(message, "FORTRESS_SHARED");
            break;
        case 56:
            start = (u32)fortressL1aObjBankRomStart;
            end = (u32)fortressL2aObjBankRomStart;
            sprintf(message, "FORTRESS_L1A");
            break;
        case 58:
            start = (u32)fortressL2aObjBankRomStart;
            end = (u32)fortressL3bObjBankRomStart;
            sprintf(message, "FORTRESS_L2A");
            break;
        case 61:
            start = (u32)fortressL3bObjBankRomStart;
            end = (u32)fortressBonusObjBankRomStart;
            sprintf(message, "FORTRESS_L3B");
            break;
        case 63:
            start = (u32)fortressBonusObjBankRomStart;
            end = (u32)fortressBossObjBankRomStart;
            sprintf(message, "FORTRESS_BONUS");
            break;
        case 62:
            start = (u32)fortressBossObjBankRomStart;
            end = (u32)hubSharedObjBankRomStart;
            sprintf(message, "FORTRESS_BOSS");
            break;
        case 27:
            start = (u32)ootwSharedObjBankRomStart;
            end = (u32)caveObjBankRomStart;
            sprintf(message, "OOTW_SHARED");
            break;
        case 29:
            start = (u32)ootwL2aObjBankRomStart;
            end = (u32)ootwL2bObjBankRomStart;
            sprintf(message, "OOTW_L2A");
            break;
        case 30:
            start = (u32)ootwL2bObjBankRomStart;
            end = (u32)ootwL3ObjBankRomStart;
            sprintf(message, "OOTW_L2B");
            break;
        case 31:
            start = (u32)ootwL3ObjBankRomStart;
            end = (u32)ootwBoss1ObjBankRomStart;
            sprintf(message, "OOTW_L3");
            break;
        case 33:
            start = (u32)ootwBoss1ObjBankRomStart;
            end = (u32)tweenObjBankRomStart;
            sprintf(message, "OOTW_BOSS1");
            break;
        case 35:
            start = (u32)ootwBonusObjBankRomStart;
            end = (u32)ootwSharedObjBankRomStart;
            sprintf(message, "OOTW_BONUS");
            break;
        case 70:
            start = (u32)titleObjBankRomStart;
            end = (u32)outroObjBankRomStart;
            sprintf(message, "TITLE_BANK");
            break;
        default:
            PRINTF("ERROR: no object bank specified....\n");
            for (;;) {
            }
            break;
    }
    len = end - start;
    ptr = (s32)JallocAlloc(DMAGetSize(start, end), 0, "ObjBank");
    DMAMemory((void *)ptr, start, end);
    if (ptr != 0) {
        PRINTF("Loaded object bank %s (size %d)\n", message, len);
    } else {
        PRINTF("Unable to load object bank %s\n", message);
        return;
    }
    while (objectBanks[++slot].data) {
    }
    if (slot >= 5) {
        PRINTF("ERROR:too many object banks\n");
        return;
    }
    objectBanks[slot].data = (void *)ptr;
    objectBanks[slot].entries = (ObjectBankEntry *)ptr;
    for (i = 0; objectBanks[slot].entries[i].inst; i++) {
        rom = (s32)objectBanks[slot].entries[i].inst;
        objectBanks[slot].entries[i].inst = (ObjectInstance *)Rom2Ram(rom, ptr);
        inst = objectBanks[slot].entries[i].inst;
        inst->obj = (Object *)Rom2Ram((s32)inst->obj, ptr);
        if (inst->anim) {
            inst->anim = (AnimState *)Rom2Ram((s32)inst->anim, ptr);
            if (inst->anim->anims) {
                inst->anim->anims = (struct AnimInfo *)Rom2Ram((s32)inst->anim->anims, ptr);
            }
        }
        RestoreObjectPointers(inst->obj, ptr);
    }
    objectBanks[slot].count = i;
    if (num == 0) {
        InitFadeOverlay();
    }
}

f32 CalcRadius(Mesh *mesh) {
    s32 i;
    f32 radius;
    f32 r;
    Vec3 *v;

    radius = 0;
    for (i = 0; i < mesh->numVerts; i++) {
        v = &mesh->verts[i];
        r = LENGTH(v);
        if (r > radius) {
            radius = r;
        }
    }
    return radius;
}

void FindObject(ObjectInstance **objCPtr, u32 objID, char *name, s32 vital) {
    s32 x;
    s32 y;
    s32 bank;

    for (x = 0; x < 5; x++) {
        if (objectBanks[x].data) {
            bank = objectBanks[x].unk00;
            for (y = 0; y < objectBanks[x].count; y++) {
                if (objectBanks[x].entries[y].crc == objID) {
                    *objCPtr = objectBanks[x].entries[y].inst;
                    return;
                }
            }
        }
    }
    *objCPtr = NULL;
    if (name) {
        if (vital) {
            PRINTF("Object '%s' not found\n", name);
            sprintf(gameInfo.message, "LEVEL UNPLAYABLE - %s MISSING", name);
        }
    } else {
        PRINTF("Whoops, object '%X' not found!\n", objID);
        if (vital) {
            sprintf(gameInfo.message, "LEVEL UNPLAYABLE - OBJECT MISSING");
        }
    }
    if (vital) {
        StopDrawing("fondobj");
        gameInfo.mode = 13;
    }
    return; // forces the original jump to the epilogue
}

Object *FindSubObjectFromObject(Object *obj, u32 objID) {
    Object *o = NULL;

    o = obj;
    if (o->crc == objID) {
        return o;
    }
    if (obj->child) {
        o = FindSubObjectFromObject(obj->child, objID);
        if (o && o->crc == objID) {
            return o;
        }
    }
    if (obj->sibling) {
        o = FindSubObjectFromObject(obj->sibling, objID);
        if (o && o->crc == objID) {
            return o;
        }
    }
    return NULL;
}

void FindSubObject(Object **out, u32 crc) {
    s32 i;
    s32 j;
    Object *obj;
    Object *found;

    for (i = 0; i < 5; i++) {
        if (objectBanks[i].data) {
            for (j = 0; j < objectBanks[i].count; j++) {
                obj = objectBanks[i].entries[j].inst->obj;
                found = FindSubObjectFromObject(obj, crc);
                if (found) {
                    *out = found;
                    return;
                }
            }
        }
    }
    *out = NULL;
    PRINTF("Whoops, object '%X' not found!\n", crc);
}

void *LoadLandscapeFile(s32 num) {
    u32 rom = (u32)landscapeFilesRomStart;
    void *buf;
    s32 i;
    char *fn; // unused, but takes the original stack slot
    s32 size;

    for (i = 0; i < num; i++) {
        osInvalDCache(&size, 4);
        osWritebackDCache(&size, 4);
        osPiStartDma(&dmaIOMessageBuf, 0, 0, rom, &size, 4, &dmaMessageQ);
        osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
        rom += size;
        if (rom >= (u32)landscapeFilesRomEnd) {
            StopDrawing("loadlandscape");
            gameInfo.mode = 13;
            sprintf(gameInfo.message, "LEVEL UNPLAYABLE - NO LEV FILE");
            return NULL;
        }
    }
    osInvalDCache(&size, 4);
    osWritebackDCache(&size, 4);
    osPiStartDma(&dmaIOMessageBuf, 0, 0, rom, &size, 4, &dmaMessageQ);
    osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
    buf = JallocAlloc(DMAGetSize(rom, rom + size), 1, "LandData");
    DMAMemory(buf, rom, rom + size);
    return buf;
}

void *LoadRecordKeyFile(s32 num) {
    u32 rom = (u32)recordKeyFilesRomStart;
    void *data;
    s32 i;
    s32 pad; // forces the frame layout
    s32 size;

    for (i = 0; i < num; i++) {
        osInvalDCache(&size, 4);
        osWritebackDCache(&size, 4);
        osPiStartDma(&dmaIOMessageBuf, 0, 0, rom, &size, 4, &dmaMessageQ);
        osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
        rom += size;
        if (rom >= (u32)recordKeyFilesRomEnd) {
            StopDrawing("loadrecord");
            gameInfo.mode = 13;
            sprintf(gameInfo.message, "LEVEL UNPLAYABLE - NO MOO FILE");
            return NULL;
        }
    }
    osInvalDCache(&size, 4);
    osWritebackDCache(&size, 4);
    osPiStartDma(&dmaIOMessageBuf, 0, 0, rom, &size, 4, &dmaMessageQ);
    osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
    rom += 4;
    data = JallocAlloc(size, 1, "RecKey");
    DMAMemory(data, rom, rom + size);
    return data;
}

void WriteObjectDisplayListFlat(Object *obj) {
    s32 i;
    s32 j;
    s32 face = 0;
    s32 k;
    Mesh *mesh = obj->mesh;
    u16 (*uv)[2] = mesh->uv;
    Vec3 *verts = mesh->verts;
    Vec3 *v;
    FaceIndices *faces = mesh->faces;
    FaceIndices *f;
    Vtx *start;
    s32 groups = mesh->numFaces / 10;
    s32 rem = mesh->numFaces % 10;
    s32 n;
    Texture *tex = NULL;
    MeshColour *col;
    MeshColour *colours = mesh->faceColours;

    gDPPipeSync(glistp++);
    for (i = 0; i < groups; i++) {
        start = vtxPtr;
        for (j = 0; j < 10; j++) {
            f = &faces[face];
            col = &colours[face];
            v = &verts[f->v[0]];
            vtxPtr[0].v.ob[0] = v->x;
            vtxPtr[0].v.ob[1] = v->y;
            vtxPtr[0].v.ob[2] = v->z;
            vtxPtr[0].v.flag = 0;
            vtxPtr[0].v.tc[0] = (*uv)[0];
            vtxPtr[0].v.tc[1] = (*uv++)[1];
            vtxPtr[0].v.cn[0] = col->r;
            vtxPtr[0].v.cn[1] = col->g;
            vtxPtr[0].v.cn[2] = col->b;
            vtxPtr[0].v.cn[3] = xluFact;
            v = &verts[f->v[1]];
            vtxPtr[1].v.ob[0] = v->x;
            vtxPtr[1].v.ob[1] = v->y;
            vtxPtr[1].v.ob[2] = v->z;
            vtxPtr[1].v.flag = 0;
            vtxPtr[1].v.tc[0] = (*uv)[0];
            vtxPtr[1].v.tc[1] = (*uv++)[1];
            vtxPtr[1].v.cn[0] = col->r;
            vtxPtr[1].v.cn[1] = col->g;
            vtxPtr[1].v.cn[2] = col->b;
            vtxPtr[1].v.cn[3] = xluFact;
            v = &verts[f->v[2]];
            vtxPtr[2].v.ob[0] = v->x;
            vtxPtr[2].v.ob[1] = v->y;
            vtxPtr[2].v.ob[2] = v->z;
            vtxPtr[2].v.flag = 0;
            vtxPtr[2].v.tc[0] = (*uv)[0];
            vtxPtr[2].v.tc[1] = (*uv++)[1];
            vtxPtr[2].v.cn[0] = col->r;
            vtxPtr[2].v.cn[1] = col->g;
            vtxPtr[2].v.cn[2] = col->b;
            vtxPtr[2].v.cn[3] = xluFact;
            face++;
            vtxPtr += 3;
        }
        gSPVertex(glistp++, start, 30, 0);
        for (k = 0; k < 10; k++) {
            if (tex != obj->mesh->textures[face - 10 + k]) {
                tex = obj->mesh->textures[face - 10 + k];
                gDPPipeSync(glistp++);
                LoadTexture(tex);
            }
            gSP1Triangle(glistp++, k * 3, k * 3 + 1, k * 3 + 2, 2);
        }
    }
    start = vtxPtr;
    for (j = 0; j < rem; j++) {
        f = &faces[face];
        col = &colours[face];
        v = &verts[f->v[0]];
        vtxPtr[0].v.ob[0] = v->x;
        vtxPtr[0].v.ob[1] = v->y;
        vtxPtr[0].v.ob[2] = v->z;
        vtxPtr[0].v.flag = 0;
        vtxPtr[0].v.tc[0] = (*uv)[0];
        vtxPtr[0].v.tc[1] = (*uv++)[1];
        vtxPtr[0].v.cn[0] = col->r;
        vtxPtr[0].v.cn[1] = col->g;
        vtxPtr[0].v.cn[2] = col->b;
        vtxPtr[0].v.cn[3] = xluFact;
        v = &verts[f->v[1]];
        vtxPtr[1].v.ob[0] = v->x;
        vtxPtr[1].v.ob[1] = v->y;
        vtxPtr[1].v.ob[2] = v->z;
        vtxPtr[1].v.flag = 0;
        vtxPtr[1].v.tc[0] = (*uv)[0];
        vtxPtr[1].v.tc[1] = (*uv++)[1];
        vtxPtr[1].v.cn[0] = col->r;
        vtxPtr[1].v.cn[1] = col->g;
        vtxPtr[1].v.cn[2] = col->b;
        vtxPtr[1].v.cn[3] = xluFact;
        v = &verts[f->v[2]];
        vtxPtr[2].v.ob[0] = v->x;
        vtxPtr[2].v.ob[1] = v->y;
        vtxPtr[2].v.ob[2] = v->z;
        vtxPtr[2].v.flag = 0;
        vtxPtr[2].v.tc[0] = (*uv)[0];
        vtxPtr[2].v.tc[1] = (*uv++)[1];
        vtxPtr[2].v.cn[0] = col->r;
        vtxPtr[2].v.cn[1] = col->g;
        vtxPtr[2].v.cn[2] = col->b;
        vtxPtr[2].v.cn[3] = xluFact;
        face++;
        vtxPtr += 3;
    }
    if (rem) {
        face -= rem;
        gSPVertex(glistp++, start, rem * 3, 0);
        n = 0;
        for (i = 0; i < rem; i++) {
            if (tex != obj->mesh->textures[face]) {
                tex = obj->mesh->textures[face];
                gDPPipeSync(glistp++);
                LoadTexture(tex);
            }
            gSP1Triangle(glistp++, n++, n++, n++, 0);
            face++;
        }
    }
}

void WriteObjectDisplayListGouraud(Object *obj) {
    s32 i;
    s32 j;
    s32 face = 0;
    s32 k;
    Mesh *mesh = obj->mesh;
    u16 (*uv)[2] = mesh->uv;
    Vec3 *verts = mesh->verts;
    Vec3 *v;
    FaceIndices *faces = mesh->faces;
    FaceIndices *f;
    Vtx *start;
    s32 groups = mesh->numFaces / 10;
    s32 rem = mesh->numFaces % 10;
    s32 n;
    Texture *tex = NULL;
    MeshColour *col;
    MeshColour *colours = (MeshColour *)mesh->colours;

    gDPPipeSync(glistp++);
    for (i = 0; i < groups; i++) {
        start = vtxPtr;
        for (j = 0; j < 10; j++) {
            f = &faces[face];
            v = &verts[f->v[0]];
            col = &colours[f->v[0]];
            vtxPtr[0].v.ob[0] = v->x;
            vtxPtr[0].v.ob[1] = v->y;
            vtxPtr[0].v.ob[2] = v->z;
            vtxPtr[0].v.flag = 0;
            vtxPtr[0].v.tc[0] = (*uv)[0];
            vtxPtr[0].v.tc[1] = (*uv++)[1];
            vtxPtr[0].v.cn[0] = col->r;
            vtxPtr[0].v.cn[1] = col->g;
            vtxPtr[0].v.cn[2] = col->b;
            vtxPtr[0].v.cn[3] = xluFact;
            v = &verts[f->v[1]];
            col = &colours[f->v[1]];
            vtxPtr[1].v.ob[0] = v->x;
            vtxPtr[1].v.ob[1] = v->y;
            vtxPtr[1].v.ob[2] = v->z;
            vtxPtr[1].v.flag = 0;
            vtxPtr[1].v.tc[0] = (*uv)[0];
            vtxPtr[1].v.tc[1] = (*uv++)[1];
            vtxPtr[1].v.cn[0] = col->r;
            vtxPtr[1].v.cn[1] = col->g;
            vtxPtr[1].v.cn[2] = col->b;
            vtxPtr[1].v.cn[3] = xluFact;
            v = &verts[f->v[2]];
            col = &colours[f->v[2]];
            vtxPtr[2].v.ob[0] = v->x;
            vtxPtr[2].v.ob[1] = v->y;
            vtxPtr[2].v.ob[2] = v->z;
            vtxPtr[2].v.flag = 0;
            vtxPtr[2].v.tc[0] = (*uv)[0];
            vtxPtr[2].v.tc[1] = (*uv++)[1];
            vtxPtr[2].v.cn[0] = col->r;
            vtxPtr[2].v.cn[1] = col->g;
            vtxPtr[2].v.cn[2] = col->b;
            vtxPtr[2].v.cn[3] = xluFact;
            face++;
            vtxPtr += 3;
        }
        gSPVertex(glistp++, start, 30, 0);
        gDPPipeSync(glistp++);
        for (k = 0; k < 10; k++) {
            if (tex != obj->mesh->textures[face - 10 + k]) {
                tex = obj->mesh->textures[face - 10 + k];
                gDPPipeSync(glistp++);
                LoadTexture(tex);
            }
            gSP1Triangle(glistp++, k * 3, k * 3 + 1, k * 3 + 2, 2);
        }
    }
    start = vtxPtr;
    for (j = 0; j < rem; j++) {
        f = &faces[face];
        v = &verts[f->v[0]];
        col = &colours[f->v[0]];
        vtxPtr[0].v.ob[0] = v->x;
        vtxPtr[0].v.ob[1] = v->y;
        vtxPtr[0].v.ob[2] = v->z;
        vtxPtr[0].v.flag = 0;
        vtxPtr[0].v.tc[0] = (*uv)[0];
        vtxPtr[0].v.tc[1] = (*uv++)[1];
        vtxPtr[0].v.cn[0] = col->r;
        vtxPtr[0].v.cn[1] = col->g;
        vtxPtr[0].v.cn[2] = col->b;
        vtxPtr[0].v.cn[3] = xluFact;
        v = &verts[f->v[1]];
        col = &colours[f->v[1]];
        vtxPtr[1].v.ob[0] = v->x;
        vtxPtr[1].v.ob[1] = v->y;
        vtxPtr[1].v.ob[2] = v->z;
        vtxPtr[1].v.flag = 0;
        vtxPtr[1].v.tc[0] = (*uv)[0];
        vtxPtr[1].v.tc[1] = (*uv++)[1];
        vtxPtr[1].v.cn[0] = col->r;
        vtxPtr[1].v.cn[1] = col->g;
        vtxPtr[1].v.cn[2] = col->b;
        vtxPtr[1].v.cn[3] = xluFact;
        v = &verts[f->v[2]];
        col = &colours[f->v[2]];
        vtxPtr[2].v.ob[0] = v->x;
        vtxPtr[2].v.ob[1] = v->y;
        vtxPtr[2].v.ob[2] = v->z;
        vtxPtr[2].v.flag = 0;
        vtxPtr[2].v.tc[0] = (*uv)[0];
        vtxPtr[2].v.tc[1] = (*uv++)[1];
        vtxPtr[2].v.cn[0] = col->r;
        vtxPtr[2].v.cn[1] = col->g;
        vtxPtr[2].v.cn[2] = col->b;
        vtxPtr[2].v.cn[3] = xluFact;
        face++;
        vtxPtr += 3;
    }
    if (rem) {
        face -= rem;
        gSPVertex(glistp++, start, rem * 3, 0);
        n = 0;
        for (i = 0; i < rem; i++) {
            if (tex != obj->mesh->textures[face]) {
                tex = obj->mesh->textures[face];
                gDPPipeSync(glistp++);
                LoadTexture(tex);
            }
            gSP1Triangle(glistp++, n++, n++, n++, 0);
            face++;
        }
    }
    gDPPipeSync(glistp++);
}

void LoadTexture(Texture *tex) {
    if (tex == NULL) {
        return;
    }
    switch ((s16)tex->siz) {
        case G_IM_SIZ_16b:
            gDPSetTextureLUT(glistp++, G_TT_NONE);
            if (mirrorTextures) {
                gDPLoadTextureBlock(glistp++, tex->data, (s16)tex->fmt, G_IM_SIZ_16b, tex->width, tex->height, 0,
                                    G_TX_MIRROR, G_TX_MIRROR, tex->maskS, tex->maskT, G_TX_NOLOD, G_TX_NOLOD);
            } else {
                gDPLoadTextureBlock(glistp++, tex->data, (s16)tex->fmt, G_IM_SIZ_16b, tex->width, tex->height, 0,
                                    G_TX_WRAP, G_TX_WRAP, tex->maskS, tex->maskT, G_TX_NOLOD, G_TX_NOLOD);
            }
            break;
        case G_IM_SIZ_8b:
            gDPSetTextureLUT(glistp++, G_TT_RGBA16);
            gDPLoadTLUT_pal256(glistp++, tex->palette);
            if (mirrorTextures) {
                gDPLoadTextureBlock(glistp++, tex->data, (s16)tex->fmt, G_IM_SIZ_8b, tex->width, tex->height, 0,
                                    G_TX_MIRROR, G_TX_MIRROR, tex->maskS, tex->maskT, G_TX_NOLOD, G_TX_NOLOD);
            } else {
                gDPLoadTextureBlock(glistp++, tex->data, (s16)tex->fmt, G_IM_SIZ_8b, tex->width, tex->height, 0,
                                    G_TX_WRAP, G_TX_WRAP, tex->maskS, tex->maskT, G_TX_NOLOD, G_TX_NOLOD);
            }
            break;
        case G_IM_SIZ_4b:
            gDPSetTextureLUT(glistp++, G_TT_RGBA16);
            gDPLoadTLUT_pal16(glistp++, 0, tex->palette);
            if (mirrorTextures) {
                gDPLoadTextureBlock_4b(glistp++, tex->data, (s16)tex->fmt, tex->width, tex->height, 0,
                                       G_TX_MIRROR, G_TX_MIRROR, tex->maskS, tex->maskT, G_TX_NOLOD, G_TX_NOLOD);
            } else {
                gDPLoadTextureBlock_4b(glistp++, tex->data, (s16)tex->fmt, tex->width, tex->height, 0,
                                       G_TX_WRAP, G_TX_WRAP, tex->maskS, tex->maskT, G_TX_NOLOD, G_TX_NOLOD);
            }
            break;
    }
}

void StopDrawing(char *name) {
    if (codeDrawingRequest == TRUE) {
        PRINTF("Stop Drawing Requested from '%s'\n", name);
    }
    codeDrawingRequest = FALSE;
    while (gfxIsDrawing == TRUE) {
    }
    disableGraphics = TRUE;
}

void StartDrawing(char *name) {
    if (!codeDrawingRequest) {
        PRINTF(D_80106E78, name);
    }
    codeDrawingRequest = TRUE;
    while (!gfxIsDrawing) {
    }
    disableGraphics = FALSE;
    onlyDrawBackdrops = FALSE;
}

INCLUDE_ASM("asm/nonmatchings/banks", pad_80155B84);

INCLUDE_RODATA("asm/nonmatchings/banks", D_80106E78);
