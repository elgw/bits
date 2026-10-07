#pragma once
#include <stdlib.h>
#include <stdint.h>

#include "bitarray.h"

typedef struct {
    u64 idx;
    u16 sub[4]; // 7 x 9bit might not be such a bad idea...
    // but rather than storing the absolute offsets we might have to
    // integrate the offsets
} select1d_64;

typedef struct {
    // L8[i] points to the i*SELECT1D_L0th 1.
    // L8[0] points to the 1'st 1 (i.e. )
    select1d_64 * L8; // the index
    const u64 * bits; // borrowed pointer from a bitarray
    u64 n_ones;
    u64 mem_allocated;
} select1d;

// An auxilary structure that can be used together with
// an existing bitarray to calculate select1/rank1 in
// almost linear time by storing an additional O(n) bits
// well about n*sizeof(select1_64)/SELECT1_L0
select1d * select1d_new(bitarray * B);
void select1d_free(select1d *);

// Get the location of the ith 1 in the underlying bit
// vector. Using 0-based indexing, i=0 gives the first 1
u64 select1d_get(const select1d *, uint64_t i);
