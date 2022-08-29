#ifndef HEURISTICS_H
#define HEURISTICS_H

#include "field_subset.h"
#include "sudoku.h"

// this function looks for fields to be cleared so that the instance remains uniquely solvable with high probability
// the candidates for clearing are stored in candidate_fields.indices
// but restricted to the index range [candidate_offset, candidate_offset + candidate_count)
// the function is expected to determine which fields are suitable for clearing next
// and swap them to the front of the index range, ordered by decreasing quality, swapping unsuitable values to the back
// if two candidates are of the same quality, it is recommended to retain their relative order (stable sorting)
// the function is expected to return the number of suitable candidates found (which should be at the front by then)
// the function is not allowed to modify the candidate fields outside this index range
// the return value shall never exceed max_candidates_to_generate (which is <= candidate_count)
// this ensures that no work is done unnecessarily e.g. if only one candidate is required
// if the heuristic could not be applied, the function is expected to return 0
// the idea is to progressively decrease the set of candidates by applying multiple heuristics in series (see combined_heuristic)
typedef uint32_t(*OrderHeuristic)(
        OrderedFieldSubset* candidate_fields,
        uint32_t candidate_offset,
        uint32_t candidate_count,
        const Sudoku* instance,
        uint32_t max_candidates_to_generate,
        void* state);

#define DECLARE_HEURISTIC(name) \
    uint32_t name (OrderedFieldSubset*, uint32_t, uint32_t, const Sudoku*, uint32_t, void*);

DECLARE_HEURISTIC(no_heuristic)
DECLARE_HEURISTIC(max_neighbors_heuristic)
DECLARE_HEURISTIC(min_neighbors_heuristic)
DECLARE_HEURISTIC(most_frequent_digit_heuristic)
DECLARE_HEURISTIC(least_frequent_digit_heuristic)
DECLARE_HEURISTIC(combined_heuristic)

typedef struct {
    OrderHeuristic* heuristics;
    void** states;
    uint32_t count;
} CombinedHeuristic;

#endif
