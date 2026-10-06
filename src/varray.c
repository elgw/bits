#include <stdint.h>
#include <stdlib.h>

#include "varray.h"

varray * varray_new(size_t n, u64 nbit)
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

u64 varray_get(const varray * V, size_t n){
    u64 r = 0;
    u64 m = 1;
    for(u64 kk = 0; kk < V->nbit; kk++){
        r += m*bitarray_get(V->B, V->nbit*n + kk);
        m*=2;
    }
    return r;
}

void varray_set(varray * V, size_t n, u64 value)
{
    u64 r = value;
    for(u64 kk = 0; kk < V->nbit; kk++){
        bitarray_set(V->B, V->nbit*n + kk, r % 2);
        r/=2;
    }
}
