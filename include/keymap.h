#ifndef KEYMAP_H
#define KEYMAP_H

#include <allegro5/allegro5.h>
#include <chipmunk/chipmunk.h>

#include <stdlib.h>
#include <stdbool.h>

void init_keymap();

void keymap_update();
void keymap_on_event(ALLEGRO_EVENT *event);

int keymap_get(const char *command);

bool keymap_down(const char *command);
bool keymap_is_held(const char *command);
bool keymap_up(const char *command);

cpVect keymap_mouse_world();

#endif // KEYMAP_H
