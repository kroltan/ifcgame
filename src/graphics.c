#include "graphics.h"

#include "game_state.h"

#include <chipmunk/chipmunk.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

void coords_use_local(Entity *ent) {
    cpBody *body = entity_body(ent);
    cpVect position = cpBodyGetPosition(body);
    float rotation = cpBodyGetAngle(body);

    ALLEGRO_TRANSFORM local;
    al_identity_transform(&local);
    al_build_transform(&local,
                       position.x, position.y,
                       1.0, 1.0, rotation);
    al_use_transform(&local);
}

void coords_use_global() {
    ALLEGRO_TRANSFORM global;
    al_identity_transform(&global);
    al_use_transform(&global);
}

void screen_to_world(float *x, float *y) {
    ALLEGRO_TRANSFORM t;
    al_identity_transform(&t);
    al_compose_transform(&t, al_get_current_projection_transform());
    al_invert_transform(&t);
    al_transform_coordinates(&t, x, y);
}

void draw_sprite(ALLEGRO_BITMAP *bmp, float x, float y, float scale) {
    ALLEGRO_TRANSFORM t, current;
    al_copy_transform(&current, al_get_current_transform());
    al_identity_transform(&t);
    al_scale_transform(&t, scale, scale);
    al_compose_transform(&t, &current);
    al_use_transform(&t);

    float width = al_get_bitmap_width(bmp);
    float height = al_get_bitmap_height(bmp);
    float aspect = width / height;

    al_draw_scaled_bitmap(bmp, 0, 0,
                          al_get_bitmap_width(bmp),
                          al_get_bitmap_height(bmp),
                          x - 0.5, y - 0.5, aspect, 1,
                          ALLEGRO_FLIP_VERTICAL);
    al_use_transform(&current);
}

void draw_textf(ALLEGRO_FONT *font, ALLEGRO_COLOR color, float x, float y, float size, TextFlags flags, const char *format, ...) {
    static const int MASK = TEXT_HALIGN_LEFT
                            | TEXT_HALIGN_CENTER
                            | TEXT_HALIGN_RIGHT
                            | TEXT_ALIGN_INTEGER;

    const int line_height = al_get_font_line_height(font);
    const float scale = size / line_height;

    if (flags & TEXT_VALIGN_TOP) {
        y += 1;
    } else if (flags & TEXT_VALIGN_MIDDLE) {
        y += 0.5;
    }

    ALLEGRO_USTR *text;

    va_list arguments;
    va_start(arguments, format);
    text = al_ustr_new("");
    al_ustr_vappendf(text, format, arguments);
    va_end(arguments);


    ALLEGRO_TRANSFORM t, current;
    al_copy_transform(&current, al_get_current_transform());

    al_identity_transform(&t);
    al_scale_transform(&t, scale, -scale);
    al_translate_transform(&t, x, y);
    al_compose_transform(&t, &current);

    al_use_transform(&t);
    al_draw_ustr(font, color, 0, 0, flags & MASK, text);
    al_use_transform(&current);

    al_ustr_free(text);
}

void camera_set(float x, float y, float size) {
    const float aspect_ratio = al_get_display_width(game.display) / (float)al_get_display_height(game.display);
    const float h_width = size * aspect_ratio / 2;
    const float h_height = size / 2;

    ALLEGRO_TRANSFORM projection;
    al_identity_transform(&projection);
    al_orthographic_transform(&projection,
                              x - h_width, y + h_height, -1,
                              x + h_width, y - h_height, 1);
    al_use_projection_transform(&projection);
}
