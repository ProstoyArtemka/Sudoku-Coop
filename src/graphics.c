
#include <graphics.h>
#include <utils.h>
#include <stdio.h>
#include <raymath.h>
#include <input.h>
#include <game.h>
#include <string.h>
#include <reasings.h>
#include <time.h>

RenderTexture2D virtual_screen;
float delta;

Vector2i last_num_position = { 0, 0 };
Vector2 lerp_num_position = { 0, 0 };
bool num_position_changed = false;

float current_num_change_animation = 0.0f;
float current_num_move_animation = 0.0f;

float game_start_animation = 0;

float won_animation = 0.0f;


Button host_game_button = {

    .text = "Host",

    .text_height = 54,
    .rect = (Rectangle) {
        .x = (1920 / 2) - 200,
        .y = 550,
        .width = 400,
        .height = 100,
    },

    .callback = host_game_button_callback
};

Button join_game_button = {

    .text = "Join",

    .text_height = 54,
    .rect = (Rectangle) {
        .x = (1920 / 2) - 200,
        .y = 675,
        .width = 400,
        .height = 100
    },

    .callback = join_game_button_callback

};

Button difficulty_button = {

    .text_height = 54,
    .rect = (Rectangle) {
        .x = (1920 / 2) - 200,
        .y = 850,
        .width = 400,
        .height = 100
    },

    .callback = difficulty_button_callback
};

InputField ip_input_field = {

    .text_max_length = 24,
    .text = "127.0.0.1:5001",
    .placeholder = "Input IP",

    .text_height = 54,
    .rect = (Rectangle) {

        .x = (1920 / 2) - 400,
        .y = 425,
        .width = 800,
        .height = 100

    },

    .selected = 0,
    .hovered = 0,

    .selected_timer = 0.0f,

    .char_pressed_callback = ip_input_field_char_pressed_callback
};

InputField name_input_field = {
  
    .text_max_length = 24,
    .text = "",
    .placeholder = "Input your name",

    .text_height = 54,
    .rect = (Rectangle) {

        .x = (1920 / 2) - 400,
        .y = 300,
        .width = 800,
        .height = 100

    },

    .selected = 0,
    .hovered = 0,

    .selected_timer = 0.0f,

    .char_pressed_callback = ip_input_field_char_pressed_callback

};

Sprite cursor_sprite = {

    .path = "./assets/cursor.png",
    .position = (Vector2) { 0, 0 },
    .size = (Vector2) { 20, 32 },
    .texture = { 0 }

};

Vector2 cursor_positions[MAX_AMOUNT_OF_PLAYERS];

Button back_to_menu_button = {

    .text = "Menu",
    .text_height = 54,

    .rect = (Rectangle) {

        .x = (1920 / 2) - 200,
        .y = (1080 / 2) - 50,
        .width = 400,
        .height = 100

    },
    
    .callback = back_to_menu_button_callback
};

char error_text[256] = "Test";

Confetti confettis[CONFETTI_AMOUNT];
float confetti_lifetime = 0.0f;

void draw_box() {

    Rectangle box_rectangle = (Rectangle) { 
        .x = 555.0f,
        .y = 135.0f,

        .width = 810.0f,
        .height = 810.0f
    };

    draw_rectangle_with_stroke(&box_rectangle, 5, BLACK, WHITE);

    for (int i = 1; i < 9; i++) {

        if (i % 3 == 0) continue;

        Vector2 start_position = { 550.0f + (90.0f * i), 140.0f };;

        Rectangle rect = { start_position.x, start_position.y, 3, 800.0f};
        DrawRectangleRec(rect, GRAY);

        start_position = (Vector2) { 555.0f, 140.0f + (90.0f * i) };
        rect = (Rectangle) { start_position.x, start_position.y, 805.0f, 3};
        DrawRectangleRec(rect, GRAY);
    }

    for (int i = 1; i < 3; i++) {

        Vector2 start_position = { 550.0f + (90.0f * i * 3), 140.0f };;

        Rectangle rect = { start_position.x, start_position.y, 5, 800.0f};
        DrawRectangleRec(rect, WHITE);

        start_position = (Vector2) { 555.0f, 140.0f + (90.0f * i * 3) };
        rect = (Rectangle) { start_position.x, start_position.y, 805.0f, 5};
        DrawRectangleRec(rect, WHITE);

    }
}

