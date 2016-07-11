#include "player_entity.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <chipmunk/chipmunk.h>

void player_draw(Entity *ent) {
    al_draw_circle(0, 0, 1, al_map_rgb(255, 255, 255), 0);
}

void player_event(Entity *ent, ALLEGRO_EVENT *ev) {
    if (ev->type == ALLEGRO_EVENT_KEY_DOWN) {
        cpBodySetVelocity(entity_body(ent), cpv(1, 0));
        entity_flag_dirty(ent, true);
    }
}

const EntityType PlayerEntity = {
    .name = "PlayerEntity",
    .on_draw = player_draw,
    .on_event = player_event
};
