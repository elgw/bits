#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <x86intrin.h>

#include "bitarray.h"
#include "cindex.h"
#include "rank9.h"
#include "select1.h"

// Get:
// - bitarray_get1, O(n)
// - rank9_get, O(1)
// - rank9b_get, O(1), cache oblivious, but use an extra copy of B
// Select:
// - bitarray_select1, O(n)
// - rank9_select1 -- todo
//
// In any case we can compare to a lookup table.

static void test_bit_array(){
    size_t n = 399;
    uint32_t * R = malloc(n*sizeof(uint32_t));
    for(u64 kk = 0; kk < n; kk++){
      R[kk] = (u32) rand() % 2;
    }

    bitarray * B = bitarray_new(n);

    for(u64 kk = 0; kk < n; kk++){
      bitarray_set(B, kk, (u8) R[kk]);
        assert(bitarray_get(B, kk) == R[kk]);
    }

    for(u64 kk = 0; kk < n; kk++){
        assert(bitarray_get(B, kk) == R[kk]);
    }

    bitarray_free(B);

    free(R);
}

static void test_varray(u64 n, int bits){
    printf("-- test_varray, n= %zu, bits = %d\n", n, bits);
    uint32_t * R = malloc(n*sizeof(uint32_t));
    u32 max = (u32) pow(2, bits);
    for(u64 kk = 0; kk < n; kk++){
      R[kk] = (u32) rand() % max;
    }

    varray * V = varray_new(n, (u32) bits);

    for(u64 kk = 0; kk < n; kk++){
        varray_set(V, kk, R[kk]);
    }

    for(u64 kk = 0; kk < n; kk++){
        assert(varray_get(V, kk) == R[kk]);
    }

    varray_free(V);

    free(R);
}

static void test_bitarray_rank1(void){
    u64 n = 512*4;
    printf("-- test_bitarray_rank1, n=%zu\n", n);
    u8 * A = calloc(n, sizeof(u8));
    u32 * R1 = calloc(n, sizeof(u32));
    bitarray * B = bitarray_new(n);
    u32 nset = 0;
    u32 nshow = (u32) n;
    nshow > 20 ? nshow = 20 : 0;

    for(u64 kk = 0; kk < n; kk++){
        if(rand() % 2){
            A[kk] = 1;
            bitarray_set(B, kk, 1);
            nset++;
        }
        R1[kk] = nset;
    }
    for(u64 kk = 0; kk < n; kk++){
        assert(bitarray_get(B, kk) == A[kk]);
    }

    rank9* r9 = rank9_init((u64*) B->data, n);
    rank9_print(r9);
    u32 nshow_select = nset;
    nshow_select > 10 ? nshow_select = 9 : 0 ;

    printf("A = ");
    for(u32 kk = 0; kk < nshow; kk++){
        printf("%u ", A[kk]);
    }
    printf("\n");
    printf("ref R1 = ");
    for(u32 kk = 0; kk < nshow_select; kk++){
        printf("%u ", R1[kk]);
    }
    printf("\n");
    printf("ba. R1 = ");
    for(u32 kk = 0; kk < nshow_select; kk++){
        printf("%lu ", bitarray_rank1(B, kk));
    }
    printf("\n");
    printf("r9. R1 = ");
    for(u32 kk = 0; kk < nshow_select; kk++){
        printf("%ld ", rank9_get(r9, kk));
    }
    printf("\n");

    for(u32 kk = 1; kk < nset; kk++){
        assert(bitarray_rank1(B, kk) == (u64) R1[kk]);
        assert(rank9_get(r9, kk) ==  R1[kk]);
    }

    rank9_free(r9);
    bitarray_free(B);
    free(A);
    free(R1);
}

static void benchmark_rank9_vs_lut(const u64 n){
    printf("n = %.1f M (%lu)\n", (double) n/ 1000000.0, n);
    bitarray * B = bitarray_new(n);
    u32 * R1 = malloc(n*sizeof(u32));
    u64 nset = 0;
    for(u64 kk = 0; kk < n; kk++){
        if(rand() % 2){
            bitarray_set(B, kk, 1);
            nset++;
        }
        R1[kk] = (u32) nset;
    }

    rank9 * r9 = rank9_init((u64*) B->data, n);
    rank9b * r9b = rank9b_init((u64*) B->data, n);

    // time for benchmark
    u64 rk_a = 0;
    u64 rk_r9 = 0;
    u64 rk_r9b = 0;
    u64 t_r9 = 0;
    u64 t_r9b = 0;
    u64 t_array = 0;
    u64 t0, t1;
    u64 idx;
    u32 cpuid;
    for(u64 ii = 0; ii < 1e7; ii++)
    {
      idx = (u64) rand() % (n-1);

        t0 = __rdtscp(&cpuid);
        rk_a += R1[idx];
        t1 = __rdtscp(&cpuid);
        t_array += t1-t0;
        //idx = rand() % (n-1);
        t0 = __rdtscp(&cpuid);
        rk_r9 += rank9_get(r9, idx);
        t1 = __rdtscp(&cpuid);
        t_r9 += t1-t0;
        //idx = rand() % (n-1);
        t0 = __rdtscp(&cpuid);
        rk_r9b += rank9b_get(r9b, idx);
        t1 = __rdtscp(&cpuid);
        t_r9b += t1-t0;
    }
    printf("t r9  = %lu (%lu)\n", t_r9,     rk_r9);
    printf("t r9b = %lu (%lu)\n", t_r9b,    rk_r9b);
    printf("t a   = %lu (%lu)\n", t_array,  rk_a);


    rank9_free(r9);
    rank9b_free(r9b);
    bitarray_free(B);
    free(R1);
}

