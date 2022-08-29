#include "field_subset.h"
#include "utils.h"


void ofs_set_identity(OrderedFieldSubset* fields) {
    for (uint32_t i = 0; i < 81; ++i) {
        fields->indices[i] = i;
    }
    fields->size = 81;
}


void ofs_find_nonempty_fields(OrderedFieldSubset* fields, Sudoku* s) {
    fields->size = 0;
    for (uint32_t i = 0; i < 81u; ++i) {
        uint32_t value = s->data[i] & LOWER;
        if (value == 0)
            continue;
        fields->indices[fields->size++] = i;
    }
}


bool fs_get_field(FieldSubset* fields, uint32_t index) {
    return fields->bits[index >> 5u] & (1 << (index & 0x1fu));
}


void fs_set_field(FieldSubset* fields, uint32_t index) {
    fields->bits[index >> 5u] |= 1 << (index & 0x1fu);
}


void fs_reset_field(FieldSubset* fields, uint32_t index) {
    fields->bits[index >> 5u] &= ~(1 << (index & 0x1fu));
}


void fs_toggle_field(FieldSubset* fields, uint32_t index) {
    fields->bits[index >> 5u] ^= 1 << (index & 0x1fu);
}


void fs_include_all_fields(FieldSubset* fields) {
    fields->bits[0] = FULL32;
    fields->bits[1] = FULL32;
    fields->bits[2] = FULL32;
}


void fs_exclude_all_fields(FieldSubset* fields) {
    fields->bits[0] = 0;
    fields->bits[1] = 0;
    fields->bits[2] = 0;
}


void fs_add_nonempty_fields(FieldSubset* fields, Sudoku* s) {
    for (uint32_t i = 0; i < 81u; ++i) {
        uint32_t value = s->data[i] & LOWER;
        if (value == 0)
            continue;
        fs_set_field(fields, i);
    }
}


void fs_add_from_ifs(FieldSubset* target, OrderedFieldSubset* source) {
    for (uint32_t i = 0; i < source->size; ++i) {
        fs_set_field(target, source->indices[i]);
    }
}


uint32_t fs_find_first_set(FieldSubset* fields) {
    uint32_t index;
    if ((index = safe_lowest_set_bit_index(fields->bits[0])) != FULL32)
        return index;
    if ((index = safe_lowest_set_bit_index(fields->bits[1])) != FULL32)
        return index + 32;
    if ((index = safe_lowest_set_bit_index(fields->bits[2])) < 17)
        return index + 64;
    return 81;
}


uint32_t fs_size(FieldSubset* fields) {
    return population_count(fields->bits[0]) + population_count(fields->bits[1]) + population_count(fields->bits[2] & 0x1ffffu);
}
