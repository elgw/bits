#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>

#include "elias_codes.h"
#define EGAMMA_MAX 0x00000000ffffffffLU
#define EDELTA_MAX 0x001fffffffffffffLU
typedef uint64_t u64;
typedef int64_t i64;

// clang12 or gcc17 required for __builtin_bitreverse64

// How many bits to write x as binary, i.e., ceil(log2(kk+1))
static u64 BB(u64 x){
    return 64 - (u64) __builtin_clzl(x);
}

static void print_binary(u64 x, u64 _nb){
    int nb = (int) _nb;
    while(nb > 0)
    {
        if(nb == 32){
            printf(" ");
        }
        printf("%ld", x % 2);
        x/=2;
        nb--;
    }
}

static void print_binaryl(const char * s, u64 x){
    printf("%s: ", s);
    print_binary(x, 64);
    printf("\n");
}

u64 egamma_bits(u64 n){
    //raise(SIGSEGV);
    assert(n > 0);
    return (BB(n)*2-1);
}

u64 egamma_encode(u64 decoded){
    assert(decoded > 0);
    assert(decoded <= EGAMMA_MAX);
    return __builtin_bitreverse64(decoded) >> (64 - 2*BB(decoded)+1);
}

u64 egamma_decode(u64 encoded){
    u64 pos = (u64) __builtin_ctzl(encoded); // pos of first 1
    // handle extra content
    //print_binaryl("encoded", encoded);
    encoded = encoded & 0xffffffffffffffff >> (64 - 2*pos-1);
    //print_binaryl("   menc", encoded);
    return __builtin_bitreverse64(encoded << (64-2*pos-1));// shift right
}

u64 edelta_bits(u64 n){
    assert(n > 0);
    return (2*BB(BB(n)) + BB(n)-1);
}

u64 edelta_encode(u64 decoded){
    assert(decoded > 0);
    assert(decoded <= 0x00008fffffffffff);
    return __builtin_bitreverse64(decoded) >> (64 - 2*BB(BB(decoded))-BB(decoded)+1) | // bin(decoded)
        __builtin_bitreverse64(BB(decoded)) >> (64 - 2*BB(BB(decoded))+1); // bin(BB(decoded))
}

u64 edelta_decode(u64 encoded){
    u64 pos = (u64) __builtin_ctzl(encoded);
    u64 nbit = __builtin_bitreverse64(encoded) >> (64-2*pos-1);
    u64 D = __builtin_bitreverse64(encoded) >> (64-2*pos-1-nbit); // number first
    u64 D2 = D & (0xffffffffffffffff >> (64-nbit)); // only number left
    return D2;
}

// Insert nB bits from B into
// target with n_target elements
// starting at bit pos
// ASSUMES that the remainding bits in B are 0
static int
insert_bits(u64 * target, u64 n_target, u64 B, u64 nB, u64 pos)
{
    assert(nB > 0);
    assert(nB <= 64);
    u64 t0 = pos / 64;
    u64 t1 = (pos + nB) / 64;
    u64 b0 = pos - t0*64; // bit position within t0
    if(t1 >= n_target){
        return 1;
    }
    if(t0 == t1){ // insert into a single u8
        u64 b1 = b0 + nB;
        // shift B
        //printf("B = "); print_binary(B, 64); printf("\n");
        B = B << b0;
        //printf("B = "); print_binary(B, 64); printf("\n");
        // clear target
        u64 mask = 0xffffffffffffffffLU;
        mask = mask << b0;
        mask = mask >> b0;
        mask = mask >> b1;
        mask = mask << b1;
        //printf("Mask = "); print_binary(mask, 64); printf("\n");
        // insert B
        //printf("Target0 = "); print_binary(target[t0], 64); printf("\n");
        target[t0] = target[t0] & ~mask; // clear target
        //printf("Target1 = "); print_binary(target[t0], 64); printf("\n");
        target[t0] = target[t0] | B; // set bits
        //printf("Target2 = "); print_binary(target[t0], 64); printf("\n");
        return 0;
    }
    assert(t0 +1 == t1);
    //printf("b0 = %lu\n", b0);
    //printf("  B = "); print_binary(B, 64); printf("\n");
    u64 BR = B << b0;
    //printf("  BR= "); print_binary(BR, 64); printf("\n");

    // Insert first bits at the end of t0
    u64 mask = 0xffffffffffffffffLU << b0;
    //printf("mask= "); print_binary(mask, 64); printf("\n");
    target[t0] = target[t0] & ~mask;
    target[t0] = target[t0] | BR;

    // Insert the last bits at the beginning of t1
    u64 b1 = b0 + nB - 64;
    //printf("b1=%lu\n", b1);
    u64 BL = B >> (64-b0);
    //printf("  BL= "); print_binary(BL, 64); printf("\n");
    mask = 0xffffffffffffffffLU >> (64-b1);
    //printf("mask= "); print_binary(mask, 64); printf("\n");
    target[t0+1] = target[t0 + 1] & ~mask;
    target[t0+1] = target[t0 + 1] | BL;
    return 0;
}

