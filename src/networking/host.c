#include <networking/host.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <process.h>

SOCKET connections_listener_host;
SOCKET connected_sockets[MAX_AMOUNT_OF_PLAYERS - 1];
CRITICAL_SECTION sockets_mutex;

HANDLE accept_connections_thread;
HANDLE recieve_packets_thread;

bool is_shutting_down = false;

int add_socket(SOCKET socket) {

    EnterCriticalSection(&sockets_mutex);

    for (int i = 0; i < numof(connected_sockets); i++) {
        if (connected_sockets[i] != INVALID_SOCKET) continue;

        connected_sockets[i] = socket;

        LeaveCriticalSection(&sockets_mutex);
        return i;
    }

    LeaveCriticalSection(&sockets_mutex);
    return -1;
}

int remove_socket(SOCKET socket) {

    EnterCriticalSection(&sockets_mutex);

    for (int i = 0; i < numof(connected_sockets); i++) {
        
        if (socket == connected_sockets[i]) {

            closesocket(socket);
            connected_sockets[i] = INVALID_SOCKET;

            LeaveCriticalSection(&sockets_mutex);
            return i;
        }
    }

    LeaveCriticalSection(&sockets_mutex);
    return -1;
}

bool is_sockets_empty() {
    
    EnterCriticalSection(&sockets_mutex);

    for (int i = 0; i < numof(connected_sockets); i++) {

        if (connected_sockets[i] != INVALID_SOCKET) {
            LeaveCriticalSection(&sockets_mutex);

            return false;
        }
    }
    LeaveCriticalSection(&sockets_mutex);

    return true;
}

void on_packet_host(PacketType type, void *data_buffer, int data_length, int socket_from_index) {

    char *data_ptr = (char*) data_buffer;

    if (is_packet_global(type)) {

        PacketHeader header = (PacketHeader) { .type = type, .length = data_length };
        send_packet_to_everyone_except(header, data_buffer, socket_from_index, socket_from_index);

    }

    switch (type) {

        case PACKET_NUMBER_UPDATED: {

            int expected_size = sizeof(int) * 4;
            if (data_length < expected_size) break;

            int x;
            int y;
            int num;
            int approximate_state;

            memcpy(&x, data_ptr, sizeof(int));
            memcpy(&y, data_ptr + sizeof(int), sizeof(int));
            memcpy(&num, data_ptr + sizeof(int) * 2, sizeof(int));
            memcpy(&approximate_state, data_ptr + sizeof(int) * 3, sizeof(int));

            if (approximate_state == -1) {
                game_state.nums[x][y] = num;

                check_solution(x, y);

                break;
            }

            game_state.approximate_nums[x][y][num] = approximate_state;

            break;
        }

        case PACKET_MOUSE_MOVED: {

            int expected_size = sizeof(int) * 2;
            if (data_length < expected_size) break;

            int player_mouse_x;
            int player_mouse_y;

            memcpy(&networking_state.players[socket_from_index].mouse_x, data_ptr, sizeof(int));
            memcpy(&networking_state.players[socket_from_index].mouse_y, data_ptr + sizeof(int), sizeof(int));

            break;
        }

        case PACKET_PLAYER_CONNECTED: {

            int expected_size = PLAYER_NAME_SIZE;
            if (expected_size != data_length) break;

            memcpy(networking_state.players[socket_from_index].name, data_ptr, PLAYER_NAME_SIZE);
            networking_state.players[socket_from_index].is_connected = true;

            break;
        }

        default: break;
    }
}

unsigned __stdcall receive_packets_thread_host(void *arg) {

    int socket_index = *(int*) arg;
    free(arg);

    SOCKET recieving_socket = connected_sockets[socket_index];

    PacketHeader header;
    char *data_buffer = NULL;
    int max_buffer_size = 0;

    while (recieving_socket != INVALID_SOCKET && !is_shutting_down) {

        int bytes_received = recv(recieving_socket, (char*) &header, sizeof(PacketHeader), 0);
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

        on_packet_host(header.type, data_buffer, header.length, socket_index);
    }

    if (data_buffer != NULL) free(data_buffer);
    if (is_shutting_down) return 0;

    remove_socket(recieving_socket);

    PacketHeader disconnect_header = (PacketHeader) { .type = PACKET_PLAYER_DISCONNECTED, .length = sizeof(int) };
    send_packet_to_everyone(disconnect_header, &socket_index, -1);

    networking_state.players[socket_index].is_connected = false;

    if (is_sockets_empty()) 
        networking_state.is_connected = false;

    return 0;
}

