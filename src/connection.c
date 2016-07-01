#include "connection.h"

#include "game_state.h"
#include "entity.h"
#include "pack.h"

#include <enet/enet.h>
#include <allegro5/allegro.h>

static const size_t CHANNEL_COUNT = 1;
static const size_t FRAMES_PER_REFRESH = 10;

static const char *ENTITY_SCHEMA = "ufffffffff";
static const size_t ENTITY_P_LEN = 4 * 10;

typedef enum  {
    PACKET_ENTITY_UPDATE = 1
} PacketType;

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

void connection_sync_body(cpBody *body, void *extra) {
    (void) extra;

    Entity *ent = cpBodyGetUserData(body);
    if (!host || !server || !entity_valid(ent)) {
        return;
    }

    char data[ENTITY_P_LEN + 1];
    data[0] = PACKET_ENTITY_UPDATE;
    cpVect pos = cpBodyGetPosition(body);
    cpVect force = cpBodyGetForce(body);
    cpVect velocity = cpBodyGetVelocity(body);
    float angularV = cpBodyGetAngularVelocity(body);
    float torque = cpBodyGetTorque(body);
    pack_format(data, ENTITY_SCHEMA, entity_id(ent),
                pos.x, pos.y, cpBodyGetAngle(body),
                velocity.x, velocity.y,
                force.x, force.y,
                angularV, torque);

    ENetPacket *packet = enet_packet_create(data + 1, sizeof(data), ENET_PACKET_FLAG_NO_ALLOCATE);
    enet_peer_send(server, 0, packet);
}

void connection_update() {
    refresh_counter++;
    if (refresh_counter != FRAMES_PER_REFRESH) {
        return;
    }

    cpSpaceEachBody(game.space, connection_sync_body, NULL);

    char entity_updates[ENTITY_P_LEN][256];
    size_t entity_update_count = 0;

    ENetEvent event;
    for (size_t i = 0; i < 10; ++i) {
        int ret = enet_host_service(host, &event, 0);
        if (ret < 0) {
            break;
        }
        if (event.type == ENET_EVENT_TYPE_RECEIVE && event.packet->dataLength > 0) {
            switch (event.packet->data[0]) {
                case PACKET_ENTITY_UPDATE:
                    memcpy(entity_updates[entity_update_count], event.packet->data + 1, ENTITY_P_LEN);
                    entity_update_count++;
                    break;
            }
        }
    }
    for (size_t i = 0; i < entity_update_count; ++i) {
        size_t id;
        cpVect position, force, velocity;
        float angularVelocity, torque;
        unpack_format(entity_updates[i], ENTITY_SCHEMA, &id,
                      &position.x, &position.y,
                      &force.x, &force.y,
                      &velocity.x, &velocity.y,
                      &angularVelocity, &torque);

    }
}
