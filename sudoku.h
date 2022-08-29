#ifndef SUDOKU_H
#define SUDOKU_H

#include <stdbool.h>
#include <stdint.h>

// expose definition in header since we do not want to allocate heap memory for sudokus
typedef struct {
    // number of blank fields, to be updated consistently!
    uint32_t blank_fields;
    // row-major sudoku grid
    // 32 bits per entry
    // lower 16: number 1-9 in one-hot encoding or 0
    // upper 16: field candidates, using a bit set of size 9
    // these are mutually exclusive: a non-empty field has en empty candidate set!
    uint32_t data[81];
} Sudoku;


Sudoku sudoku_new_empty();
void sudoku_init(Sudoku *sudoku);
void sudoku_from_buffer(Sudoku* sudoku, const uint32_t* buffer);
void sudoku_from_string(Sudoku* sudoku, const char* buffer);
bool sudoku_solve(Sudoku *sudoku);
bool sudoku_solve_reverse(Sudoku *sudoku);
bool sudoku_solve_random(Sudoku *sudoku);
void sudoku_print(const Sudoku *sudoku);
void sudoku_pprint(const Sudoku *sudoku);
bool sudoku_equal_values(const Sudoku *lhs, const Sudoku *rhs);
void sudoku_put_one_hot_value(Sudoku *sudoku, uint32_t field, uint32_t candidate);
void sudoku_put_value(Sudoku *sudoku, uint32_t field, uint32_t candidate);
void sudoku_clear_field(Sudoku *sudoku, uint32_t field);
uint32_t sudoku_read_value(const Sudoku *sudoku, uint32_t field);

#endif
