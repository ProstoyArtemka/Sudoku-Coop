#include <utils.h>

#include <game.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

void swap(int *a, int *b) {

    int temp = *a;
    *a = *b;
    *b = temp;

}

void shuffle(int arr[], int n) {

    for (int i = n - 1; i > 0; i--) {

        int j = rand() % (i + 1);

        swap(&arr[i], &arr[j]);
    }

}

void expand_rectangle(Rectangle *rectangle, float height, float width) {

    rectangle->x -= width;
    rectangle->y -= height;

    rectangle->width += width * 2.0f;
    rectangle->height += height * 2.0f;

}

void draw_rectangle_with_stroke(Rectangle *rectangle, float stroke_width, Color background, Color stroke) {

    DrawRectangleRec(*rectangle, stroke);

    expand_rectangle(rectangle, -stroke_width, -stroke_width);

    DrawRectangleRec(*rectangle, background);

}

void draw_centered_text(char* text, float text_size, Vector2 position, Color color) {

    float text_width = MeasureText(text, text_size);
    Vector2 text_position = {
        position.x - (text_width / 2.0f),
        position.y - (text_size / 2.0f)
    };

    DrawText(text, text_position.x, text_position.y, text_size, color);
}

Vector2i get_number_position_by_mouse(Vector2 mouse) {

    return (Vector2i) {
        (int) (mouse.x - 550.0f) / 90,
        (int) (mouse.y - 140.0f) / 90
    };;    

}

void tick(bool add, float *value, float speed, float delta_time) {

    float delta = delta_time * speed;

    if (add) {
        *value = fminf(*value + delta, 1.0f);
    
        return;
    }

    *value = fmaxf(*value - delta, 0.0f);

}