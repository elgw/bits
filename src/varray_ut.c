#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <signal.h>

#include "varray.h"
#include "varray_ut.h"


static void test_varray_size(int verbose, u64 n, int bits){
    if(verbose > 1){
        printf("-- test_varray, n= %zu, bits = %d\n", n, bits);
    }
    uint32_t * R = malloc(n*sizeof(uint32_t));
    u32 max = (u32) pow(2, bits) - 1;
    for(u64 kk = 0; kk < n; kk++){
        R[kk] = (u32) rand() % max;
    }

    varray * V = varray_new(n, (u32) bits);

    for(u64 kk = 0; kk < n; kk++){
        varray_set(V, kk, R[kk]);
    }

    for(u64 kk = 0; kk < n; kk++){
        #ifndef NDEBUG
        if(varray_get(V, kk) != R[kk]){
            printf("Element %lu should be %u but got %lu\n",
                   kk,
                   R[kk],
                   varray_get(V, kk));
            raise(SIGSEGV);
            assert(varray_get(V, kk) == R[kk]);
        }
        #endif

    }

    varray_free(V);

    free(R);
}


void varray_ut(int verbose){
    if(verbose > 0){
        printf("%s\n", __FILE__);
    }
    for(int bits = 1; bits < 17; bits++){
        test_varray_size(verbose, 100, bits);
    }
}
