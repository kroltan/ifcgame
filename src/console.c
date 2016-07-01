#include "console.h"

#include "game_state.h"
#include "graphics.h"

#include "connection.h"

#include <allegro5/allegro_primitives.h>

#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <math.h>

#define BUFFER_SIZE 512

typedef struct {
    char buffer[BUFFER_SIZE];
    size_t buffer_len;
    size_t caret_pos;
    bool open;
} ConsoleState;

ConsoleState console;

void init_console() {
    memset(console.buffer, 0, BUFFER_SIZE);
    console.buffer_len = 0;
    console.caret_pos = 0;
    console.open = false;
}

static void console_clear() {
    console.buffer[0] = '\0';
    console.buffer_len = 0;
    console.caret_pos = 0;
}

static void console_addchar(size_t pos, char chr) {
    if (console.buffer_len + 1 >= BUFFER_SIZE) {
        return;
    }
    char *src = console.buffer + pos;
    size_t len = console.buffer_len - pos + 1;
    memmove(src + 1, src, len);
    console.buffer[pos] = chr;
    console.buffer_len++;
    console.caret_pos++;
}

static void console_delchar(size_t pos) {
    if (console.buffer_len - 1 < pos) {
        return;
    }
    char *dest = console.buffer + pos;
    size_t len = console.buffer_len - pos + 1;
    memmove(dest, dest + 1, len);
    if (pos < console.caret_pos) {
        console.caret_pos--;
    }
    console.buffer_len--;
    console.buffer[console.buffer_len] = '\0';
}

bool console_on_event(ALLEGRO_EVENT *event) {
    if (event->type == ALLEGRO_EVENT_KEY_DOWN && event->keyboard.keycode == ALLEGRO_KEY_F1) {
        console.open = !console.open;
    }
    if (console.open && event->type == ALLEGRO_EVENT_KEY_CHAR) {
        if (event->keyboard.unichar > 0 && event->keyboard.unichar <= 127 && isprint(event->keyboard.unichar)) {
            char chr = event->keyboard.unichar;
            console_addchar(console.caret_pos, chr);
        } else {
            switch (event->keyboard.keycode) {
            case ALLEGRO_KEY_BACKSPACE: {
                    if (console.caret_pos == 0) {
                        break;
                    }
                    console_delchar(console.caret_pos - 1);
                    break;
                }
                case ALLEGRO_KEY_DELETE: {
                    console_delchar(console.caret_pos);
                    break;
                }
                case ALLEGRO_KEY_LEFT: {
                    if (console.caret_pos > 0) {
                        console.caret_pos--;
                    }
                    break;
                }
                case ALLEGRO_KEY_RIGHT: {
                    if (console.caret_pos < console.buffer_len) {
                        console.caret_pos++;
                    }
                    break;
                }
                case ALLEGRO_KEY_HOME: {
                    console.caret_pos = 0;
                    break;
                }
                case ALLEGRO_KEY_END: {
                    console.caret_pos = console.buffer_len;
                    break;
                }
                case ALLEGRO_KEY_ENTER: {
                    char hostname[128];
                    unsigned int port, max_players;
                    if (sscanf(console.buffer, "connect %128s %u", hostname, &port) == 5) {
                        connection_join(hostname, port);
                    } else if (sscanf(console.buffer, "host %u %u", &port, &max_players) == 2) {
                        connection_host(port, max_players);
                        printf("created\n");
                    }
                    printf("%s\n", console.buffer);
                    console_clear();
                    break;
                }
            }
        }
    }

    return !console.open;
}

void console_draw() {
    if (console.open) {
        static const float font_size = 16;
        static const char *prompt = "> ";
        const float scale = font_size / al_get_font_line_height(game.default_font);

        //get position of caret
        char under = console.buffer[console.caret_pos];
        console.buffer[console.caret_pos] = '\0';
        float pos = scale * (al_get_text_width(game.default_font, console.buffer) + al_get_text_width(game.default_font, prompt));
        console.buffer[console.caret_pos] = under;

        //blink caret with 1s period
        if (fmod(al_get_time(), 1.0) > 0.5) {
            al_draw_filled_rectangle(pos, 0, pos + 2, -font_size, al_map_rgb(255, 255, 255));
        }
        draw_textf(game.default_font, al_map_rgb(255, 255, 255), 0, 0, font_size, TEXT_VALIGN_BOTTOM, "%s%s", prompt, console.buffer);
    }
}