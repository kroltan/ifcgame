#ifndef CONNECTION_H
#define CONNECTION_H

#include <chipmunk/chipmunk.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

extern const unsigned int CONNECTION_EVENT_ID;

bool init_connection();

bool connection_is_host();

bool connection_host(uint16_t port, size_t max_players);
bool connection_join(const char *hostname, uint16_t port);

void connection_update();
void connection_send(uint32_t id, uint8_t channel, const char *data, size_t size);
void connection_send_raw(uint8_t channel, const char *data, size_t size);

#endif // CONNECTION_H
