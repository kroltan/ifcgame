#ifndef CVARS_H
#define CVARS_H

#include <allegro5/allegro.h>

#include <stdint.h>

const char *cvar_get(const char *var);
const char *cvar_get_player(uint32_t id, const char *var);
int cvar_getd(const char *var);
int cvar_getd_player(uint32_t id, const char *var);

void cvar_set(const char *var, const char *value);
void cvar_set_player(uint32_t id, const char *var, const char *value);



void cvar_on_event(ALLEGRO_EVENT *event);

#endif // CVARS_H
