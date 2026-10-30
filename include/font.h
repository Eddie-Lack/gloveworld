#ifndef FONT_H
#define FONT_H

#include "common.h"
#include "texture.h"

typedef struct Font {
    /* 0x00 */ char *index;
    /* 0x04 */ Bitmap *bitmaps;
    /* 0x08 */ void *data;
    /* 0x0C */ Texture *tex;
    /* 0x10 */ s32 spacing;
} Font; /* size = 0x14 */

extern Font letters_font;
extern Font help_letters_font;
extern Font *currentFont;

void font_init(Gfx **glistp);
void font_finish(Gfx **glistp);
void font_set_pos(s32 px, s32 py);
void font_set_scale(f64 x, f64 y);
void font_set_color(u8 rr, u8 gg, u8 bb, u8 aa);
void font_show_string(Gfx **glistp, char *str, Font *font);
void KanafyLetterString(char *str);

#endif
