#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdlib.h>

static const uint32_t FULL32 = 0xffffffffu;
static const uint32_t UPPER  = 0xffff0000u;
static const uint32_t LOWER  = 0x0000ffffu;
static const uint32_t SHIFT  = 16u;

static const uint32_t ALL_CANDIDATES = 0x1ffu;
static const uint32_t NO_CANDIDATES  = 0x000u;

#if _MSC_VER
#include <intrin.h>
#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanReverse)
#define MSVC
#endif


inline uint32_t random(uint32_t inclusive_start, uint32_t exclusive_end) {
    return rand() % (exclusive_end - inclusive_start) + inclusive_start;
}

inline void random_shuffle(uint32_t *buffer, uint32_t length) {
    for (uint32_t i = 0; i < length - 1; ++i) {
        uint32_t j = random(i, length);
        uint32_t temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;
    }
}

inline uint32_t lowest_set_bit_index(uint32_t value) {
#ifdef MSVC
    unsigned long index;
    _BitScanForward(&index, value);
    return (uint32_t) index;
#else
    return __builtin_ctz(value);
#endif
}

inline uint32_t safe_lowest_set_bit_index(uint32_t value) {
    return value == 0 ? FULL32 : lowest_set_bit_index(value);
}

inline uint32_t highest_set_bit_index(uint32_t value) {
#ifdef MSVC
    unsigned long index;
    _BitScanReverse(&index, value);
    return (uint32_t) index;
#else
    return sizeof(unsigned int) - 1 - __builtin_clz(value);
#endif
}

inline uint32_t safe_highest_set_bit_index(uint32_t value) {
    return value == 0 ? FULL32 : highest_set_bit_index(value);
}

inline uint32_t population_count(uint32_t value) {
#ifdef MSVC
    return __popcnt(value);
#else
    return __builtin_popcount(value);
#endif
}

#endif
