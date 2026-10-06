#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <signal.h>

#include "bitarray.h"

bitarray * bitarray_new(u64 n)
{
    bitarray * B = malloc(sizeof(bitarray));
    if(B == NULL){
        return NULL;
    }
    B->n_bits = n;
    u64 nel = (n+63) / 64;
    B->data = calloc(nel, sizeof(u64));
    if(B == NULL){
        free(B);
        return NULL;
    }
    B->mem_allocated = nel*sizeof(u64) + sizeof(bitarray);
    return B;
}

void bitarray_free(bitarray * B)
{
    assert(B != NULL);
    free(B->data);
    free(B);
    return;
}

u8 bitarray_get(const bitarray * B, const u64 n)
{
    if(n >= B->n_bits) { raise(SIGSEGV); }
    assert(n < B->n_bits);
    u64 aidx = n / 64;
    u64 sh = n - aidx*64;
    return (B->data[aidx] & (1LU << sh)) > 0;
}

void bitarray_set(bitarray * B, const u64 n, const u8 value)
{
#ifndef NDEBUG
    if(n >= B->n_bits){
        printf("bitarray_set: trying to set bit %lu but the last possible is %lu\n",
               n, B->n_bits-1);
    }
    assert(n < B->n_bits);
#endif
    u64 aidx = n / 64;
    u64 sh = n - aidx*64;

    if(value == 1){
        B->data[aidx] |= (1LU << sh);
    } else {
        B->data[aidx] &= ~(1LU << sh);
    }
}

u64 bitarray_rank1(const bitarray * B, const u64 n)
{
    u64 nfound = 0;
    u64 nw = n/64;
    u64 nbit = n - nw*64;
    // count ones in full words
    for(u64 w = 0; w < nw; w++) {
      nfound += (u64) __builtin_popcountl(B->data[w]);
    }
    // count remaining bits
    for(u64 i = 0; i <= nbit; i++){
        nfound += bitarray_get(B, nw*64+i);
    }
    return nfound;
}

static u64 select1_raw(const u64 * BA, const u64 n_BA,
                const u64 n)
{
    u64 nfound = 0;
    u64 i = 0;

    while(nfound + 64 < n){
        nfound += (u64) __builtin_popcountl(BA[i++]);
    }

    i*=64;

    while(1)
    {
        if(i == n_BA){
            assert(0);
            return 0;
        }
        {
            u64 aidx = i / 64;
            u64 sh = i - aidx*64;
            assert(sh < 64);
            nfound+= (BA[aidx] & (1LU << sh)) > 0;
        }
        if(nfound == n){
            return i;
        }
        i++;
    }
    assert(0);
    return 0;
}

u64 bitarray_select1(const bitarray * B, const u64 n)
{
    //if(n == 0){
    //    raise(SIGSEGV);
    //    exit(EXIT_FAILURE);
    //}
    assert( (n > 0) && "bitarray_select1 uses 1-indexing");
    return select1_raw(B->data, B->n_bits, n);
}

void bitarray_reset(bitarray * B)
{
    memset(B->data, 0, B->mem_allocated);
}

int pos_first_one(const u64 w){
    // TODO: Need to pair with popcount or check for 0?
    // 0->64
    // 1->63
    // ...
    // 134217728 -> 36
    return __builtin_ctzl(w) + 1;
}


u64 bitarray_sum_ones(const bitarray * B){
    u64 n_ones = 0;
    for(u64 kk = 0; kk < B->n_bits/64; kk++){
        n_ones += (u64) __builtin_popcountl(B->data[kk]);
    }
    return n_ones;
}

void bitarray_print(const bitarray * B){
    u64 nshow = 20;
    B->n_bits < nshow ? nshow = B->n_bits : 0;
    for(u64 kk = 0; kk < nshow; kk++)
    {
        printf("%d ", (int) bitarray_get(B, kk));
    }
    if(nshow < B->n_bits){
        printf("... (and %lu more)", B->n_bits - nshow);
    }
    printf("\n");
}
