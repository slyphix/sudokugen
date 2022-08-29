#include "sudoku.h"
#include "utils.h"

#include <stdio.h>


Sudoku sudoku_new_empty() {
    Sudoku sudoku;
    sudoku_init(&sudoku);
    return sudoku;
}

void sudoku_init(Sudoku *sudoku) {
    sudoku->blank_fields = 81;
    for (uint32_t i = 0; i < 81u; ++i) {
        sudoku->data[i] = ALL_CANDIDATES << SHIFT;
    }
}

typedef struct {
    uint32_t start;
    uint32_t inc;
    uint32_t skip;
} BlockDefinition;

// a block is either a row, or a column, or a 3x3 square
// this is faster than enumerating all blocks using loops
static const BlockDefinition block_definitions[27] = {
        // rows
        {.start =  0, .inc = 1, .skip = 0}, {.start =  9, .inc = 1, .skip = 0}, {.start = 18, .inc = 1, .skip = 0},
        {.start = 27, .inc = 1, .skip = 0}, {.start = 36, .inc = 1, .skip = 0}, {.start = 45, .inc = 1, .skip = 0},
        {.start = 54, .inc = 1, .skip = 0}, {.start = 63, .inc = 1, .skip = 0}, {.start = 72, .inc = 1, .skip = 0},
        // columns
        {.start =  0, .inc = 9, .skip = 0}, {.start =  1, .inc = 9, .skip = 0}, {.start =  2, .inc = 9, .skip = 0},
        {.start =  3, .inc = 9, .skip = 0}, {.start =  4, .inc = 9, .skip = 0}, {.start =  5, .inc = 9, .skip = 0},
        {.start =  6, .inc = 9, .skip = 0}, {.start =  7, .inc = 9, .skip = 0}, {.start =  8, .inc = 9, .skip = 0},
        // squares
        {.start =  0, .inc = 1, .skip = 6}, {.start =  3, .inc = 1, .skip = 6}, {.start =  6, .inc = 1, .skip = 6},
        {.start = 27, .inc = 1, .skip = 6}, {.start = 30, .inc = 1, .skip = 6}, {.start = 33, .inc = 1, .skip = 6},
        {.start = 54, .inc = 1, .skip = 6}, {.start = 57, .inc = 1, .skip = 6}, {.start = 60, .inc = 1, .skip = 6}
};

void remove_adjacent(Sudoku *sudoku, uint32_t field, uint32_t candidate) {
    uint32_t r = field / 9u * 9u, c = field % 9u, sq = field / 27u * 27u + c / 3u * 3u;
    uint32_t mask = (~candidate << SHIFT) | LOWER;
    for (uint32_t i = 0; i < 9; ++i) {
        sudoku->data[r + i] &= mask;
        sudoku->data[c + i * 9] &= mask;
    }
    for (uint32_t i = 0; i < 3; ++i) {
        for (uint32_t j = 0; j < 3; ++j) {
            sudoku->data[sq + 9 * i + j] &= mask;
        }
    }
}

void put(Sudoku *sudoku, uint32_t field, uint32_t candidate) {
    sudoku->blank_fields -= 1;
    sudoku->data[field] = candidate;
    remove_adjacent(sudoku, field, candidate);
}



// candidates are stored in the lower 9 bits
typedef struct {
    uint32_t row_candidates[9];
    uint32_t column_candidates[9];
    uint32_t square_candidates[9];
} CandidateSummary;

CandidateSummary summarize_candidates(Sudoku *s) {
    CandidateSummary out;
    for (uint32_t row = 0; row < 9; ++row) {
        uint32_t candidates = ALL_CANDIDATES;
        for (uint32_t column = 0; column < 9; ++column) {
            candidates &= ~s->data[row * 9 + column] & LOWER;
        }
        out.row_candidates[row] = candidates;
    }
    for (uint32_t column = 0; column < 9; ++column) {
        uint32_t candidates = ALL_CANDIDATES;
        for (uint32_t row = 0; row < 9; ++row) {
            candidates &= ~s->data[row * 9 + column] & LOWER;
        }
        out.column_candidates[column] = candidates;
    }
    for (uint32_t sqr = 0; sqr < 3; ++sqr) {
        for (uint32_t sqc = 0; sqc < 3; ++sqc) {
            uint32_t candidates = ALL_CANDIDATES;
            for (uint32_t row = 0; row < 3; ++row) {
                for (uint32_t column = 0; column < 3; ++column) {
                    candidates &= ~s->data[sqr * 27 + row * 9 + sqc * 3 + column] & LOWER;
                }
            }
            out.square_candidates[sqr * 3 + sqc] = candidates;
        }
    }
    return out;
}

