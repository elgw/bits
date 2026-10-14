#include <math.h>
#include <stdio.h>
#include <assert.h>

#include "cindex.h"

static u64 BB(u64 x){
    return 64 - (u64) __builtin_clzl(x);
}

cindex * cindex_new(const u64 * A, u64 n)
{
    cindex * C = calloc(1, sizeof(cindex));
    C->mem_allocated += sizeof(cindex);

    C->n = n;
    C->bits_upper = (u32) BB(n);
    C->bits_lower = (u32) BB(A[n-1]) - C->bits_upper;

    if(0){
        printf("cindex_new, n=%lu, maxval=%lu, Upper bits: %lu, lower bits %lu\n",
               n, A[n-1],
               C->bits_upper, C->bits_lower);
    }

    // storage for upper
    u64 nbin = (u64) powl(2, C->bits_upper);
    u64 nbits_upper = nbin+n;
    assert(nbin <= 2*n);

    // Histogram over upper bits
    u64 * H = calloc(nbin, sizeof(u64));
    for(u64 kk = 0; kk < n; kk++){
        //printf("%u -> %u\n", A[kk], A[kk] >> bits_lower);
        u64 bin = A[kk] >> C->bits_lower;
        H[bin]++;
    }

    // Create upper array
    C->upper = bitarray_new(nbits_upper + 512 - (512 % nbits_upper));
    C->mem_allocated += C->upper->mem_allocated;
    u64 wpos = 0;
    for(u64 kk = 0; kk < nbin; kk++){
        for(u64 ll = 0; ll < H[kk]; ll++){
            bitarray_set1(C->upper, wpos++);
        }
        wpos++;
        //bitarray_set0(C->upper, wpos++); // not needed since we initialize to 0
    }

    C->S1 = select1c_new(C->upper);
    C->mem_allocated += C->S1->mem_allocated;
    free(H);

    // Create lower array
    C->lower = varray_new(n, C->bits_lower);
    C->mem_allocated += C->lower->mem_allocated;
    for(u64 kk = 0; kk < n; kk++){
        varray_set(C->lower, kk, A[kk]);
    }

    return C;
}

u64 cindex_get(const cindex * C, u64 kk)
{
    u64 lower = varray_get(C->lower, kk);
    u64 ra = select1c_get(C->S1, kk);
    u64 upper = (u64) (ra-kk) << C->bits_lower;

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
    select1c_free(C->S1);
    varray_free(C->lower);
    free(C);
    return;
}
