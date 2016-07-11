#ifndef ALL_ENT_TYPES_H
#define ALL_ENT_TYPES_H

#include <entities/configurable_entity.h>
#include <entities/player_entity.h>

#include <stdlib.h>

const EntityType *ALL_ENTITY_TYPES[] = {
	&ConfigurableEntity,
	&PlayerEntity,
};
const size_t ALL_ENTITY_TYPES_COUNT = sizeof(ALL_ENTITY_TYPES) / sizeof (*ALL_ENTITY_TYPES);

#endif //ALL_ENT_TYPES_H