u64
egamma_encode_array(const u64 * values, u64 n_value,
                    u64 * target, u64 n_target)
{
    u64 bits_used = 0;
    u64 n_encoded = 0;
    for(u64 kk = 0; kk < n_value; kk++){
        u64 b = egamma_bits(values[kk]);
        if(bits_used + b > 64*n_target){
            return n_encoded;
        }
        u64 E = egamma_encode(values[kk]);
        if(insert_bits(target, n_target, E, b, bits_used)){
            return n_encoded;
        }
        n_encoded++;
        bits_used += b;
    }
    return n_encoded;
}

u64
edelta_encode_array(const u64 * values, u64 n_value,
                    u64 * target, u64 n_target)
{
    u64 bits_used = 0;
    u64 n_encoded = 0;
    for(u64 kk = 0; kk < n_value; kk++){
        u64 b = edelta_bits(values[kk]);
        if(bits_used + b > 64*n_target){
            return n_encoded;
        }
        u64 E = edelta_encode(values[kk]);
        if(insert_bits(target, n_target, E, b, bits_used)){
            return n_encoded;
        }
        n_encoded++;
        bits_used += b;
    }
    return n_encoded;
}


// Returns 64 bits starting from the bit at bitpos
// if outside of the boundary, 0s are filled in.
// Return 0 if no more bits can be extracted
u64 get_word(const u64 * W, u64 nW, u64 bitpos){
    //print_binaryl("W0",  W[0]);
    if(bitpos / 64 >= nW){
        return 0;
    }
    if(bitpos % 64 == 0){
        return W[bitpos/64];
    }

    //print_binaryl("WL",  W[bitpos/64]);

    u64 l = bitpos - (bitpos/64)*64;
    //printf("bitpos = %lu, l = %lu\n", bitpos, l);
    u64 L = W[bitpos/64] >> l;
    //print_binaryl("L", L);
    u64 R = 0;
    if(bitpos/64 + 1 < nW){
        R = W[bitpos/64 + 1] << (64-l);
    }
    //print_binaryl("WR",  W[bitpos/64 + 1]);
    //print_binaryl("R", R);
    return L | R;
}

u64 egamma_decode_array(const u64 * encoded, u64 n_encoded,
                        u64 * decoded, u64 n_decoded)
{
    u64 readpos = 0; // bit to read from
    for(u64 kk = 0; kk < n_decoded; kk++){
        u64 D = get_word(encoded, n_encoded, readpos);
        if(D == 0){
            return kk;
        }
        //print_binaryl("D", D);
        decoded[kk] = egamma_decode(D);
        assert(decoded[kk] > 0);
        //printf("decoded[%lu] = %lu\n", kk, decoded[kk]);
        readpos += egamma_bits(decoded[kk]);
    }
    return n_decoded;
}

u64 edelta_decode_array(const u64 * encoded, u64 n_encoded,
                        u64 * decoded, u64 n_decoded)
{
    u64 readpos = 0; // bit to read from
    for(u64 kk = 0; kk < n_decoded; kk++){
        u64 D = get_word(encoded, n_encoded, readpos);
        if(D == 0){
            return kk;
        }
        //print_binaryl("D", D);
        decoded[kk] = edelta_decode(D);
        assert(decoded[kk] > 0);
        //printf("decoded[%lu] = %lu\n", kk, decoded[kk]);
        readpos += edelta_bits(decoded[kk]);
    }
    return n_decoded;
}


