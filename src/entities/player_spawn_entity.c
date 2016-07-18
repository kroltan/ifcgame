#include "entities/player_spawn_entity.h"

#include "console.h"
#include "connection.h"
#include "entities/player_entity.h"

#include <allegro5/allegro.h>
#include <chipmunk/chipmunk.h>

void player_spawn_init(Entity *ent) {
    cpBodySetType(entity_body(ent), CP_BODY_TYPE_KINEMATIC);
}

void player_spawn_on_event(Entity *ent, ALLEGRO_EVENT *event) {
    (void) ent;
    if (event->type == CONSOLE_EVENT_ID) {
        const char *command = (const char *)event->user.data1;
        if (!strcmp(command, "spawn")) {
            player_new_with_id(connection_get_user_id());
        }
    }
}

const EntityType PlayerSpawnEntity = {
    .name = "PlayerSpawnEntity",
    .on_init = player_spawn_init,
    .on_event = player_spawn_on_event
};