void replace_candidates_if_empty(Sudoku *sudoku, CandidateSummary *cs, uint32_t row, uint32_t column) {
    // nonempty field
    if (sudoku->data[row * 9 + column] & LOWER)
        return;
    uint32_t square = row / 3 * 3 + column / 3;
    uint32_t candidates = cs->row_candidates[row] & cs->column_candidates[column] & cs->square_candidates[square];
    sudoku->data[row * 9 + column] = candidates << SHIFT;
}

void recompute_adjacent(Sudoku *sudoku, uint32_t field) {
    CandidateSummary cs = summarize_candidates(sudoku);
    uint32_t r = field / 9u, c = field % 9u, sqr = r / 3u, sqc = c / 3u;
    for (uint32_t i = 0; i < 9; ++i) {
        // rows
        replace_candidates_if_empty(sudoku, &cs, i, c);
        // columns
        replace_candidates_if_empty(sudoku, &cs, r, i);
    }
    for (uint32_t i = 0; i < 3; ++i) {
        for (uint32_t j = 0; j < 3; ++j) {
            // squares
            replace_candidates_if_empty(sudoku, &cs, sqr + i, sqc + j);
        }
    }
}

// clear a field and recompute adjacent candidates
void sudoku_clear_field(Sudoku *sudoku, uint32_t field) {
    sudoku->blank_fields += 1;
    sudoku->data[field] = 0;
    recompute_adjacent(sudoku, field);
}

bool singles(Sudoku *sudoku) {
    bool found = false;

    for (uint32_t i = 0; i < 81; ++i) {
        if (sudoku->data[i] & LOWER)
            continue;
        uint32_t candidates = sudoku->data[i] >> SHIFT;
        uint32_t count = population_count(candidates);
        if (count == 0)
            return false;
        if (count == 1) {
            uint32_t c = candidates;
            put(sudoku, i, c);
            found = true;
        }
    }
    return found;
}

bool hidden_singles_block(Sudoku *sudoku, const BlockDefinition *block) {

    uint32_t once_or_more = 0, twice_or_more = 0;
    uint32_t index = block->start;

    // unroll this please
    for (uint32_t i = 0; i < 3; ++i, index += block->skip) {
        for (uint32_t j = 0; j < 3; ++j, index += block->inc) {
            uint32_t candidates = sudoku->data[index];
            twice_or_more |= once_or_more & candidates;
            once_or_more |= candidates;
        }
    }

    uint32_t once = once_or_more & ~twice_or_more & UPPER;
    if (!once)
        return false;

    index = block->start;
    for (uint32_t i = 0; i < 3; ++i, index += block->skip) {
        for (uint32_t j = 0; j < 3; ++j, index += block->inc) {
            uint32_t intersect = sudoku->data[index] & once;
            if (!intersect)
                continue;

            uint32_t candidate = intersect >> SHIFT;
            put(sudoku, index, candidate);
        }
    }
    return true;
}

bool hidden_singles(Sudoku *sudoku) {
    bool found = false;
    for (uint32_t n = 0; n < 27; ++n) {
        found |= hidden_singles_block(sudoku, block_definitions + n);
    }
    return found;
}

// bitset -> one-hot
typedef uint32_t (*_CandidateExtractor)(uint32_t);

uint32_t extract_smallest_candidate(uint32_t set) {
    return set & -set;
}

uint32_t extract_largest_candidate(uint32_t set) {
    return 1u << highest_set_bit_index(set);
}

