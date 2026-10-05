#pragma once
#include <stdlib.h>
#include <stdint.h>

#include "bitarray.h"

// select1_X represents up to 2^X bits of the binary
// array.

typedef struct {
    u8 pos;
    u64 * B;
} select1_8;

typedef struct {
    u16 pos;
    union {
        u16 * tab16;
        select1_8 * sub8;
    };
} select1_16;

typedef struct {
    u32 pos;
    union {
        select1_16 * sub16;
        u32 * tab32;
    };
} select1_32;


typedef struct { // Holds 2^9 or 512 1's can be up to 2^64 bits wide
    // Indexes of B array
    u64 left; //
    u64 n_below; // At most ii*512 bit before this.
    union {
        select1_16 * sub16; // size <= 2^16
        select1_32 * sub32; // size > 2^16 & size < 2^18
        u64 * tab64; // size > 2^18 (2^) (2^9 * 2^8 = 2^17)
    };
} select1_64;

typedef struct {
    bitarray * B;
    select1_64 * L8;
    u64 n_ones;
    u64 mem_allocated;
} select1;


// An auxilary structure that can be used together with
// an existing bitarray to calculate select1/rank1 in
// almost linear time by storing an additional O(n) bits
select1 * select1_new(bitarray * B);
void select1_free(select1 *);
u64 select1_get(const select1 *, size_t i);
