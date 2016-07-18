#ifndef CONNECTION_H
#define CONNECTION_H

#include <allegro5/allegro.h>
#include <chipmunk/chipmunk.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

extern const unsigned int CONNECTION_RECEIVE_EVENT_ID;
extern const unsigned int CONNECTION_JOIN_EVENT_ID;
extern const unsigned int CONNECTION_LEAVE_EVENT_ID;

bool init_connection();

bool connection_is_host();
uint32_t connection_get_user_id();

bool connection_host(uint16_t port, size_t max_players);
bool connection_join(const char *hostname, uint16_t port);

void connection_update();
void connection_on_event(ALLEGRO_EVENT *event);
void connection_send(uint32_t id, uint8_t channel, const char *data, size_t size);
void connection_send_raw(uint8_t channel, const char *data, size_t size);

#endif // CONNECTION_H
