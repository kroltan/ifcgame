#include "cvars.h"

#include "pack.h"
#include "hashmap.h"
#include "console.h"
#include "connection.h"

#include <stdio.h>

static const uint32_t CVAR_CHANGE_PACKET = 1236;
static const size_t MAX_CVAR_LENGTH = 128;

HashMap *cvar_map; /**< key=string, value=string */

void _player_var(char buf[], uint32_t id, const char *var) {
    snprintf(buf, MAX_CVAR_LENGTH, "player%u_%s", id, var);
}

const char *cvar_get(const char *var) {
    return hashmap_get(cvar_map, var);
}

const char *cvar_get_player(uint32_t id, const char *var) {
    char name[MAX_CVAR_LENGTH];
    _player_var(name, id, var);
    return hashmap_get(cvar_map, name);
}

int _atod_default(const char *str) {
    if (!str) str = "0";
    return atoi(str);
}

int cvar_getd(const char *var) {
    return _atod_default(cvar_get(var));
}

int cvar_getd_player(uint32_t id, const char *var) {
    return _atod_default(cvar_get_player(id, var));
}

char *_alloc_string(const char *from) {
    char *dest = malloc(MAX_CVAR_LENGTH);
    memcpy(dest, from, MAX_CVAR_LENGTH);
    dest[MAX_CVAR_LENGTH - 1] = '\0';
    return dest;
}
void _cvar_set(const char *var, const char *value) {
    void *value_alloc = NULL;
    if (hashmap_try_get(cvar_map, var, &value_alloc)) {
        free(value_alloc);
    }
    value_alloc = _alloc_string(value);

    hashmap_set(cvar_map, var, (char *) value_alloc);
}
void cvar_set(const char *var, const char *value) {
    _cvar_set(var, value);

    char data[2 * MAX_CVAR_LENGTH];
    pack_format(data, "s s", var, value);
    connection_send(CVAR_CHANGE_PACKET, 1, data, sizeof(data));
}

void cvar_set_player(uint32_t id, const char *var, const char *value) {
    char name[MAX_CVAR_LENGTH];
    _player_var(name, id, var);
    cvar_set(name, value);
}

void cvar_setd(const char *var, int value) {
    char value_str[MAX_CVAR_LENGTH];
    sprintf(value_str, "%d", value);
    cvar_set(var, value_str);
}

void cvar_setd_player(uint32_t id, const char *var, int value) {
    char value_str[MAX_CVAR_LENGTH];
    sprintf(value_str, "%d", value);
    cvar_set_player(id, var, value_str);
}

void cvar_on_event(ALLEGRO_EVENT *event) {
    if (!cvar_map) {
        cvar_map = hashmap_new_str_key();
    }

    if (event->type == CONNECTION_RECEIVE_EVENT_ID && (uint32_t)event->user.data1 == CVAR_CHANGE_PACKET) {
        char var[MAX_CVAR_LENGTH];
        char value[MAX_CVAR_LENGTH];
        unpack_format((const char *)event->user.data2, "s s", var, value);
        var[MAX_CVAR_LENGTH - 1] = '\0';
        value[MAX_CVAR_LENGTH - 1] = '\0';

        _cvar_set(var, value);
    } else if (event->type == CONSOLE_EVENT_ID) {
        const char *command = (const char *)event->user.data1;
        char var[MAX_CVAR_LENGTH];
        char value[MAX_CVAR_LENGTH];
        if (sscanf(command, "cset %s %s", var, value) == 2) {
            cvar_set(var, value);
        }
    }
}
