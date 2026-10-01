#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "select1.h"


select1 * select1_new(bitarray * B)
{
    select1 * S = malloc(sizeof(select1));
    if(S == NULL){
        return NULL;
    }

    S->B = B;
    u32 n1 = 0;

    u32 l = pow(ceil(log(B->n)/2), 2);
    S->l = l;
    printf("select1: n = %lu, l = %u\n", B->n, l);
    u32 n_first = ceil(B->n/l)+1;
    printf("n_first = %u\n", n_first);
    size_t first_size = n_first*sizeof(u32);
    S->first = malloc(n_first*sizeof(u32));
    S->first[0] = 0;

    u32 k = ceil(log(B->n)/2);
    S->k = k;
    u32 n_second = B->n / k + 1;
    printf("k=%u\n", k);
    size_t second_size = n_second*sizeof(u8);
    S->second = malloc(second_size);
    // TODO: This describes the rank not the select.
    for(u32 i = 0; i < B->n; i++){
        if(bitarray_get(B, i)){
            n1++;
        }
        if(i % l == 0){
            //printf("i=%u, l=%u, i/l=%u\n", i, l, i/l);
            S->first[i/l] = n1;
        }
        if(i % k == 0){
            S->second[i/k] = n1 - S->first[i/l];
        }
    }
    S->first[0] = 0;
    S->n1 = n1;
    S->mem_allocated = first_size + second_size + sizeof(select1);

    return S;
}

i64 select1_get(const select1 * S, size_t i)
{
    if(i > S->n1){
        return -1;
    }
    i64 count = 0;
    if(i % S->k > 0){
        u64 i0 = (i/S->k)*S->k+1;
        printf("[i=%zu, i/l = %zu, i/k = %zu, i0=%lu] ", i, i/S->l, i/S->k, i0);
        // TODO popcnt or similar
        for(u64 ii = i0; ii <= i; ii++){
            count += bitarray_get(S->B, ii);
        }
    }
    printf("(%u + %u + %ld)\n", S->first[i/S->l], S->second[i/S->k], count);
    return S->first[i/S->l] + S->second[i/S->k] + count;
}

void select1_free(select1 * S)
{
    if(S == NULL){
        return;
    }
    free(S->first);
    free(S->second);
    free(S);
    return;
}
