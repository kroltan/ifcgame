#include "physics.h"

#include "game_state.h"

void physics_add_top_down_friction(cpBody *body, cpBody *control, float friction, cpConstraint **out_pivot, cpConstraint **out_gear) {
    //emulates linear friction
    cpConstraint *pivot = cpSpaceAddConstraint(game.space, cpPivotJointNew2(
        control,
        body,
        cpvzero,
        cpvzero
    ));
    cpConstraintSetErrorBias(pivot, cpBodyGetMass(body) * friction);
    cpConstraintSetMaxForce(pivot, cpBodyGetMass(body) * friction);

    //emulates angular friction
    cpConstraint *gear = cpSpaceAddConstraint(game.space, cpGearJointNew(
        control,
        body,
        0, 1
    ));
    cpConstraintSetMaxBias(gear, 0);
    cpConstraintSetMaxForce(gear, friction / cpBodyGetMass(body) / 10);

    if (out_pivot) *out_pivot = pivot;
    if (out_gear) *out_gear = gear;
}

cpBody *physics_arbiter_get_other(cpBody *self, cpArbiter *arbiter) {
    cpBody *a, *b;
    cpArbiterGetBodies(arbiter, &a, &b);

    if (self == a) {
        return b;
    } else {
        return a;
    }
}
