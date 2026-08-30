#include <networking/networking.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <game.h>

NetworkingState networking_state;

IPv4 parse_ip(char *str) {

    IPv4 ip = { 0 };
    sscanf(str, "%15[^:]:%9s", ip.ip, ip.port);

    return ip;
}

bool is_packet_global(PacketType type) {
    return type == PACKET_NUMBER_UPDATED || type == PACKET_MOUSE_MOVED || type == PACKET_PLAYER_CONNECTED || type == PACKET_PLAYER_DISCONNECTED;
}

void send_packet(PacketHeader header, void *data) { networking_state.is_host ? send_packet_to_everyone(header, data, -1) : send_packet_to_host(header, data); }

void init_networking() {



}

void close_sockets() {

    if (networking_state.is_host) {
        close_sockets_host();

        return;
    }

    close_sockets_client();
}