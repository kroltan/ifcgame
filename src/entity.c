#include "entity.h"

#include "game_state.h"
#include "entity_types.gen.h"

#include <allegro5/allegro.h>

#include <stdint.h>
#include <stdbool.h>

static const uint32_t Entity_TAG = AL_ID('e', 'n', 't', 'y');

struct Entity {
    uint32_t TAG;
    bool keep;
    const EntityType *type;
    cpBody *body;
    void *data;
};

Entity *entity_new(const EntityType *type) {
    cpBody *body = cpBodyNew(0, 0);

    Entity *ent = malloc(sizeof(Entity));
    ent->TAG = Entity_TAG;
    ent->type = type;
    ent->body = body;
    ent->data = type->data_size
                ? malloc(type->data_size)
                : NULL;

    cpBodySetUserData(body, ent);
    cpSpaceAddBody(game.space, body);

    entity_cb cb = ent->type->on_init;
    if (cb) cb(ent);

    return ent;
}

void entity_destroy(Entity *ent) {
    entity_cb cb = ent->type->on_destroy;
    if (cb) cb(ent);

    cpSpaceRemoveBody(game.space, ent->body);
    cpBodyFree(ent->body);
    free(ent);
}

void entity_update(Entity *ent) {
    entity_cb cb = ent->type->on_update;
    if (cb) cb(ent);
}

void entity_draw(Entity *ent) {
    entity_cb cb = ent->type->on_draw;
    if (cb) cb(ent);
}

void entity_gui(Entity *ent) {
    entity_cb cb = ent->type->on_gui;
    if (cb) cb(ent);
}

void entity_do_event(Entity *ent, ALLEGRO_EVENT *event) {
    entity_event_cb cb = ent->type->on_event;
    if (cb) cb(ent, event);
}

cpBody *entity_body(const Entity *ent) {
    return ent->body;
}

void *entity_data(const Entity *ent) {
    return ent->data;
}

bool entity_valid(const Entity *ent) {
    return ent->TAG == Entity_TAG;
}

void entity_set_keep(Entity *ent, bool keep) {
    ent->keep = keep;
}

bool entity_get_keep(const Entity *ent) {
    return ent->keep;
}

const EntityType *entity_type_from_name(const char *ent_type_name) {
    for (size_t i = 0; i < ALL_ENTITY_TYPES_COUNT; ++i) {
        const EntityType *type = ALL_ENTITY_TYPES[i];
        if (strcmp(type->name, ent_type_name) == 0) {
            return type;
        }
    }
    return NULL;
}