void draw_current_num() {

    bool is_shift_down = IsKeyDown(KEY_LEFT_SHIFT);
    float size = is_shift_down ? 24 : 54;

    if (game_state.current_num <= 0) return;

    Vector2i num_position = get_number_position_by_mouse(mouse_position);

    if (num_position.x != last_num_position.x || num_position.y != last_num_position.y) {
        
        if (game_state.editable_nums[num_position.x][num_position.y]) {
            lerp_num_position = Vector2Normalize((Vector2) { num_position.x - last_num_position.x, num_position.y - last_num_position.y });
        
            current_num_move_animation = 0.0f;
        }

        last_num_position = num_position;
    }

    if (num_position.x < 0 || num_position.x > 8 || num_position.y < 0 || num_position.y > 8) return;
    if (!game_state.editable_nums[num_position.x][num_position.y]) return;

    tick(true, &current_num_change_animation, 10.0f, delta);
    tick(true, &current_num_move_animation, 10.0f, delta);

    float move_ease = EaseExpoOut(current_num_move_animation, 0.0f, 1.0f, 1.0f);

    Vector2 new_num_position = {
        (num_position.x * 90) - ((lerp_num_position.x * 90) * (1.0f - move_ease)),
        (num_position.y * 90) - ((lerp_num_position.y * 90) * (1.0f - move_ease))
    };

    Vector2 text_position = {
        new_num_position.x + 595.0f - (is_shift_down ? 22.5f : 0.0f) + (is_shift_down ? ((game_state.current_num - 1) % 3) * 24.0f : 0.0f),
        new_num_position.y + 185.0f - (is_shift_down ? 22.5f : 0.0f) + (is_shift_down ? ((game_state.current_num - 1) / 3) * 24.0f : 0.0f),
    };

    char text[2];
    snprintf(text, sizeof(text), "%d", game_state.current_num);

    draw_centered_text(text, size * EaseCircInOut(current_num_change_animation, 0.0f, 1.0f, 1.0f), text_position, WHITE);

}

void draw_approximate_nums(int x, int y) {

    Vector2 cell_position = { 572.5f + (x * 90.0f), 162.5f + (y * 90.0f) };

    for (int i = 0; i < 9; i++) {
        if (!game_state.approximate_nums[x][y][i]) continue;

        int grid_x = i % 3;
        int grid_y = i / 3;

        Vector2 grid_position = { cell_position.x + (24.0f * grid_x), cell_position.y + (24.0f * grid_y) };
        
        char text[4];
        snprintf(text, sizeof(text), "%d", i + 1);

        draw_centered_text(text, 24, grid_position, GRAY);

    }
}

void draw_nums() {
    
    for (int x = 0; x < 9; x++) {
        for (int y = 0; y < 9; y++) {
            
            if (game_state.nums[x][y] == -1) {
                draw_approximate_nums(x, y);
            
                continue;
            }

            int index = x + y;
            if (game_start_animation == 0) return; 

            float local_animation = ((game_start_animation * 2.0f) - (index / 18.0f));            
            float ease = EaseCubicInOutNormalized(Clamp(local_animation, 0.0f, 1.0f));
            float inverseEase = (1.0f - ease);

            Vector2 position = { (595.0f + (x * 90.0f)) * ease + (1920.0f / 2.0f * inverseEase), (185.0f + (y * 90.0f)) * ease + (1080.0f / 2.0f * inverseEase) };

            char text[4];
            snprintf(text, sizeof(text), "%d", game_state.nums[x][y]);

            draw_centered_text(text, 54 * ease, position, game_state.editable_nums[x][y] ? game_state.invalid_nums[x][y] ? RED : GRAY : WHITE);

        }
    }

}

