#include "connection.h"

#include "game_state.h"
#include "console.h"
#include "entity.h"
#include "pack.h"
#include "list.h"

#include <enet/enet.h>
#include <allegro5/allegro.h>

#include <stdio.h>

static const size_t CHANNEL_COUNT = 3;
static const uint32_t USER_ID_ASSIGN_PACKET = 0xDEADBEEF;

#define CONNECTION_EVENT_ID_BASE 0x1E6E7
const unsigned int CONNECTION_RECEIVE_EVENT_ID = CONNECTION_EVENT_ID_BASE;
const unsigned int CONNECTION_JOIN_EVENT_ID = CONNECTION_EVENT_ID_BASE + 1;
const unsigned int CONNECTION_LEAVE_EVENT_ID = CONNECTION_EVENT_ID_BASE + 2;

List *peers;
ENetHost *host;
ENetPeer *server;
uint32_t user_id;
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
        enet_peer_reset(server);
    }

    enet_host_destroy(host);
    user_id = 0;
    host = NULL;
    server = NULL;
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

    if (host) {
        ALLEGRO_EVENT event;
        event.type = CONNECTION_JOIN_EVENT_ID;
        al_emit_user_event(&event_source, &event, NULL);
    }

    return host != NULL;
}

uint32_t connection_get_user_id() {
    return user_id;
}

bool connection_join(const char *hostname, uint16_t port) {
    destroy_host();

    ENetAddress address;
    enet_address_set_host(&address, hostname);
    address.port = port;

    host = enet_host_create(NULL, 1, CHANNEL_COUNT, 0, 0);
    server = enet_host_connect(host, &address, CHANNEL_COUNT, 0);

    ENetEvent event;
    if (enet_host_service(host, &event, 1000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {

    }

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

void connection_send_to(uint32_t id, uint8_t channel, const char *data, size_t size, ENetPeer* target) {
    ENetPacket *packet = enet_packet_create(NULL, sizeof(user_id) +sizeof(id) + sizeof(uint32_t) + size, 0);
    pack_format((char *)packet->data, "uua", user_id, id, data, size);

    if (target) {
        enet_peer_send(target, channel, packet);
        return;
    }

    if (connection_is_host()) {
        broadcast_except(NULL, packet, channel);
    } else if (server) {
        enet_peer_send(server, channel, packet);
    }
}

void connection_update() {
    if (!host) return;

    ENetEvent ev;
    while (enet_host_service(host, &ev, 0)) {
        switch (ev.type) {
        case ENET_EVENT_TYPE_CONNECT: {
            if (connection_is_host()) {
                uint32_t new_sender_id = list_push(peers, ev.peer) + 1;

                connection_send_to(
                    USER_ID_ASSIGN_PACKET,
                    CHANNEL_COUNT - 1,
                    (const char *)&new_sender_id,
                    sizeof(new_sender_id),
                    ev.peer
                );
            }
            break;
        }
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
                broadcast_except(ev.peer, ev.packet, ev.channelID);
            }

            if (ev.channelID > 0) {
                uint32_t sender_id, id;
                char *data = NULL;
                size_t data_len;
                unpack_format((char *)ev.packet->data, "uua", &sender_id, &id, &data, &data_len);

                ALLEGRO_EVENT event;
                event.type = CONNECTION_RECEIVE_EVENT_ID;
                event.user.data1 = id;
                event.user.data2 = (intptr_t)data;
                event.user.data3 = sender_id;
                al_emit_user_event(&event_source, &event, NULL);
            } else {
                //channel 0 is exclusive for entity updates
                entity_sync((const char *)ev.packet->data);
            }
            break;
        case ENET_EVENT_TYPE_NONE:
            break;
        }
    }
}

void connection_on_event(ALLEGRO_EVENT *event) {
    if (event->type == CONNECTION_RECEIVE_EVENT_ID) {
        if (event->user.data1 == USER_ID_ASSIGN_PACKET) {
            uint32_t *new_id = (uint32_t *)event->user.data2;
            user_id = *new_id;

            ALLEGRO_EVENT event;
            event.type = CONNECTION_JOIN_EVENT_ID;
            al_emit_user_event(&event_source, &event, NULL);
        }
    } else if (event->type == CONSOLE_EVENT_ID) {
        const char *command = (const char *)event->user.data1;
        char hostname[128];
        unsigned int port, max_players;
        if (sscanf(command, "join %128s %u", hostname, &port) == 2) {
            connection_join(hostname, port);
        } else if (sscanf(command, "host %u %u", &port, &max_players) == 2) {
            connection_host(port, max_players);
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
    connection_send_to(id, channel, data, size, NULL);
}
