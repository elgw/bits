#pragma once

#include <stdlib.h>
#include <stdint.h>

// Some common definitions are also put here since all
// other header files will import this.

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int32_t i32;
typedef int64_t i64;


typedef struct {
    u64 * data;
    u64 n_bits;
    u64 mem_allocated;
} bitarray;

// A bit array or bitvector which supports
// reading and setting individual bits
bitarray * bitarray_new(u64 n);
void bitarray_free(bitarray * B);

// Get a specific bit
u8 bitarray_get(const bitarray * B, const u64 n);

// set a specific bit
void bitarray_set(bitarray * B, const u64 n, const u8 value);

// number of 1s below n
// O(n) time but with no memory overhead.
u64 bitarray_rank1(const bitarray * B, const u64 n);

// location of the nth 1
// O(n) time but with no memory overhead.
u64 bitarray_select1(const bitarray * B, const u64 n);

// set all bits to 0
void bitarray_reset(bitarray * );

// probably better to define as static where needed
int pos_first_one(const u64 w);

u64 bitarray_sum_ones(const bitarray *);