void draw_button(Button button) {

    Rectangle button_rect = button.rect;
    draw_rectangle_with_stroke(&button_rect, 5, BLACK, WHITE);

    Vector2 text_position = { button.rect.x + (button.rect.width / 2.0f), button.rect.y + (button.rect.height / 2.0f)};
    draw_centered_text(button.text, button.text_height, text_position, button.hovered ? GRAY : WHITE);

}

void draw_input_field(InputField field) {

    Rectangle button_rect = field.rect;
    draw_rectangle_with_stroke(&button_rect, 5, BLACK, WHITE);

    Vector2 text_position = { field.rect.x + (field.rect.width / 2.0f), field.rect.y + (field.rect.height / 2.0f)};

    bool is_placeholder = strlen(field.text) == 0;
    char *text = is_placeholder ? field.placeholder : field.text;

    draw_centered_text(text, field.text_height, text_position, is_placeholder ? GRAY : WHITE);

    if (field.selected) {

        int text_width = !is_placeholder ? MeasureText(text, field.text_height) : 0.0f;
        Rectangle selected_rect = (Rectangle) { 
            .x = field.rect.x + (field.rect.width / 2.0f) + (text_width / 2.0f) + 10.0f,
            .y = field.rect.y + 10.0f,
            .width = 5.0f,
            .height = field.rect.height - 20.0f,
        };

        DrawRectangleRec(selected_rect, field.selected_timer >= 1.0f ? WHITE : TRANSPARENT);
    }
}

void draw_sprite(Sprite sprite, Vector2 position) {

    Rectangle source = { 0.0f, 0.0f, (float) sprite.texture.width, (float) sprite.texture.height };
    Rectangle destination = { sprite.position.x + position.x, sprite.position.y + position.y, sprite.size.x, sprite.size.y };
    Vector2 origin = { 0.0f, 0.0f };

    DrawTexturePro(sprite.texture, source, destination, origin, 0.0f, WHITE);

}

void draw_cursors() {

    for (int i = 0; i < MAX_AMOUNT_OF_PLAYERS; i++) {
        if (!networking_state.players[i].is_connected) continue;

        draw_sprite(cursor_sprite, cursor_positions[i]);
        DrawText(networking_state.players[i].name, cursor_positions[i].x + 24, cursor_positions[i].y + 24, 24, WHITE);
    }
    
}

void lerp_cursors() {

    float t = 0.25f * (delta * GetFPS());
    if (t > 1.0f) t = 1.0f;

    for (size_t i = 0; i < MAX_AMOUNT_OF_PLAYERS; i++) {

        ConnectedPlayer player = networking_state.players[i];
        if (!player.is_connected) continue;

        cursor_positions[i] = Vector2Lerp(
            cursor_positions[i],
            (Vector2) { player.mouse_x, player.mouse_y },
            t
        );
    }
}

void draw_timer() {

    int raw_time = current_screen == GAME ? (time(NULL) - game_state.game_started_time) : game_state.game_won_time;
    int minutes = (int) raw_time / 60;
    int seconds = (int) raw_time % 60;

    char timer_text[16];
    sprintf(timer_text, "%02d:%02d", minutes, seconds); 
    
    Vector2 text_position = (Vector2) { 1920.0f / 2.0f, 100.0f };
    draw_centered_text(timer_text, 54, text_position, WHITE);
}

void update_confetti() {
    confetti_lifetime += delta;

    for (int i = 0; i < CONFETTI_AMOUNT; i++) {
        float fall_speed = (confettis[i].seed + 0.5f) * 150.0f; 
        
        confettis[i].y += fall_speed * delta;
        
        if (confettis[i].y > VIRTUAL_SCREEN_HEIGHT + 100)
            confettis[i].y = -100;
    }
}


void spawn_confetti() {

    for (int i = 0; i < CONFETTI_AMOUNT; i++) {
        confettis[i].seed = (rand() % 100) / 100.0f;
        
        confettis[i].x = (rand() % VIRTUAL_SCREEN_WIDTH);
        confettis[i].y = -(rand() % VIRTUAL_SCREEN_HEIGHT);
        confettis[i].color = (rand() << 16) | rand();

        confettis[i].speed_y = 0.0f;

    }

    confetti_lifetime = 0;
}

