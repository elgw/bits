#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "rank1.h"

static u64 count_bits_u64(const u64 B){
  return (u64) __builtin_popcountl(B);
}

static u64 count_n_bits_u64(u64 B, u64 n)
{
    if(n == 0){ return 0;}
    u64 T = B  << (64-n); // undefined to shift by 64 bits
    return (u64) __builtin_popcountl(T);
}

rank1 * rank1_init(u64 * bits, u64 nbit)
{
    assert((nbit % 512) == 0);
    u64 nbin = (nbit) / 512 + 1;
    assert(nbin*512 >= nbit);
    rank1 * r1 = calloc(1, sizeof(rank1));
    r1->nbit = nbit;
    r1->bin = malloc(nbin*sizeof(rank1_bin));
    r1->bits = bits;

    u64 count = 0; // value of first bit
    for(u64 bb = 0; bb + 1 < nbin; bb++)
    {
        //        printf("bb = %lu\n", bb);
        r1->bin[bb].rankp = count;
        count += count_bits_u64(bits[bb*8]);
        u64 seven = 0;
        //printf("count=%lu\n", count);
        for(u64 ll = 1; ll < 8; ll++){
            seven = seven << 9;
            seven = seven | (count - r1->bin[bb].rankp);
            //printf("%lu\n", seven % 256);
            count += count_bits_u64(bits[bb*8+ll]);
        }
        r1->bin[bb].seven = seven;
    }
    r1->bin[nbin-1].rankp = count; // grand total
    // TODO: Stupid! should be able to ask about the last bit,
    // i.e. with -1 I guess we need to decide wether the first
    // bit of each block should be included in the index, or if it is the
    // bits up to that block ...
    // but of course we could add an extra block at the end just to
    // store the total number of 1s ... but then that would ask for
    // a larger B where it could calculate the trail...
    // but adding a little more memory is probably preferential compared to
    // having an extra conditional every time ...
    r1->n_one = rank1_get(r1, r1->nbit-2);
    return r1;
}

void rank1_print(const rank1 * r1)
{
    u64 nbin = r1->nbit / 512;
    for(u64 bb = 0; bb < nbin; bb++){
        printf("R1[%lu] = %8lu [", bb, r1->bin[bb].rankp);
        for(int ll = 1; ll < 8; ll++){
            u64 seven = r1->bin[bb].seven;
            u64 sub = seven >> ((7 -ll)*9);
            printf("%lu ", sub % 512);
        }
        printf("]\n");
    }
    return;
}

u64 rank1_get(const rank1 * r1, u64 b)
{
    b++; // to get the number of bits up to including b
    u64 b0 = b / 512; // "bucket" at level0
    u64 l0 = r1->bin[b0].rankp;
    u64 b1 = (b - b0*512) / 64; // "bucket" at level1
    u64 l1 = 0;
    if(b1 > 0){
        l1 = r1->bin[b0].seven >> ((7-b1))*9; // can we use builtin here to avoid the if?
        l1 = l1 % 512;
    }
    u64 b2 = b % 64;
    u64 l2 = count_n_bits_u64(r1->bits[b/64], b2);
    return l0+l1+l2;
}

void rank1_free(rank1 * r1){
    free(r1->bin);
    free(r1);
    return;
}

int rank1_select1_bs(const rank1 * r1, u64 b, u64 * s1)
{
    if(b == 0){
        return -1;
    }
    if(b > r1->n_one){
        return -1;
    }
    // Hey! TODO!
    // need to find the position where rank1 switches from b-1 to b
    // to do so we need to the number of 1's in order to bound the search region.
    // maybe we need two ranks for each position, since we are looking for a location
    // where rank(l-1) == r-1 and rank(l) == r
    u64 low = 0;
    u64 high = r1->nbit;

    while(low < high){
        u64 pos = low/2+high/2;
        u64 r = rank1_get(r1, pos);
        if(r < b){
            low = pos;
        } else {
            if(r > b) {
                high = pos;
            } else {
                // found the correct rank,
                // but possibly not the correct position
                // so we have to work backwards... or do another binary search.
                *s1 = pos;
            }
        }
    }

    return 0;
}

rank1b * rank1b_init(u64 * bits, u64 nbit)
{
    assert((nbit % 512) == 0);
    u64 nbin = (nbit+1) / 512;
    assert(nbin*512 >= nbit);
    rank1b * r1 = calloc(1, sizeof(rank1b));
    r1->nbit = nbit;
    r1->bin = malloc(nbin*sizeof(rank1b_bin));

    u64 count = 0; // value of first bit
    for(u64 bb = 0; bb+1 < nbin; bb++)
    {
        //        printf("bb = %lu\n", bb);
        r1->bin[bb].rankp = count;
        count += count_bits_u64(bits[bb*8]);
        u64 seven = 0;
        //printf("count=%lu\n", count);
        for(u64 ll = 1; ll < 8; ll++){
            seven = seven << 9;
            seven = seven | (count - r1->bin[bb].rankp);
            //printf("%lu\n", seven % 256);
            count += count_bits_u64(bits[bb*8 + ll]);
        }
        r1->bin[bb].seven = seven;
        memcpy(r1->bin[bb].bits, bits+bb*8, 8*sizeof(u64));
    }

    return r1;
}

u64 rank1b_get(const rank1b * r1, u64 b)
{
    b++; // to get the number of bits up to including b
    u64 bin = b / 512;
    u64 l0 = r1->bin[bin].rankp;
    u64 ll = (b - bin*512) / 64;
    u64 l1 = 0;

    l1 = r1->bin[bin].seven >> ((7-ll))*9;
    l1 = l1 % 512;
    l1 *= (l1 > 0);

    u64 l2 = count_n_bits_u64(r1->bin[bin].bits[ll], b % 64);
    return l0+l1+l2;
}

void rank1b_free(rank1b * r1){
    free(r1->bin);
    free(r1);
    return;
}
