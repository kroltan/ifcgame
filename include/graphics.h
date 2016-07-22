#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "entity.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

typedef enum {
    TEXT_HALIGN_LEFT    = 0,
    TEXT_HALIGN_CENTER  = 1,
    TEXT_HALIGN_RIGHT   = 2,
    TEXT_ALIGN_INTEGER  = 4,
    TEXT_VALIGN_TOP     = 8,
    TEXT_VALIGN_MIDDLE  = 16,
    TEXT_VALIGN_BOTTOM  = 32
} TextFlags;

void coords_use_local(Entity *ent);
void coords_use_global();
void screen_to_world(float *x, float *y);

void draw_sprite(ALLEGRO_BITMAP *bmp, float x, float y, float scale, float angle);
void draw_textf(ALLEGRO_FONT *font, ALLEGRO_COLOR color, float x, float y, float size, TextFlags flags, const char *format, ...);

void camera_set(float x, float y, float size);

#endif // GRAPHICS_H