void draw_confetti() {

    for (int i = 0; i < CONFETTI_AMOUNT; i++) {

        float size = (confettis[i].seed + 0.2f) * 20.0f;

        Rectangle rect = { confettis[i].x, confettis[i].y, size, size * 0.6f };
        Vector2 origin = { size / 2.0f, (size * 0.6f) / 2.0f };

        Color color = (Color) { confettis[i].color & 0xFF, (confettis[i].color >> 8) & 0xFF, (confettis[i].color >> 16) & 0xFF, 255 };
        float rotation = (confettis[i].seed * 360.0f) + (confetti_lifetime * 200.0f * (confettis[i].seed + 0.5f));

        DrawRectanglePro(rect, origin, rotation, color);

    }

}

void draw_screen() {

    delta = GetFrameTime();

    switch (current_screen) {

        case (MENU): {

            draw_centered_text("Sudoku COOP", 96, (Vector2) {1920 / 2, 200}, WHITE);

            draw_input_field(name_input_field);
            draw_input_field(ip_input_field);

            draw_button(host_game_button);
            draw_button(join_game_button);

            draw_button(difficulty_button);

            draw_centered_text(error_text, 54, (Vector2) { 1920 / 2, 75}, RED);

            break;
        }

        case (GAME): {
            
            draw_box();
            draw_nums();
            draw_current_num();
            
            tick(networking_state.is_connected, &game_start_animation, 0.4f, delta);

            if (!networking_state.is_connected) {
            
                draw_centered_text("Waiting for players...", 54, (Vector2) { 1920.0f / 2.0f, 100.0f }, WHITE);
            
                break;
            }

            draw_cursors();
            lerp_cursors();
            draw_timer();

            break;
        }

        case (WON): {


            draw_confetti();
            update_confetti();

            draw_centered_text("Game Won!", 96, (Vector2) { 1920 / 2, 200 }, WHITE);

            draw_cursors();
            lerp_cursors();

            draw_button(back_to_menu_button);

            draw_timer();

            break;
        }

    }
}

void draw_virtual_texture() {

        BeginDrawing();

        ClearBackground(BLACK);

        float scale = fminf(
            (float) GetScreenWidth() / VIRTUAL_SCREEN_WIDTH, 
            (float) GetScreenHeight() / VIRTUAL_SCREEN_HEIGHT
        );

        Rectangle source = { 0.0f, 0.0f, (float) virtual_screen.texture.width, (float) -virtual_screen.texture.height };
        Rectangle destination = { 
            ((float) GetScreenWidth() - ((float) VIRTUAL_SCREEN_WIDTH * scale)) * 0.5f,
            ((float) GetScreenHeight() - ((float) VIRTUAL_SCREEN_HEIGHT * scale)) * 0.5f,
            (float) VIRTUAL_SCREEN_WIDTH * scale,
            (float) VIRTUAL_SCREEN_HEIGHT * scale
        };
        Vector2 origin = { 0.0f, 0.0f };

        DrawTexturePro(virtual_screen.texture, source, destination, origin, 0.0f, WHITE);

        EndDrawing();

}

void load_sprite(Sprite *sprite) {

    sprite->texture = LoadTexture(sprite->path);
    SetTextureFilter(sprite->texture, TEXTURE_FILTER_BILINEAR);

}

void load_resources() {

    virtual_screen = LoadRenderTexture(VIRTUAL_SCREEN_WIDTH, VIRTUAL_SCREEN_HEIGHT);
    SetTextureFilter(virtual_screen.texture, TEXTURE_FILTER_POINT);

    load_sprite(&cursor_sprite);

    difficulty_button.text = malloc(32);
    strcpy(difficulty_button.text, "Easy");

    for (size_t i = 0; i < MAX_AMOUNT_OF_PLAYERS; i++)
        cursor_positions[i] = (Vector2) {-1, -1};

    snprintf(name_input_field.text, sizeof(name_input_field.text), "Player%d", (rand() % 100));

}

void unload_resources() {

    UnloadRenderTexture(virtual_screen);
    UnloadTexture(cursor_sprite.texture);

    if (difficulty_button.text != NULL) free(difficulty_button.text);

}