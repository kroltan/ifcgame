#ifndef SCENE_H
#define SCENE_H

#include <allegro5/allegro.h>

#include <stdbool.h>
#include <stdio.h>

bool scene_load(const char *filename);

void scene_on_event(ALLEGRO_EVENT *event);

#endif // SCENE_H
