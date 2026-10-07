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
#include "select1c.h"
#include "select1c_ut.h"
#include "select1d.h"
#include "select1d_ut.h"

static void benchmark_varray_vs_array(const u64 n, const u64 bits){
    u64 * A = malloc(n*sizeof(u64));
    varray * V = varray_new(n, bits);
    // Set A and V to contain the same things
    const u64 lim = (u64) powl(2, bits);
    for(u64 kk = 0; kk < n; kk++){
        u64 val = (u64) ((u32) rand() % lim);
        A[kk] = val;
        varray_set(V, kk, A[kk]);
    }

    // time for benchmark
    u64 res_array = 0;
    u64 res_varray = 0;
    u64 t_array = 0;
    u64 t_varray = 0;
    u64 t0, t1;
    u64 idx;
    u32 cpuid;
    u64 n_sample = 1e6;
    for(u64 ii = 0; ii < n_sample; ii++)
    {
        idx = (u64) rand() % n;
        if( rand() % 2 == 0){
            t0 = __rdtscp(&cpuid);
            res_array = A[idx];
            t1 = __rdtscp(&cpuid);
            t_array += t1-t0;
            //idx = rand() % (n-1);
            t0 = __rdtscp(&cpuid);
            res_varray = varray_get(V, idx);
            t1 = __rdtscp(&cpuid);
            t_varray += t1-t0;
        } else {
            t0 = __rdtscp(&cpuid);
            res_varray = varray_get(V, idx);
            t1 = __rdtscp(&cpuid);
            t_varray += t1-t0;

            t0 = __rdtscp(&cpuid);
            res_array = A[idx];
            t1 = __rdtscp(&cpuid);
            t_array += t1-t0;
        }

        if(res_varray != res_array){
            printf("%s:%d Error results differ\n", __FILE__, __LINE__);
            printf("varray->%lu vs array->%lu\n", res_varray, res_array);
            printf("%lx vs %lx\n", res_varray, res_array);
            exit(EXIT_FAILURE);
        }
    }

    printf("| %'lu | %'.0f | %'.0f |\n",
           n,
           (double) t_varray / (double) n_sample,
           (double) t_array  / (double) n_sample);

    varray_free(V);
    free(A);
    return;
}


static void run_benchmark_varray_vs_array(void){

    printf("Reporting average rdts time\n");
    u64 n = 512*1;
    u64 bits = 33;
    printf("| N   | T_varray_u%lu  | T_array_u64 |\n", bits);
    printf("| --: |       --: |     --: |\n");
    while(n < 3e9){
        benchmark_varray_vs_array(n, bits);
        n*=2;
    }
}


