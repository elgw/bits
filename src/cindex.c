#include <math.h>
#include <stdio.h>
#include <assert.h>

#include "cindex.h"

cindex * cindex_new(const u64 * A, u64 n)
{
    cindex * C = calloc(1, sizeof(cindex));
    C->mem_allocated += sizeof(cindex);

    u64 maxval = A[n-1];
    C->n = n;
    // bits_upper
    // TODO: no need for floats
    u32 bits_upper = (u32) ceil(log2((double) n));
    u32 bits_lower = (u32) ceil(log2((double) maxval)) - bits_upper;

    // bits_lower

    C->lower_bits = bits_lower;
    if(0){
    printf("cindex_new, n=%lu, maxval=%lu, Upper bits: %u, lower bits %u\n",
           n, maxval,
           bits_upper, bits_lower);
    }
    // storage for upper

    u64 nbin = (u64) powl(2, bits_upper);
    u64 nbits_upper = nbin+n; //2*n;
    C->upper = bitarray_new(nbits_upper + 512 - (512 % nbits_upper));
    C->mem_allocated += C->upper->mem_allocated;
    C->lower = varray_new(n, bits_lower);
    C->mem_allocated += C->lower->mem_allocated;

    // Histogram over upper bits

    assert(nbin <= 2*n);
    //printf("nbin = %lu\n", nbin);
    u64 * H = calloc(nbin, sizeof(u64));
    for(u64 kk = 0; kk < n; kk++){
        //printf("%u -> %u\n", A[kk], A[kk] >> bits_lower);
        u64 bin = A[kk] >> bits_lower;
        H[bin]++;
    }
    // Create upper array
    u64 wpos = 0;
    for(u64 kk = 0; kk < nbin; kk++){
        for(u64 ll = 0; ll < H[kk]; ll++){
            bitarray_set(C->upper, wpos++, 1);
        }
        bitarray_set(C->upper, wpos++, 0);
    }
    if(0){
    bitarray_print(C->upper);
    }
    C->S1 = select1_new(C->upper);
    C->mem_allocated += C->S1->mem_allocated;
    free(H);
    // Create lower array
    for(u64 kk = 0; kk < n; kk++){
        varray_set(C->lower, kk, A[kk]);
        //printf("stored as %u\n", varray_get(C->lower, kk));
    }

    return C;
}

u64 cindex_get(const cindex * C, u64 kk)
{
    u64 lower = varray_get(C->lower, kk);
    u64 ra = select1_get(C->S1, kk+1);
    //u64 ra = bitarray_select1(C->S1->B, kk+1);
    //u64 ra2 = bitarray_select1(C->upper, kk+1);
    //assert(ra == ra2);
    u64 upper = (u64) (ra-kk) << C->lower_bits;

    if(0){
    printf("-> kk=%lu,lower=%lu,ra=%lu,upper=%lu\n",
           kk, lower, ra, upper);
    }
    return lower+upper;
}

void cindex_free(cindex * C){
    if(C == NULL){
        return;
    }
    bitarray_free(C->upper);
    select1_free(C->S1);
    varray_free(C->lower);
    free(C);
    return;
}
