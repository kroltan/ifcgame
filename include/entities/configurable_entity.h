#ifndef CONFIGURABLE_ENTITY_H
#define CONFIGURABLE_ENTITY_H

#include "entity.h"

const EntityType ConfigurableEntity;

Entity *entity_instantiate(const char *definition);

void init_configurable_entities();

#endif // CONFIGURABLE_ENTITY_H
