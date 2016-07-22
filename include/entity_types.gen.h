#ifndef ENTITY_TYPES_GEN_H
#define ENTITY_TYPES_GEN_H

#include <entities/weapon_entity.h>
#include <entities/player_spawn_entity.h>
#include <entities/menu_entity.h>
#include <entities/configurable_entity.h>
#include <entities/player_entity.h>

#include <stdlib.h>

const EntityType *ALL_ENTITY_TYPES[] = {
	&WeaponEntity,
	&PlayerSpawnEntity,
	&MenuEntity,
	&ConfigurableEntity,
	&PlayerEntity,
};
const size_t ALL_ENTITY_TYPES_COUNT = sizeof(ALL_ENTITY_TYPES) / sizeof (*ALL_ENTITY_TYPES);

#endif //ENTITY_TYPES_GEN_H
