#include <networking/client.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <time.h>

SOCKET host = INVALID_SOCKET;

void on_packet_client(PacketType type, void *data_buffer, int data_length) {

    char *data_ptr = (char*) data_buffer;

    int player_index = -1;
    if (is_packet_global(type)) {
        player_index = *(data_ptr + data_length - sizeof(int)) + 1;
    
        data_length -= sizeof(int);
    }

    switch (type) {

        case PACKET_INIT_GAME: {

            int expected_size = sizeof(game_state.nums) + sizeof(game_state.editable_nums) + sizeof(game_state.approximate_nums) + sizeof(int);
            if (data_length != expected_size) break;
            
            int timer_seconds;

            memcpy(game_state.nums, data_ptr, sizeof(game_state.nums));
            memcpy(game_state.editable_nums, data_ptr + sizeof(game_state.nums), sizeof(game_state.editable_nums));
            memcpy(game_state.approximate_nums, data_ptr + sizeof(game_state.nums) + sizeof(game_state.editable_nums), sizeof(game_state.approximate_nums));
            memcpy(&timer_seconds, data_ptr + sizeof(game_state.nums) + sizeof(game_state.editable_nums) + sizeof(game_state.approximate_nums), sizeof(int));

            networking_state.is_connected = true;

            game_state.game_started_time = time(NULL) - timer_seconds;

            break;
        }

        case PACKET_NUMBER_UPDATED: {

            int expected_size = sizeof(int) * 4;
            if (data_length != expected_size) break;

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
            if (data_length != expected_size) break;
            if (player_index == -1) break;

            memcpy(&networking_state.players[player_index].mouse_x, data_ptr, sizeof(int));
            memcpy(&networking_state.players[player_index].mouse_y, data_ptr + sizeof(int), sizeof(int));

            break;
        }

        case PACKET_GAME_WON: {

            int expected_size = sizeof(int);
            if (data_length != expected_size) break;

            memcpy(&game_state.game_won_time, data_ptr, sizeof(int));

            on_game_won();

            break;
        }

        case PACKET_PLAYER_CONNECTED: {

            memcpy(networking_state.players[player_index].name, data_ptr, PLAYER_NAME_SIZE);
            networking_state.players[player_index].is_connected = true;

            break;
        }

        case PACKET_PLAYER_DISCONNECTED: {

            networking_state.players[player_index].is_connected = false;

            break;
        }

        default: break;
    }
}

unsigned __stdcall receive_packets_thread_client(void *arg) {

    PacketHeader header;
    char *data_buffer = NULL;
    int max_buffer_size = 0;

    while (host != INVALID_SOCKET) {

        int bytes_received = recv(host, (char*) &header, sizeof(PacketHeader), 0);
        if (bytes_received <= 0) break;

        if (header.length > 0) {
            if (header.length > max_buffer_size) {
                free(data_buffer);

                data_buffer = malloc(header.length);
                max_buffer_size = header.length;
            }

            int body_bytes = recv(host, data_buffer, header.length, 0);
            if (body_bytes <= 0) break;
        }

        on_packet_client(header.type, data_buffer, header.length);
    }

    if (data_buffer != NULL) free(data_buffer);

    on_player_disconnected(0);

    return 0;
}

NetworkingResult connect_to_host(IPv4 address) {
    
    struct WSAData wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
        return NR_WSA_STARTUP_ERROR;

    struct addrinfo hints = { 0 };
    struct addrinfo *res = NULL;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(address.ip, address.port, &hints, &res) != 0) {
     
        WSACleanup();
     
        return NR_GET_ADDR_INFO_ERROR;
    }

    host = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (host == INVALID_SOCKET) {
        freeaddrinfo(res);

        WSACleanup();
        
        return NR_SOCKET_ERROR;
    }

    if (connect(host, res->ai_addr, (int) res->ai_addrlen) == SOCKET_ERROR) {
        closesocket(host);
        host = INVALID_SOCKET;
        
        freeaddrinfo(res);
        WSACleanup();

        return NR_CONNECTION_FAILED_ERROR;
    }

    freeaddrinfo(res);

    return NR_SUCCESS;
}

bool join_lobby(IPv4 address, char *name) {

    networking_state = (NetworkingState) {

        .players = { 0 },

        .is_host = false,
        .is_connected = false

    };

    for (int i = 0; i < MAX_AMOUNT_OF_PLAYERS; i++)
        networking_state.players[i] = (ConnectedPlayer) { 0, 0 };

    if (connect_to_host(address) != NR_SUCCESS)
        return false;

    _beginthreadex(NULL, 0, receive_packets_thread_client, 0, 0, NULL);

    PacketHeader header = (PacketHeader) { .type = PACKET_PLAYER_CONNECTED, .length = PLAYER_NAME_SIZE };
    send_packet_to_host(header, name);

    return true;
}

void send_packet_to_host(PacketHeader header, void *payload) {

    int result = send(host, (char*) &header, sizeof(PacketHeader), 0);
    if (result == SOCKET_ERROR) return;

    if (payload != NULL && header.length > 0) {
        result = send(host, (char*) payload, header.length, 0);

        if (result == SOCKET_ERROR) return;
    }
}

void close_sockets_client() {

    if (host != INVALID_SOCKET) closesocket(host);

    WSACleanup();

}