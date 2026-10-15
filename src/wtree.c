#include <assert.h>
#include <stdio.h>
#include <x86intrin.h> // for pext
#include "util.h"

#include "wtree.h"

wtree2 * wtree2_new(u64 * data, u64 _n_char){
    wtree2 * W = calloc(1, sizeof(wtree2));
    if(W == NULL){
        return NULL;
    }
    W->n_char = _n_char;
    u64 data_n_bits = W->n_char * 2;;
    assert(W->n_char % 64 == 0);

    // create bit string and tree representation
    const u64 mask_low = 0x5555555555555555; // (b1010 x 16)
    const u64 mask_high = 0xaaaaaaaaaaaaaaaa; // (b0101 x 16)
    assert( (mask_low & 1) == 1);
    assert( (mask_low & 2) == 0);
    assert( (mask_high & 1) == 0);
    assert( (mask_high & 2) == 2);
    assert( (mask_high & 4) == 0);
    assert( (mask_high & 8) == 8);

    W->B = bitarray_new(data_n_bits);
    W->mem_allocated += W->B->mem_allocated;

    // Set the high bits of the wavelet tree and count how many 1's there are
    u64 n_high_1s = 0;
    for(u64 kk = 0; kk < W->n_char / 64; kk++){
        u64 high0 = _pext_u64(data[2*kk], mask_high);
        u64 high1 = _pext_u64(data[2*kk+1], mask_high);
        W->B->data[kk] = high0 + (high1 << 32);
        n_high_1s += (u64) __builtin_popcountl( W->B->data[kk] );
    }

    // Where to write to the left and right child
    u64 lowpos = W->n_char;
    u64 highpos = W->n_char + (W->n_char - n_high_1s);
    W->rstart = highpos;
    // Set the low bits
    for(u64 kk = 0; kk < data_n_bits / 64; kk++){
        {
            u64 w = data[kk];
            for(i64 ll = 0; ll < 32; ll++){
                if((w & 2LU) == 0)
                {
                    bitarray_set(W->B, lowpos++, (w & 1LU));
                } else {
                    bitarray_set(W->B, highpos++, (w & 1LU));
                }
                w = w >> 2;
            }
        }
    }

    W->S1 = select1c_new(W->B);
    W->mem_allocated += W->S1->mem_allocated;
    W->R1 = rank1_init(W->B->data, data_n_bits); // TODO rank1 needs to support size % 64 = 0
    W->mem_allocated += W->R1->mem_allocated;

    assert(n_high_1s == rank1_get(W->R1, W->n_char-1));

    return W;
}

void wtree2_free(wtree2 * W){
    if(W == NULL){
        return;
    }
    bitarray_free(W->B);
    rank1_free(W->R1);
    select1c_free(W->S1);
    free(W);
    return;
}


u64 wtree2_access(const wtree2 * W, u64 n)
{
    u64 high = bitarray_get(W->B, n);
    if(high == 0){
        u64 lpos = W->n_char + (n - rank1_get(W->R1, n));
        //printf("[L=%lu]\n", lpos);
        return bitarray_get(W->B, lpos);

    } else {
        u64 rpos = W->rstart
            + rank1_get(W->R1, n) - 1; // pos in right

        assert(rpos < 2*W->n_char);
        assert(rank1_get(W->R1, W->n_char) == bitarray_rank1(W->B, W->n_char));
        assert(rank1_get(W->R1, n) == bitarray_rank1(W->B, n));

        return 2 + bitarray_get(W->B, rpos);
    }
}
