#include "entities/weapon_entity.h"

#include "entities/player_entity.h"

#include "game_state.h"
#include "connection.h"
#include "graphics.h"
#include "physics.h"
#include "keymap.h"
#include "cvars.h"

#include <allegro5/allegro_primitives.h>
#include <chipmunk/chipmunk.h>

#include <stdio.h>

static const float WEAPON_WIDTH = 0.25;
static const float WEAPON_HEIGHT = 2.0;
static const float WEAPON_DAMAGE = 1.5;
static const float WEAPON_HURT_THRESHOLD = 1;
static const float WEAPON_SWING_FORCE = 7;

typedef struct {
    Entity *owner;
    cpConstraint *player_joint;
} WeaponEntityData;

ALLEGRO_BITMAP *weapon_sprite;

void weapon_init(Entity *ent) {
    if (!weapon_sprite) {
        ALLEGRO_PATH *path = game_asset_path("sword.png");
        weapon_sprite = al_load_bitmap(al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP));
        al_destroy_path(path);
    }
    cpBody *body = entity_body(ent);

    cpShape *shape = cpBoxShapeNew(body, WEAPON_WIDTH, WEAPON_HEIGHT, 0);
    cpSpaceAddShape(game.space, shape);
    cpShapeSetFriction(shape, 1);
    cpShapeSetElasticity(shape, 0);

    cpBodySetMass(body, 1);
    cpBodySetMoment(body, cpMomentForBox(1, WEAPON_WIDTH, WEAPON_HEIGHT));
}

void weapon_update(Entity *ent) {
    cpBody *body = entity_body(ent);
    WeaponEntityData *data = entity_data(ent);

    if (data->owner && !data->player_joint) {
        data->player_joint = cpSpaceAddConstraint(game.space, cpPivotJointNew2(
            body,
            entity_body(data->owner),
            cpv(0, -WEAPON_HEIGHT / 2), cpv(1, 0)
        ));
        cpConstraintSetErrorBias(data->player_joint, 0);
    }

    int mult = 0;
    if (keymap_is_held("mouse1")) mult++;
    if (keymap_is_held("mouse2")) mult--;

    if (mult) {
        cpVect force_pos = cpv(0, WEAPON_HEIGHT);
        cpVect weapon_pos = cpBodyGetPosition(body);
        cpVect world_force_pos = cpvadd(weapon_pos, cpvrotate(cpBodyGetRotation(body), force_pos));
        cpVect mouse_delta = cpvsub(keymap_mouse_world(), world_force_pos);
        cpVect force = cpvmult(cpvnormalize(mouse_delta), mult *WEAPON_SWING_FORCE);
        cpBodyApplyForceAtWorldPoint(body, force, world_force_pos);
    }
}

void weapon_draw(Entity *ent) {
    (void) ent;

    if (weapon_sprite) {
        draw_sprite(weapon_sprite, 2 *WEAPON_WIDTH / 3, 0, 1.2 * WEAPON_HEIGHT, 0);
    }
}

void weapon_collide(Entity *ent, cpArbiter *arb) {
    cpBody *body = entity_body(ent);
    cpBody *other = physics_arbiter_get_other(body, arb);
    Entity *other_ent = cpBodyGetUserData(other);

    //only damage players
    if (!entity_valid(other_ent) || entity_type(other_ent) != &PlayerEntity) {
        return;
    }
    //only damage enemies
    if (entity_owner(ent) != connection_get_user_id()
        || entity_owner(other_ent) == connection_get_user_id()) {
        return;
    }

    float speed = cpvlength(cpBodyGetVelocity(body));
    if (speed < WEAPON_HURT_THRESHOLD) {
        return;
    }

    player_hurt(entity_owner(other_ent), WEAPON_DAMAGE * speed, entity_owner(ent));
}

const EntityType WeaponEntity = {
    .name = "WeaponEntity",
    .data_size = sizeof(WeaponEntityData),
    .on_init = weapon_init,
    .on_update = weapon_update,
    .on_draw = weapon_draw,
    .on_collide = weapon_collide
};

Entity *weapon_create_for_player(Entity *player_ent) {
    if (player_get_for_id(connection_get_user_id()) != player_ent) {
        return NULL;
    }
    Entity *ent = entity_new(&WeaponEntity);

    WeaponEntityData *data = entity_data(ent);
    data->owner = player_ent;

    return ent;
}
