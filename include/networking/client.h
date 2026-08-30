#ifndef NETWORKING_CLIENT_H
#define NETWORKING_CLIENT_H

#include <networking/networking.h>
#include <game.h>

bool join_lobby(IPv4 address, char *name);

void send_packet_to_host(PacketHeader header, void *payload);

void close_sockets_client();

#endif