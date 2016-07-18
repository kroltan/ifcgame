#ifndef WEAPON_ENTITY_H
#define WEAPON_ENTITY_H

#include "entity.h"

extern const EntityType WeaponEntity;

Entity *weapon_create_for_player(Entity *player_ent);

#endif // WEAPON_ENTITY_H