unsigned __stdcall accept_connections_thread_host(void *arg) {

    SOCKET listener = *((SOCKET*) arg);
    while (listener != INVALID_SOCKET && !is_shutting_down) {

        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);

        SOCKET incoming_connection = accept(listener, (struct sockaddr*) &client_addr, &client_addr_len);   
        if (incoming_connection == INVALID_SOCKET) break;

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);


        if (is_shutting_down) return 0;
        int socket_index = add_socket(incoming_connection);

        if (socket_index == -1) {
            closesocket(incoming_connection);

            break;
        }

        networking_state.is_connected = true;
        on_player_connected(socket_index);

        int* socket_index_buffer = malloc(sizeof(int));
        *socket_index_buffer = socket_index;

        recieve_packets_thread = (HANDLE) _beginthreadex(NULL, 0, receive_packets_thread_host, socket_index_buffer, 0, NULL);
    }

    return 0;
}

void send_packet_to_everyone(PacketHeader header, void *payload, int from_player) {

    for (int i = 0; i < numof(connected_sockets); i++) {
        if (connected_sockets[i] == INVALID_SOCKET) continue;

        send_packet_to_client(i, header, payload, from_player);
    }
}

void send_packet_to_everyone_except(PacketHeader header, void *payload, int except_socket, int from_player) {

    for (int i = 0; i < numof(connected_sockets); i++) {
        if (connected_sockets[i] == INVALID_SOCKET) continue;
        if (i == except_socket) continue;

        send_packet_to_client(i, header, payload, from_player);
    }
}

void send_packet_to_client(int socket_index, PacketHeader header, void *payload, int from_player) {

    if (socket_index < 0 || socket_index >= numof(connected_sockets)) return;

    if (is_packet_global(header.type)) {
    
        int size = header.length + sizeof(int);
        char* global_payload = malloc(size);

        memcpy(global_payload, (char*) payload, header.length);
        *(global_payload + header.length) = from_player;
        
        header.length = size;
        int result = send(connected_sockets[socket_index], (char*) &header, sizeof(PacketHeader), 0);
        if (result == SOCKET_ERROR) return;

        send(connected_sockets[socket_index], global_payload, size, 0);

        free(global_payload);

        return;
    }

    int result = send(connected_sockets[socket_index], (char*) &header, sizeof(PacketHeader), 0);
    if (result == SOCKET_ERROR) return;

    if (payload == NULL || header.length == 0) return;
        
    send(connected_sockets[socket_index], (char*) payload, header.length, 0);
}

NetworkingResult start_connection_listener(char *port, SOCKET *listener) {

    struct WSAData wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
        return NR_WSA_STARTUP_ERROR;

    if (*listener != INVALID_SOCKET) {
        closesocket(*listener);

        *listener = INVALID_SOCKET;
    }

    struct addrinfo hints = { 0 };
    struct addrinfo *res = NULL;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, port, &hints, &res) != 0) {

        WSACleanup();

        return NR_GET_ADDR_INFO_ERROR;
    }

    *listener = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (*listener == INVALID_SOCKET) {

        freeaddrinfo(res);
        WSACleanup();

        return NR_SOCKET_ERROR;
    }

    if (bind(*listener, res->ai_addr, (int) res->ai_addrlen) == SOCKET_ERROR) {

        closesocket(*listener);
        freeaddrinfo(res);
        WSACleanup();

        return NR_BIND_ERROR;

    }

    freeaddrinfo(res);

    struct sockaddr_in bound_addr;
    int addr_len = sizeof(bound_addr);

    if (listen(*listener, SOMAXCONN) == SOCKET_ERROR) {

        closesocket(*listener);
        WSACleanup();

        return NR_SOCKET_ERROR;

    }
    
    return NR_SUCCESS;
}


bool host_lobby(IPv4 address) {

    networking_state = (NetworkingState) {

        .players = { 0 },

        .is_host = true,
        .is_connected = false

    };

    for (int i = 0; i < MAX_AMOUNT_OF_PLAYERS; i++) {
        networking_state.players[i] = (ConnectedPlayer) { 0, 0 };
    
        if (i != MAX_AMOUNT_OF_PLAYERS - 1)
            connected_sockets[i] = INVALID_SOCKET;
    }

    InitializeCriticalSection(&sockets_mutex);

    if (start_connection_listener(address.port, &connections_listener_host) != NR_SUCCESS)
        return false;

    accept_connections_thread = (HANDLE) _beginthreadex(NULL, 0, accept_connections_thread_host, &connections_listener_host, 0, NULL);

    return true;
}

void close_sockets_host() {

    is_shutting_down = true;

    WaitForSingleObject(receive_packets_thread_host, 1000);
    WaitForSingleObject(accept_connections_thread, 1000);

    if (connections_listener_host != INVALID_SOCKET) closesocket(connections_listener_host);
    connections_listener_host = INVALID_SOCKET;

    for (int i = 0; i < MAX_AMOUNT_OF_PLAYERS; i++) {
        if (connected_sockets[i] != INVALID_SOCKET) closesocket(connected_sockets[i]);

        connected_sockets[i] = INVALID_SOCKET;
    }

    DeleteCriticalSection(&sockets_mutex);

    WSACleanup();
}