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
wtree2 * wtree2_new(u64 * data, u64 n_char);

u64 wtree2_rank1(const wtree2 *, u64 n);
u64 wtree2_select1(const wtree2 *, u64 n);
u64 wtree2_access(const wtree2 *, u64 n);
u64 wtree2_range_query(const wtree2 *, u64 i, u64 j, u64 l, u64 r);

void wtree2_free(wtree2 * );
