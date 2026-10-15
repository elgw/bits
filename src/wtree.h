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
} wtree;


// A new wavelet tree for n_char characters of char_bits bits each
wtree * wtree_new(u64 * data, u64 n_char, u64 char_bits);

u64 wtree_rank1(const wtree *, u64 n);
u64 wtree_select1(const wtree *, u64 n);
u64 wtree_access(const wtree *, u64 n);
u64 wtree_range_query(const wtree *, u64 i, u64 j, u64 l, u64 r);

void wtree_free(wtree * );
