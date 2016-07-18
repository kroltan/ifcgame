#include "entities/player_entity.h"

#include "cvars.h"
#include "keymap.h"
#include "physics.h"
#include "graphics.h"
#include "game_state.h"
#include "connection.h"
#include "entities/weapon_entity.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <chipmunk/chipmunk.h>

#include <stdio.h>
#include <stdlib.h>

static const float MOVE_SPEED = 10;
static const int MAX_HEALTH = 100;
const unsigned int PLAYER_DEATH_EVENT_ID = 131415;

typedef struct {
    cpBody *controller;
    cpShape *head;
} PlayerEntityData;

void player_init(Entity *ent) {
    PlayerEntityData *data = entity_data(ent);
    cpBody *body = entity_body(ent);

    data->controller = NULL;

    data->head = cpCircleShapeNew(body, 0.5, cpvzero);
    cpSpaceAddShape(game.space, data->head);
    cpShapeSetFriction(data->head, 0.5);

    cpBodySetMass(body, 60);
    cpBodySetMoment(body, cpMomentForCircle(10, .5, 0, cpvzero));
}

void player_update(Entity *ent) {
    //only update local player
    if (entity_owner(ent) != connection_get_user_id()) return;

    cpBody *body = entity_body(ent);
    PlayerEntityData *data = entity_data(ent);

    if (!data->controller) {
        data->controller = cpSpaceAddBody(game.space, cpBodyNew(1, 1));

        cpConstraint *pivot, *gear;
        physics_add_top_down_friction(body, data->controller, 10 * MOVE_SPEED, &pivot, &gear);

        cpConstraintSetErrorBias(pivot, 0);
    }

    if (cvar_getd_player(entity_owner(ent), "health") < 0) {
        entity_destroy(ent);
    }

    cpVect move_direction = cpvzero;
    if (keymap_is_held("left")) {
        move_direction = cpvadd(move_direction, cpv(0, 1));
    }
    if (keymap_is_held("right")) {
        move_direction = cpvadd(move_direction, cpv(0, -1));
    }
    if (keymap_is_held("up")) {
        move_direction = cpvadd(move_direction, cpv(1, 0));
    }
    if (keymap_is_held("down")) {
        move_direction = cpvadd(move_direction, cpv(-1, 0));
    }
    move_direction = cpvmult(cpvnormalize(move_direction), MOVE_SPEED);
    move_direction.y *= 0.8;
    if (move_direction.x < 0) {
        move_direction.x *= 0.5;
    }
    move_direction = cpvrotate(cpBodyGetRotation(body), move_direction);

    cpBodySetPosition(data->controller, cpBodyGetPosition(body));
    cpBodySetVelocity(data->controller, move_direction);

    cpVect mouse_delta = cpvsub(keymap_mouse_world(), cpBodyGetPosition(body));
    float turn = cpvtoangle(mouse_delta);
    cpBodySetAngle(body, turn);

    entity_flag_dirty(ent, true);
}

void player_draw(Entity *ent) {
    PlayerEntityData *data = entity_data(ent);

    const char *health_str = cvar_get_player(entity_owner(ent), "health");
    if (!health_str) health_str = "0";
    float health_percent = atof(health_str) / MAX_HEALTH;
    float radius = cpCircleShapeGetRadius(data->head);
    al_draw_circle(0, 0, radius, al_map_rgb(255, 255, 255), 0);
    al_draw_arc(0, 0, 1.3 * radius, 0, 2 * ALLEGRO_PI * health_percent, al_map_rgb(0, 255, 0), 0.1);
    draw_textf(
        game.default_font,
        al_map_rgb(255, 255, 255), 0,
        -cpCircleShapeGetRadius(data->head),
        1, TEXT_HALIGN_CENTER,
        "%u", entity_id(ent)
    );
}

const EntityType PlayerEntity = {
    .name = "PlayerEntity",
    .data_size = sizeof(PlayerEntityData),
    .on_init = player_init,
    .on_update = player_update,
    .on_draw = player_draw
};

Entity *player_new_with_id(uint32_t user_id) {
    {
        Entity *old = player_get_for_id(user_id);
        if (entity_valid(old)) {
            entity_destroy(old);
        }
    }

    Entity *ent = entity_new(&PlayerEntity);
    char id_str[16];
    sprintf(id_str, "%u", entity_id(ent));
    cvar_set_player(user_id, "ent_id", id_str);
    char health_str[16];
    sprintf(health_str, "%d", MAX_HEALTH);
    cvar_set_player(user_id, "health", health_str);

    Entity *weapon = weapon_create_for_player(ent);
    cpBodySetPosition(entity_body(weapon), cpvadd(
        cpv(2, 0),
        cpBodyGetPosition(entity_body(ent))
    ));

    return ent;
}

Entity *player_get_for_id(uint32_t user_id) {
    uint32_t ent_id = cvar_getd_player(user_id, "ent_id");
    return entity_from_id(ent_id);
}
