#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "cindex.h"
#include "cindex_ut.h"

static void
cindex_ut_size(int verbose, u64 n)
{
    // An increasing sequence
    u64 * A = malloc(n*sizeof(u64));
    A[0] = 1;
    for(u64 kk = 1; kk < n; kk++){
        A[kk] = A[kk-1] + 1 + (u64) (rand() % 13);
    }
    if(n == 7){
        A[0] = 2;
        A[1] = 3;
        A[2] = 5;
        A[3] = 7;
        A[4] = 11;
        A[5] = 13;
        A[6] = 25;
    }

    if(verbose > 0){
        printf("A=");
        for(u64 kk = 0; kk < 7; kk++){
            printf("%lu ", A[kk]);
        }
        printf("\n");
    }
    cindex * C = cindex_new(A, n);
    if(verbose > 0){
        printf("C=");
        for(u64 kk = 0; kk < 7; kk++){
            printf("%lu ", cindex_get(C, kk));
        }
        printf("\n");
    }
    for(u64 kk = 0; kk < n; kk++){
        if(cindex_get(C, kk) != A[kk]){
            printf("Wrong result!\n");
            printf("A[%lu] = %lu while cindex(C, %lu)=%lu\n",
                   kk, A[kk],
                   kk, cindex_get(C, kk));
            exit(EXIT_FAILURE);
        }
    }
    if(verbose > 1){
        printf("cindex used %lu bytes\n", C->mem_allocated);
        printf("the array used %lu bytes\n", n*sizeof(u64));
    }
    cindex_free(C);
    free(A);
    return;
}

void cindex_ut(int verbose){
    if(verbose > 0){
        printf("%s\n", __FILE__);
    }
    cindex_ut_size(verbose, 7);
    cindex_ut_size(verbose, 7000);
    cindex_ut_size(verbose, 70000);
    cindex_ut_size(verbose, 700000);
}
