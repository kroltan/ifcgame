#include "game_state.h"

ALLEGRO_PATH *game_asset_path(const char *asset) {
    ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
    ALLEGRO_PATH *asset_path = al_create_path(asset);
    al_append_path_component(path, "assets");
    al_join_paths(path, asset_path);
    al_destroy_path(asset_path);
    return path;
}
