#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <signal.h>
#include <x86intrin.h>

#include "select1d.h"

#define SELECT1D_L0 (1024LU) // Number of 1's per chunk
#define SELECT1D_L1 (256LU)  // Number of 1's per sub chunk

// Return the 0-based position of the ith 1 of the word
// i.e., select1_word(0xffffffffffffffff, kk) -> kk, kk \in [0, 63]
// returns 64 if there are less than i 1s
// i.e. select1_word(0LU, kk) -> 64
static u64 select1_word(u64 word, u64 i){
    // https://ziggit.dev/t/find-the-nth-set-bit/14391
    return (u64) __builtin_ctzl(_pdep_u64(1LU << i, word));
}

select1d * select1d_new(bitarray * B)
{
    if(B->n_bits % 64 != 0){
        fprintf(stderr, "select1d can only work with bitarrays having k*64 bits\n");
        raise(SIGSEGV);
    }

    select1d * S = calloc(1, sizeof(select1d));
    if(S == NULL){
        return NULL;
    }

    // Need to know the number of 1s for pre-allocation
    S->n_ones = bitarray_sum_ones(B);
    assert(S->n_ones > 0);
    S->mem_allocated += sizeof(select1d);
    S->bits = B->data; // borrowed, owned by B and not freed

    u64 n_level = (S->n_ones + SELECT1D_L0 -1) / SELECT1D_L0;
    S->L8 = calloc(n_level, sizeof(select1d_64));
    S->mem_allocated += n_level*sizeof(select1d_64);

    u64 wpos = 0; // word position
    u64 found_1s = 0;
    u64 bpos = 0; // bit position
    for(u64 chunk = 0; chunk < n_level; chunk++)
    {
        // Fast forward until the wanted '1' can be found in the next word
        u64 wanted_1s = SELECT1D_L0*chunk + 1;
        while(found_1s + (u64) __builtin_popcountl(B->data[wpos]) < wanted_1s)
        {
            found_1s += (u64) __builtin_popcountl(B->data[wpos++]);
        }
        // Position with bit-accuracy
        bpos = 64*wpos + select1_word(B->data[wpos], wanted_1s - found_1s - 1);
        assert(bitarray_select1(B, chunk*SELECT1D_L0 + 1) == bpos);
        S->L8[chunk].idx = bpos;
        for(u64 ss = 1; ss < 4; ss++){
            wanted_1s = SELECT1D_L0*chunk + SELECT1D_L1*ss + 1;
            if(wanted_1s > S->n_ones){
                break;
            }
            while(found_1s + (u64) __builtin_popcountl(B->data[wpos]) < wanted_1s)
            {
                found_1s += (u64) __builtin_popcountl(B->data[wpos++]);
            }
            // Position with bit-accuracy
            bpos = 64*wpos + select1_word(B->data[wpos], wanted_1s - found_1s - 1);

            u64 sspos = bpos - S->L8[chunk].idx;
            if(sspos >= pow(2, 16)){
                // TODO: Switch to a sparse table for this chunk.
                goto fail_construct;
            }
            S->L8[chunk].sub[ss] = (u16) sspos;
            assert(bitarray_select1(B, chunk*SELECT1D_L0 + ss*SELECT1D_L1 + 1) == bpos);
        }
    }
    return S;
fail_construct:
    fprintf(stderr, "select1d_new failed, the 1s are too sparse\n");
    select1d_free(S);
    return NULL;
}


u64 select1d_get(const select1d * S, u64 i)
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

    u64 l0 = i/SELECT1D_L0; // bin index
    u64 l1 = (i - SELECT1D_L0*(i/SELECT1D_L0))/SELECT1D_L1;
    assert(l1 < 4);
    select1d_64 chunk = S->L8[l0];; // bin
    u64 bpos = (chunk.idx + (u64) chunk.sub[l1]);
    //printf("(%lu, %lu)\n", chunk.idx, (u64) chunk.sub[l1]);
    u64 wpos =  bpos / 64; // position of word in bitarray
    u64 nfound = l0*SELECT1D_L0+l1*SELECT1D_L1+1; // number of 1's found so far, up to chunk.idx
    i++;

    // wind back to the start of the word
    // by subtracting the number of 1's found
    // up to subpos
    u64 subpos = bpos - wpos*64;
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

void select1d_free(select1d * S)
{
    if(S == NULL){
        return;
    }
    free(S->L8);
    free(S);
    return;
}
