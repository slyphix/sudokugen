#ifndef FIELD_SUBSET_H
#define FIELD_SUBSET_H

#include "sudoku.h"

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    uint32_t indices[81];
    uint32_t size;
} OrderedFieldSubset;

void ofs_set_identity(OrderedFieldSubset* fields);
void ofs_find_nonempty_fields(OrderedFieldSubset* fields, Sudoku* s);


typedef struct {
    uint32_t bits[3];
} FieldSubset;

bool fs_get_field(FieldSubset* fields, uint32_t index);
void fs_set_field(FieldSubset* fields, uint32_t index);
void fs_reset_field(FieldSubset* fields, uint32_t index);
void fs_toggle_field(FieldSubset* fields, uint32_t index);
void fs_include_all_fields(FieldSubset* fields);
void fs_exclude_all_fields(FieldSubset* fields);
void fs_add_nonempty_fields(FieldSubset* fields, Sudoku* s);
void fs_add_from_ifs(FieldSubset* target, OrderedFieldSubset* source);
uint32_t fs_find_first_set(FieldSubset* fields);
uint32_t fs_size(FieldSubset* fields);

#endif
