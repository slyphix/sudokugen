#include "utils.h"
#include "sudoku.h"
#include "generator.h"
#include "tests.h"
#include "errno.h"

#include <time.h>

int main(int argc, char** argv) {
    srand(time(NULL));

    uint32_t num_instances_to_generate = 1;
    if (argc > 1) {
        num_instances_to_generate = strtoul(argv[1], NULL, 10);
        if (errno == ERANGE) exit(1);
    }

    float max_seconds_per_instance = 0.1f;
    if (argc > 2) {
        max_seconds_per_instance = strtof(argv[2], NULL);
        if (errno == ERANGE) exit(1);
    }

    for (uint32_t i = 0; i < num_instances_to_generate; ++i) {
        Sudoku s = generate_sudoku_with_min_hints_time_bounded(max_seconds_per_instance, min_neighbors_heuristic, NULL);
        sudoku_print(&s);
    }
}
