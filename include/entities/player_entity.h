#ifndef PLAYER_ENTITY_H
#define PLAYER_ENTITY_H

#include "entity.h"

extern const unsigned int PLAYER_DEATH_EVENT_ID;

extern const EntityType PlayerEntity;

Entity *player_new_with_id(uint32_t user_id);
Entity *player_get_for_id(uint32_t user_id);

void player_hurt(uint32_t user_id, int damage, uint32_t inflictor);

#endif // PLAYER_ENTITY_H
