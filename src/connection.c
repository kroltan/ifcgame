#include "connection.h"

#include "game_state.h"
#include "entity.h"
#include "pack.h"

#include <enet/enet.h>
#include <allegro5/allegro.h>

static const size_t CHANNEL_COUNT = 1;

ENetHost *host;
ENetPeer *server;
size_t refresh_counter = 0;

bool connection_init() {
    int ret = enet_initialize();

    atexit(enet_deinitialize);

    return ret == 0;
}

static inline void destroy_host() {
    if (!host) {
        return;
    }
    if (server) {
        enet_peer_disconnect(server, 0);
    }

    enet_host_destroy(host);
    host = NULL;
}

bool connection_is_host() {
    return host && server == NULL;
}

bool connection_host(uint16_t port, size_t max_players) {
    destroy_host();

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    host = enet_host_create(&address, max_players, CHANNEL_COUNT, 0, 0);

    return host != NULL;
}

bool connection_join(const char *hostname, uint16_t port) {
    destroy_host();

    ENetAddress address;
    enet_address_set_host(&address, hostname);
    address.port = port;

    host = enet_host_create(NULL, 1, CHANNEL_COUNT, 0, 0);
    enet_host_connect(host, &address, CHANNEL_COUNT, 0);

    return host != NULL;
}
