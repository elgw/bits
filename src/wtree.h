#pragma once
#include "bitarray.h"
#include "select1c.h"
#include "rank1.h"

typedef struct {
    u64 char_bits;
    u64 n_char;
    bitarray * B;
    rank1 * R1;
    select1c * S1;
    // book keeping / cached results for r1 and s1 for each block?
    size_t mem_allocated;
    u64 rstart;
} wtree2;


// A new wavelet tree for n_char characters of char_bits bits each
// TODO: n_char has to be a multiple of 256 ...
// The input data consists of n_char values encoded by two bits each
wtree2 * wtree2_new(u64 * data, u64 n_char);

// Retrieve the value of the nth element, i.e. returns a value in [0 .. 3]
u64 wtree2_access(const wtree2 *, u64 n);

// Return the number of c up to and including position n
// i.e. wtree2_rank2(W, 2, 0) will say 1 if the first "character" is 2
u64 wtree2_rankc(const wtree2 *, u64 c, u64 n);

u64 wtree2_selectc(const wtree2 *, u64 c, u64 n);

u64 wtree2_range_query(const wtree2 *, u64 i, u64 j, u64 l, u64 r);

void wtree2_free(wtree2 * );
