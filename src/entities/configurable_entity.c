#include "entities/configurable_entity.h"

#include "game_state.h"
#include "connection.h"
#include "graphics.h"
#include "physics.h"
#include "hashmap.h"
#include "config.h"
#include "pack.h"
#include "list.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <chipmunk/chipmunk.h>

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define POLY_MAX_POINTS 256
const int CONFIGURABLE_ENTITY_PACKET = 0x90;
const size_t MAX_DEFINITION_NAME_LENGTH = 32;

typedef struct {
    bool initialized;
    ALLEGRO_BITMAP *sprite;
    float sprite_scale;
    cpVect sprite_offset;
    cpBody *controller;
} ConfigurableEntityData;


void configurable_init(Entity *ent) {
    ConfigurableEntityData *data = entity_data(ent);
    data->initialized = false;
    cpBodySetType(entity_body(ent), CP_BODY_TYPE_KINEMATIC); //until initialized
}

void configurable_update(Entity *ent) {
    ConfigurableEntityData *data = entity_data(ent);
    if (!data->initialized) return;

    if (data->controller) {
        cpBodySetPosition(data->controller, cpBodyGetPosition(entity_body(ent)));
    }
}

void configurable_draw(Entity *ent) {
    ConfigurableEntityData *data = entity_data(ent);
    if (!data->initialized) return;

    if (data->sprite) {
        draw_sprite(data->sprite, 0, 0, data->sprite_scale);
    }
}

bool instantiate_definition(const char *definition, Entity *ent);
void configurable_event(Entity *ent, ALLEGRO_EVENT *ev) {
    if (ev->type == CONNECTION_RECEIVE_EVENT_ID && ev->user.data1 == CONFIGURABLE_ENTITY_PACKET) {
        uint32_t eid;
        char definition_name[MAX_DEFINITION_NAME_LENGTH];

        unpack_format((const char *)ev->user.data2, "us", &eid, definition_name);
        if (eid == entity_id(ent)) {
            instantiate_definition(definition_name, ent);
        }
    }
}

const EntityType ConfigurableEntity = {
    .name = "ConfigurableEntity",
    .data_size = sizeof(ConfigurableEntityData),
    .on_init = configurable_init,
    .on_update = configurable_update,
    .on_draw = configurable_draw,
    .on_event = configurable_event
};

typedef enum {
    CE_SD_CIRCLE,
    CE_SD_POLY,
    CE_SD_SEGMENT
} ShapeDescriptionKind;

typedef struct {
    ShapeDescriptionKind kind;
    float friction;
    union {
        struct {
            cpVect points[POLY_MAX_POINTS];
            size_t point_count;
            float radius;
        } polygon;
        struct {
            cpVect offset;
            float radius;
        } circle;
        struct {
            cpVect a;
            cpVect b;
            float radius;
        } segment;
    };
} ShapeDescription;

typedef struct {
    List *shapes; /**< item=ShapeDescription */
    float mass;
    float moment;
    float groundFriction;
    cpBodyType type;
} CEPhysics;

typedef struct {
    cpVect offset;
    float scale;
    ALLEGRO_BITMAP *bitmap;
} CEVisual;

typedef struct {
    CEPhysics physics;
    CEVisual visual;
} CEDescription;

HashMap *configurable_kinds = NULL; /**< key=string, value=CEDescription */

bool instantiate_definition(const char *definition, Entity *ent) {
    void *desc_out;
    if (hashmap_try_get(configurable_kinds, definition, &desc_out)) {
        CEDescription *desc = desc_out;

        ConfigurableEntityData *data = entity_data(ent);
        data->sprite = desc->visual.bitmap;
        data->sprite_offset = desc->visual.offset;
        data->sprite_scale = desc->visual.scale;
        data->initialized = true;

        cpBody *body = entity_body(ent);
        cpBodySetType(body, desc->physics.type);
        if (desc->physics.type == CP_BODY_TYPE_DYNAMIC) {
            cpBodySetMass(body, desc->physics.mass);
            cpBodySetMoment(body, desc->physics.moment);

            data->controller = cpSpaceAddBody(game.space,cpBodyNewKinematic());
            physics_add_top_down_friction(body, data->controller, desc->physics.groundFriction, NULL, NULL);
        }
        size_t shapes_length = list_length(desc->physics.shapes);
        for (size_t i = 0; i < shapes_length; ++i) {
            ShapeDescription *shape_desc = list_nth(desc->physics.shapes, i);
            cpShape *shape;
            switch (shape_desc->kind) {
                case CE_SD_SEGMENT:
                    shape = cpSegmentShapeNew(body,
                                              shape_desc->segment.a,
                                              shape_desc->segment.b,
                                              shape_desc->segment.radius);
                    break;
                case CE_SD_CIRCLE:
                    shape = cpCircleShapeNew(body,
                                             shape_desc->circle.radius,
                                             shape_desc->circle.offset);
                    break;
                case CE_SD_POLY:
                    shape = cpPolyShapeNew(body,
                                           shape_desc->polygon.point_count,
                                           shape_desc->polygon.points,
                                           cpTransformIdentity,
                                           shape_desc->polygon.radius);
                    break;
                default:
                    shape = NULL;
                    break;
            }
            if (shape) {
                cpShapeSetFriction(shape, shape_desc->friction);
                cpSpaceAddShape(game.space, shape);
            }
        }
        return true;
    }

    return false;
}

Entity *entity_instantiate(const char *definition) {
    Entity *ent = entity_new(&ConfigurableEntity);

    if (instantiate_definition(definition, ent)) {
        uint32_t eid = entity_id(ent);
        char packet[sizeof(eid) + MAX_DEFINITION_NAME_LENGTH];
        pack_format(packet, "us", eid, definition);
        connection_send(CONFIGURABLE_ENTITY_PACKET, 1, packet, sizeof(packet));

        return ent;
    }
    entity_destroy(ent);
    return NULL;
}

