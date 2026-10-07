#pragma once
#include <stdlib.h>
#include <stdint.h>

#include "bitarray.h"

// I choose not to split the universe perfectly
// rather, rather restrict the break points to
// align with u64.
// Each bin is allowed to contain up to
// SELECT1_L0 bits, but can contain as few as
// SELECT1_L0 - 64.
typedef struct {
    // TODO: Faster to index the bit, not the word to save some space, and do the
    // extra book keeping?
    u64 i_word; // index into a word of  B
    // Number of 1's below B[64*i_word] = i_word*SELECT1_L1 - delta_count[0]
    u8 delta_count[1];
    // 56 unused bits.
} select1_64;

typedef struct {
    select1_64 * L8; // the index
    const u64 * bits; // borrowed pointer from a bitarray
    u64 n_ones;
    u64 mem_allocated;
} select1;

// An auxilary structure that can be used together with
// an existing bitarray to calculate select1/rank1 in
// almost linear time by storing an additional O(n) bits
// well about n*sizeof(select1_64)/SELECT1_L0
select1 * select1_new(bitarray * B);
void select1_free(select1 *);
u64 select1_get(const select1 *, size_t i);
