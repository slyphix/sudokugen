#include "heuristics.h"
#include "utils.h"


uint32_t no_heuristic(
        OrderedFieldSubset* candidate_fields,
        uint32_t candidate_offset,
        uint32_t candidate_count,
        const Sudoku* instance,
        uint32_t max_candidates_to_generate,
        void* state) {

    return max_candidates_to_generate;
}


uint32_t count_neighbors(const Sudoku *sudoku, uint32_t index) {
    uint32_t r = index / 9u * 9u, c = index % 9u, sq = index / 27u * 27u + c / 3u * 3u;
    uint32_t neighbors = 0;
    for (uint32_t i = 0; i < 9; ++i) {
        neighbors += 0 != (sudoku->data[r + i] & LOWER);
        neighbors += 0 != (sudoku->data[c + i * 9] & LOWER);
    }
    for (uint32_t i = 0; i < 3; ++i) {
        for (uint32_t j = 0; j < 3; ++j) {
            neighbors += 0 != (sudoku->data[sq + 9 * i + j] & LOWER);
        }
    }
    return neighbors;
}


uint32_t max_neighbors_heuristic(
        OrderedFieldSubset* candidate_fields,
        uint32_t candidate_offset,
        uint32_t candidate_count,
        const Sudoku* instance,
        uint32_t max_candidates_to_generate,
        void* state) {

    uint32_t neighbor_counts[81];

    uint32_t max = 0;
    for (uint32_t i = candidate_offset; i < candidate_offset + candidate_count; ++i) {
        neighbor_counts[i] = count_neighbors(instance, candidate_fields->indices[i]);
        if (neighbor_counts[i] > max) {
            max = neighbor_counts[i];
        }
    }

    uint32_t generated_candidates = 0;
    for (uint32_t i = candidate_offset; i < candidate_offset + candidate_count; ++i) {
        if (generated_candidates >= max_candidates_to_generate)
            break;

        if (neighbor_counts[i] == max) {
            uint32_t temp = candidate_fields->indices[generated_candidates];
            candidate_fields->indices[generated_candidates] = candidate_fields->indices[i];
            candidate_fields->indices[i] = temp;
            ++generated_candidates;
        }
    }

    return generated_candidates;
}


uint32_t min_neighbors_heuristic(
        OrderedFieldSubset* candidate_fields,
        uint32_t candidate_offset,
        uint32_t candidate_count,
        const Sudoku* instance,
        uint32_t max_candidates_to_generate,
        void* state) {

    uint32_t neighbor_counts[81];

    uint32_t min = FULL32;
    for (uint32_t i = candidate_offset; i < candidate_offset + candidate_count; ++i) {
        neighbor_counts[i] = count_neighbors(instance, candidate_fields->indices[i]);
        if (neighbor_counts[i] < min) {
            min = neighbor_counts[i];
        }
    }

    uint32_t generated_candidates = 0;
    for (uint32_t i = candidate_offset; i < candidate_offset + candidate_count; ++i) {
        if (generated_candidates >= max_candidates_to_generate)
            break;

        if (neighbor_counts[i] == min) {
            uint32_t temp = candidate_fields->indices[generated_candidates];
            candidate_fields->indices[generated_candidates] = candidate_fields->indices[i];
            candidate_fields->indices[i] = temp;
            ++generated_candidates;
        }
    }

    return generated_candidates;
}


