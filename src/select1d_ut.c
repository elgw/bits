#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "bitarray.h"
#include "select1d.h"
#include "select1d_ut.h"

static void test_select1d(int verbose, u64 n, double density){
    if(verbose > 1){
        printf("-- test_select1d n=%zu\n", n);
    }
    u8 * A = calloc(n, sizeof(u8));
    u32 * REF = calloc(n, sizeof(u32));
    bitarray * B = bitarray_new(n);
    u32 nset = 0;
    u32 nshow = (u32) n;
    nshow > 20 ? nshow = 20 : 0;

    for(u64 kk = 0; kk < n; kk++){
        if((double) rand() / (double) RAND_MAX < density){
            A[kk] = 1;
            bitarray_set(B, kk, 1);
            REF[nset++] = (u32) kk;
        }
    }

    select1d * S1 = select1d_new(B);

    u32 nshow_select = nset;
    nshow_select > 10 ? nshow_select = 9 : 0 ;
    if(verbose > 1){
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

        printf("s1c S1 = ");
        for(u32 kk = 0; kk < nshow_select; kk++){
            printf("%ld ", select1d_get(S1, kk));
        }
        printf("\n");
    }
    for(u32 kk = 0; kk < nset; kk++){
        if(select1d_get(S1, kk) != (u64) REF[kk]){
            printf("kk: %u, select1d:%lu ref:%u (nset=%u)\n",
                   kk, select1d_get(S1, kk), REF[kk], nset);
        }
        assert(select1d_get(S1, kk) == (u64) REF[kk]);
    }

    select1d_free(S1);
    bitarray_free(B);
    free(REF);
    free(A);
}


void select1d_ut(int verbose){
    if(verbose > 0){
        printf("%s\n", __FILE__);
    }
    for(double density = 0.5; density < 1.0; density += 0.01)
    {
        test_select1d(verbose, 7001, density);
        test_select1d(verbose, 17000, density);
        test_select1d(verbose, 77000, density);
    }
}
