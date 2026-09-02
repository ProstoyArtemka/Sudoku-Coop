#include <game.h>

#include <raylib.h>
#include <stdlib.h>
#include <generation.h>
#include <graphics.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

GameState game_state;

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

    IPv4 ipv4 = parse_ip(ip_input_field.text);
    if (strlen(ipv4.ip) == 0 || strlen(ipv4.port) == 0) {

        sprintf(error_text, "Invalid IP");

        return;
    };

    char* name = name_input_field.text;
    if (strlen(name) == 0 || strlen(name) > 24) {

        sprintf(error_text, "Invalid name");

        return;
    }

    if (is_host) {

        fill_nums(game_state.nums);
        memcpy(game_state.solution, game_state.nums, sizeof(game_state.nums));
        remove_nums(game_state.nums, game_state.editable_nums);

        
        if (!host_lobby(ipv4)) {
            sprintf(error_text, "Can't host lobby.");

            return;
        };

        current_screen = GAME;

        return;
    }

    if (!join_lobby(ipv4, name)) {
        sprintf(error_text, "Can't join lobby.");
        
        return;
    }

    current_screen = GAME;
}

void stop_game() {

    game_state = (GameState) { 0 };
    networking_state = (NetworkingState) { 0 };

    close_sockets();

}

void check_solution(int x, int y) {

    if (x != -1 && y != -1)
        update_validity(x, y);

    if (memcmp(game_state.nums, game_state.solution, sizeof(game_state.nums)) != 0) return;

    game_state.game_won_time = time(NULL) - game_state.game_started_time;

    on_game_won();

    if (networking_state.is_host) {
        PacketHeader header = (PacketHeader) { .type = PACKET_GAME_WON, .length = sizeof(int) };
        
        char* buffer = malloc(sizeof(int));
        memcpy(buffer, &game_state.game_won_time, sizeof(int));

        send_packet_to_everyone(header, buffer, 0);

        free(buffer);
    }
}

void on_game_won() {

    current_screen = WON;

    spawn_confetti();

}

void on_player_connected(int player_index) {

    if (game_state.game_started_time == 0)
        game_state.game_started_time = time(NULL);

    int timer_seconds = (time(NULL) - game_state.game_started_time);



    int size = sizeof(game_state.nums) + sizeof(game_state.editable_nums) + sizeof(game_state.approximate_nums) + sizeof(int);
    char *buffer = malloc(size);

    memcpy(buffer, game_state.nums, sizeof(game_state.nums));
    memcpy(buffer + sizeof(game_state.nums), game_state.editable_nums, sizeof(game_state.editable_nums));
    memcpy(buffer + sizeof(game_state.nums) + sizeof(game_state.editable_nums), game_state.approximate_nums, sizeof(game_state.approximate_nums));
    memcpy(buffer + sizeof(game_state.nums) + sizeof(game_state.editable_nums) + sizeof(game_state.approximate_nums), &timer_seconds, sizeof(int));

    PacketHeader header = (PacketHeader) { .type = PACKET_INIT_GAME, .length = size };
    send_packet_to_client(player_index, header, buffer, -1);
    free(buffer);



    size = PLAYER_NAME_SIZE;
    buffer = malloc(size);
    memcpy(buffer, name_input_field.text, PLAYER_NAME_SIZE);

    header = (PacketHeader) { .type = PACKET_PLAYER_CONNECTED, .length = size };
    send_packet_to_client(player_index, header, buffer, -1);

    for (int i = 1; i < MAX_AMOUNT_OF_PLAYERS; i++) {
        if (i == player_index) continue;
        if (!networking_state.players[i].is_connected) continue;

        memcpy(buffer, networking_state.players[i].name, PLAYER_NAME_SIZE);
        send_packet_to_client(player_index, header, buffer, i);
    }

    free(buffer);
}

void on_player_disconnected(int player_index) {

    if (networking_state.is_host) {

        PacketHeader header = (PacketHeader) { .type = PACKET_PLAYER_DISCONNECTED, .length = sizeof(int) };
        send_packet(header, &player_index);

        return;
    }
    
    stop_game();

    current_screen = MENU;
    strcpy(error_text, "Host disconnected...");

}

void on_num_changed(int x, int y, int new_num, int approximate_state) {

    int size = sizeof(int) * 4;
    char *buffer = malloc(size);

    memcpy(buffer, &x, sizeof(int));
    memcpy(buffer + sizeof(int), &y, sizeof(int));
    memcpy(buffer + sizeof(int) * 2, &new_num, sizeof(int));
    memcpy(buffer + sizeof(int) * 3, &approximate_state, sizeof(int));

    PacketHeader header = (PacketHeader) { .type = PACKET_NUMBER_UPDATED, .length = size };
    send_packet(header, buffer);

    free(buffer);
}

void on_mouse_moved(int x, int y) {

    int size = (sizeof(int) * 2);
    char *buffer = malloc(size);

    memcpy(buffer, &x, sizeof(int));
    memcpy(buffer + sizeof(int), &y, sizeof(int));

    PacketHeader header = (PacketHeader) { .type = PACKET_MOUSE_MOVED, .length = size };
    send_packet(header, buffer);

    free(buffer);
}