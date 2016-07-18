#include "physics_debug.h"

#include "entity.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <math.h>
#include <stdio.h>

ALLEGRO_COLOR _cp_to_al_rgbaf(cpSpaceDebugColor color) {
    return al_map_rgba_f(color.r, color.g, color.b, color.a);
}
void _pd_circle(cpVect pos, cpFloat angle, cpFloat radius, cpSpaceDebugColor outline, cpSpaceDebugColor fill, cpDataPointer data) {
    (void) data;

    al_draw_filled_circle(pos.x, pos.y, radius, _cp_to_al_rgbaf(fill));
    al_draw_circle(pos.x, pos.y, radius, _cp_to_al_rgbaf(outline), 0);
    al_draw_line(pos.x, pos.y, pos.x + cos(angle) * radius, pos.y + sin(angle) * radius, al_map_rgb(255, 0, 0), 0);
}
void _pd_segment(cpVect a, cpVect b, cpSpaceDebugColor color, void *data) {
    (void) data;

    al_draw_line(a.x, a.y, b.x, b.y, _cp_to_al_rgbaf(color), 0);
}
void _pd_fat_segment(cpVect a, cpVect b, cpFloat radius, cpSpaceDebugColor outline, cpSpaceDebugColor fill, cpDataPointer data) {
    (void) data;

    al_draw_filled_circle(a.x, a.y, radius, _cp_to_al_rgbaf(fill));
    al_draw_filled_circle(b.x, b.y, radius, _cp_to_al_rgbaf(fill));
    al_draw_line(a.x, a.y, b.x, b.y, _cp_to_al_rgbaf(outline), 0);
}
void _pd_polygon(int count, const cpVect *verts, cpFloat radius, cpSpaceDebugColor outlineColor, cpSpaceDebugColor fillColor, cpDataPointer data) {
    (void) data;
    (void) radius;
    (void) outlineColor;

    for (int i = 1; i < count; ++i) {
        cpVect curr = verts[i];
        cpVect prev = verts[i - 1];
        al_draw_line(curr.x, curr.y, prev.x, prev.y, _cp_to_al_rgbaf(fillColor), 0);
    }
}
void _pd_dot(cpFloat size, cpVect pos, cpSpaceDebugColor color, cpDataPointer data) {
    (void) data;

    al_draw_filled_circle(pos.x, pos.y, size * 0.01, _cp_to_al_rgbaf(color));
}
cpSpaceDebugColor _pd_shape_color(cpShape *shape, cpDataPointer data) {
    (void) data;

    void *userdata = cpBodyGetUserData(cpShapeGetBody(shape));
    cpSpaceDebugColor color = {0, 0, 0, 1};
    if (entity_valid(userdata)) {
        color.g = 1;
        if (!entity_get_keep(userdata)) {
            color.r = 1;
        }
    } else {
        color.r = 1;
    }
    return color;
}
cpSpaceDebugDrawOptions physics_debug = {
    .drawCircle = _pd_circle,
    .drawSegment = _pd_segment,
    .drawFatSegment = _pd_fat_segment,
    .drawPolygon = _pd_polygon,
    .drawDot = _pd_dot,
    .flags = CP_SPACE_DEBUG_DRAW_SHAPES | CP_SPACE_DEBUG_DRAW_CONSTRAINTS,
    .shapeOutlineColor = {1, 1, 1, 1},
    .colorForShape = _pd_shape_color,
    .constraintColor = {1, 0, 1, 1},
    .collisionPointColor = {1, 1, 0, 1}
};
