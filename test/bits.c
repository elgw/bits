#include <assert.h>
#include <getopt.h>
#include <locale.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <x86intrin.h>

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

static void benchmark_cindex_vs_lut(const u64 n){
    u64 * L = malloc(n*sizeof(u64));
    L[0] = 0;
    for(u64 kk = 1; kk < n; kk++){
        L[kk] = L[kk-1] + rand() % 13;
    }

    cindex * C = cindex_new(L, n);

    // time for benchmark
    u64 res_array = 0;
    u64 res_cindex = 0;
    u64 t_array = 0;
    u64 t_cindex = 0;
    u64 t0, t1;
    u64 idx;
    u32 cpuid;
    u64 n_sample = 1e6;
    for(u64 ii = 0; ii < n_sample; ii++)
    {
        idx = (u64) rand() % (n-1);
        //if( rand() % 2 == 0){
        t0 = __rdtscp(&cpuid);
        res_array = L[idx];
        t1 = __rdtscp(&cpuid);
        t_array += t1-t0;
        //idx = rand() % (n-1);
        t0 = __rdtscp(&cpuid);
        res_cindex = cindex_get(C, idx);
        t1 = __rdtscp(&cpuid);
        t_cindex += t1-t0;

        if(res_cindex != res_array){
            printf("%s:%d Error results differ\n", __FILE__, __LINE__);
            printf("L[%lu] = %lu, cindex->%lu\n", idx, L[idx], cindex_get(C, idx));
            exit(EXIT_FAILURE);
        }
    }



    printf("| %'lu | %'.0f | %'.0f |\n",
           n,
           (double) t_cindex/ (double) n_sample,
           (double) t_array/(double) n_sample);

    cindex_free(C);
    free(L);
}


static void run_benchmark_cindex_vs_lut(void){

    printf("Reporting average rdts time\n");
    u64 n = 512*1;
    printf("| N   | T_cindex  | T_array |\n");
    printf("| --: |       --: |     --: |\n");
    while(n < 3e9){
        benchmark_cindex_vs_lut(n);
        n*=2;
    }
}


static void benchmark_rank1_vs_lut(const u64 n){
    //printf("n = %.1f M (%lu)\n", (double) n/ 1000000.0, n);
    bitarray * B = bitarray_new(n);
    u64 * R1 = malloc(n*sizeof(u64));
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
    const u64 n_sample = 1e7;
    for(u64 ii = 0; ii < n_sample; ii++)
    {
        idx = (u64) rand() % (n-1);

        t0 = __rdtscp(&cpuid);
        rk_a += R1[idx];
        t1 = __rdtscp(&cpuid);
        t_array += t1-t0;

        t0 = __rdtscp(&cpuid);
        rk_r1b += rank1b_get(r1b, idx);
        t1 = __rdtscp(&cpuid);
        t_r1b += t1-t0;

        t0 = __rdtscp(&cpuid);
        rk_r1 += rank1_get(r1, idx);
        t1 = __rdtscp(&cpuid);
        t_r1 += t1-t0;


    }
    if(0){
        printf("t r1  = %lu (%lu)\n", t_r1,     rk_r1);
        printf("t r1b = %lu (%lu)\n", t_r1b,    rk_r1b);
        printf("t a   = %lu (%lu)\n", t_array,  rk_a);
    }
    printf("| %'lu | %'.0f | %'.0f | %'.0f |\n",
           n,
           (double) t_r1/ (double) n_sample,
           (double) t_r1b/ (double) n_sample,
           (double) t_array/(double) n_sample);
    if(rk_r1 != rk_a){
        printf("Results differ\n");
        exit(EXIT_FAILURE);
    }
    if(rk_r1b != rk_a){
        printf("Results differ\n");
        printf("rk1: %lu, rk1b: %lu, rka: %lu\n", rk_r1, rk_r1b, rk_a);
        exit(EXIT_FAILURE);
    }
    rank1_free(r1);
    rank1b_free(r1b);
    bitarray_free(B);
    free(R1);
}

static void run_benchmark_rank1_vs_lut(void){

    printf("Reporting average rdts time\n");
    u64 n = 512*1;
    printf("| N   | T_rank1   | T_rank1b | T_array |\n");
    printf("| --: |       --: |     --:  |     --: |\n");
    while(n < 3e9){
        benchmark_rank1_vs_lut(n);
        n*=2;
    }
}

