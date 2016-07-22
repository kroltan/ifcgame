#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "graphics.h"

#include <allegro5/allegro.h>

#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    char *buffer;
    size_t max_len;
    size_t buffer_len;
    size_t caret_pos;
    bool focus;

    float font_size;
    float caret_width;
    ALLEGRO_COLOR color;
    TextFlags text_flags;
} TextBox;

void textbox_init(TextBox *box, size_t max_length);

void textbox_clear(TextBox *box);
void textbox_addchar(TextBox *box, size_t pos, char chr);
void textbox_delchar(TextBox *box, size_t pos);

bool textbox_on_event(TextBox *box, const ALLEGRO_EVENT *event);
void textbox_draw(const TextBox *box, float x, float y);

#endif // TEXTBOX_H
