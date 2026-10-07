#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <signal.h>
#include <x86intrin.h>

#include "select1c.h"

// To test:
// - Index to specific bit, not word to save memory at the cost of some more bit fiddling
// - Two level index (then sparse regions have to be handled separately).

#define SELECT1C_L0 (1024LU) // Number of 1's per chunk. 2048 seems good

// Return the 0-based position of the ith 1 of the word
// i.e., select1_word(0xffffffffffffffff, kk) -> kk
// returns 64 if there are less than i 1s
// i.e. select1_word(0LU, kk) -> 64
static u64 select1_word(u64 word, u64 i){
    // https://ziggit.dev/t/find-the-nth-set-bit/14391
    return (u64) __builtin_ctzl(_pdep_u64(1LU << i, word));
}

select1c * select1c_new(bitarray * B)
{
#ifndef NDEBUG
    if(B->n_bits % 64 != 0){
        raise(SIGSEGV);
    }
#endif
    select1c * S = calloc(1, sizeof(select1c));
    if(S == NULL){
        return NULL;
    }
    S->mem_allocated += sizeof(select1c);
    S->bits = B->data; // borrowed, owned by B and not freed
    S->BV = B; // TODO: REMOVE
    // Need to know the number of 1s for pre-allocation
    S->n_ones = bitarray_sum_ones(B);
    u64 n_level = (S->n_ones + SELECT1C_L0 -1) / SELECT1C_L0;
    S->L8 = calloc(n_level, sizeof(select1c_64));
    S->mem_allocated += n_level*sizeof(select1c_64);

    u64 pos = 0; // In terms of B, B[pos]
    const u64 * restrict BA = B->data;
    u64 found_1s = 0;
    u64 bpos = 0;
    for(u64 chunk = 0; chunk < n_level; chunk++)
    {
        // Until the wanted 1 can be found in the next word
        u64 wanted_1s = SELECT1C_L0*chunk + 1;
        while(found_1s + (u64) __builtin_popcountl(BA[pos]) < wanted_1s)
        {
            found_1s += (u64) __builtin_popcountl(BA[pos++]);
        }
        bpos = 64*pos + select1_word(BA[pos], wanted_1s - found_1s - 1);
        assert(bitarray_select1(B, chunk*SELECT1C_L0 + 1) == bpos);
        S->L8[chunk].idx = bpos;
    }
    return S;
}


u64 select1c_get(const select1c * S, u64 i)
{
#ifndef NDEBUG
    if(i >= S->n_ones){
        printf("Asking for the %lu:th 1 but there are only %lu ones\n",
               i+1, S->n_ones);
        printf("%s:L%d\n", __FILE__, __LINE__);
        raise(SIGSEGV);
    }
    assert(i < S->n_ones);
#endif

    u64 l0 = i/SELECT1C_L0; // bin index
    select1c_64 chunk = S->L8[l0]; // bin
    u64 wpos = chunk.idx / 64; // position of word in bitarray
    u64 nfound = l0*SELECT1C_L0+1; // number of 1's found so far, up to chunk.idx
    i++;

    // wind back to the start of the word
    // by subtracting the number of 1's found
    // up to subpos
    u64 subpos = chunk.idx - wpos*64;
    nfound -= (u64) __builtin_popcountl(S->bits[wpos] << (64-(subpos+1)));

    // Fast forward until the relevant word
    u64 nnb = (u64) __builtin_popcountl(S->bits[wpos]);
    while((nfound + nnb) < i){
        nfound += nnb;
        wpos++;
        nnb = (u64) __builtin_popcountl(S->bits[wpos]);
    }
    assert(nfound < i);

    // Final selection in a single word
    assert(select1_word(S->bits[wpos], i-nfound-1) < 64);
    return wpos*64 + select1_word(S->bits[wpos], i-nfound-1);
}

void select1c_free(select1c * S)
{
    if(S == NULL){
        return;
    }
    free(S->L8);
    free(S);
    return;
}
