#include <assert.h>
#include <x86intrin.h> // for pext
#include "util.h"

#include "wtree.h"

wtree * wtree_new(u64 * data, u64 n_char, u64 char_bits){
    wtree * W = calloc(1, sizeof(wtree));
    if(W == NULL){
        return NULL;
    }
    u64 n_bits = n_char * char_bits;
    assert(n_char % 64 == 0);
    assert(char_bits == 2);

    // create bit string and tree representation
    u64 mask_low = 0x5555555555555555; // (b1010 x 16)
    u64 mask_high = 0xaaaaaaaaaaaaaaaa; // (b0101 x 16)
    //print_binaryl("mlow", mask_low);
    //print_binaryl("mhigh", mask_high);
    u64 * B = calloc(n_bits/sizeof(u64), sizeof(u64));
    for(u64 kk = 0; kk < n_bits / 64; kk+=2){ // 128 bits at a time
        //print_binaryl("IN0", data[kk]);
        //print_binaryl("IN1", data[kk+1]);
        u64 low0 = _pext_u64(data[kk], mask_low);
        u64 high0 = _pext_u64(data[kk], mask_high);
        u64 low1 = _pext_u64(data[kk+1], mask_low);
        u64 high1 = _pext_u64(data[kk+1], mask_high);
        B[kk/2] = high0 + (high1 << 32);
        B[kk/2+ n_char/64] = low0 + (low1 << 32);
        //print_binaryl(" H", B[kk]);
        //print_binaryl(" L", B[kk + n_char/64]);
    }

    W->B = bitarray_new_from_data(B, n_bits);
    W->mem_allocated += W->B->mem_allocated;
    W->S1 = select1c_new(W->B);
    W->mem_allocated += W->S1->mem_allocated;
    W->R1 = rank1_init(B, n_bits); // TODO rank1 needs to support size % 64 = 0
    W->mem_allocated += W->R1->mem_allocated;

    return W;
}

void wtree_free(wtree * W){
    if(W == NULL){
        return;
    }
    bitarray_free(W->B);
    rank1_free(W->R1);
    select1c_free(W->S1);
    free(W);
    return;
}
