#include <input.h>

#include <utils.h>
#include <generation.h>
#include <stdio.h>
#include <math.h>
#include <raymath.h>
#include <graphics.h>
#include <game.h>
#include <string.h>

Vector2 mouse_position;
char char_pressed;

float time_backspace_pressed = 0.0f;

Vector2 last_sent_mouse_position = { 0 };
double last_sent_mouse_time = 0;

char *difficulty_names[] = { "Easy", "Normal", "Hard", "Pizdec" };

void host_game_button_callback() {

    start_game(true);

}

void join_game_button_callback() {

    if (strlen(ip_input_field.text) == 0) return;

    start_game(false);

}

void back_to_menu_button_callback() {

    stop_game();
    current_screen = MENU;

}

void difficulty_button_callback() {

    difficulty += 1;
    if (difficulty > PIZDEC) difficulty = EASY;

    strncpy(difficulty_button.text, difficulty_names[difficulty], sizeof(difficulty_button.text) - 1);
    difficulty_button.text[sizeof(difficulty_button.text) - 1] = '\0'; 
}

void ip_input_field_char_pressed_callback(char c) {}

void update_button(Button *button) {

    button->hovered = CheckCollisionPointRec(mouse_position, button->rect);

    if (button->hovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) button->callback();

}

void update_input_field(InputField *field) {

    field->hovered = CheckCollisionPointRec(mouse_position, field->rect);
    
    field->selected_timer += delta;
    if (field->selected_timer >= 2.0f) field->selected_timer = 0.0f;

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) 
        field->selected = field->hovered;

    if (!field->selected) return;

    if (char_pressed != 0) {

        int len = strlen(field->text);

        if (len < field->text_max_length) {
            field->text[len] = char_pressed;
            field->text[len + 1] = '\0';
        }
    }

    if (IsKeyPressed(KEY_BACKSPACE) || time_backspace_pressed >= 0.5f) {

        int len = strlen(field->text);

        if (len > 0) field->text[len - 1] = '\0';
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V)) {

        const char *clipboard = GetClipboardText();

        if (clipboard != NULL) 
            strncat(field->text, clipboard, (field->text_max_length - strlen(field->text) - 1));
    }
}

bool is_num_editable(Vector2i position) {
    return game_state.editable_nums[position.x][position.y];
}

void select_num() {

    int key_pressed = GetKeyPressed();

    if (key_pressed >= KEY_ONE && key_pressed <= KEY_NINE) {
        game_state.current_num = key_pressed - KEY_ONE + 1;
    
        current_num_change_animation = 0.0f;
    }

}

void set_num() {

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && !IsKeyDown(KEY_LEFT_SHIFT) && game_state.current_num != -1) {

        Vector2i position = get_number_position_by_mouse(mouse_position);
        if (!is_num_editable(position)) return;

        game_state.nums[position.x][position.y] = game_state.current_num;
        check_solution(position.x, position.y);

        on_num_changed(position.x, position.y, game_state.current_num, -1);
    }

}

void remove_num() {


    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {

        Vector2i position = get_number_position_by_mouse(mouse_position);
        if (!is_num_editable(position)) return;

        game_state.nums[position.x][position.y] = -1;
        check_solution(position.x, position.y);
        
        on_num_changed(position.x, position.y, -1, -1);
    }
}

void change_approximate_nums() {

    if (!IsKeyDown(KEY_LEFT_SHIFT)) return;
    int num = game_state.current_num - 1;

    Vector2i position = get_number_position_by_mouse(mouse_position);
    if (!is_num_editable(position)) return;

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && game_state.current_num != -1) {

        game_state.approximate_nums[position.x][position.y][num] = true;

        on_num_changed(position.x, position.y, num, true);
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT) && game_state.current_num != -1) {

        game_state.approximate_nums[position.x][position.y][num] = false;

        on_num_changed(position.x, position.y, num, false);
    }
}

void update_mouse_position() {

    Vector2 mouse = GetMousePosition();

    float scale = fminf(
        (float) GetScreenWidth() / VIRTUAL_SCREEN_WIDTH, 
        (float) GetScreenHeight() / VIRTUAL_SCREEN_HEIGHT
    );

    Rectangle destination = { 
        ((float) GetScreenWidth() - ((float) VIRTUAL_SCREEN_WIDTH * scale)) * 0.5f,
        ((float) GetScreenHeight() - ((float) VIRTUAL_SCREEN_HEIGHT * scale)) * 0.5f,
        (float) VIRTUAL_SCREEN_WIDTH * scale,
        (float) VIRTUAL_SCREEN_HEIGHT * scale
    };

    mouse_position = (Vector2) {
        (int) Clamp((mouse.x - destination.x) / scale, 0.0f, VIRTUAL_SCREEN_WIDTH),
        (int) Clamp((mouse.y - destination.y) / scale, 0.0f, VIRTUAL_SCREEN_HEIGHT)
    };
}

void update_mouse_network() {

    if (last_sent_mouse_position.x == mouse_position.x || last_sent_mouse_position.y == mouse_position.y) return;
    
    double time = GetTime();
    if (time - last_sent_mouse_time < 0.02) return;

    last_sent_mouse_position = mouse_position;
    last_sent_mouse_time = time;

    on_mouse_moved(mouse_position.x, mouse_position.y);

}

void process_inputs() {

    update_mouse_position();
    update_mouse_network();

    char_pressed = GetCharPressed();

    if (IsKeyDown(KEY_BACKSPACE)) time_backspace_pressed += delta;
    if (IsKeyReleased(KEY_BACKSPACE)) time_backspace_pressed = 0.0f;

    switch (current_screen) {
    
        case (MENU): {

            update_button(&host_game_button);
            update_button(&join_game_button);
            update_button(&difficulty_button);

            update_input_field(&ip_input_field);
            update_input_field(&name_input_field);

            break;
        }

        case (GAME): {

            select_num();
            
            if (networking_state.is_connected) {
                
                set_num();
                remove_num();
                change_approximate_nums();

            }

            break;
        }

        case (WON): {

            update_button(&back_to_menu_button);

        }

    };    
}