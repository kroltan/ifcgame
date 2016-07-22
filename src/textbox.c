#include "textbox.h"

#include "game_state.h"
#include "graphics.h"

#include <allegro5/allegro_primitives.h>

#include <ctype.h>

void textbox_init(TextBox *box, size_t max_length) {
    box->buffer = malloc(max_length);
    memset(box->buffer, 0, max_length);
    box->max_len = max_length;
    box->buffer_len = 0;
    box->caret_pos = 0;
    box->focus = false;

    box->font_size = 1;
    box->caret_width = al_get_text_width(game.default_font, "|");
    box->color = al_map_rgb(255, 255, 255);
    box->text_flags = 0;
}

void textbox_clear(TextBox *box) {
    box->buffer[0] = '\0';
    box->buffer_len = 0;
    box->caret_pos = 0;
}

void textbox_addchar(TextBox *box, size_t pos, char chr) {
    if (box->buffer_len + 1 >= box->max_len) {
        return;
    }
    char *src = box->buffer + pos;
    size_t len = box->buffer_len - pos + 1;
    memmove(src + 1, src, len);
    box->buffer[pos] = chr;
    box->buffer_len++;
    box->caret_pos++;
}

void textbox_delchar(TextBox *box, size_t pos) {
    if (box->buffer_len - 1 < pos) {
        return;
    }
    char *dest = box->buffer + pos;
    size_t len = box->buffer_len - pos + 1;
    memmove(dest, dest + 1, len);
    if (pos < box->caret_pos) {
        box->caret_pos--;
    }
    box->buffer_len--;
    box->buffer[box->buffer_len] = '\0';
}

bool textbox_on_event(TextBox *box, const ALLEGRO_EVENT *event) {
    if (box->focus && event->type == ALLEGRO_EVENT_KEY_CHAR) {
        if (event->keyboard.unichar > 0 && event->keyboard.unichar <= 127 && isprint(event->keyboard.unichar)) {
            char chr = event->keyboard.unichar;
            textbox_addchar(box, box->caret_pos, chr);
        } else {
            switch (event->keyboard.keycode) {
            case ALLEGRO_KEY_BACKSPACE: {
                    if (box->caret_pos == 0) {
                        break;
                    }
                    textbox_delchar(box, box->caret_pos - 1);
                    break;
                }
                case ALLEGRO_KEY_DELETE: {
                    textbox_delchar(box, box->caret_pos);
                    break;
                }
                case ALLEGRO_KEY_LEFT: {
                    if (box->caret_pos > 0) {
                        box->caret_pos--;
                    }
                    break;
                }
                case ALLEGRO_KEY_RIGHT: {
                    if (box->caret_pos < box->buffer_len) {
                        box->caret_pos++;
                    }
                    break;
                }
                case ALLEGRO_KEY_HOME: {
                    box->caret_pos = 0;
                    break;
                }
                case ALLEGRO_KEY_END: {
                    box->caret_pos = box->buffer_len;
                    break;
                }
            }
        }
    }
    return box->focus && (
        event->type == ALLEGRO_EVENT_KEY_CHAR
        || event->type == ALLEGRO_EVENT_KEY_UP
        || event->type == ALLEGRO_EVENT_KEY_DOWN
    );
}

void textbox_draw(const TextBox *box, float x, float y) {
    const float scale = box->font_size / al_get_font_line_height(game.default_font);

    ALLEGRO_TRANSFORM t, current;
    al_copy_transform(&current, al_get_current_transform());
    al_identity_transform(&t);
    al_translate_transform(&t, x, y);
    al_compose_transform(&t, al_get_current_transform());
    al_use_transform(&t);

    if (box->focus) {
        //get position of caret
        char under = box->buffer[box->caret_pos];
        box->buffer[box->caret_pos] = '\0';
        float pos = scale * al_get_text_width(game.default_font, box->buffer);
        box->buffer[box->caret_pos] = under;

        //blink caret with 1s period
        if (fmod(al_get_time(), 1.0) > 0.5) {
            al_draw_filled_rectangle(pos, 0, pos + box->caret_width, -box->font_size, al_map_rgb(255, 255, 255));
        }
    }

    draw_textf(game.default_font, al_map_rgb(255, 255, 255), 0, 0, box->font_size, TEXT_VALIGN_BOTTOM, "%s", box->buffer);

    al_use_transform(&current);
}
