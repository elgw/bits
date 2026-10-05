#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <x86intrin.h>
#include <getopt.h>

#include "bitarray.h"
#include "bitarray_ut.h"
#include "cindex.h"
#include "cindex_ut.h"
#include "varray.h"
#include "varray_ut.h"
#include "rank1.h"
#include "rank1_ut.h"
#include "select1.h"
#include "select1_ut.h"


static void benchmark_rank1_vs_lut(const u64 n){
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

    rank1 * r1 = rank1_init((u64*) B->data, n);
    rank1b * r1b = rank1b_init((u64*) B->data, n);

    // time for benchmark
    u64 rk_a = 0;
    u64 rk_r1 = 0;
    u64 rk_r1b = 0;
    u64 t_r1 = 0;
    u64 t_r1b = 0;
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
        rk_r1 += rank1_get(r1, idx);
        t1 = __rdtscp(&cpuid);
        t_r1 += t1-t0;
        //idx = rand() % (n-1);
        t0 = __rdtscp(&cpuid);
        rk_r1b += rank1b_get(r1b, idx);
        t1 = __rdtscp(&cpuid);
        t_r1b += t1-t0;
    }
    printf("t r1  = %lu (%lu)\n", t_r1,     rk_r1);
    printf("t r1b = %lu (%lu)\n", t_r1b,    rk_r1b);
    printf("t a   = %lu (%lu)\n", t_array,  rk_a);


    rank1_free(r1);
    rank1b_free(r1b);
    bitarray_free(B);
    free(R1);
}


static void benchmark_select1_vs_lut(const u64 n){
    printf("benchmark_select1_vs_lut\n");
    printf("n = %.1f M (%lu)\n", (double) n/ 1000000.0, n);
    bitarray * B = bitarray_new(n);
    u32 * S1 = malloc(n*sizeof(u32));
    u64 nset = 0;
    for(u64 kk = 0; kk < n; kk++){
        if(rand() % 2){
            bitarray_set(B, kk, 1);
            nset++;
            S1[nset] = (u32) kk;
        }
    }

    select1 * s1 = select1_new(B);
    const u64 n_ones = s1->n_ones;
    // time for benchmark
    u64 s1_array = 0;
    u64 s1_s1 = 0;
    u64 t_array = 0;
    u64 t_s1 = 0;
    u64 t0, t1;
    u64 idx;
    u32 cpuid;
    for(u64 ii = 0; ii < 1e6; ii++)
    {
        idx = 1 + (u64) rand() % (n_ones-1);
        t0 = __rdtscp(&cpuid);
        s1_array += S1[idx];
        t1 = __rdtscp(&cpuid);
        t_array += t1-t0;
        //idx = rand() % (n-1);
        t0 = __rdtscp(&cpuid);
        s1_s1 += select1_get(s1, idx);
        t1 = __rdtscp(&cpuid);
        t_s1 += t1-t0;
    }
    printf("t s1  = %lu (%lu)\n", t_s1,     s1_s1);
    printf("t a   = %lu (%lu)\n", t_array,  s1_array);

    select1_free(s1);
    bitarray_free(B);
    free(S1);
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


typedef struct {
    int verbose;
    int benchmark;
} config;

static config * config_new(int argc, char ** argv)
{
    struct option longopts[] = {
        { "verbose",  required_argument, NULL, '1' },
        { "benchmark",  required_argument, NULL, '2' },
        { NULL,           0,                 NULL,   0   }
    };
    config * conf = calloc(1, sizeof(config));
    int ch;
    while((ch = getopt_long(argc, argv,
                            "1:2:",
                            longopts, NULL)) != -1) {
        switch(ch) {
        case '1':
            conf->verbose = atoi(optarg);
            break;
        case '2':
            conf->benchmark = atoi(optarg);
            break;
        default:
            break;
        }
    }
    return conf;
}

static void config_free(config * conf){
    free(conf);
    return;
}

int main(int argc, char ** argv)
{
    config * conf = config_new(argc, argv);
    if(conf->benchmark == 1){
        u64 n = 512*1;
        while(n < 3e9){
            benchmark_rank1_vs_lut(n);
            n*=2;
        }
    }
    if(conf->benchmark == 2){
        u64 n = 512*1;
        while(n < 3e9){
            benchmark_select1_vs_lut(n);
            n*=2;
        }
    }


    srand((u32) time(NULL));

    bitarray_ut(conf->verbose); // bitarray_ut.c
    varray_ut(conf->verbose);
    rank1_ut(conf->verbose);
    select1_ut(conf->verbose);
    cindex_ut(conf->verbose);
    config_free(conf);
    return EXIT_SUCCESS;
}
