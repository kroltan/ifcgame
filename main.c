#include "scene.h"
#include "config.h"
#include "entity.h"
#include "console.h"
#include "graphics.h"
#include "game_state.h"
#include "connection.h"
#include "entities/configurable_entity.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include "physics_debug.h"

void game_error(const char *error_message) {
    fprintf(stderr, error_message);
    exit(1);
}

void init_config() {
    ALLEGRO_PATH *config_path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
	al_append_path_component(config_path, "assets");
	al_set_path_filename(config_path, "config.ini");

	const char *path_str = al_path_cstr(config_path, ALLEGRO_NATIVE_PATH_SEP);
    game.config = al_load_config_file(path_str);
    al_destroy_path(config_path);
}

void init_core() {
    game.running = true;
    ALLEGRO_PATH *font_path = game_asset_path("Ubuntu.ttf");
	game.default_font = al_load_font(al_path_cstr(font_path, ALLEGRO_NATIVE_PATH_SEP), -32, 0);
	al_destroy_path(font_path);

	if (!game.default_font) {
        game_error("Could not load default font");
	}

    int target_fps = config_get_ll(game.config, "Display", "TargetFramerate", 60);
    game.main_timer = al_create_timer(1.0 / target_fps);

    al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_OPENGL | ALLEGRO_RESIZABLE);
	al_set_new_display_refresh_rate(target_fps);
	al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
	al_set_new_display_option(ALLEGRO_VSYNC, config_get_ll(game.config, "Display", "VSync", 1), ALLEGRO_SUGGEST);
	al_set_new_display_option(
		ALLEGRO_SAMPLES,
		config_get_ll(game.config, "Display", "AntiAliasLevel", 0),
		ALLEGRO_SUGGEST
	);
	game.display = al_create_display(
		config_get_ll(game.config, "Display", "Width", 640),
		config_get_ll(game.config, "Display", "Height", 480)
	);

	game.event_queue = al_create_event_queue();
	al_register_event_source(game.event_queue, al_get_keyboard_event_source());
	al_register_event_source(game.event_queue, al_get_display_event_source(game.display));
	al_register_event_source(game.event_queue, al_get_timer_event_source(game.main_timer));
	al_start_timer(game.main_timer);
	game.last_frame = al_get_time();
}

void game_init() {
    if (!( al_init()
        && al_install_keyboard()
        && al_init_primitives_addon()
        && al_init_image_addon()
        && al_init_font_addon()
        && al_init_ttf_addon())) {

        game_error("Could not initialize Allegro");
    }

    init_config();
    init_core();
    init_connection();

	game.space = cpSpaceNew();
	cpSpaceSetGravity(game.space, cpv(0, -1));

	if (!( game.main_timer
        && game.display
        && game.event_queue
        && game.space)) {

        game_error("Could not initialize game state");
    }

    init_console();
    init_configurable_entities();
}

void _handle_event(Entity *ent, void *event) {
    entity_do_event(ent, (ALLEGRO_EVENT *) event);
}

void _update_single_entity(Entity *ent, void *data) {
    (void) data;

    coords_use_local(ent);

    entity_update(ent);
    entity_draw(ent);
}

void _gui_single_entity(Entity *ent, void *data) {
    (void) data;

    entity_gui(ent);
}

void game_loop() {
    ALLEGRO_EVENT event;
    al_wait_for_event(game.event_queue, &event);

    if (console_on_event(&event)) {
        entity_each(_handle_event, &event);
    }


    if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE && event.display.source == game.display) {
        al_acknowledge_resize(game.display);
    }

    if (event.type == ALLEGRO_EVENT_TIMER && event.timer.source == game.main_timer) {
        cpSpaceStep(game.space, 0.016);
        connection_update();

        ALLEGRO_TRANSFORM identity;
        al_identity_transform(&identity);

        camera_set(0, 0, 10);
        al_clear_to_color(al_map_rgb(0, 0, 0));

        #ifdef PHYSICS_DRAW_DEBUG
        cpSpaceDebugDraw(game.space, &physics_debug);
        #endif // PHYSICS_DRAW_DEBUG
        entity_each(_update_single_entity, NULL);

        al_use_transform(&identity);

        ALLEGRO_TRANSFORM backup;
        al_copy_transform(&backup, al_get_current_projection_transform());

        int half_width = al_get_display_width(game.display) / 2;
        int half_height = al_get_display_height(game.display) / 2;
        camera_set(half_width / 2, -half_height / 2, half_height);

        entity_each(_gui_single_entity, NULL);
        console_draw();

        al_use_projection_transform(&backup);

        al_flip_display();
    }
}

void game_shutdown() {
    cpSpaceDestroy(game.space);
    al_destroy_display(game.display);
    al_destroy_timer(game.main_timer);
    al_destroy_event_queue(game.event_queue);
}

int main() {
    srand(time(NULL));
    game_init();
    //scene_load("test");

    while (game.running) game_loop();

    game_shutdown();

    return 0;
}
