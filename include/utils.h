#ifndef UTILS_H
#define UTILS_H

#include <raylib.h>
#include <stdlib.h>

#define numof(array) ( (int) ( sizeof(array) / sizeof((array)[0]) ) )

typedef struct {

    int x;
    int y;

} Vector2i;

void swap(int *a, int *b);
void shuffle(int arr[], int n);

void expand_rectangle(Rectangle *rectangle, float width, float height);
void draw_rectangle_with_stroke(Rectangle *rectangle, float stroke_width, Color background, Color stroke);

void draw_centered_text(char* text, float text_size, Vector2 position, Color color);

Vector2i get_number_position_by_mouse(Vector2 mouse);

void tick(bool add, float *value, float speed, float delta_time);

#endif