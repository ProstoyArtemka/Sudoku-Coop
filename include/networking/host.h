#ifndef NETWORKING_HOST_H
#define NETWORKING_HOST_H

#include <networking/networking.h>
#include <game.h>

bool host_lobby(IPv4 address);

void send_packet_to_everyone(PacketHeader header, void *payload, int from_player);
void send_packet_to_client(int socket_index, PacketHeader header, void *payload, int from_player);
void send_packet_to_everyone_except(PacketHeader header, void *payload, int except_socket, int from_player);

void close_sockets_host();

#endif