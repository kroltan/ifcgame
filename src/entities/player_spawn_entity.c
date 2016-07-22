#include "entities/player_spawn_entity.h"

#include "console.h"
#include "connection.h"
#include "entities/player_entity.h"

#include <allegro5/allegro.h>
#include <chipmunk/chipmunk.h>

#include <stdlib.h>
#include <stdbool.h>

static const int SPAWN_CHANCE_PARTS = 2;

void player_spawn_init(Entity *ent) {
    cpBodySetType(entity_body(ent), CP_BODY_TYPE_KINEMATIC);
}

void player_spawn_do(Entity *ent, uint32_t user_id) {
    Entity *player = player_new_with_id(user_id);
    cpBodySetPosition(entity_body(player), cpBodyGetPosition(entity_body(ent)));
}

void _pspawn_random(Entity *ent, void *data) {
    (void) data;
    if (rand() % SPAWN_CHANCE_PARTS == 0) {
        player_spawn_do(ent, connection_get_user_id());
    }
}
void player_spawn_on_event(Entity *ent, ALLEGRO_EVENT *event) {
    (void) ent;
    if (event->type == CONSOLE_EVENT_ID) {
        const char *command = (const char *)event->user.data1;
        if (!strcmp(command, "spawn")) {

            entity_all_of_type(&PlayerSpawnEntity, _pspawn_random, NULL);
            player_spawn_do(ent, connection_get_user_id());
        }
    }
}

const EntityType PlayerSpawnEntity = {
    .name = "PlayerSpawnEntity",
    .on_init = player_spawn_init,
    .on_event = player_spawn_on_event
};
