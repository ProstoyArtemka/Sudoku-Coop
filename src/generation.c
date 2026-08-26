#include <generation.h>

#include <utils.h>
#include <game.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Difficulty difficulty;

void transponate_nums(int nums[9][9]) {

    for (int x = 0; x < 9; x++) {
        for (int y = 0; y < x; y++) {

            int buffer = nums[x][y];

            nums[x][y] = nums[y][x];
            nums[y][x] = buffer;
        }
    }
}

void swap_rows(int nums[9][9]) {

    int block_row = (rand() % 3);
    int line_a = (rand() % 3);
    int line_b = (rand() % 3);

    if (line_a == line_b) return;

    int absolute_row_a = block_row * 3 + line_a;
    int absolute_row_b = block_row * 3 + line_b;

    for (int column = 0; column < 9; column++) {

        int buffer = nums[absolute_row_a][column];
        nums[absolute_row_a][column] = nums[absolute_row_b][column];
        nums[absolute_row_b][column] = buffer;

    }
}

void swap_columns(int nums[9][9]) {

    int block_column = (rand() % 3);
    int line_a = (rand() % 3);
    int line_b = (rand() % 3);

    if (line_a == line_b) return;

    int absolute_row_a = block_column * 3 + line_a;
    int absolute_row_b = block_column * 3 + line_b;

    for (int row = 0; row < 9; row++) {

        int buffer = nums[row][absolute_row_a];
        nums[row][absolute_row_a] = nums[row][absolute_row_b];
        nums[row][absolute_row_b] = buffer;

    }
}

void swap_nums(int nums[9][9]) {

    int num_a = (rand() % 9) + 1;
    int num_b = (rand() % 9) + 1;

    if (num_a == num_b) return;

    for (int x = 0; x < 9; x++) {
        for (int y = 0; y < 9; y++) {

            int num = nums[x][y];

            if (num == num_a) {
                nums[x][y] = num_b;
        
                continue;
            }

            if (num == num_b) {
                nums[x][y] = num_a;

                continue;
            }
        }
    }
}

void fill_nums(int nums[9][9]) {
    
    for (int x = 0; x < 9; x++) {
        for (int y = 0; y < 9; y++) {

            int num = ((x * 3 + (x / 3) + y) % 9) + 1;

            nums[x][y] = num;

        }
    }

    int amount_of_operations = 100 + (rand() % 250);

    void (*operations[])(int[9][9]) = { 

        transponate_nums, 
        swap_rows, 
        swap_columns,
        swap_nums
    };

    for (int i = 0; i < amount_of_operations; i++) {
        int index = rand() % numof(operations);
 
        operations[index](nums);
    }

}

bool check_validity(int nums[9][9], int row, int col, int num) {

    if (num == -1)
        return true;

    for (int i = 0; i < 9; i++) {
        if ((i != col && nums[row][i] == num) || (i != row && nums[i][col] == num)) {
            return false;
        }
    }

    int start_row = (row / 3) * 3;
    int start_col = (col / 3) * 3;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {

            if (start_row + i == row && start_col + j == col) continue;

            if (nums[start_row + i][start_col + j] == num)
                return false;
            
        }
    }

    return true;

}

void update_validity(int x, int y) {

    game_state.invalid_nums[x][y] = !check_validity(game_state.nums, x, y, game_state.nums[x][y]);

    for (int row = 0; row < 9; row++) {
        if (row == x) continue;;

        game_state.invalid_nums[row][y] = !check_validity(game_state.nums, row, y, game_state.nums[row][y]);
    }

    for (int column = 0; column < 9; column++) {
        if (column == y) continue;

        game_state.invalid_nums[x][column] = !check_validity(game_state.nums, x, column, game_state.nums[x][column]);
    }

    int block_x = (x / 3) * 3;
    int block_y = (y / 3) * 3;

    for (int row = 0; row < 3; row++) {
        for (int column = 0; column < 3; column++) {

            int absolute_x = block_x + row;
            int absolute_y = block_y + column;

            if (absolute_x == x && absolute_y == y) continue;

            game_state.invalid_nums[absolute_x][absolute_y] = !check_validity(game_state.nums, absolute_x, absolute_y, game_state.nums[absolute_x][absolute_y]);
        }
    }
}

void count_solutions(int nums[9][9], int *solutions) {

    if (*solutions > 1) return;

    int row = -1;
    int col = -1;

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
        
            if (nums[i][j] == -1) {
                row = i;
                col = j;

                break;
            }
        }

        if (row != -1) break;
    }

    if (row == -1) {
        (*solutions)++;

        return;
    }

    for (int num = 1; num <= 9; num++) {

        if (!check_validity(nums, row, col, num)) continue;

        nums[row][col] = num;

        count_solutions(nums, solutions);

        nums[row][col] = -1;
    }
}

int get_amount_to_remove() {

    switch (difficulty) {
    
        case EASY: return (rand() % 5) + 20;
        case NORMAL: return (rand() % 5) + 32;
        case HARD: return (rand() % 5) + 43;
        case PIZDEC: return (rand() % 5) + 54;

        default: return 0;
    
    }

    return 0;
}

void remove_nums(int nums[9][9], bool editable_nums[9][9]) {

    int all_positions[81];
    for (int i = 0; i < 81; i++) all_positions[i] = i;
    shuffle(all_positions, 81);

    int amount_to_remove = get_amount_to_remove();

    for (int i = 0; i < 81; i++) {

        if (amount_to_remove == 0) break;

        int random_position = all_positions[i];
        int x = random_position / 9;
        int y = random_position % 9;

        int num = nums[x][y];
        nums[x][y] = -1;
        editable_nums[x][y] = true;

        int solutions = 0;
        count_solutions(nums, &solutions);
        
        if (solutions == 1) {
            amount_to_remove--;
            
            continue;
        }

        nums[x][y] = num;
        editable_nums[x][y] = false;
    }
}