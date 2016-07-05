#ifndef CONNECTION_H
#define CONNECTION_H

#include <chipmunk/chipmunk.h>
#include <allegro5/allegro5.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

ALLEGRO_EVENT_SOURCE *connection_init();

bool connection_is_host();

bool connection_host(uint16_t port, size_t max_players);
bool connection_join(const char *hostname, uint16_t port);

void connection_update();

#endif // CONNECTION_H
