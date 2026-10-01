#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "rank9.h"

#define BINBITS 512 // THE 64*8 bits that each bin represents

static u64 count_bits_u64(const u64 B){
    return __builtin_popcountl(B);
}

static u64 count_n_bits_u64(u64 B, u64 n)
{
    if(n == 0){ return 0;}
    u64 T = B  << (64-n); // undefined to shift by 64 bits
    return __builtin_popcountl(T);
}

rank9 * rank9_init(u64 * bits, u64 nbit)
{
    assert((nbit % BINBITS) == 0);
    u64 nbin = (nbit+1) / BINBITS;
    assert(nbin*BINBITS >= nbit);
    rank9 * r9 = calloc(1, sizeof(rank9));
    r9->nbit = nbit;
    r9->bin = malloc(nbin*sizeof(rank9_bin));
    r9->bits = bits;

    u64 count = 0; // value of first bit
    for(u64 bb = 0; bb < nbin; bb++)
    {
        //        printf("bb = %lu\n", bb);
        r9->bin[bb].rankp = count;
        count += count_bits_u64(bits[bb*8]);
        u64 seven = 0;
        //printf("count=%lu\n", count);
        for(int ll = 1; ll < 8; ll++){
            seven = seven << 9;
            seven = seven | (count - r9->bin[bb].rankp);
            //printf("%lu\n", seven % 256);
            count += count_bits_u64(bits[bb*8+ll]);
        }
        r9->bin[bb].seven = seven;
    }
    return r9;
}

void rank9_print(const rank9 * r9)
{
    u64 nbin = r9->nbit / BINBITS;
    for(u64 bb = 0; bb < nbin; bb++){
        printf("R9[%lu] = %8lu [", bb, r9->bin[bb].rankp);
        for(int ll = 1; ll < 8; ll++){
            u64 seven = r9->bin[bb].seven;
            u64 sub = seven >> ((7 -ll)*9);
            printf("%lu ", sub % 512);
        }
        printf("]\n");
    }
    return;
}

u64 rank9_get(const rank9 * r9, u64 b)
{
    b++; // to get the number of bits up to including b
    u64 bin = b / BINBITS;
    u64 l0 = r9->bin[bin].rankp;
    u64 ll = (b - bin*BINBITS) / 64;
    u64 l1 = 0;
    if(ll > 0){
        l1 = r9->bin[bin].seven >> ((7-ll))*9;
        l1 = l1 % 512;
    }
    u64 l2 = count_n_bits_u64(r9->bits[b/64], b % 64);
    return l0+l1+l2;
}

void rank9_free(rank9 * r9){
    free(r9->bin);
    free(r9);
    return;
}


rank9b * rank9b_init(u64 * bits, u64 nbit)
{
    assert((nbit % BINBITS) == 0);
    u64 nbin = (nbit+1) / BINBITS;
    assert(nbin*BINBITS >= nbit);
    rank9b * r9 = calloc(1, sizeof(rank9b));
    r9->nbit = nbit;
    r9->bin = malloc(nbin*sizeof(rank9b_bin));

    u64 count = 0; // value of first bit
    for(u64 bb = 0; bb < nbin; bb++)
    {
        //        printf("bb = %lu\n", bb);
        r9->bin[bb].rankp = count;
        count += count_bits_u64(bits[bb*8]);
        u64 seven = 0;
        //printf("count=%lu\n", count);
        for(int ll = 1; ll < 8; ll++){
            seven = seven << 9;
            seven = seven | (count - r9->bin[bb].rankp);
            //printf("%lu\n", seven % 256);
            count += count_bits_u64(bits[bb*8+ll]);
        }
        r9->bin[bb].seven = seven;
        memcpy(r9->bin[bb].bits, bits+bb*8, 8*sizeof(u64));
    }
    return r9;
}

u64 rank9b_get(const rank9b * r9, u64 b)
{
    b++; // to get the number of bits up to including b
    u64 bin = b / BINBITS;
    u64 l0 = r9->bin[bin].rankp;
    u64 ll = (b - bin*BINBITS) / 64;
    u64 l1 = 0;

    l1 = r9->bin[bin].seven >> ((7-ll))*9;
    l1 = l1 % 512;
    l1 *= (l1 > 0);

    u64 l2 = count_n_bits_u64(r9->bin[bin].bits[ll], b % 64);
    return l0+l1+l2;
}

void rank9b_free(rank9b * r9){
    free(r9->bin);
    free(r9);
    return;
}