uint32_t most_frequent_digit_heuristic(
        OrderedFieldSubset* candidate_fields,
        uint32_t candidate_offset,
        uint32_t candidate_count,
        const Sudoku* instance,
        uint32_t max_candidates_to_generate,
        void* state) {

    // count digit frequency
    uint32_t digit_counts[9];
    for (uint32_t i = 0; i < 9u; ++i) {
        digit_counts[i] = 0;
    }
    for (uint32_t i = 0; i < 81u; ++i) {
        uint32_t digit = sudoku_read_value(instance, i);
        if (digit != 0) ++digit_counts[digit - 1];
    }

    // find the highest count among candidates
    uint32_t max = 0;
    for (uint32_t i = candidate_offset; i < candidate_offset + candidate_count; ++i) {
        uint32_t digit = sudoku_read_value(instance, candidate_fields->indices[i]);
        if (digit_counts[digit - 1] > max) {
            max = digit_counts[digit - 1];
        }
    }

    // move all candidates with the highest count to the front
    uint32_t generated_candidates = 0;
    for (uint32_t i = candidate_offset; i < candidate_offset + candidate_count; ++i) {
        if (generated_candidates >= max_candidates_to_generate)
            break;

        uint32_t digit = sudoku_read_value(instance, candidate_fields->indices[i]);
        if (digit_counts[digit - 1] == max) {
            uint32_t temp = candidate_fields->indices[generated_candidates];
            candidate_fields->indices[generated_candidates] = candidate_fields->indices[i];
            candidate_fields->indices[i] = temp;
            ++generated_candidates;
        }
    }

    return generated_candidates;
}


uint32_t least_frequent_digit_heuristic(
        OrderedFieldSubset* candidate_fields,
        uint32_t candidate_offset,
        uint32_t candidate_count,
        const Sudoku* instance,
        uint32_t max_candidates_to_generate,
        void* state) {

    // count digit frequency
    uint32_t digit_counts[9];
    for (uint32_t i = 0; i < 9u; ++i) {
        digit_counts[i] = 0;
    }
    for (uint32_t i = 0; i < 81u; ++i) {
        uint32_t digit = sudoku_read_value(instance, i);
        if (digit != 0) ++digit_counts[digit - 1];
    }

    // find the lowest count among candidates
    uint32_t min = FULL32;
    for (uint32_t i = candidate_offset; i < candidate_offset + candidate_count; ++i) {
        uint32_t digit = sudoku_read_value(instance, candidate_fields->indices[i]);
        if (digit_counts[digit - 1] < min) {
            min = digit_counts[digit- 1];
        }
    }

    // move all candidates with the lowest count to the front
    uint32_t generated_candidates = 0;
    for (uint32_t i = candidate_offset; i < candidate_offset + candidate_count; ++i) {
        if (generated_candidates >= max_candidates_to_generate)
            break;

        uint32_t digit = sudoku_read_value(instance, candidate_fields->indices[i]);
        if (digit_counts[digit - 1] == min) {
            uint32_t temp = candidate_fields->indices[generated_candidates];
            candidate_fields->indices[generated_candidates] = candidate_fields->indices[i];
            candidate_fields->indices[i] = temp;
            ++generated_candidates;
        }
    }

    return generated_candidates;
}


// applies multiple heuristics in sequence
uint32_t combined_heuristic(
        OrderedFieldSubset* candidate_fields,
        uint32_t candidate_offset,
        uint32_t candidate_count,
        const Sudoku* instance,
        uint32_t max_candidates_to_generate,
        void* state) {

    CombinedHeuristic* heuristic_stack = (CombinedHeuristic*) state;

    for (uint32_t i = 0; i < heuristic_stack->count; ++i) {
        OrderHeuristic local_heuristic = heuristic_stack->heuristics[i];
        void* local_state = heuristic_stack->states[i];
        uint32_t local_max_candidates_to_generate = candidate_count;
        // generate a reduced candidate set in the last iteration only
        if (i == heuristic_stack->count - 1) {
            local_max_candidates_to_generate = min(max_candidates_to_generate, local_max_candidates_to_generate);
        }
        uint32_t generated_candidates = local_heuristic(
                candidate_fields,
                candidate_offset,
                candidate_count,
                instance,
                local_max_candidates_to_generate,
                local_state);
        // ignore failed heuristics
        if (generated_candidates == 0) continue;
        candidate_count = generated_candidates;
    }

    return min(max_candidates_to_generate, candidate_count);
}
