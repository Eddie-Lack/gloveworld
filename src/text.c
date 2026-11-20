#include "common.h"
#include "ident.h"
#include "text.h"
#include "font.h"
#include "jalloc.h"

extern s32 frameCount;
extern s32 frameCount2;
extern Texture *fontTex;
extern Texture *helpFontTex;
extern s16 dontClearText;
extern char letters_index[];
extern char help_letters_index[];
extern u16 letters_offset[];
extern u16 help_letters_offset[];
extern Bitmap letters_bitmaps[];
extern Bitmap help_letters_bitmaps[];

u32 UpdateCRC(char *ptr);
void LoadTextureBank(s32 num);
void FindTexture(Texture **texPtr, u32 texID, s32 verbose);

extern Gfx *glistp;

void SetFont(u8 type) {
    if (type == 0) {
        currentFont = &letters_font;
    } else {
        currentFont = &help_letters_font;
    }
}

void LoadFont(void) {
    s32 len;
    s32 i;

    frameCount2 = 0;
    frameCount = 0;
    LoadTextureBank(FONT_TEX_BANK);
    FindTexture(&fontTex, UpdateCRC("bigfinal.bmp"), 0);
    FindTexture(&helpFontTex, UpdateCRC("helpfnt3.bmp"), 0);
    letters_font.tex = fontTex;
    help_letters_font.tex = helpFontTex;
    len = strlen(letters_index);
    for (i = 0; i < len; i++) {
        letters_bitmaps[i].buf = (void *)(letters_offset[i] + (u32)fontTex->data);
    }
    letters_font.data = fontTex->data;
    len = strlen(help_letters_index);
    for (i = 0; i < len; i++) {
        help_letters_bitmaps[i].buf = (void *)(help_letters_offset[i] + (u32)helpFontTex->data);
    }
    help_letters_font.data = helpFontTex->data;
    currentFont = &letters_font;
}

void InitTextList(void) {
    textList.head.next = textList.head.prev = &textList.head;
    textList.count = 0;
}

s16 GetStringPixelLength(char *str, f32 sx, f32 sy) {
    s16 len;
    s16 i;
    f32 width = 0.0f;
    char *c;

    len = strlen(str);
    for (i = 0; i < len; i++) {
        c = my_strchr(currentFont->index, str[i]);
        if (c == NULL) {
            c = currentFont->index;
        }
        width += currentFont->bitmaps[c - currentFont->index].width;
    }
    width *= sx;
    if (len >= 2) {
        width += len * currentFont->spacing;
    }
    return width;
}

s16 GetStringPixelLengthChar(char *str, s16 idx, f32 sx, f32 sy) {
    f32 width;
    char *c;

    c = my_strchr(currentFont->index, str[idx]);
    if (c == NULL) {
        c = currentFont->index;
    }
    width = currentFont->bitmaps[c - currentFont->index].width;
    width *= sx;
    return width;
}

void AddText(Text *text, s32 centre) {
    Text *ptr;
    f32 width = 0.0f;

    text->font = currentFont;
    if (centre) {
        width = GetStringPixelLength(text->str, text->scaleX, text->scaleY);
        text->x -= width / 2.0f;
        text->y -= text->scaleY * 12.0f;
    }
    dontClearText = TRUE;
    ptr = textList.head.next;
    text->prev = ptr;
    text->next = ptr->next;
    ptr->next->prev = text;
    ptr->next = text;
    dontClearText = FALSE;
    textList.count++;
}

void SubText(Text *text) {
    if (text != NULL && text->next != NULL) {
        text->prev->next = text->next;
        text->next->prev = text->prev;
        textList.count--;
        text->next = NULL;
        JallocFree(&text);
    }
}

void ClearText(void) {
    Text *text;
    Text *text2;

    if (dontClearText) {
        return;
    }
    for (text = textList.head.next; text != &textList.head; text = text2) {
        text2 = text->next;
        if (text->oneShot == 2) {
            SubText(text);
        }
    }
}

void ClearAllText(void) {
    Text *text;
    Text *text2;

    for (text = textList.head.next; text != &textList.head; text = text2) {
        text2 = text->next;
        SubText(text);
    }
}

void ShowText(void) {
    Text *text;

    font_init(&glistp);
    for (text = textList.head.next; text != &textList.head; text = text->next) {
        if (text->oneShot != 2) {
            font_set_pos(text->x - 1, text->y - 1);
            font_set_color(text->r, text->g, text->b, text->a);
            font_set_scale(text->scaleX, text->scaleY);
            font_show_string(&glistp, text->str, text->font);
            if (text->oneShot == 1) {
                text->oneShot = 2;
            }
        }
    }
    font_finish(&glistp);
}

void text_sprite(Sprite *txt, char *str, Font *fnt, s32 xlen, s32 ylen) {
    s32 i;
    s32 ci;
    s32 x;
    s32 y;
    char *indx;
    Bitmap *bm;

    txt->width = xlen * 16;
    txt->height = ylen * 16;
    bm = txt->bitmap;
    i = 0;
    ci = 0;
    for (y = 0; y < ylen; y++) {
        for (x = 0; x < xlen; x++, i++, ci++) {
            if (str[ci] == '\0') {
                bm[i] = fnt->bitmaps[0];
                bm[i].width = -1;
                txt->nbitmaps = i;
                return;
            }
            if (str[ci] == '\n') {
                bm[i] = fnt->bitmaps[0];
                bm[i].buf = NULL;
                ci--;
            } else {
                indx = my_strchr(fnt->index, str[ci]);
                if (indx != NULL) {
                    bm[i] = fnt->bitmaps[indx - fnt->index];
                }
            }
        }
        if (str[ci] == '\n') {
            ci++;
        }
    }
    txt->nbitmaps = i;
}

char *my_strchr(char *s, char c) {
    while (*s != '\0' && *s != c) {
        s++;
    }
    if (*s != '\0') {
        return s;
    }
    return NULL;
}
