#include <networking.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <game.h>

SOCKET connect_socket = INVALID_SOCKET;
SOCKET listen_socket = INVALID_SOCKET;
SOCKET target_socket = INVALID_SOCKET;

IPv4 parse_ip(char* str) {

    IPv4 ip = { 0 };
    sscanf(str, "%15[^:]:%9s", ip.ip, ip.port);

    return ip;
}

unsigned __stdcall receive_packets_thread(void *arg) {

    SOCKET recieving_socket = *(SOCKET*) arg;
    free(arg);

    PacketHeader header;
    char* data_buffer = NULL;
    int max_buffer_size = 0;

    while (recieving_socket != INVALID_SOCKET) {

        int bytes_received = recv(recieving_socket, (char*)&header, sizeof(PacketHeader), 0);
        if (bytes_received <= 0) break;

        if (header.length > 0) {
            if (header.length > max_buffer_size) {
                free(data_buffer);

                data_buffer = malloc(header.length);
                max_buffer_size = header.length;
            }

            int body_bytes = recv(recieving_socket, data_buffer, header.length, 0);
            if (body_bytes <= 0) break;
        }

        on_packet(header.type, data_buffer, header.length);
    }

    free(data_buffer);
    closesocket(recieving_socket);

    if (recieving_socket == target_socket) target_socket = INVALID_SOCKET;
    
    return 0;
}

unsigned __stdcall accept_connections_thread(void *arg) {

    while (listen_socket != INVALID_SOCKET) {

        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);

        SOCKET incoming_peer = accept(listen_socket, (struct sockaddr*) &client_addr, &client_addr_len);   
        if (incoming_peer == INVALID_SOCKET) break;

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

        printf("Peer connected! %s\n", client_ip);

        target_socket = incoming_peer;

        on_peer_connected();

        SOCKET* buffer_socket = malloc(sizeof(SOCKET));
        *buffer_socket = incoming_peer;
        
        _beginthreadex(NULL, 0, receive_packets_thread, buffer_socket, 0, NULL);
    }

    return 0;
}

NetworkingResult start_p2p_listener(char* port) {

    struct WSAData wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
        return NR_WSA_STARTUP_ERROR;

    struct addrinfo hints = { 0 };
    struct addrinfo *res = NULL;
    listen_socket = INVALID_SOCKET;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, port, &hints, &res) != 0) {

        WSACleanup();

        return NR_GET_ADDR_INFO_ERROR;
    }

    listen_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (listen_socket == INVALID_SOCKET) {

        freeaddrinfo(res);
        WSACleanup();

        return NR_SOCKET_ERROR;
    }

    if (bind(listen_socket, res->ai_addr, (int) res->ai_addrlen) == SOCKET_ERROR) {

        closesocket(listen_socket);
        freeaddrinfo(res);
        WSACleanup();

        return NR_BIND_ERROR;

    }

    freeaddrinfo(res);

    struct sockaddr_in bound_addr;
    int addr_len = sizeof(bound_addr);
    
    if (getsockname(listen_socket, (struct sockaddr*)&bound_addr, &addr_len) == 0) {
        int assigned_port = ntohs(bound_addr.sin_port);
        
        printf("[P2P Server]: Listening on randomly assigned port: %d\n", assigned_port);
    }

    if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {

        closesocket(listen_socket);
        WSACleanup();

        return NR_SOCKET_ERROR;

    }

    _beginthreadex(NULL, 0, accept_connections_thread, NULL, 0, NULL);
    
    return NR_SUCCESS;
}

NetworkingResult connect_to_peer(const char* peer_ip, const char* peer_port) {
    struct addrinfo hints = { 0 };
    struct addrinfo *res = NULL;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(peer_ip, peer_port, &hints, &res) != 0)
        return NR_GET_ADDR_INFO_ERROR;

    connect_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (connect_socket == INVALID_SOCKET) {
        freeaddrinfo(res);
        
        return NR_SOCKET_ERROR;
    }

    if (connect(connect_socket, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) {
        closesocket(connect_socket);
        connect_socket = INVALID_SOCKET;
        
        freeaddrinfo(res);

        return NR_CONNECTION_FAILED_ERROR;
    }

    freeaddrinfo(res);
    printf("Connected to peer!\n");

    target_socket = connect_socket;

    SOCKET* buffer_socket = malloc(sizeof(SOCKET));
    *buffer_socket = connect_socket;

    _beginthreadex(NULL, 0, receive_packets_thread, buffer_socket, 0, NULL);

    return NR_SUCCESS;
}

bool send_packet(int type, void* payload, int payload_size) {

    PacketHeader header;
    header.type = type;
    header.length = payload_size;

    int result = send(target_socket, (char*) &header, sizeof(PacketHeader), 0);
    if (result == SOCKET_ERROR) return false;

    if (payload != NULL && payload_size > 0) {
        result = send(target_socket, (char*) payload, payload_size, 0);
        if (result == SOCKET_ERROR) return false;
    }

    return true;
}

void stop_socket() {

    if (listen_socket != INVALID_SOCKET) {
        closesocket(listen_socket);
     
        listen_socket = INVALID_SOCKET;
    }

    if (connect_socket != INVALID_SOCKET) {
        closesocket(connect_socket);
        
        connect_socket = INVALID_SOCKET;
    }

    target_socket = INVALID_SOCKET;

    WSACleanup();

}