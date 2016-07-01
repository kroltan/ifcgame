#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "config.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <chipmunk/chipmunk.h>

#include <stdbool.h>

typedef struct {
    ALLEGRO_DISPLAY *display;
    ALLEGRO_TIMER *main_timer;
    ALLEGRO_EVENT_QUEUE *event_queue;
    cpSpace *space;
    ALLEGRO_CONFIG *config;
    ALLEGRO_FONT *default_font;
    bool running;
    float last_frame, delta_time;
} GameState;

GameState game;

ALLEGRO_PATH *game_asset_path(const char *asset);

#endif // GAME_STATE_H
