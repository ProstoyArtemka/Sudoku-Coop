#ifndef NETWORKING
#define NETWORKING

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <basetsd.h>

#define MAX_AMOUNT_OF_PLAYERS 4
#define PLAYER_NAME_SIZE (sizeof(char) * 32)

#define numof(array) ( (int) ( sizeof(array) / sizeof((array)[0]) ) )

#pragma comment(lib, "ws2_32.lib")

typedef enum {

    PACKET_INIT_GAME,
    PACKET_GAME_WON,

    PACKET_NUMBER_UPDATED,
    PACKET_MOUSE_MOVED,
    PACKET_PLAYER_CONNECTED,
    PACKET_PLAYER_DISCONNECTED,

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

typedef struct {

    int mouse_x;
    int mouse_y;

    char name[PLAYER_NAME_SIZE];

    bool is_connected;

} ConnectedPlayer;

typedef struct {

    ConnectedPlayer players[MAX_AMOUNT_OF_PLAYERS];

    bool is_host;
    bool is_connected;

} NetworkingState;

extern NetworkingState networking_state;

IPv4 parse_ip(char *str);
bool is_packet_global(PacketType type);

void init_networking();
void close_sockets();

#include <networking/host.h>
#include <networking/client.h>

void send_packet(PacketHeader header, void *data);

#endif