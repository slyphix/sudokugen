#ifndef GENERATOR_H
#define GENERATOR_H

#include "sudoku.h"
#include "heuristics.h"

Sudoku generate_sudoku_naive();
Sudoku generate_sudoku_with_min_hints_exhaustive(uint32_t max_hints, OrderHeuristic heuristic, void* state);
Sudoku generate_sudoku_with_min_hints_bounded(uint32_t max_attempts_per_field, OrderHeuristic heuristic, void* state);
Sudoku generate_sudoku_with_min_hints_time_bounded(float max_seconds, OrderHeuristic heuristic, void* state);

#endif
