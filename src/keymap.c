#include "keymap.h"

#include "config.h"
#include "hashmap.h"
#include "graphics.h"
#include "game_state.h"
#include "keymap_codes.gen.h"

#include <stdio.h>


HashMap *named_keys; /**< key=string, value=NamedKeyCode */
HashMap *keymap; /**< key=string, value=NamedKeyCode */

bool previous_keyboard_states[ALLEGRO_KEY_MAX];
bool current_keyboard_states[ALLEGRO_KEY_MAX];
uint32_t previous_mouse_state;
uint32_t current_mouse_state;

bool _keymap_config_iter(ALLEGRO_CONFIG *config, const char *section, const char *key) {
    const char *keyname = config_get_string(config, section, key, NULL);
    if (!keyname) return true;

    NamedKeyCode *mapping = hashmap_get(named_keys, keyname);
    if (!mapping) return true;

    hashmap_set(keymap, key, mapping);

    return true;
}
void init_keymap() {
    named_keys = hashmap_new_str_key();
    for (size_t i = 0; i < KEYCODE_NAMES_COUNT; ++i) {
        NamedKeyCode *current = &KEYCODE_NAMES[i];
        hashmap_set(named_keys, current->name, current);
    }

    keymap = hashmap_new_str_key();
    config_each_entry(game.config, "Keymap", _keymap_config_iter);
}

void keymap_update() {
    for (size_t i = 0; i < ALLEGRO_KEY_MAX; ++i) {
        previous_keyboard_states[i] = current_keyboard_states[i];
    }
    previous_mouse_state = current_mouse_state;
}

void keymap_on_event(ALLEGRO_EVENT *event) {
    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        current_keyboard_states[event->keyboard.keycode] = true;
    } else if (event->type == ALLEGRO_EVENT_KEY_UP) {
        current_keyboard_states[event->keyboard.keycode] = false;
    } else if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        current_mouse_state |= 1 << event->mouse.button;
    } else if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
        current_mouse_state &= ~(1 << event->mouse.button);
    }
}

int keymap_get(const char *command) {
    int mouse_button;
    if (sscanf(command, "mouse%d", &mouse_button)) {
        return -mouse_button;
    }
    NamedKeyCode *mapping = hashmap_get(keymap, command);
    if (mapping) {
        return mapping->keycode;
    }
    return 0;
}

void get_both_states(int keycode, bool *previous, bool *current) {
    if (keycode < 0) { //is mouse
        int button = 1 << -keycode;
        *previous = (previous_mouse_state & button) != 0;
        *current = (current_mouse_state & button) != 0;
    } else {
        *previous = previous_keyboard_states[keycode];
        *current = current_keyboard_states[keycode];
    }
}

bool keymap_down(const char *command) {
    int keycode = keymap_get(command);
    bool prev, curr;
    get_both_states(keycode, &prev, &curr);
    return curr && !prev;
}

bool keymap_is_held(const char *command) {
    int keycode = keymap_get(command);
    bool prev, curr;
    get_both_states(keycode, &prev, &curr);
    return curr;
}

bool keymap_up(const char *command) {
    int keycode = keymap_get(command);
    bool prev, curr;
    get_both_states(keycode, &prev, &curr);
    return !curr && prev;
}

cpVect keymap_mouse_world() {
    ALLEGRO_MOUSE_STATE mouse;

    al_get_mouse_state(&mouse);
    float x = 2 * (mouse.x / (float) al_get_display_width(game.display) - 0.5);
    float y = -2 * (mouse.y / (float) al_get_display_height(game.display) - 0.5);

    screen_to_world(&x, &y);

    return cpv(x, y);
}
