#ifndef NETWORKING
#define NETWORKING

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#pragma comment(lib, "ws2_32.lib")

typedef enum {

    PACKET_INIT_GAME,
    PACKET_NUMBER_UPDATED,
    PACKET_MOUSE_MOVED,
    PACKET_GAME_WON

} PacketType;

#pragma pack(push, 1)
typedef struct {
    uint32_t type;
    uint32_t length;
} PacketHeader;
#pragma pack(pop)

typedef struct {

    char port[10];
    char ip[16];

} IPv4;

typedef enum {

    NR_SUCCESS,
    NR_WSA_STARTUP_ERROR,
    NR_GET_ADDR_INFO_ERROR,
    NR_PACKET_SEND_ERROR,
    NR_BIND_ERROR,
    NR_LISTEN_ERRROR,
    NR_CONNECTION_FAILED_ERROR,
    NR_SOCKET_ERROR

} NetworkingResult;

IPv4 parse_ip(char* str);

NetworkingResult start_p2p_listener(char* port);
NetworkingResult connect_to_peer(const char* peer_ip, const char* peer_port);
bool send_packet(int type, void* payload, int payload_size);

void stop_socket();

#endif