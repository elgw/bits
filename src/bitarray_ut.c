#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "bitarray.h"

static void test_get_and_set(int verbose);
static void test_rank1(int verbose);
static void test_select1(int verbose);

static void test_get_and_set(int verbose)
{
    if(verbose > 0){
        printf("%s : get and set\n", __FILE__);
    }
    u64 n = (u64) rand() % 2000LU;
    uint32_t * R = malloc(n*sizeof(uint32_t));
    for(u64 kk = 0; kk < n; kk++){
        R[kk] = (u32) rand() % 2;
    }

    bitarray * B = bitarray_new(n);

    for(u64 kk = 0; kk < n; kk++){
        bitarray_set(B, kk, (u8) R[kk]);
        assert(bitarray_get(B, kk) == R[kk]);
    }

    for(u64 kk = 0; kk < n; kk++){
        assert(bitarray_get(B, kk) == R[kk]);
    }

    bitarray_free(B);

    free(R);
}

static void test_rank1(int verbose){
    if(verbose > 0){
        printf("%s : rank1\n", __FILE__);
    }
    u64 n = 512*4;
    u8 * A = calloc(n, sizeof(u8));
    u32 * R1 = calloc(n, sizeof(u32));
    bitarray * B = bitarray_new(n);
    u32 nset = 0;
    u32 nshow = 20;
    u32 nshow_rank = 10;
    nshow > (u32) n ? nshow = (u32) n : 0;
    nshow_rank > (u32) n ? nshow_rank = (u32) n : 0;

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

    if(verbose > 1){
        printf("A = ");
        for(u32 kk = 0; kk < nshow; kk++){
            printf("%u ", A[kk]);
        }
        printf("\n");
        printf("ref R1 = ");
        for(u32 kk = 0; kk < nshow_rank; kk++){
            printf("%u ", R1[kk]);
        }
        printf("\n");
        printf("ba. R1 = ");
        for(u32 kk = 0; kk < nshow_rank; kk++){
            printf("%lu ", bitarray_rank1(B, kk));
        }
        printf("\n");
    }
    for(u32 kk = 1; kk < nset; kk++){
        assert(bitarray_rank1(B, kk) == (u64) R1[kk]);
    }

    bitarray_free(B);
    free(A);
    free(R1);
}

static void test_select1(int verbose){
    if(verbose > 0){
        printf("%s : select1\n", __FILE__);
    }
    u64 n = 1000;
    u8 * A = calloc(n, sizeof(u8));
    u32 * S1 = calloc(n, sizeof(u32));
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
    }

    u32 nshow_select = nset;
    nshow_select > 20 ? nshow_select = 19 : 0 ;

    u32 pos = 0;

    for(u64 kk = 0; kk < n; kk++){
        if(A[kk] == 1){
            S1[++pos] = (u32) kk;
        }
    }
    if(verbose > 1){
        printf("A = ");
        for(u32 kk = 0; kk < nshow; kk++){
            printf("%u ", A[kk]);
        }
        printf("\n");
        printf("REF S1 = ");
        for(u32 kk = 0; kk < nshow_select; kk++){
            printf("%u ", S1[kk]);
        }
        printf("\n");
        printf("ba. S1 = ");
        for(u32 kk = 0; kk < nshow_select; kk++){
            printf("%lu ", bitarray_select1(B, kk+1));
        }
        printf("\n");
    }
    for(u32 kk = 1; kk < nset; kk++){
        assert(bitarray_select1(B, kk) == (u64) S1[kk]);
    }

    bitarray_free(B);
    free(A);
    free(S1);
}

void bitarray_ut(int verbose){
    if(verbose > 0){
        printf("bitarray_ut\n");
    }
    test_get_and_set(verbose);
    test_rank1(verbose);
    test_select1(verbose);
}
