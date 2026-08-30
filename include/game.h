#ifndef GAME_H
#define GAME_H

#include <networking/networking.h>

typedef enum {

    MENU,
    GAME,
    WON

} Screen;

typedef struct {

    int nums[9][9];
    int solution[9][9];
    bool approximate_nums[9][9][9];

    bool editable_nums[9][9];
    bool invalid_nums[9][9];

    int current_num;

    double game_started_time;
    double game_won_time;

} GameState;

extern GameState game_state;
extern NetworkingState networking_state;

extern Screen current_screen;

void start_game(bool is_host);
void stop_game();

void check_solution(int x, int y);

void on_player_connected(int player_index);
void on_player_disconnected(int player_index);

void on_num_changed(int x, int y, int new_num, int approximate_state);
void on_mouse_moved(int x, int y);

#endif