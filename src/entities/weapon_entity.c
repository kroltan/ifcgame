#include "entities/weapon_entity.h"

#include "entities/player_entity.h"

#include "game_state.h"
#include "connection.h"
#include "physics.h"
#include "cvars.h"

#include <allegro5/allegro_primitives.h>
#include <chipmunk/chipmunk.h>

#include <stdio.h>

static const float WEAPON_WIDTH = 0.25;
static const float WEAPON_HEIGHT = 2.0;
static const float WEAPON_DAMAGE = 3;

typedef struct {
    Entity *owner;
    cpConstraint *player_joint;
} WeaponEntityData;

void weapon_init(Entity *ent) {
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
            cpv(-WEAPON_WIDTH, -WEAPON_HEIGHT / 2), cpv(1, 0)
        ));
        cpConstraintSetErrorBias(data->player_joint, 0);
    }
}

void weapon_draw(Entity *ent) {
    (void) ent;

    al_draw_rectangle(
        -WEAPON_WIDTH / 2, -WEAPON_HEIGHT / 2,
        WEAPON_WIDTH / 2, WEAPON_HEIGHT / 2,
        al_map_rgb(255, 255, 255), 0
    );
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
    int target_health = cvar_getd_player(entity_owner(other_ent), "health");
    int damage = WEAPON_DAMAGE * cpvlength(cpBodyGetVelocity(body));
    char new_health_str[32];
    sprintf(new_health_str, "%d", target_health - damage);
    cvar_set_player(entity_owner(other_ent), "health", new_health_str);
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
