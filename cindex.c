#include <math.h>
#include <stdio.h>
#include <assert.h>

#include "cindex.h"

cindex * cindex_new(const u32 * A, u32 n)
{
    cindex * C = calloc(1, sizeof(cindex));
    u32 maxval = A[n-1];
    C->n = n;
    // bits_upper
    u32 bits_upper = ceil(log2(n));
    // bits_lower
    u32 bits_lower = ceil(log2(maxval) - log2(n));
    C->lower_bits = bits_lower;
    printf("Upper bits: %u, lower bits %u\n", bits_upper, bits_lower);
    // storage for upper

    u32 nbin = pow(2, bits_upper);
    u32 nbits_upper = nbin+n; //2*n;
    C->upper = bitarray_new(nbits_upper);
    C->lower = varray_new(n, bits_lower);

    // Histogram over upper bits

    assert(nbin <= 2*n);
    printf("nbin = %u\n", nbin);
    u32 * H = calloc(nbin, sizeof(u32));
    for(u32 kk = 0; kk < n; kk++){
        //printf("%u -> %u\n", A[kk], A[kk] >> bits_lower);
        u32 bin = A[kk] >> bits_lower;
        H[bin]++;
    }
    // Create upper array
    u32 wpos = 0;
    for(u32 kk = 0; kk < nbin; kk++){
        for(u32 ll = 0; ll < H[kk]; ll++){
            bitarray_set(C->upper, wpos++, 1);
        }
        bitarray_set(C->upper, wpos++, 0);
    }
    free(H);
    // Create lower array
    for(u32 kk = 0; kk < n; kk++){
        varray_set(C->lower, kk, A[kk]);
        //printf("stored as %u\n", varray_get(C->lower, kk));
    }
    C->mem_allocated = C->upper->mem_allocated + C->lower->mem_allocated;
    return C;
}

u32 cindex_get(const cindex * C, u32 kk)
{
    return (u32) varray_get(C->lower, kk)
        + ((bitarray_rank1(C->upper, kk+1)-kk) << C->lower_bits);
}

void cindex_free(cindex * C){
    if(C == NULL){
        return;
    }
    bitarray_free(C->upper);
    varray_free(C->lower);
    free(C);
    return;
}
