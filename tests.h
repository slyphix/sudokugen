#ifndef TESTS_H
#define TESTS_H

#include "sudoku.h"
#include "generator.h"

#include <stdio.h>
#include <stdint.h>
#include <time.h>


inline void test() {
    Sudoku s;
    uint32_t wikipedia[81] = {
            5, 3, 0,   0, 7, 0,   0, 0, 0,
            6, 0, 0,   1, 9, 5,   0, 0, 0,
            0, 9, 8,   0, 0, 0,   0, 6, 0,

            8, 0, 0,   0, 6, 0,   0, 0, 3,
            4, 0, 0,   8, 0, 3,   0, 0, 1,
            7, 0, 0,   0, 2, 0,   0, 0, 6,

            0, 6, 0,   0, 0, 0,   2, 8, 0,
            0, 0, 0,   4, 1, 9,   0, 0, 5,
            0, 0, 0,   0, 8, 0,   0, 7, 9
    };
    sudoku_from_buffer(&s, wikipedia);
    sudoku_print(&s);
    // solve an instance twice to check for a unique solution
    Sudoku copy = s;
    sudoku_solve(&copy);
    sudoku_print(&copy);
    copy = s;
    sudoku_solve_reverse(&copy);
    sudoku_print(&copy);

    // heuristics reorder non-blank field indices according to some metric
    {
        OrderedFieldSubset ofs;
        ofs_find_nonempty_fields(&ofs, &s);
        uint32_t k = min_neighbors_heuristic(&ofs, 0, ofs.size, &s, ofs.size, NULL);
        printf("%u candidates\n", k);
        for (uint32_t i = 0; i < k; ++i) {
            printf("%u ", ofs.indices[i]);
        }
        printf("\n");
    }
    {
        OrderedFieldSubset ofs;
        ofs_find_nonempty_fields(&ofs, &s);
        uint32_t k = most_frequent_digit_heuristic(&ofs, 0, ofs.size, &s, ofs.size, NULL);
        printf("%u candidates\n", k);
        for (uint32_t i = 0; i < k; ++i) {
            printf("%u ", ofs.indices[i]);
        }
        printf("\n");
    }
    // heuristics can be combined to form a new heuristic
    {
        OrderedFieldSubset ofs;
        ofs_find_nonempty_fields(&ofs, &s);

        OrderHeuristic heuristics[2] = {most_frequent_digit_heuristic, min_neighbors_heuristic};
        void* states[2] = {NULL, NULL};
        CombinedHeuristic ch;
        ch.count = 2;
        ch.heuristics = heuristics;
        ch.states = states;

        uint32_t k = combined_heuristic(&ofs, 0, ofs.size, &s, ofs.size, &ch);
        printf("%u candidates\n", k);
        for (uint32_t i = 0; i < ofs.size; ++i) {
            printf("%u ", ofs.indices[i]);
        }
        printf("\n");
    }
}


inline void generate() {
    {
        Sudoku s = generate_sudoku_naive();
        sudoku_print(&s);
    }
    {
        const float max_seconds_per_instance = 5;
        Sudoku s = generate_sudoku_with_min_hints_time_bounded(max_seconds_per_instance, min_neighbors_heuristic, NULL);
        sudoku_print(&s);
    }
    {
        const uint32_t max_hints_per_instance = 22;
        Sudoku s = generate_sudoku_with_min_hints_exhaustive(max_hints_per_instance, min_neighbors_heuristic, NULL);
        sudoku_print(&s);
    }
    {
        const uint32_t max_attempts_per_field = 2;
        Sudoku s = generate_sudoku_with_min_hints_bounded(max_attempts_per_field, min_neighbors_heuristic, NULL);
        sudoku_print(&s);
    }
}


inline void measure() {
    uint32_t max_candidates = 23;
    uint32_t runs = 100;
    {
        clock_t start = clock();
        for (uint32_t i = 0; i < runs; ++i) {
            Sudoku s = generate_sudoku_with_min_hints_exhaustive(max_candidates, max_neighbors_heuristic, NULL);
        }
        clock_t end = clock();
        float seconds = (float) (end - start) / CLOCKS_PER_SEC;
        printf("Max neighbors heuristic: %.5f seconds\n", seconds);
    }
    {
        clock_t start = clock();
        for (uint32_t i = 0; i < runs; ++i) {
            Sudoku s = generate_sudoku_with_min_hints_exhaustive(max_candidates, min_neighbors_heuristic, NULL);
        }
        clock_t end = clock();
        float seconds = (float) (end - start) / CLOCKS_PER_SEC;
        printf("Min neighbors heuristic: %.5f seconds\n", seconds);
    }
    {
        clock_t start = clock();
        for (uint32_t i = 0; i < runs; ++i) {
            Sudoku s = generate_sudoku_with_min_hints_exhaustive(max_candidates, most_frequent_digit_heuristic, NULL);
        }
        clock_t end = clock();
        float seconds = (float) (end - start) / CLOCKS_PER_SEC;
        printf("Most frequent digit heuristic: %.5f seconds\n", seconds);
    }
    {
        clock_t start = clock();
        for (uint32_t i = 0; i < runs; ++i) {
            Sudoku s = generate_sudoku_with_min_hints_exhaustive(max_candidates, least_frequent_digit_heuristic, NULL);
        }
        clock_t end = clock();
        float seconds = (float) (end - start) / CLOCKS_PER_SEC;
        printf("Least frequent digit heuristic: %.5f seconds\n", seconds);
    }
    {
        clock_t start = clock();
        for (uint32_t i = 0; i < runs; ++i) {
            Sudoku s = generate_sudoku_with_min_hints_exhaustive(max_candidates, no_heuristic, NULL);
        }
        clock_t end = clock();
        float seconds = (float) (end - start) / CLOCKS_PER_SEC;
        printf("No heuristic: %.5f seconds\n", seconds);
    }
}

#endif
