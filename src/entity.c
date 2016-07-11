#include "entity.h"

#include "game_state.h"
#include "connection.h"
#include "pack.h"
#include "list.h"

#include "entity_types.gen.h"


#include <allegro5/allegro.h>

#include <stdint.h>
#include <stdbool.h>

static const uint32_t Entity_TAG = AL_ID('e', 'n', 't', 'y');
static const size_t MAX_TYPE_NAME_LENGTH = 128;

List *all_entities; /**< item=Entity */

struct Entity {
    uint32_t TAG;
    uint32_t id;
    bool keep, dirty;
    const EntityType *type;
    cpBody *body;
    void *data;
};

typedef enum {
    EPACKET_NEW     = 0,
    EPACKET_DESTROY = 1,
    EPACKET_UPDATE  = 2
} EntityPacketOperation;

void entity_each(void (*iter)(Entity *, void *), void *argument) {
    if (!all_entities) return;

    const size_t last_index = list_length(all_entities) - 1;
    for (size_t i = 0; i <= last_index; ++i) {
        // reverse iteration so `iter` can safely destroy the current entity
        iter(list_nth(all_entities, last_index - i), argument);
    }
}

Entity *entity_from_id(uint32_t id) {
    if (!all_entities) return NULL;

    size_t length = list_length(all_entities);
    for (size_t i = 0; i < length; ++i) {
        Entity *ent = list_nth(all_entities, i);
        if (ent->id == id) {
            return ent;
        }
    }
    return NULL;
}

Entity *_entity_new(const EntityType *type, bool broadcast, uint32_t id) {
    if (!all_entities) {
        all_entities = list_new();
    }

    printf("%u\n", id);

    if (broadcast) {
        uint8_t operation = EPACKET_NEW;
        char message[sizeof(operation) + sizeof(id) + MAX_TYPE_NAME_LENGTH];
        pack_format(message, "bu s", operation, id, type->name);
        connection_send_raw(0, message, sizeof(message));
    }

    Entity *ent = malloc(sizeof(Entity));
    ent->TAG = Entity_TAG;
    ent->id = id;
    ent->dirty = false;
    ent->type = type;
    ent->body = cpBodyNew(0, 0);
    ent->data = type->data_size
                ? malloc(type->data_size)
                : NULL;
    if (ent->data) {
        memset(ent->data, 0, type->data_size);
    }

    cpBodySetUserData(ent->body, ent);
    cpSpaceAddBody(game.space, ent->body);

    list_push(all_entities, ent);

    entity_cb cb = ent->type->on_init;
    if (cb) cb(ent);

    return ent;
}
Entity *entity_new(const EntityType *type) {
    uint32_t id = 0;
    if (all_entities && list_length(all_entities)) {
        const Entity *last_ent = list_nth(
            all_entities, list_length(all_entities) - 1
        );
        id = last_ent->id + rand() % 10;
    }
    return _entity_new(type, true, id);
}

void _delete_shapes(cpBody *body, cpShape *shape, void *data) {
    (void) data;
    cpSpaceRemoveShape(cpBodyGetSpace(body), shape);
    cpShapeDestroy(shape);
}
void _delete_constraints(cpBody *body, cpConstraint *constraint, void *data) {
    (void) data;
    cpSpaceRemoveConstraint(cpBodyGetSpace(body), constraint);
    cpConstraintDestroy(constraint);
}
void entity_destroy(Entity *ent) {
    entity_cb cb = ent->type->on_destroy;
    if (cb) cb(ent);

    size_t index;
    if (list_index_of(all_entities, ent, &index)) {
        list_remove(all_entities, index);
    }
    ent->TAG = 0;
    cpBodyEachShape(ent->body, _delete_shapes, NULL);
    cpBodyEachConstraint(ent->body, _delete_constraints, NULL);
    cpSpaceRemoveBody(game.space, ent->body);
    cpBodyDestroy(ent->body);
    free(ent->data);
    free(ent);
}

void entity_update(Entity *ent) {
    entity_cb cb = ent->type->on_update;
    if (cb) cb(ent);

    if (ent->dirty) {
        cpBody *body = ent->body;
        cpVect position = cpBodyGetPosition(body);
        cpVect velocity = cpBodyGetVelocity(body);
        float angle = cpBodyGetAngle(body);
        float angularVelocity = cpBodyGetAngularVelocity(body);

        uint8_t operation = EPACKET_UPDATE;
        char params[sizeof(operation) + sizeof(ent->id) + 6 * sizeof(float)];
        pack_format(
            params, "bu ff ff f f",
            operation, ent->id,
            position.x, position.y,
            velocity.x, velocity.y,
            angle, angularVelocity
        );
        connection_send_raw(0, params, sizeof(params));

        ent->dirty = false;
    }
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

uint32_t entity_id(const Entity *ent) {
    return ent->id;
}

cpBody *entity_body(const Entity *ent) {
    return ent->body;
}

bool entity_flag_dirty(Entity *ent, bool dirty) {
    bool old = ent->dirty;
    ent->dirty = dirty;
    return old;
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

#include <stdio.h>
void entity_sync(const char *data) {
    uint8_t operation;
    uint32_t id;
    unpack_format(data, "bu", &operation, &id);
    const char *params = data + sizeof(operation) + sizeof(id);

    switch (operation) {
        case EPACKET_NEW: {
            char type_name[MAX_TYPE_NAME_LENGTH];
            unpack_format(params, "s", type_name);
            _entity_new(entity_type_from_name(type_name), false, id);
            break;
        }
        case EPACKET_DESTROY: {
            Entity *ent = entity_from_id(id);
            if (ent) entity_destroy(ent);
            break;
        }
        case EPACKET_UPDATE: {
            Entity *ent = entity_from_id(id);
            if (!ent) break;

            cpVect position, velocity;
            float angle, angularVelocity;
            unpack_format(
                params, "ff ff f f",
                &position.x, &position.y,
                &velocity.x, &velocity.y,
                &angle, &angularVelocity
            );
            cpBody *body = ent->body;
            cpBodySetPosition(body, position);
            cpBodySetVelocity(body, velocity);
            cpBodySetAngle(body, angle);
            cpBodySetAngularVelocity(body, angularVelocity);
            break;
        }
    }
}