static List *parse_shape_descriptions(const char *text) {
    List *list = list_new();

    const size_t text_len = strlen(text);
    const char *content = text;

    do {
        int scan_len;
        float friction;
        char defname[MAX_DEFINITION_NAME_LENGTH];
        if (sscanf(content, "%32s %f%n", defname, &friction, &scan_len) == 2)  {
            content += scan_len;
        } else {
            break;
        }
        ShapeDescription *shape = malloc(sizeof(ShapeDescription));
        shape->friction = friction;

        if (!strcmp(defname, "polygon")) {
            shape->kind = CE_SD_POLY;

            float x, y;

            size_t i;
            for (i = 0; i < POLY_MAX_POINTS; ++i) {
                if (sscanf(content, "%f %f%n", &x, &y, &scan_len) == 2) {
                    content += scan_len;
                    shape->polygon.points[i] = cpv(x, y);
                } else {
                    break;
                }
            }
            shape->polygon.point_count = i;
            shape->polygon.radius = 0;
        } else if (!strcmp(defname, "circle")) {
            shape->kind = CE_SD_CIRCLE;

            float x, y, radius;
            if (sscanf(content, "%f %f %f%n", &x, &y, &radius, &scan_len) == 3) {
                content += scan_len;
                shape->circle.offset = cpv(x, y);
                shape->circle.radius = radius;
            } else {
                break;
            }
        } else if (!strcmp(defname, "segment")) {
            shape->kind = CE_SD_SEGMENT;

            float a_x, a_y, b_x, b_y, radius;
            if (sscanf(content, "%f %f %f %f %f%n", &a_x, &a_y, &b_x, &b_y, &radius, &scan_len) == 5) {
                content += scan_len;
                shape->segment.a = cpv(a_x, a_y);
                shape->segment.b = cpv(b_x, b_y);
                shape->segment.radius = radius;
            } else {
                break;
            }
        } else {
            free(shape);
            shape = NULL;
            break;
        }
        list_push(list, shape);
        const char *next_def = strchr(content, ';');
        if (next_def) {
            //safe because the while condition protects
            //access outside bounds of original string
            content = next_def + 1;
        }
    } while (content < text + text_len);
    return list;
}

static CEDescription *load_entity_description(ALLEGRO_FILE *file) {
    ALLEGRO_CONFIG *config = al_load_config_file_f(file);

    CEDescription *desc = malloc(sizeof(CEDescription));

    desc->physics.shapes = parse_shape_descriptions(config_get_string(config, "Physics", "Shapes", ""));
    desc->physics.mass = config_get_double(config, "Physics", "Mass", 0.0);
    desc->physics.moment = config_get_double(config, "Physics", "Moment", 0.0);
    desc->physics.groundFriction = config_get_double(config, "Physics", "GroundFriction", 1.0);
    desc->physics.type = config_get_ll(config, "Physics", "Type", 0);

    ALLEGRO_PATH *path = game_asset_path(config_get_string(config, "Visual", "Sprite", NULL));
    desc->visual.bitmap = al_load_bitmap(al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP));
    desc->visual.scale = config_get_double(config, "Visual", "Scale", 1);
    al_destroy_path(path);

    desc->visual.offset.x = config_get_double(config, "Visual", "OffsetX", 0.0);
    desc->visual.offset.y = config_get_double(config, "Visual", "OffsetY", 0.0);

    al_destroy_config(config);
    return desc;
}

static int _init_fs_iter(ALLEGRO_FS_ENTRY *entry, void *cache_hashmap) {
    HashMap *const cache = cache_hashmap;

    if (al_get_fs_entry_mode(entry) & ALLEGRO_FILEMODE_ISDIR) {
        return al_for_each_fs_entry(entry, _init_fs_iter, cache);
    }

    ALLEGRO_PATH *path = al_create_path(al_get_fs_entry_name(entry));
    const char *extension = al_get_path_extension(path);
    if (strcmp(extension, ".ini") == 0) {
        ALLEGRO_FILE *file = al_open_fs_entry(entry, "r");

        const char *file_basename = al_get_path_basename(path);
        char *description_name = malloc(strlen(file_basename) + 1);
        strcpy(description_name, file_basename);

        CEDescription *desc = load_entity_description(file);

        void *old_v;
        if (hashmap_try_get(cache, description_name, &old_v)) {
            CEDescription *old = old_v;
            al_destroy_bitmap(old->visual.bitmap);

            size_t shapes_len = list_length(old->physics.shapes);
            for (size_t i = 0; i < shapes_len; ++i) {
                free(list_nth(old->physics.shapes, i));
            }
            list_destroy(old->physics.shapes);
        }
        hashmap_set(cache, description_name, desc);

        al_fclose(file);
    }

    al_destroy_path(path);
    return ALLEGRO_FOR_EACH_FS_ENTRY_OK;
}

void init_configurable_entities() {
    if (configurable_kinds) return;

    configurable_kinds = hashmap_new_str_key();

    ALLEGRO_PATH *root_path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
    al_append_path_component(root_path, "assets");
    al_append_path_component(root_path, "entities");
    ALLEGRO_FS_ENTRY *root_entry = al_create_fs_entry(al_path_cstr(root_path, ALLEGRO_NATIVE_PATH_SEP));
    al_for_each_fs_entry(root_entry, _init_fs_iter, configurable_kinds);

    al_destroy_fs_entry(root_entry);
    al_destroy_path(root_path);
}
