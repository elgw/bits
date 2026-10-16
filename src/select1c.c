#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <signal.h>
#include <x86intrin.h>

#include "bitpos.h"
#include "select1c.h"

#define SELECT1C_L0 (1024LU) // Number of 1's per chunk

select1c * select1c_new(bitarray * B)
{
    if(B->n_bits % 64 != 0){
        fprintf(stderr, "select1c can only work with bitarrays having k*64 bits\n");
        raise(SIGSEGV);
    }

    select1c * S = calloc(1, sizeof(select1c));
    if(S == NULL){
        return NULL;
    }
    S->mem_allocated += sizeof(select1c);

    // Need to know the number of 1s for pre-allocation
    S->n_ones = bitarray_sum_ones(B);
    if(S->n_ones == 0){
        return S;
    }
    S->bits = B->data; // borrowed, owned by B and not freed

    u64 n_level = (S->n_ones + SELECT1C_L0 -1) / SELECT1C_L0;
    S->L8 = calloc(n_level, sizeof(select1c_64));
    S->mem_allocated += n_level*sizeof(select1c_64);

    u64 wpos = 0; // word position
    u64 found_1s = 0;
    u64 bpos = 0; // bit position
    for(u64 chunk = 0; chunk < n_level; chunk++)
    {
        // Fast forward until the wanted '1' can be found in the next word
        u64 wanted_1s = SELECT1C_L0*chunk + 1;
        while(found_1s + (u64) __builtin_popcountl(B->data[wpos]) < wanted_1s)
        {
            found_1s += (u64) __builtin_popcountl(B->data[wpos++]);
        }
        // Position with bit-accuracy
        //bpos = 64*wpos + select1_word(B->data[wpos], wanted_1s - found_1s - 1);
        bpos = 64*wpos + bitpos(B->data[wpos], wanted_1s - found_1s - 1 );
        assert(bitarray_select1(B, chunk*SELECT1C_L0 + 1) == bpos);
        S->L8[chunk].idx = bpos;
    }
    return S;
}


u64 select1c_get(const select1c * S, u64 i)
{

    if(i >= S->n_ones){
        return -1LU;
    }

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
    return wpos*64 + bitpos(S->bits[wpos], i-nfound-1);
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