static void benchmark_cindex_vs_lut(const u64 n){
    u64 * L = malloc(n*sizeof(u64));
    L[0] = 0;
    for(u64 kk = 1; kk < n; kk++){
        L[kk] = L[kk-1] + (u64) rand() % 13;
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
        idx = (u64) rand() % n;
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


    double mem_quota = (double) C->mem_allocated / (double) (n*sizeof(u64));
    printf("| %'lu | %'.0f | %'.0f | %.2f |\n",
           n,
           (double) t_cindex/ (double) n_sample,
           (double) t_array/(double) n_sample,
           mem_quota);

    cindex_free(C);
    free(L);
}

// benchmark 3
static void run_benchmark_cindex_vs_lut(void){

    printf("Reporting average rdts time\n");
    u64 n = 512*1;
    printf("| N   | T_cindex  | T_array | cindex_mem_Q | \n");
    printf("| --: |       --: |     --: |          --: |\n");
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
        if(1){//} (rand() % 5) == 0){
            bitarray_set(B, kk, 1);
            S1[nset++] = (u32) kk;
        }
    }

    select1 * s1 = select1_new(B);
    select1c * s1c = select1c_new(B);
    select1c * s1d = select1d_new(B);

    const u64 n_ones = s1->n_ones;
    // time for benchmark


    u64 t_array = 0;
    u64 t_s1 = 0;
    u64 t_s1c = 0;
    u64 t_s1d = 0;

    u64 t0, t1;
    u64 idx;
    u32 cpuid;
    u64 n_sample = 1e6;
    for(u64 ii = 0; ii < n_sample; ii++)
    {
        u64 s1_array = 0;
        u64 s1_s1 = 0;
        u64 s1_s1c = 0;
        u64 s1_s1d = 0;

        idx = 1 + (u64) rand() % (n_ones-1);

        t0 = __rdtscp(&cpuid);
        s1_array = S1[idx-1];
        t1 = __rdtscp(&cpuid);
        t_array += t1-t0;
        //idx = rand() % (n-1);
        t0 = __rdtscp(&cpuid);
        s1_s1 = select1_get(s1, idx);
        t1 = __rdtscp(&cpuid);
        t_s1 += t1-t0;

        t0 = __rdtscp(&cpuid);
        s1_s1c = select1c_get(s1c, idx-1);
        t1 = __rdtscp(&cpuid);
        t_s1c += t1-t0;

        t0 = __rdtscp(&cpuid);
        s1_s1d = select1c_get(s1c, idx-1);
        t1 = __rdtscp(&cpuid);
        t_s1d += t1-t0;


        if(0){
            printf("t s1  = %lu (%lu)\n", t_s1,     s1_s1);
            printf("t a   = %lu (%lu)\n", t_array,  s1_array);
        }
        if((s1_s1 != s1_array) | (s1_s1c != s1_array)){
            printf("%s:%d Error results differ\n", __FILE__, __LINE__);
            printf("idx=%lu, ref: %lu, s1: %lu, s1c: %lu\n",
                   idx, s1_array, s1_s1, s1_s1c);
            exit(EXIT_FAILURE);
        }
    }
    printf("| %'lu | %'.0f | %'.0f | %'.0f | %'.0f |\n",
           n,
           (double) t_s1/ (double) n_sample,
           (double) t_s1c/ (double) n_sample,
           (double) t_s1d/ (double) n_sample,
           (double) t_array/(double) n_sample);

    select1_free(s1);
    bitarray_free(B);
    free(S1);
}

static void run_benchmark_select1_vs_lut(void){
    printf("Reporting average rdts time\n");
    u64 n = 512*1;
    printf("| N   | T_select1 | T_select1c | T_select1d   | T_array |\n");
    printf("| --: |       --: |        --: |          --: |     --: |\n");
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
    printf("Usage: bits [ARGS]\n\n");
    printf("Possible arguments:\n");
    printf("--verbose v\n\tSet verbosity level to v\n");
    printf("--benchmark n\n\t"
           "n=1 benchmark rank1\n\t"
           "n=2 benchmark select1\n\t"
           "n=3 benchmark cindex\n\t"
           "n=4 benchmark varray\n");

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
    switch(conf->benchmark){
    case 0:
        break;
    case 1:
        run_benchmark_rank1_vs_lut();
        break;
    case 2:
        run_benchmark_select1_vs_lut();
        break;
    case 3:
        run_benchmark_cindex_vs_lut();
        break;
    case 4:
        run_benchmark_varray_vs_array();
        break;
    default:
        printf("No benchmark with id %d\n", conf->benchmark);
        config_free(conf);
        return EXIT_FAILURE;
    }

    srand((u32) time(NULL));
#ifdef NDEBUG
    printf("Compiled with -DNDEBUG, not all tests are enabled\n");
#endif
    bitarray_ut(conf->verbose); // bitarray_ut.c
    varray_ut(conf->verbose);
    rank1_ut(conf->verbose);
    select1_ut(conf->verbose);
    select1c_ut(conf->verbose);
    select1d_ut(conf->verbose);
    cindex_ut(conf->verbose);
    config_free(conf);
    printf("All tests passed successfully. Tests not tested :)\n");
    return EXIT_SUCCESS;
}
