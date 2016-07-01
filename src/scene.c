#include "scene.h"

#include <chipmunk/chipmunk.h>

#include "game_state.h"
#include "entity.h"
#include "config.h"
#include "list.h"
#include "entities/configurable_entity.h"

ALLEGRO_PATH *get_scene_path(const char *filename) {
    ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
    al_append_path_component(path, "assets");
    al_append_path_component(path, "scenes");
    al_set_path_filename(path, filename);
    al_set_path_extension(path, ".ini");
    return path;
}

bool scene_load(const char *filename) {
    ALLEGRO_PATH *path = get_scene_path(filename);
    ALLEGRO_FILE *fp = al_fopen(al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP), "r");
    al_destroy_path(path);

    if (!fp) return false;

    bool status = scene_loadf(fp);
    al_fclose(fp);

    return status;
}

void _delete_free_ents(cpBody *body, void *data) {
    (void) data;

    if (!entity_valid(cpBodyGetUserData(body))) {
        return;
    }
    Entity *ent = cpBodyGetUserData(body);
    if (!entity_get_keep(ent)) {
        entity_destroy(ent);
    }
}
bool _ent_loader(ALLEGRO_CONFIG *scene, const char *section, const char *key) {
    char ent_type[65];
    float x, y, rotation;
    char confent_name[128];
    int fields = sscanf(config_get_string(scene, section, key, ""), "%65s %f %f %f %s", ent_type, &x, &y, &rotation, confent_name);

    cpBody *body;
    if (fields == 5 && strcmp(ent_type, ConfigurableEntity.name) == 0) {
        body = entity_body(entity_instantiate(confent_name));
    } else if (fields == 4) {
        body = entity_body(entity_new(entity_type_from_name(ent_type)));
    } else {
        return true;
    }
    cpBodySetPosition(body, cpv(x, y));
    cpBodySetAngle(body, rotation * ALLEGRO_PI / 180);
    if (cpBodyGetType(body) == CP_BODY_TYPE_STATIC) {
        cpSpaceReindexShapesForBody(game.space, body);
    }

    return true;
}
bool scene_loadf(ALLEGRO_FILE *fp) {
    ALLEGRO_CONFIG *scene = al_load_config_file_f(fp);
    if (!scene) return false;

    cpSpaceEachBody(game.space, _delete_free_ents, NULL);
    config_each_entry(scene, "Scene", _ent_loader);

    return true;
}

bool scene_save(const char *filename) {
    ALLEGRO_PATH *path = get_scene_path(filename);
    ALLEGRO_FILE *fp = al_fopen(al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP), "w");
    al_destroy_path(path);

    if (!fp) return false;

    bool status = scene_savef(fp);
    al_fclose(fp);

    return status;
}

bool scene_savef(ALLEGRO_FILE *fp) {
    return false;
}