static void benchmark_select1_vs_lut(const u64 n){
    //printf("benchmark_select1_vs_lut\n");
    //printf("n = %.1f M (%lu)\n", (double) n/ 1000000.0, n);
    bitarray * B = bitarray_new(n);
    u64 * S1 = malloc(n*sizeof(u64));
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
    u64 n_sample = 1e6;
    for(u64 ii = 0; ii < n_sample; ii++)
    {
        idx = 1 + (u64) rand() % (n_ones-1);
        if( rand() % 2 == 0){
            t0 = __rdtscp(&cpuid);
            s1_array += S1[idx];
            t1 = __rdtscp(&cpuid);
            t_array += t1-t0;
            //idx = rand() % (n-1);
            t0 = __rdtscp(&cpuid);
            s1_s1 += select1_get(s1, idx);
            t1 = __rdtscp(&cpuid);
            t_s1 += t1-t0;
        } else {
            t0 = __rdtscp(&cpuid);
            s1_s1 += select1_get(s1, idx);
            t1 = __rdtscp(&cpuid);
            t_s1 += t1-t0;

            t0 = __rdtscp(&cpuid);
            s1_array += S1[idx];
            t1 = __rdtscp(&cpuid);
            t_array += t1-t0;
        }
    }
    if(0){
        printf("t s1  = %lu (%lu)\n", t_s1,     s1_s1);
        printf("t a   = %lu (%lu)\n", t_array,  s1_array);
    }
    if(s1_s1 != s1_array){
        printf("%s:%d Error results differ\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    printf("| %'lu | %'.0f | %'.0f |\n",
           n,
           (double) t_s1/ (double) n_sample,
           (double) t_array/(double) n_sample);

    select1_free(s1);
    bitarray_free(B);
    free(S1);
}

static void run_benchmark_select1_vs_lut(void){
    printf("Reporting average rdts time\n");
    u64 n = 512*1;
    printf("| N   | T_select1 | T_array |\n");
    printf("| --: |       --: |     --: |\n");
    while(n < 3e10){
        benchmark_select1_vs_lut(n);
        n*=2;
    }
}

void dummy(void){
    u64 test = (u64) rand();
    for(u64 kk = 0; kk < 65; kk++){
        u64 res = (test << kk) | (test >> (64 - kk));
        printf("%lu << %lu = %lu\n", test, kk, res); // ROL
    }

    bitarray * B = bitarray_new(64);
    for(int kk = 0; kk < 64; kk++)
    {
        bitarray_reset(B);
        bitarray_set(B, (u64) kk, 1); // fills from right to left...

        printf("bit %d: %lu (%d)\n", kk, B->data[0],
               __builtin_ctzl((u64) B->data[0]));
    }
    bitarray_free(B);
// Two instructions go get the position of the nth
    // set bit.
    for(u32 kk = 0; kk < 16; kk++)
    {
        u32 where = _pdep_u32(1LU << 0, kk); // requires -mtune=native
        i32 pos = __builtin_ctz(where);
        printf("%b, %b, %d\n", kk, where, pos);
    }

}


typedef struct {
    int verbose;
    int benchmark;
} config;

static void
config_free(config * conf){
    free(conf);
    return;
}

static void usage(void){
    printf("Usage:\n");
    printf("--verbose v\n\tSet verbosity level to v\n");
    printf("--benchmark n\n\t"
           "n=1 benchmark rank1\n\t"
           "n=2 benchmark select1\n");
    printf("--help\n\tShow this help message\n");
    printf("--dummy\n\tUsed for some quick dev tests\n");
    printf("\n");
    printf("If not arguments are given, self-tests are run\n");
    printf("\n");
    return;
}

static config *
config_new(int argc, char ** argv)
{
    struct option longopts[] = {
        { "verbose",    required_argument, NULL, '1' },
        { "benchmark",  required_argument, NULL, '2' },
        { "dummy",      no_argument,       NULL, 'd' },
        { "help",       no_argument,       NULL, 'h' },
        { NULL,           0,               NULL,  0  }
    };
    config * conf = calloc(1, sizeof(config));
    conf->verbose = 1;
    int ch;
    while((ch = getopt_long(argc, argv,
                            "1:2:dh",
                            longopts, NULL)) != -1) {
        switch(ch) {
        case '1':
            conf->verbose = atoi(optarg);
            break;
        case '2':
            conf->benchmark = atoi(optarg);
            break;
        case 'd':
            dummy();
            config_free(conf);
            exit(EXIT_SUCCESS);
        case 'h':
            usage();
            exit(EXIT_SUCCESS);
        default:
            break;
        }
    }
    return conf;
}


int main(int argc, char ** argv)
{
    setlocale(LC_NUMERIC, "");
    config * conf = config_new(argc, argv);
    if(conf->benchmark == 1){
        run_benchmark_rank1_vs_lut();
    }
    if(conf->benchmark == 2){
        run_benchmark_select1_vs_lut();
    }
    if(conf->benchmark == 3){
        run_benchmark_cindex_vs_lut();
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
