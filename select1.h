#pragma once
#include <stdlib.h>
#include <stdint.h>

#include "bitarray.h"

// An "upgraded bitvector" with faster select1 operator.
// following Mäkinen, p. 23.
// however, skipping the last "four Russians technique" step
// for the moment. Might revise some time to make sure that popcount
// is used.
typedef struct {
    bitarray * B;
    u32 n1; // number of 1s
    u32 l;
    u32 * first;
    u32 k;
    u8 * second;
    u32 mem_allocated;
} select1;


// An auxilary structure that can be used together with
// an existing bitarray to calculate select1/rank1 in
// almost linear time by storing an additional O(n) bits
select1 * select1_new(bitarray * B);
void select1_free(select1 *);
i64 select1_get(const select1 *, size_t i);
