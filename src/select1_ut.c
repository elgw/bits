#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "bitarray.h"
#include "select1.h"
#include "select1_ut.h"

static void test_select1(int verbose, u64 n){
    if(verbose > 1){
        printf("-- test_select1 n=%zu\n", n);
    }
    u8 * A = calloc(n, sizeof(u8));
    u32 * REF = calloc(n, sizeof(u32));
    bitarray * B = bitarray_new(n);
    u32 nset = 0;
    u32 nshow = (u32) n;
    nshow > 20 ? nshow = 20 : 0;

    for(u64 kk = 0; kk < n; kk++){
        if(rand() % 2){
            A[kk] = 1;
            bitarray_set(B, kk, 1);
            REF[nset++] = (u32) kk;
        }
    }

    select1 * S1 = select1_new(B);

    u32 nshow_select = nset;
    nshow_select > 10 ? nshow_select = 9 : 0 ;

    printf("A = ");
    for(u32 kk = 0; kk < nshow; kk++){
        printf("%u ", A[kk]);
    }
    printf("\n");

    printf("REF S1 = ");
    for(u32 kk = 0; kk < nshow_select; kk++){
        printf("%u ", REF[kk]);
    }
    printf("\n");

    printf("ba. S1 = ");
    for(u32 kk = 1; kk <= nshow_select; kk++){
        printf("%lu ", bitarray_select1(B, kk));
    }
    printf("\n");

    printf("s1. S1 = ");
    for(u32 kk = 1; kk <= nshow_select; kk++){
        printf("%ld ", select1_get(S1, kk));
    }
    printf("\n");

    for(u32 kk = 1; kk < nset; kk++){
        //printf("%lu -- %u\n", select1_get(S1, kk), REF[kk-1]);
        assert(select1_get(S1, kk) == (u64) REF[kk-1]);
    }

    select1_free(S1);
    bitarray_free(B);
    free(REF);
    free(A);
}


void select1_ut(int verbose){
    if(verbose > 0){
        printf("%s\n", __FILE__);
    }
    test_select1(verbose, 7001);
    test_select1(verbose, 17000);
    test_select1(verbose, 77000);
}
