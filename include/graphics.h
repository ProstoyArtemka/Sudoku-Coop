#ifndef GRAPHICS_H
#define GRAPHICS

#include <game.h>
#include <raylib.h>
#include <utils.h>

#define VIRTUAL_SCREEN_WIDTH 1920
#define VIRTUAL_SCREEN_HEIGHT 1080

#define TRANSPARENT (Color) { 0, 0, 0, 0 }

typedef void (*ButtonCallback)();
typedef void (*InputFieldCharPressed)(char c);

typedef struct {

    char *text;

    int text_height;

    Rectangle rect;

    bool hovered;

    ButtonCallback callback;

} Button;

typedef struct {

    int text_max_length;
    char text[256];
    char *placeholder;

    int text_height;

    Rectangle rect;

    bool hovered;
    bool selected;

    float selected_timer;

    InputFieldCharPressed char_pressed_callback;

} InputField;

typedef struct {

    char *path;
    Texture2D texture;

    Vector2 position;
    Vector2 size;

} Sprite;

extern RenderTexture2D virtual_screen;
extern float delta;

extern float num_animation;
extern float connect_animation;

extern Button host_game_button;
extern Button join_game_button;
extern Button back_to_menu_button;
extern Button difficulty_button;

extern InputField ip_input_field;
extern InputField name_input_field;

extern char error_text[256];
extern char connect_text[64];

void draw_screen();

void draw_virtual_texture();

void load_resources();
void unload_resources();

#endif