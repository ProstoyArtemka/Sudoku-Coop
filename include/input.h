#ifndef INPUT_H
#define INPUT_H

#include <game.h>
#include <raylib.h>

extern Vector2 mouse_position;

void host_game_button_callback();
void join_game_button_callback();
void back_to_menu_button_callback();
void difficulty_button_callback();

void ip_input_field_char_pressed_callback(char c);

void process_inputs();

#endif