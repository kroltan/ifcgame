#ifndef ENTITY_H
#define ENTITY_H

#include <allegro5/allegro.h>
#include <chipmunk/chipmunk.h>

#include <stdlib.h>
#include <stdbool.h>

typedef struct Entity Entity;

typedef void (*entity_cb)(Entity *entity);
typedef void (*entity_event_cb)(Entity *entity, ALLEGRO_EVENT *event);
typedef struct {
    const char *name;
    size_t data_size;
    entity_cb on_init;
    entity_cb on_destroy;
    entity_cb on_update;
    entity_cb on_draw;
    entity_cb on_gui;
    entity_event_cb on_event;
} EntityType;

Entity *entity_new(const EntityType *type);
void entity_destroy(Entity *ent);
void entity_update(Entity *ent);
void entity_draw(Entity *ent);
void entity_gui(Entity *ent);
void entity_do_event(Entity *ent, ALLEGRO_EVENT *event);
size_t entity_id(const Entity *ent);
cpBody *entity_body(const Entity *ent);
bool entity_flag_dirty(Entity *ent, bool dirty);
void *entity_data(const Entity *ent);
bool entity_valid(const Entity *ent);
void entity_set_keep(Entity *ent, bool keep);
bool entity_get_keep(const Entity *ent);

const EntityType *entity_type_from_name(const char *ent_type_name);

#endif // ENTITY_H
