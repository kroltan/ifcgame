#ifndef PHYSICS_H
#define PHYSICS_H

#include <chipmunk/chipmunk.h>

void physics_add_top_down_friction(cpBody *body, cpBody *control, float friction, cpConstraint **pivot, cpConstraint **gear);
cpBody *physics_arbiter_get_other(cpBody *self, cpArbiter *arbiter);

#endif // PHYSICS_H
