#pragma once
#include <stdlib.h>
#include <stdint.h>

#include "bitarray.h"

typedef struct {
    u64 idx;
} select1c_64;

typedef struct {
    // L8[i] points to the i*SELECT1C_L0th 1.
    // L8[0] points to the 1'st 1 (i.e. )
    select1c_64 * L8; // the index
    bitarray * BV; // remove me
    const u64 * bits; // borrowed pointer from a bitarray
    u64 n_ones;
    u64 mem_allocated;
} select1c;

// An auxilary structure that can be used together with
// an existing bitarray to calculate select1/rank1 in
// almost linear time by storing an additional O(n) bits
// well about n*sizeof(select1_64)/SELECT1_L0
select1c * select1c_new(bitarray * B);
void select1c_free(select1c *);

// Get the location of the ith 1 in the underlying bit
// vector. Using 0-based indexing, i=0 gives the first 1
u64 select1c_get(const select1c *, uint64_t i);
