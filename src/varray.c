#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "varray.h"

varray * varray_new(u64 n, u64 nbit)
{
    varray * V = malloc(sizeof(varray));
    if(V == NULL){
        return NULL;
    }
    V->nbit = nbit;
    V->nel = n;
    V->B = bitarray_new(n*nbit);
    if(V->B == NULL) {
        free(V);
        return NULL;
    }
    V->mem_allocated = sizeof(varray) + V->B->mem_allocated;
    V->bitmask = 0xffffffffffffffff >> (64-nbit);
    //printf("bitmask: %lu\n", V->bitmask);
    return V;
}

void varray_free(varray * V){
    if(V == NULL) {
        return;
    }
    bitarray_free(V->B);
    free(V);
    return;
}

u64 varray_get(const varray * V, u64 n){


    u64 pos = n*V->nbit/64;
    u64 rem = n*V->nbit - pos*64;
    if(rem + V->nbit <= 64){ // The number if within a single word
        return (V->B->data[pos] >> rem) & V->bitmask;
    } else { // stored over two words
        //printf("n=%lu, pos=%lu, bits=%lu, rem=%lu\n",
        // n, pos, V->nbit, rem);
        u64 low = (V->B->data[pos] >> rem) & V->bitmask;
        u64 high =  V->B->data[pos+1] << (64-rem); //rem);
        u64 ret = (low + high) & V->bitmask;
        return ret;
    }
    assert(0);
    return 0;
}

#if 0 // this should do the same thing
    u64 r = 0;
    u64 m = 1;
    for(u64 kk = 0; kk < V->nbit; kk++){
        r += m*bitarray_get(V->B, V->nbit*n + kk);
        m*=2;
    }
    return r;
#endif


void varray_set(varray * V, u64 n, u64 value)
{
    u64 r = value;
    for(u64 kk = 0; kk < V->nbit; kk++){
        bitarray_set(V->B, V->nbit*n + kk, r % 2);
        r/=2;
    }
}
