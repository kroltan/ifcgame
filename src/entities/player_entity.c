#include "entities/player_entity.h"

#include "cvars.h"
#include "keymap.h"
#include "physics.h"
#include "console.h"
#include "graphics.h"
#include "game_state.h"
#include "connection.h"
#include "entities/weapon_entity.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>
#include <chipmunk/chipmunk.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static const float MOVE_SPEED = 5;
static const float ROTATION_CAP = 15;
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

    cpBodySetMass(body, 80);
    cpBodySetMoment(body, cpMomentForCircle(10, .5, 0, cpvzero));
}

void player_update(Entity *ent) {
    //only update local player
    if (entity_owner(ent) != connection_get_user_id()) return;

    cpBody *body = entity_body(ent);
    PlayerEntityData *data = entity_data(ent);

    if (!data->controller) {
        data->controller = cpSpaceAddBody(game.space, cpBodyNewKinematic());

        cpConstraint *pivot, *gear;
        physics_add_top_down_friction(data->controller, body, 10 * MOVE_SPEED, &pivot, &gear);

        cpConstraintSetErrorBias(pivot, 0);
        cpConstraintSetErrorBias(gear, 0);
        cpConstraintSetMaxBias(gear, ROTATION_CAP);
        cpConstraintSetMaxForce(gear, INFINITY);
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

    cpVect mouse_delta = cpvsub(keymap_mouse_world(), cpBodyGetPosition(body));
    float turn = cpvtoangle(cpvunrotate(cpBodyGetRotation(body), mouse_delta));

    if (cpvlength(mouse_delta) < cpCircleShapeGetRadius(data->head)) {
        move_direction = cpvzero;
    }

    cpBodySetVelocity(data->controller, move_direction);
    cpBodySetAngle(data->controller, cpBodyGetAngle(body) - turn);

    entity_flag_dirty(ent, true);
}

void player_draw(Entity *ent) {
    PlayerEntityData *data = entity_data(ent);
    cpBody *body = entity_body(ent);

    float radius = cpCircleShapeGetRadius(data->head);
    al_draw_circle(0, 0, radius, al_map_rgb(255, 255, 255), 0);

    float health_percent = cvar_getd_player(entity_owner(ent), "health") / (float) MAX_HEALTH;
    al_draw_arc(
        0, 0, 1.3 * radius,
        0, 2 * ALLEGRO_PI * health_percent,
        al_color_hsl(120 * health_percent, 1, 0.5), //hue 0 = red, hue 120 = green
        0.1
    );

    ALLEGRO_TRANSFORM text_adjust;
    al_identity_transform(&text_adjust);
    al_rotate_transform(&text_adjust, -cpBodyGetAngle(body));
    al_compose_transform(&text_adjust, al_get_current_transform());
    al_use_transform(&text_adjust);

    float edge = -cpCircleShapeGetRadius(data->head) * 1.3;
    draw_textf(
        game.default_font,
        al_map_rgb(255, 255, 255),
        0, edge,
        0.5, TEXT_HALIGN_CENTER,
        "%s", cvar_get_player(entity_owner(ent), "name")
    );
    draw_textf(
        game.default_font,
        al_map_rgb(255, 255, 255),
        0, edge - 0.5,
        0.3, TEXT_HALIGN_CENTER,
        "%d", cvar_getd_player(entity_owner(ent), "kills")
    );
}

void player_event(Entity *ent, ALLEGRO_EVENT *event) {
    (void) ent;

    if (event->type == CONSOLE_EVENT_ID) {
        char name[32];
        if (sscanf((const char *)event->user.data1, "name %s", name) == 1) {
            cvar_set_player(connection_get_user_id(), "name", name);
        }
    }
}

const EntityType PlayerEntity = {
    .name = "PlayerEntity",
    .data_size = sizeof(PlayerEntityData),
    .on_init = player_init,
    .on_update = player_update,
    .on_draw = player_draw,
    .on_event = player_event
};

Entity *player_new_with_id(uint32_t user_id) {
    {
        Entity *old = player_get_for_id(user_id);
        if (entity_valid(old)) {
            return old;
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
    const char *entid_str = cvar_get_player(user_id, "ent_id");
    if (!entid_str) {
        return NULL;
    }
    return entity_from_id(atoi(entid_str));
}

void player_hurt(uint32_t user_id, int damage, uint32_t inflictor) {
    int health = cvar_getd_player(user_id, "health");
    health -= damage;
    cvar_setd_player(user_id, "health", health);

    if (health < 0) {
        entity_destroy(entity_from_id(cvar_getd_player(user_id, "ent_id")));

        int kills = cvar_getd_player(inflictor, "kills");
        cvar_setd_player(inflictor, "kills", kills + 1);
    }
}