uint32_t extract_random_candidate(uint32_t set) {
    uint32_t count = population_count(set);
    uint32_t idx = random(0, count) + 1;
    uint32_t candidate = 0;
    for (uint32_t i = 0; i < idx; ++i) {
        candidate = extract_smallest_candidate(set);
        set &= ~candidate;
    }
    return candidate;
}


bool solve(Sudoku *sudoku, _CandidateExtractor extract) {

    while (singles(sudoku) || hidden_singles(sudoku));

    const uint32_t not_found = (uint32_t) (-1);
    uint32_t mindex = not_found;
    uint32_t min = 10u;

    // find an empty field with the lowest candidate count
    for (uint32_t i = 0; i < 81u; ++i) {
        uint32_t count = population_count(sudoku->data[i] & UPPER);

        if (sudoku->data[i] & LOWER)
            continue;

        if (count >= min)
            continue;

        min = count;
        mindex = i;
    }

    // no field is empty
    if (mindex == not_found)
        return true;

    // test all candidates
    Sudoku sudoku_copy;
    sudoku_copy = *sudoku;

    uint32_t candidates = sudoku->data[mindex] >> SHIFT;
    while (candidates) {

        uint32_t c = extract(candidates);
        put(sudoku, mindex, c);

        if (solve(sudoku, extract))
            return true;

        // restore copy
        *sudoku = sudoku_copy;

        // pop candidate
        candidates &= ~c;
    }
    return false;
}

bool sudoku_solve(Sudoku *sudoku) {
    return solve(sudoku, extract_smallest_candidate);
}

bool sudoku_solve_reverse(Sudoku *sudoku) {
    return solve(sudoku, extract_largest_candidate);
}

bool sudoku_solve_random(Sudoku *sudoku) {
    return solve(sudoku, extract_random_candidate);
}

bool sudoku_equal_values(const Sudoku *lhs, const Sudoku *rhs) {
    for (uint32_t i = 0; i < 81u; ++i) {
        if ((lhs->data[i] & LOWER) != (rhs->data[i] & LOWER))
            return false;
    }
    return true;
}

void sudoku_put_one_hot_value(Sudoku *sudoku, uint32_t field, uint32_t candidate) {
    put(sudoku, field, candidate);
}

void sudoku_put_value(Sudoku *sudoku, uint32_t field, uint32_t candidate) {
    sudoku_put_one_hot_value(sudoku, field, 1u << (candidate - 1));
}

uint32_t sudoku_read_value(const Sudoku *sudoku, uint32_t field) {
    uint32_t digit = sudoku->data[field] & LOWER;
    return digit ? 1 + lowest_set_bit_index(digit) : 0;
}

void sudoku_from_buffer(Sudoku* sudoku, const uint32_t* buffer) {
    sudoku_init(sudoku);
    for (uint32_t i = 0; i < 81u; ++i) {
        if (!buffer[i])
            continue;
        sudoku_put_value(sudoku, i, buffer[i]);
    }
}

void sudoku_from_string(Sudoku* sudoku, const char* buffer) {
    sudoku_init(sudoku);
    for (uint32_t i = 0; i < 81u; ++i) {
        if (buffer[i] == '0')
            continue;
        sudoku_put_value(sudoku, i, buffer[i] - '0');
    }
}

void sudoku_to_string(const Sudoku *sudoku, char *out) {
    for (uint32_t i = 0; i < 81u; ++i) {
        out[i] = sudoku_read_value(sudoku, i) + '0';
    }
}

void sudoku_print(const Sudoku *sudoku) {
    char line[82];
    line[81] = '\n';
    sudoku_to_string(sudoku, line);
    fwrite(line, 1, 82, stdout);
}

void sudoku_pprint(const Sudoku *sudoku) {
    char line[81];
    sudoku_to_string(sudoku, line);
    for (uint32_t row = 0; row < 9; ++row) {
        for (uint32_t col = 0; col < 9; ++col) {
            printf("%c ", line[row * 9 + col]);
            if (col % 3 == 2)
                printf("  ");
        }
        if (row % 3 == 2)
            printf("\n");
        printf("\n");
    }
}
