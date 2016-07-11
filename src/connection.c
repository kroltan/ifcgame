#include "connection.h"

#include "game_state.h"
#include "entity.h"
#include "pack.h"
#include "list.h"

#include <enet/enet.h>
#include <allegro5/allegro.h>

static const size_t CHANNEL_COUNT = 2;

const unsigned int CONNECTION_EVENT_ID = 513;

List *peers;
ENetHost *host;
ENetPeer *server;
size_t refresh_counter = 0;
ALLEGRO_EVENT_SOURCE event_source;

bool init_connection() {
    int ret = enet_initialize();
    peers = list_new();
    if (!ret) {
        atexit(enet_deinitialize);
        al_init_user_event_source(&event_source);
        al_register_event_source(game.event_queue, &event_source);
        return true;
    }

    return false;
}

static inline void destroy_host() {
    if (!host) {
        return;
    }
    if (server) {
        list_clear(peers);
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

void broadcast_except(ENetPeer *peer, ENetPacket *packet, enet_uint8 channel_id) {
    const size_t length = list_length(peers);
    for (size_t i = 0; i < length; ++i) {
        if (list_nth(peers, i) == peer) {
            continue;
        }
        enet_peer_send(list_nth(peers, i), channel_id, packet);
    }
}

void connection_update() {
    if (!host) return;

    ENetEvent ev;
    while (enet_host_service(host, &ev, 0)) {
        switch (ev.type) {
        case ENET_EVENT_TYPE_CONNECT:
            if (connection_is_host()) {
                list_push(peers, ev.peer);
            }
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            if (connection_is_host()) {
                size_t i;
                if (list_index_of(peers, ev.peer, &i)) {
                    list_remove(peers, i);
                }
            }
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            if (connection_is_host()) {
                if (ev.channelID > 0) {
                    uint32_t id;
                    char *data = NULL;
                    size_t data_len;
                    unpack_format((char *)ev.packet->data, "ua", &id, &data, &data_len);

                    ALLEGRO_EVENT event;
                    event.type = CONNECTION_EVENT_ID;
                    event.user.data1 = id;
                    event.user.data2 = (intptr_t)data;
                    al_emit_user_event(&event_source, &event, NULL);
                } else {
                    //channel 0 is entity update
                    entity_sync((const char *)ev.packet->data);
                }

                broadcast_except(ev.peer, ev.packet, ev.channelID);
            }
            break;
        case ENET_EVENT_TYPE_NONE:
            break;
        }
    }
}

void connection_send_raw(uint8_t channel, const char *data, size_t size) {
    ENetPacket *packet = enet_packet_create(data, size, 0);
    if (connection_is_host()) {
        broadcast_except(NULL, packet, channel);
    } else if (server) {
        enet_peer_send(server, channel, packet);
    }
}

void connection_send(uint32_t id, uint8_t channel, const char *data, size_t size) {
    ENetPacket *packet = enet_packet_create(NULL, 0, 0);
    enet_packet_resize(packet, sizeof(id) + sizeof(uint32_t) + size);
    pack_format((char *)packet->data, "ua", id, data, size);
    if (connection_is_host()) {
        broadcast_except(NULL, packet, channel);
    } else if (server) {
        enet_peer_send(server, channel, packet);
    }
}
