#include "menu_entity.h"

#include "textbox.h"
#include "console.h"
#include "graphics.h"
#include "game_state.h"

#include <chipmunk/chipmunk.h>
#include <allegro5/allegro.h>

#include <stdio.h>

static const float MENU_FONT_SIZE = 0.8;

typedef struct {
    TextBox server;
    TextBox port;
    ALLEGRO_BITMAP *logo;
} MenuEntityData;

void menu_init(Entity *ent) {
    cpBodySetType(entity_body(ent), CP_BODY_TYPE_KINEMATIC);
    MenuEntityData *data = entity_data(ent);

    ALLEGRO_PATH *path = game_asset_path("logo.png");
    data->logo = al_load_bitmap(al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP));
    al_destroy_path(path);

    textbox_init(&data->server, 64);
    textbox_init(&data->port, 8);

    data->server.text_flags = TEXT_HALIGN_LEFT;
    data->server.caret_width = 0.1;
    data->server.font_size = MENU_FONT_SIZE;
    data->server.focus = true;
    data->port.text_flags = TEXT_HALIGN_LEFT;
    data->port.caret_width = 0.1;
    data->port.font_size = MENU_FONT_SIZE;

    game.camera_position = cpvzero;
}

void menu_draw(Entity *ent) {
    MenuEntityData *data = entity_data(ent);

    if (data->logo) {
        draw_sprite(data->logo, -0.4, 0.35, 6, 0);
    }

    draw_textf(game.default_font, al_map_rgb(255, 255, 255), 0, 0, MENU_FONT_SIZE, TEXT_HALIGN_RIGHT, "IP: ");
    textbox_draw(&data->server, 0, 0);

    draw_textf(game.default_font, al_map_rgb(255, 255, 255), 0, -1, MENU_FONT_SIZE, TEXT_HALIGN_RIGHT, "PORT: ");
    textbox_draw(&data->port, 0, -1);

    draw_textf(game.default_font, al_map_rgb(255, 255, 255), 0, -3, MENU_FONT_SIZE, TEXT_HALIGN_CENTER, "Press ENTER to join");
}

void menu_event(Entity *ent, ALLEGRO_EVENT *ev) {
    MenuEntityData *data = entity_data(ent);

    if (ev->type == ALLEGRO_EVENT_KEY_DOWN) {
        if (ev->keyboard.keycode == ALLEGRO_KEY_DOWN || ev->keyboard.keycode == ALLEGRO_KEY_UP) {
            data->server.focus = !data->server.focus;
            data->port.focus = !data->port.focus;
        } else if (ev->keyboard.keycode == ALLEGRO_KEY_ENTER) {
            if (strlen(data->server.buffer) && strlen(data->port.buffer)) {
                char full_command[256];
                sprintf(full_command, "join %s %s", data->server.buffer, data->port.buffer);
                console_run(full_command);
            }
        }
    }

    textbox_on_event(&data->server, ev);
    textbox_on_event(&data->port, ev);
}

const EntityType MenuEntity = {
    .name = "MenuEntity",
    .data_size = sizeof(MenuEntityData),
    .on_init = menu_init,
    .on_draw = menu_draw,
    .on_event = menu_event
};
