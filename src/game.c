#include <game.h>

#include <raylib.h>
#include <stdlib.h>
#include <generation.h>
#include <graphics.h>
#include <stdio.h>
#include <string.h>

GameState game_state;
NetworkingState networking_state;

Screen current_screen;

void start_game(bool is_host) {

    game_state = (GameState) {

        .current_num = -1,
        
        .nums = { -1 },
        .approximate_nums = { 0 },
        .editable_nums = { 0 },
        
        .game_started_time = 0,
        .game_won_time = 0

    };

    networking_state = (NetworkingState) {

        .mouse_x = 0,
        .mouse_y = 0,

        .is_host = is_host,
        .is_connected = false

    };

    IPv4 ipv4 = parse_ip(ip_input_field.text);
    if (strlen(ipv4.ip) == 0 || strlen(ipv4.port) == 0) {

        sprintf(error_text, "Invalid IP");

    };

    NetworkingResult result;

    if (networking_state.is_host) {

        fill_nums(game_state.nums);
        memcpy(game_state.solution, game_state.nums, sizeof(game_state.nums));
        remove_nums(game_state.nums, game_state.editable_nums);

        result = start_p2p_listener(ipv4.port);
        if (result != NR_SUCCESS) {
            sprintf(error_text, "Can't start p2p listener (%d)", result);

            return;
        };

        current_screen = GAME;

        return;
    }

    result = start_p2p_listener("0");
    if (result != NR_SUCCESS) {
        sprintf(error_text, "Can't start p2p listener (%d)", result);

        return;
    };

    result = connect_to_peer(ipv4.ip, ipv4.port);
    if (result != NR_SUCCESS) {
        sprintf(error_text, "Can't connect to peer (%d)", result);

        return;
    }

    current_screen = GAME;
}

void stop_game() {

    game_state = (GameState) { 0 };
    networking_state = (NetworkingState) { 0 };

    stop_socket();

}

void check_solution(int x, int y) {

    if (x != -1 && y != -1) 
        update_validity(x, y);

    if (memcmp(game_state.nums, game_state.solution, sizeof(game_state.nums)) != 0) return;

    current_screen = WON;
    game_state.game_won_time = GetTime() - game_state.game_started_time;

    if (networking_state.is_host) 
        send_packet(PACKET_GAME_WON, 0, sizeof(0));
}

void on_packet(PacketType type, void* data_buffer, int data_length) {

    char* data_ptr = (char*)data_buffer;

    switch (type) {
        case PACKET_INIT_GAME: {

            int expected_size = sizeof(game_state.nums) + sizeof(game_state.editable_nums);
            if (data_length == expected_size) {
                
                memcpy(game_state.nums, data_ptr, sizeof(game_state.nums));
                memcpy(game_state.editable_nums, data_ptr + sizeof(game_state.nums), sizeof(game_state.editable_nums));
                
                if (!networking_state.is_host) networking_state.is_connected = true;

                game_state.game_started_time = GetTime();
            }

            break;
        }

        case PACKET_NUMBER_UPDATED: {

            int expected_size = sizeof(int) * 4;
            if (data_length == expected_size) {

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
            }

            break;
        }

        case PACKET_MOUSE_MOVED: {

            int expected_size = sizeof(int) * 2;
            if (data_length == expected_size) {

                memcpy(&networking_state.mouse_x, data_ptr, sizeof(int));
                memcpy(&networking_state.mouse_y, data_ptr + sizeof(int), sizeof(int));

            }

            break;
        }

        case PACKET_GAME_WON: {

            current_screen = WON;
            game_state.game_won_time = GetTime() - game_state.game_started_time;


        }

        default: break;
    }
}

void on_peer_connected() {

    networking_state.is_connected = true;

    int size = sizeof(game_state.nums) + sizeof(game_state.editable_nums);
    char* buffer = malloc(size);
    memcpy(buffer, game_state.nums, sizeof(game_state.nums));
    memcpy(buffer + sizeof(game_state.nums), game_state.editable_nums, sizeof(game_state.editable_nums));

    send_packet(PACKET_INIT_GAME, buffer, size);

    game_state.game_started_time = GetTime();

    free(buffer);
}

void on_num_changed(int x, int y, int new_num, int approximate_state) {

    int size = sizeof(int) * 4;
    char* buffer = malloc(size);

    memcpy(buffer, &x, sizeof(int));
    memcpy(buffer + sizeof(int), &y, sizeof(int));
    memcpy(buffer + sizeof(int) * 2, &new_num, sizeof(int));
    memcpy(buffer + sizeof(int) * 3, &approximate_state, sizeof(int));

    send_packet(PACKET_NUMBER_UPDATED, buffer, size);

    free(buffer);
}

void on_mouse_moved(int x, int y) {

    int size = sizeof(int) * 2;
    char *buffer = malloc(size);

    memcpy(buffer, &x, sizeof(int));
    memcpy(buffer + sizeof(int), &y, sizeof(int));

    send_packet(PACKET_MOUSE_MOVED, buffer, size);

    free(buffer);
}