#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "select1.h"


select1 * select1_new(bitarray * B)
{
    select1 * S = calloc(1, sizeof(select1));
    if(S == NULL){
        return NULL;
    }
    S->mem_allocated += sizeof(select1);
    S->B = B; // borrowed, not owned by S

    // Need to know the number of 1s for pre-allocation
    u64 n_ones = bitarray_sum_ones(B);
    S->n_ones = n_ones;
    // printf("bvector length: %lu, number of 1s: %lu\n", B->n_bits, n_ones);
    const u64 l0 = 512; // Number of 1's per chunk.
    u64 n_level0 = n_ones / l0;
    if(n_level0*l0 < n_ones){
        n_level0++;
    }
    assert(n_level0*l0 >= n_ones);
    S->L8 = calloc(n_level0, sizeof(select1_64));
    S->mem_allocated += n_level0*sizeof(select1_64);

    double mem_quota =  (double) (S->mem_allocated*8) / (double) B->n_bits;
    printf("Mem allocated: %lu bits (%.1f)\n", S->mem_allocated*8,
           mem_quota);

    u64 pos = 0; // In terms of B, B[pos]
    u64 * BA = B->data;
    u64 found_1s = 0;

    for(u64 chunk = 0; chunk < n_level0; chunk++)
    {
        S->L8[chunk].left = pos;
        S->L8[chunk].n_below = found_1s;
        //printf("Chunk %lu starts at pos %lu\n", chunk, pos);
        //printf("Bits below: %lu\n", S->L8[chunk].n_below);

        u64 most_ones = l0*(chunk+1);
        most_ones >S->n_ones ? most_ones = S->n_ones : 0;

        while(found_1s + (u64) __builtin_popcountl(BA[pos]) < most_ones)
        {
            found_1s += (u64) __builtin_popcountl(BA[pos]);
            pos++;
        }

        int is_sparse = (pos - S->L8[chunk].left) > pow(2, 16);
        if(is_sparse) {
            // TODO
        }
        if(! is_sparse) {
            S->L8->sub16 = calloc(4, sizeof(select1_16));
        }
    }
    // TODO: Recursively update the lower levels
    return S;
}

static u64
select1_raw(const u64 * BA,
            const u64 n_BA, // number of u64s to scan
            const u64 n) // number of the '1' to find
{
    u64 nfound = 0;
    u64 i = 0;

    // Scan a 64-bit word at a time until we know
    // that the bit that we are searching for is in the next
    while(nfound + (u64) __builtin_popcountl(BA[i]) < n){
        nfound += (u64) __builtin_popcountl(BA[i++]);
    }

    u64 j = 0;
    while(1)
    {
        nfound+= (BA[i] & (1LU << j)) > 0;
        if(nfound == n){
            //printf("(i=%lu, j=%lu)\n", i, j);
            return i*64+j;
        }
        j++;
    }
    assert(0);
    return 0;
}

u64 select1_get(const select1 * S, u64 i)
{
    u64 l0 = i/512;
    select1_64 chunk = S->L8[l0];
    if(0){
        printf("[i=%lu l0=%lu chunk.left= %lu, chunk.n_below= %lu]\n",
               i, l0, chunk.left, chunk.n_below);
    }
    if(0){
    u64 nfound = 0;
    for(u64 kk = 0; kk < chunk.left; kk++)
    {
        nfound += (u64) __builtin_popcountl(S->B->data[kk]);
    }
    if(nfound != chunk.n_below){
        printf("nfound=%lu nbelow=%lu\n", nfound, chunk.n_below);
        assert(nfound == chunk.n_below);
    }
    }
    return chunk.left*64LU + select1_raw(S->B->data + chunk.left,
                                         S->B->n_bits/64 + 1 - chunk.left,
                                         i - chunk.n_below);
}

void select1_free(select1 * S)
{
    if(S == NULL){
        return;
    }

    free(S);
    return;
}