static void test_bitarray_select1(void){
    u64 n = 1000;
    printf("-- test_bitarray_select1, n=%zu\n", n);
    u8 * A = calloc(n, sizeof(u8));
    u32 * S1 = calloc(n, sizeof(u32));
    bitarray * B = bitarray_new(n);
    u32 nset = 0;
    u32 nshow = (u32) n;
    nshow > 20 ? nshow = 20 : 0;

    for(u64 kk = 0; kk < n; kk++){
        if(rand() % 2){
            A[kk] = 1;
            bitarray_set(B, kk, 1);
            nset++;
        }
    }


    u32 nshow_select = nset;
    nshow_select > 20 ? nshow_select = 19 : 0 ;

    u32 pos = 0;

    for(u64 kk = 0; kk < n; kk++){
        if(A[kk] == 1){
	  S1[++pos] = (u32) kk;
        }
    }

    printf("A = ");
    for(u32 kk = 0; kk < nshow; kk++){
        printf("%u ", A[kk]);
    }
    printf("\n");
    printf("REF S1 = ");
    for(u32 kk = 0; kk < nshow_select; kk++){
        printf("%u ", S1[kk]);
    }
    printf("\n");
    printf("ba. S1 = ");
    for(u32 kk = 0; kk < nshow_select; kk++){
        printf("%lu ", bitarray_select1(B, kk));
    }
    printf("\n");

    for(u32 kk = 1; kk < nset; kk++){
        assert(bitarray_select1(B, kk) == (u64) S1[kk]);
    }

    bitarray_free(B);
    free(A);
    free(S1);
}


static void test_select1(void){
    u64 n = 1000;
    printf("-- test_select1 n=%zu\n", n);
    u8 * A = calloc(n, sizeof(u8));
    u32 * REF = calloc(n, sizeof(u32));
    bitarray * B = bitarray_new(n);
    u32 nset = 0;
    u32 nshow = (u32) n;
    nshow > 20 ? nshow = 20 : 0;

    for(u64 kk = 0; kk < n; kk++){
        if(rand() % 2){
            A[kk] = 1;
            bitarray_set(B, kk, 1);
            nset++;
        }
    }

    select1 * S1 = select1_new(B);

    u32 nshow_select = nset;
    nshow_select > 10 ? nshow_select = 9 : 0 ;

    printf("A = ");
    for(u32 kk = 0; kk < nshow; kk++){
        printf("%u ", A[kk]);
    }
    printf("\n");
    printf("ba. S1 = ");
    for(u32 kk = 0; kk < nshow_select; kk++){
        printf("%lu ", bitarray_select1(B, kk));
    }
    printf("\n");
    printf("s1. S1 = ");
    for(u32 kk = 0; kk < nshow_select; kk++){
        printf("%ld ", select1_get(S1, kk));
    }
    printf("\n");

    for(u32 kk = 1; kk < nset; kk++){
        assert(select1_get(S1, kk) == (i32) REF[kk]);
    }

    select1_free(S1);
    bitarray_free(B);
    free(A);
    free(S1);
}


static void test_eliasfano(void){
    printf("-- test_eliasfano\n");
    u32 n = 7000;
    u32 * A = malloc(n*sizeof(u32));
    #if 1
    A[0] = 1;
    for(u32 kk = 1; kk < n; kk++)
    {
      A[kk] = A[kk-1] + 1 + (u32) (rand() % 3);
    }
#else
    A[0] = 2; A[1] = 3; A[2] = 5; A[3] = 7;
    A[4] = 11; A[5] = 13; A[6] = 24;
    #endif
    printf("A=");
    for(u32 kk = 0; kk < 7; kk++){
        printf("%u ", A[kk]);
    }
    printf("\n");
    cindex * C = cindex_new(A, n);

    printf("C=");
    for(u32 kk = 0; kk < 7; kk++){
        printf("%u ", cindex_get(C, kk));
    }
    printf("\n");
    for(u32 kk = 0; kk < n; kk++){
        assert(cindex_get(C, kk) == A[kk]);
    }
    printf("cindex used %u bytes\n", C->mem_allocated);
    printf("the array used %lu bytes\n", n*sizeof(u32));
    cindex_free(C);
    free(A);
    return;
}

void dummy(void){
    bitarray * B = bitarray_new(64);
    for(int kk = 0; kk < 64; kk++)
    {
        bitarray_reset(B);
        bitarray_set(B, (u64) kk, 1); // fills from right to left...
        printf("bit %d: %lu (%d)\n", kk, B->data[0],
               __builtin_ctzl((u64) B->data[0]));
    }
}

int main(int argc, char ** argv)
{
    //dummy();
    if(argc > 1){
        if(atoi(argv[1]) == 1){
            u64 n = 512*1;
            while(n < 3e9){
                benchmark_rank9_vs_lut(n);
                n*=2;
            }
        }
    }
    srand((u32) time(NULL));
    for(int kk = 0; kk < 10; kk++){
        test_bit_array();
    }

    for(int bits = 1; bits < 17; bits++){
        test_varray(100, bits);
    }

    test_bitarray_rank1();
    test_bitarray_select1();

    test_select1();

    test_eliasfano();

    return EXIT_SUCCESS;
}