static void test_egamma_encode_array(int verbose){
    u64 w_encoded = 4; // w_encoded * 64 bits
    u64 * encoded = calloc(w_encoded, sizeof(u64));
    u64 w_decoded = 2*16; // words for the decoded, also number of values to insert
    u64 * decoded = calloc(w_decoded, sizeof(u64));
    u64 * values = calloc(w_decoded, sizeof(u64));
    for(u64 kk = 0; kk < w_decoded; kk++){
        values[kk] = 1 + (u64) rand() % 17000;
    }

    u64 nenc = egamma_encode_array(values, w_decoded, encoded, w_encoded);
    if(verbose > 1){
        printf("Could encode %lu/%lu values (%.1f bits per value)\n", nenc, w_decoded,
               (double)(64*w_encoded) / (double) nenc);
    }
    u64 ndec = egamma_decode_array(encoded, w_encoded, decoded, w_decoded);
    assert(ndec == nenc);
    if(verbose > 1){
        printf("Could decode %lu/%lu values\n", ndec, w_decoded);
    }
    if(verbose > 1){
        for(u64 kk = 0; kk < ndec; kk++){
            printf("%lu -- %lu\n", decoded[kk], values[kk]);
            assert(decoded[kk] == values[kk]);
        }
    }
    free(encoded);
    free(decoded);
    free(values);
}

static void test_edelta_encode_array(int verbose){
    u64 w_encoded = 4; // w_encoded * 64 bits
    u64 * encoded = calloc(w_encoded, sizeof(u64));
    u64 w_decoded = 2*16; // words for the decoded, also number of values to insert
    u64 * decoded = calloc(w_decoded, sizeof(u64));
    u64 * values = calloc(w_decoded, sizeof(u64));
    for(u64 kk = 0; kk < w_decoded; kk++){
        values[kk] = 1 + (u64) rand() % 17000;
    }

    u64 nenc = edelta_encode_array(values, w_decoded, encoded, w_encoded);
    if(verbose > 1){
        printf("Could encode %lu/%lu values (%.1f bits per value)\n", nenc, w_decoded,
               (double)(64*w_encoded) / (double) nenc);
    }
    u64 ndec = edelta_decode_array(encoded, w_encoded, decoded, w_decoded);
    assert(ndec == nenc);
    if(verbose > 1){
        printf("Could decode %lu/%lu values\n", ndec, w_decoded);
    }
    if(verbose > 1){
        for(u64 kk = 0; kk < ndec; kk++){
            printf("%lu -- %lu\n", decoded[kk], values[kk]);
            assert(decoded[kk] == values[kk]);
        }
    }
    free(encoded);
    free(decoded);
    free(values);
}


void elias_codes_ut(int verbose){
    if(verbose > 0){
        printf("%s : Elias Gamma (egamma_encode, egamma_decode)\n", __FILE__);
    }

    for(u64 kk = 1; kk < 1090; kk++){
        u64 G = egamma_encode(kk);
        u64 iG = egamma_decode(G);
        if(verbose > 1){
            if(kk == 5 | kk == 77){
                printf(" %lu ->  ", kk);
                print_binary(G, egamma_bits(kk));
                printf(" -> %lu\n", iG);
            }
        }
        assert(kk == iG);
    }
    if(verbose > 0){
        printf("%s : Elias Delta (edelta_encode, edelta_encode)\n", __FILE__);
    }
    for(u64 kk = 1; kk < 1090; kk++){
        u64 D = edelta_encode(kk);
        u64 iD = edelta_decode(D);
        if(verbose > 1){
            if(kk == 5 | kk == 77 | (kk != iD)){
                printf(" %lu ->  ", kk);
                print_binary(D, edelta_bits(kk));
                printf(" -> %lu\n", iD);
            }
        }
        assert(kk == iD);
    }

    if(verbose > 0){
        printf("%s : Elias Gamma (egamma_encode_array, decode_array)\n",
               __FILE__);
    }
    test_egamma_encode_array(verbose);
    test_edelta_encode_array(verbose);

    return;
}
