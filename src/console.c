#include "console.h"

#include "game_state.h"
#include "graphics.h"
#include "textbox.h"
#include "scene.h"

#include <allegro5/allegro_primitives.h>

#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <math.h>

static const size_t BUFFER_SIZE = 512;

const unsigned int CONSOLE_EVENT_ID = 1919191;


TextBox console;
ALLEGRO_EVENT_SOURCE console_event_source;

void init_console() {
    textbox_init(&console, BUFFER_SIZE);
    console.text_flags = TEXT_VALIGN_BOTTOM;
    console.font_size = 16;
    al_init_user_event_source(&console_event_source);
    al_register_event_source(game.event_queue, &console_event_source);
}


void _free_console_event(ALLEGRO_USER_EVENT *event_data) {
    free((char *)event_data->data1);
}

bool console_on_event(ALLEGRO_EVENT *event) {
    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        switch (event->keyboard.keycode) {
            case ALLEGRO_KEY_F1:
                console.focus = !console.focus;
                break;
            case ALLEGRO_KEY_ENTER: {
                console_run(console.buffer);
                textbox_clear(&console);
                break;
            }
        }
    }

    return textbox_on_event(&console, event);
}

void console_draw() {
    if (console.focus) {
        textbox_draw(&console, 0, 0);
    }
}

void console_run(const char *command) {
    char *command_alloc = malloc(BUFFER_SIZE);
    memcpy(command_alloc, command, BUFFER_SIZE);
    ALLEGRO_EVENT new_event;
    new_event.type = CONSOLE_EVENT_ID;
    new_event.user.data1 = (intptr_t) command_alloc;
    al_emit_user_event(&console_event_source, &new_event, _free_console_event);
}
