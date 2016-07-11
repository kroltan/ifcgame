#ifndef SCENE_H
#define SCENE_H

#include <allegro5/allegro.h>

#include <stdbool.h>
#include <stdio.h>

bool scene_load(const char *filename);

bool scene_save(const char *filename);
bool scene_savef(ALLEGRO_FILE *fp);

#endif // SCENE_H
