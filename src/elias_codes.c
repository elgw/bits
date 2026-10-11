#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "elias_codes.h"

// clang12 or gcc17 required for __builtin_bitreverse64

// How many bits to write x as binary, i.e., ceil(log2(kk+1))
static u64 BB(u64 x){
    return 64 - __builtin_clzl(x);
}

static void print_binary(u64 x, int nb){
    while(nb > 0)
    {
        printf("%ld", x % 2);
        x/=2;
        nb--;
    }
}

u64 encode_gamma(u64 number){

    return __builtin_bitreverse64(number) >> (64 - 2*BB(number)+1);
}

u64 decode_gamma(u64 encoded){
    u64 pos = __builtin_ctzl(encoded); // pos of first 1
    return __builtin_bitreverse64(encoded << (64-2*pos-1));// shift right
}


u64 encode_delta(u64 decoded){
    return __builtin_bitreverse64(decoded) >> (64 - 2*BB(BB(decoded))-BB(decoded)+1) | // bin(decoded)
        __builtin_bitreverse64(BB(decoded)) >> (64 - 2*BB(BB(decoded))+1); // bin(BB(decoded))
}


u64 decode_delta(u64 encoded){
    u64 pos = __builtin_ctzl(encoded); // pos of first 1, where the number of bits is stored
    u64 nbit = __builtin_bitreverse64(encoded >> (64-pos)); // TODO!
    printf("[nbit: %lu]", nbit);
    return 0;
}

void elias_codes_ut(void){
    printf("Elias Gamma\n");
    for(u64 kk = 1; kk < 1090; kk++){
        u64 G = encode_gamma(kk);
        u64 iG = decode_gamma(G);
        if(kk == 5 | kk == 77){
        printf(" %lu ->  ", kk);
        print_binary(G, BB(kk)*2-1);
        printf(" -> %lu\n", iG);
        }
        assert(kk == iG);
    }
    printf("Elias Delta\n");
    for(u64 kk = 1; kk < 78; kk++){
        u64 D = encode_delta(kk);
        u64 iD = decode_delta(D);
        if(kk == 5 | kk == 77 | (kk != iD)){
            printf(" %lu ->  ", kk);
            print_binary(D, 2*BB(BB(kk)) + BB(kk)-1);
            //print_binary(D, 64);
            printf(" -> %lu\n", iD);
        }
    }

}
