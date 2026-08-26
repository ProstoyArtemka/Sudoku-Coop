#ifndef GENERATION_H
#define GENERATION_H

#include <stdbool.h>

typedef enum {

    EASY,
    NORMAL,
    HARD,
    PIZDEC

} Difficulty;

extern Difficulty difficulty;

bool check_validity(int nums[9][9], int row, int col, int num);
void update_validity(int x, int y);

void fill_nums(int nums[9][9]);
void remove_nums(int nums[9][9], bool editable_nums[9][9]);

#endif