#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "cindex.h"
#include "cindex_ut.h"

static void
cindex_ut_size(int verbose, u32 n)
{
    u32 * A = malloc(n*sizeof(u32));
#if 1
    A[0] = 1;
    for(u32 kk = 1; kk < n; kk++)
    {
        A[kk] = A[kk-1] + 1 + (u32) (rand() % 3);
    }
#else
    A[0] = 2; A[1] = 3; A[2] = 5; A[3] = 7;
    A[4] = 11; A[5] = 13; A[6] = 24;
#endif
    if(verbose > 0){
        printf("A=");
        for(u32 kk = 0; kk < 7; kk++){
            printf("%u ", A[kk]);
        }
        printf("\n");
    }
    cindex * C = cindex_new(A, n);
    if(verbose > 0){
        printf("C=");
        for(u32 kk = 0; kk < 7; kk++){
            printf("%u ", cindex_get(C, kk));
        }
        printf("\n");
    }
    for(u32 kk = 0; kk < n; kk++){
        assert(cindex_get(C, kk) == A[kk]);
    }
    if(verbose > 1){
        printf("cindex used %lu bytes\n", C->mem_allocated);
        printf("the array used %lu bytes\n", n*sizeof(u32));
    }
    cindex_free(C);
    free(A);
    return;
}

void cindex_ut(int verbose){
    if(verbose > 0){
        printf("%s\n", __FILE__);
    }
    cindex_ut_size(verbose, 7000);
}
