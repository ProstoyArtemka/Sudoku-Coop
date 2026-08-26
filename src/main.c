
#include <raylib.h>
#include <raymath.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include <game.h>
#include <input.h>
#include <graphics.h>
#include <networking.h>

void init_window() {

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Sudoku-Coop");
    SetTargetFPS(60);

    SetExitKey(KEY_NULL);

}

int main() {

    srand(time(NULL));

    init_window();
    load_resources();

    while (!WindowShouldClose()) {

        process_inputs();

        BeginTextureMode(virtual_screen);
        ClearBackground(BLACK);

        draw_screen();

        EndTextureMode();
        
        draw_virtual_texture();
    }

    unload_resources();
    stop_socket();

    CloseWindow();
}