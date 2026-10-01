#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "bitarray.h"


bitarray * bitarray_new(u64 n)
{
    bitarray * B = malloc(sizeof(bitarray));
    if(B == NULL){
        return NULL;
    }
    B->n = n;
    u64 nel = 1 + n / 64;
    B->data = calloc(nel, sizeof(u64));
    if(B == NULL){
        free(B);
        return NULL;
    }
    B->mem_allocated = nel*sizeof(u64);
    return B;
}

void bitarray_free(bitarray * B)
{
    assert(B != NULL);
    free(B->data);
    free(B);
}

u8 bitarray_get(const bitarray * B, const u64 n)
{
    assert(n < B->n);
    u64 aidx = n / 64;
    u64 sh = n - aidx*64;
    return (B->data[aidx] & (1LU << sh)) > 0;
}

void bitarray_set(bitarray * B, const u64 n, const u8 value)
{
#ifndef NDEBUG
    if(n >= B->n){
        printf("bitarray_set: trying to set bit %lu but the last possible is %lu\n",
               n, B->n-1);
    }
    assert(n < B->n);
#endif
    u64 aidx = n / 64;
    u64 sh = n - aidx*64;

    if(value == 1){
        B->data[aidx] |= (1LU << sh);
    } else {
        B->data[aidx] &= ~(1LU << sh);
    }
}

i32 bitarray_rank1(const bitarray * B, const u64 n)
{
    u64 nfound = 0;
    u64 nw = n/64;
    u64 nbit = n - nw*64;
    // count ones in full words
    for(u64 w = 0; w < nw; w++) {
        nfound += __builtin_popcountl(B->data[w]);
    }
    // count remaining bits
    for(u64 i = 0; i <= nbit; i++){
        nfound += bitarray_get(B, nw*64+i);
    }
    return nfound;
}

i32 bitarray_select1(const bitarray * B, const u64 n)
{
    // here we should do as in rank1, i.e.,
    // first loop over words
    if(n == 0){
        return 0;
    }
    u64 nfound = 0;
    u64 i = 0;
    while(1)
    {

        if(i == B->n){
            assert(0);
            return 0;
        }
        nfound += bitarray_get(B, i);
        if(nfound == n){
            return i;
        }
        i++;
    }
    assert(0);
    return 0;
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
