#include "generator.h"
#include "utils.h"
#include "field_subset.h"

#include <stdint.h>
#include <stdio.h>
#include <time.h>


bool uniquely_solvable(Sudoku *s) {
    Sudoku copy = *s;
    sudoku_solve(&copy);
    Sudoku rcopy = *s;
    sudoku_solve_reverse(&rcopy);
    return sudoku_equal_values(&copy, &rcopy);
}


// generate instances by selecting and clearing fields from a solved instance
// naive strategy: remove fields until the instance is not uniquely solvable, then terminate
Sudoku generate_sudoku_naive() {
    Sudoku out = sudoku_new_empty();
    sudoku_solve_random(&out);
    while (true) {
        // select any nonempty field
        OrderedFieldSubset nonempty_fields;
        ofs_find_nonempty_fields(&nonempty_fields, &out);
        uint32_t index = nonempty_fields.indices[random(0, nonempty_fields.size)];
        uint32_t value = out.data[index];

        // delete field
        sudoku_clear_field(&out, index);

        // if not unique, put value pack and return board
        if (!uniquely_solvable(&out)) {
            sudoku_put_one_hot_value(&out, index, value);
            return out;
        }
    }
}


// generate instances by selecting and clearing fields from a solved instance
// las vegas strategy: random exhaustive search
// enumerate all possible removal paths in a random order
// prune path as soon as the instance is not uniquely solvable
// return once a sufficiently good solution has been found
bool try_remove_exhaustive(Sudoku *sudoku, OrderedFieldSubset *shuffled_fields, uint32_t index_index, uint32_t target_hints, OrderHeuristic heuristic, void* state) {
    uint32_t current_hints = 81 - sudoku->blank_fields;

    if (current_hints <= target_hints)
        return true;

    if (index_index >= shuffled_fields->size)
        return false;

    heuristic(shuffled_fields, index_index, 81 - index_index, sudoku, 1, state);

    uint32_t index = shuffled_fields->indices[index_index];
    uint32_t value = sudoku->data[index];

    // remove the field, advance if the puzzle has a unique solution
    sudoku_clear_field(sudoku, index);
    if (uniquely_solvable(sudoku) && try_remove_exhaustive(sudoku, shuffled_fields, index_index + 1, target_hints, heuristic, state)) {
        return true;
    }
    // reinsert value
    sudoku_put_one_hot_value(sudoku, index, value);
    // this effectively loops over all fields starting with index_index
    return try_remove_exhaustive(sudoku, shuffled_fields, index_index + 1, target_hints, heuristic, state);
}


Sudoku generate_sudoku_with_min_hints_exhaustive(uint32_t max_hints, OrderHeuristic heuristic, void* state) {
    Sudoku out = sudoku_new_empty();
    sudoku_solve_random(&out);

    OrderedFieldSubset all_fields;
    ofs_set_identity(&all_fields);
    // generate a random traversal order in the beginning and move left-to-right only!
    // this is sufficient because (remove field 1 then 2) == (remove field 2 then 1)
    random_shuffle(all_fields.indices, all_fields.size);

    try_remove_exhaustive(&out, &all_fields, 0, max_hints, heuristic, state);

    return out;
}


// generate instances by selecting and clearing fields from a solved instance
// monte carlo strategy: random bounded search
// generate only a limited number of removal candidates per field, try all of them
// prune path as soon as the instance is not uniquely solvable
// return the instance with the least candidates among all paths
void try_remove_bounded(Sudoku *sudoku, OrderedFieldSubset *shuffled_fields, uint32_t index_index, uint32_t max_attempts_per_field, Sudoku *best_so_far, OrderHeuristic heuristic, void* state) {

    if (sudoku->blank_fields > best_so_far->blank_fields) {
        *best_so_far = *sudoku;
    }

    // only try removing the next max_attempts many fields starting from index_index
    for (uint32_t attempt = 0; attempt < max_attempts_per_field; ++attempt) {

        uint32_t shifted_index_index = index_index + attempt;

        if (shifted_index_index >= shuffled_fields->size)
            break;

        heuristic(shuffled_fields, index_index, 81 - index_index, sudoku, 1, state);

        uint32_t index = shuffled_fields->indices[shifted_index_index];
        uint32_t value = sudoku->data[index];

        sudoku_clear_field(sudoku, index);
        if (uniquely_solvable(sudoku)) {
            try_remove_bounded(sudoku, shuffled_fields, shifted_index_index + 1, max_attempts_per_field, best_so_far, heuristic, state);
        }
        sudoku_put_one_hot_value(sudoku, index, value);
    }
}


Sudoku generate_sudoku_with_min_hints_bounded(uint32_t max_attempts_per_field, OrderHeuristic heuristic, void* state) {
    Sudoku sudoku = sudoku_new_empty();
    sudoku_solve_random(&sudoku);
    Sudoku best = sudoku;

    OrderedFieldSubset all_fields;
    ofs_set_identity(&all_fields);
    random_shuffle(all_fields.indices, all_fields.size);

    try_remove_bounded(&sudoku, &all_fields, 0, max_attempts_per_field, &best, heuristic, state);

    return best;
}


// generate instances by selecting and clearing fields from a solved instance
// monte carlo strategy: random time-bounded search
// prune path as soon as the instance is not uniquely solvable
// take the instance with the least candidates after a set time limit
// returns true if the time limit was reached
bool try_remove_time_bounded(Sudoku *sudoku, OrderedFieldSubset *shuffled_fields, uint32_t index_index, clock_t start, float max_seconds, Sudoku *best_so_far, OrderHeuristic heuristic, void* state) {

    if (sudoku->blank_fields > best_so_far->blank_fields) {
        *best_so_far = *sudoku;
    }

    clock_t current = clock();
    float seconds = (float) (current - start) / CLOCKS_PER_SEC;
    if (seconds > max_seconds)
        return true;

    if (index_index >= shuffled_fields->size)
        return false;

    heuristic(shuffled_fields, index_index, 81 - index_index, sudoku, 1, state);

    uint32_t index = shuffled_fields->indices[index_index];
    uint32_t value = sudoku->data[index];

    // remove the field, advance if the puzzle has a unique solution
    sudoku_clear_field(sudoku, index);
    if (uniquely_solvable(sudoku) && try_remove_time_bounded(sudoku, shuffled_fields, index_index + 1, start, max_seconds, best_so_far, heuristic, state)) {
        return true;
    }
    // reinsert value
    sudoku_put_one_hot_value(sudoku, index, value);
    // this effectively loops over all fields starting with index_index
    return try_remove_time_bounded(sudoku, shuffled_fields, index_index + 1, start, max_seconds, best_so_far, heuristic, state);
}


Sudoku generate_sudoku_with_min_hints_time_bounded(float max_seconds, OrderHeuristic heuristic, void* state) {
    Sudoku sudoku = sudoku_new_empty();
    sudoku_solve_random(&sudoku);
    Sudoku best = sudoku;

    OrderedFieldSubset all_fields;
    ofs_set_identity(&all_fields);
    random_shuffle(all_fields.indices, all_fields.size);

    clock_t start = clock();
    try_remove_time_bounded(&sudoku, &all_fields, 0, start, max_seconds, &best, heuristic, state);

    return best;
}
