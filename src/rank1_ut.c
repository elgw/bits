#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "bitarray.h"
#include "rank1.h"
#include "rank1_ut.h"


void rank1_ut(int verbose){
    if(verbose > 0){
        printf("%s\n", __FILE__);
    }

    u64 n = 512*4;

    u8 * A = calloc(n, sizeof(u8));
    u32 * R1 = calloc(n, sizeof(u32));
    bitarray * B = bitarray_new(n);
    u32 nset = 0;
    u32 nshow = (u32) n;
    nshow > 20 ? nshow = 20 : 0;

    for(u64 kk = 0; kk < n; kk++){
        if(rand() % 2){
            A[kk] = 1;
            bitarray_set(B, kk, 1);
            nset++;
        }
        R1[kk] = nset;
    }
    for(u64 kk = 0; kk < n; kk++){
        assert(bitarray_get(B, kk) == A[kk]);
    }

    rank1* r1 = rank1_init((u64*) B->data, n);
    if(verbose > 2){
        rank1_print(r1);
    }
    u32 nshow_select = nset;
    nshow_select > 10 ? nshow_select = 9 : 0 ;
    if(verbose > 1){
        printf("A = ");
        for(u32 kk = 0; kk < nshow; kk++){
            printf("%u ", A[kk]);
        }
        printf("\n");
        printf("ref R1 = ");
        for(u32 kk = 0; kk < nshow_select; kk++){
            printf("%u ", R1[kk]);
        }
        printf("\n");
        printf("ba. R1 = ");
        for(u32 kk = 0; kk < nshow_select; kk++){
            printf("%lu ", bitarray_rank1(B, kk));
        }
        printf("\n");
        printf("r1. R1 = ");
        for(u32 kk = 0; kk < nshow_select; kk++){
            printf("%ld ", rank1_get(r1, kk));
        }
        printf("\n");
    }

    for(u32 kk = 0; kk < n; kk++){
        assert(bitarray_rank1(B, kk) == (u64) R1[kk]);
        assert(rank1_get(r1, kk) ==  R1[kk]);
    }

    rank1_free(r1);
    bitarray_free(B);
    free(A);
    free(R1);
}
