#define _GNU_SOURCE
#include <assert.h>
#include <getopt.h>
#include <locale.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <x86intrin.h>
#include <sched.h>
#include <unistd.h>
#include <err.h>
#include <pthread.h>

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
#include "elias_codes.h"
#include "wtree.h"
#include "wtree_ut.h"

#include "bitpos.h"

static double
clockdiff(struct timespec* start, struct timespec * finish)
{
    double elapsed = ((double) finish->tv_sec - (double) start->tv_sec);
    elapsed += ((double) finish->tv_nsec - (double) start->tv_nsec) / 1000000000.0;
    return elapsed;
}


typedef void (*ut_fun)(int);
typedef struct {
    ut_fun fun;
    const char * name;
} unit_test;
typedef void (*benchmark_fn)(void);
typedef struct {
    benchmark_fn fun;
    const char * name;
} benchmark;

// forward declarations
static void run_benchmark_rank1_vs_lut(void);
static void run_benchmark_select1_vs_lut(void);
static void run_benchmark_cindex_vs_lut(void);
static void run_benchmark_varray_vs_array(void);
static void run_benchmark_bitarray_vs_u8(void);
static void run_benchmark_elias_codes(void);
static void run_benchmark_linear_select1(void);
static void run_benchmark_wtree2_construct(void);

benchmark benchmarks[] = {
    {run_benchmark_rank1_vs_lut, "rank1"},
    {run_benchmark_select1_vs_lut, "select1"},
    {run_benchmark_cindex_vs_lut, "cindex"},
    {run_benchmark_varray_vs_array, "varray"},
    {run_benchmark_bitarray_vs_u8, "bitarray"},
    {run_benchmark_elias_codes, "Elias codes"},
    {run_benchmark_linear_select1, "Linear select1"},
    {run_benchmark_wtree2_construct, "wtree2 construct"},
    {NULL, NULL}
};

unit_test unit_tests[] = {
    {bitarray_ut, "bitarray"},
    {varray_ut, "varray"},
    {elias_codes_ut, "Elias Codes, gamma and delta"},
    {rank1_ut, "rank1"},
    {select1_ut, "select1"},
    {select1c_ut, "select1c"},
    {select1d_ut, "select1d"},
    {cindex_ut, "cindex (Compact index, i.e. Elias Fano)"},
    {wtree_ut, "wtree"},
    {NULL, NULL}
};

u64 n_bm = 0;
u64 n_ut = 0;

// pin the program to a specific cpu. 0-based id
static void pin_to_cpu(size_t cpu_id){
    cpu_set_t * set = CPU_ALLOC(cpu_id+1);
    assert(set != NULL);
    CPU_ZERO_S(CPU_ALLOC_SIZE(cpu_id+1), set);
    CPU_SET(cpu_id, set);
    if (sched_setaffinity(getpid(), sizeof(&set), set) == -1){
        fprintf(stderr, "sched_setaffinity failed\n");
        exit(EXIT_FAILURE);
    }
    CPU_FREE(set);
    if( sched_yield() ){ // switch if not on that cpu
        fprintf(stderr, "sched_yield failed\n");
        exit(EXIT_FAILURE);
    }
}


static void run_benchmark_wtree2_construct(void){
    u64 n_nt = 2048;
    printf("Time to access random elements\n");
    printf("|    N | u8 array | wtree2 |\n");
    printf("| -- : |      --: |    --: |\n");

    while(n_nt < 4294967296)
    {
        if(0){
            printf("wtree2_construct, n = %lu (%lu M)\n", n_nt, n_nt / 1000000LU);
        }
        struct timespec t0, t1;
        clock_gettime(CLOCK_REALTIME, &t0);
        u64 * data = malloc(n_nt/32*sizeof(u64));
        for(u64 kk = 0; kk < n_nt/32; kk++){
            data[kk] = (u64) rand() + (((u64) rand()) << 32);
        }
        clock_gettime(CLOCK_REALTIME, &t1);
        if(0){
            printf("Construct random input: %.3f s\n", clockdiff(&t0, &t1));
        }
        clock_gettime(CLOCK_REALTIME, &t0);
        wtree2 * W = wtree2_new(data, n_nt);
        if(0){
            printf("wtree memory usage: %lu byte\n", W->mem_allocated);
            printf("Construct wtree2  %.3f s\n", clockdiff(&t0, &t1));
        }
        free(data);
        clock_gettime(CLOCK_REALTIME, &t1);


        u32 cpuid;
        u64 c0, c1;
        u64 t_wtree2 = 0;
        u64 t_array8 = 0;
        // Read random nucleotides

        u64 res = 0;
        u64 n_sample = 1000000;
        srand(0);
        for(u64 kk = 0; kk < n_sample; kk++)
        {
            u64 idx = (u64) rand() % n_nt;
            c0 = __rdtscp(&cpuid);
            res += wtree2_access(W, idx);
            c1 = __rdtscp(&cpuid);
            t_wtree2 += (c1-c0);
        }

        if(0){
            printf("\rTo access random elements\n");
            printf("wtree2: %.1f ticks in average\n", (double) t_wtree2 / (double) n_sample);
        }

        u8 * data8 = malloc(n_nt*sizeof(u8));
        for(u64 kk = 0; kk < n_nt; kk++){
            data8[kk] = (u8) (rand() % 4);
        }
        srand(0);
        for(u64 kk = 0; kk < n_sample; kk++)
        {
            u64 idx = (u64) rand() % n_nt;
            c0 = __rdtscp(&cpuid);
            res += data8[idx];
            c1 = __rdtscp(&cpuid);
            t_array8 += (c1-c0);
        }
        if(0){

            printf("\r8-array: %.1f ticks in average\n", (double) t_array8 / (double) n_sample);
        }
        free(data8);
        wtree2_free(W);
        n_nt *= 2;
        printf("%lu", res);
        printf("\r| %lu | %.1f | %.1f |\n", n_nt,
               (double) t_array8 / (double) n_sample,
               (double) t_wtree2 / (double) n_sample);
    }
    return;
}
static void benchmark_bitarray(const u64 n)
{
    u8 * A = calloc(n, sizeof(u8));
    bitarray * B = bitarray_new(n);
    for(u64 kk = 0; kk < n; kk++){
        if(rand()%2){
            A[kk] = 1;
            bitarray_set(B, kk, 1);
        }
    }
    // time for benchmark
    u64 res_array = 0;
    u64 res_bit = 0;
    u64 t_array = 0;
    u64 t_bit = 0;
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
            res_bit = bitarray_get(B, idx);
            t1 = __rdtscp(&cpuid);
            t_bit += t1-t0;
        } else {
            t0 = __rdtscp(&cpuid);
            res_bit = bitarray_get(B, idx);
            t1 = __rdtscp(&cpuid);
            t_bit += t1-t0;

            t0 = __rdtscp(&cpuid);
            res_array = A[idx];
            t1 = __rdtscp(&cpuid);
            t_array += t1-t0;
        }

        if(res_bit != res_array){
            printf("%s:%d Error results differ\n", __FILE__, __LINE__);
            printf("bitarray->%lu vs array->%lu\n", res_bit, res_array);
            exit(EXIT_FAILURE);
        }
    }

    printf("| %'lu | %'.0f | %'.0f |\n",
           n,
           (double) t_bit    / (double) n_sample,
           (double) t_array  / (double) n_sample);

    bitarray_free(B);
    free(A);
    return;
}

static void run_benchmark_bitarray_vs_u8(void){

    printf("Reporting average rdts time\n");
    u64 n = 128;
    printf("| N   | T_bitarray  |    T_u8 |\n");
    printf("| --: |         --: |     --: |\n");
    while(n < (u64) 3e9){
        benchmark_bitarray(n);
        n*=2;
    }
}



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
    while(n < (u64) 3e9){
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
    while(n < (u64) 3e9){
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
    while(n < (u64) 3e9){
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
    select1d * s1d = select1d_new(B);

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
        s1_s1d = select1d_get(s1d, idx-1);
        t1 = __rdtscp(&cpuid);
        t_s1d += t1-t0;


        if(0){
            printf("t s1  = %lu (%lu)\n", t_s1,     s1_s1);
            printf("t a   = %lu (%lu)\n", t_array,  s1_array);
        }
        if(0){
            if(s1_s1d != s1_array){
                assert(0);
            }
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
    while(n < (u64) 3e10){
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
    u64 benchmark;
    u64 test;
} config;

static void
config_free(config * conf){
    free(conf);
    return;
}

static void
usage(void){
    printf("Usage: bits [ARGS]\n\n");
    printf("Possible arguments:\n");
    printf("--test n\n");
    printf("\tWhere n can be\n");
    for(u64 kk = 0; kk < n_ut; kk++){
        printf("\t%lu %s\n", kk+1, unit_tests[kk].name);
    }
    printf("--benchmark n\n");
    printf("\tWhere n can be\n");
    for(u64 kk = 0; kk < n_bm; kk++){
        printf("\t%lu %s\n", kk+1, benchmarks[kk].name);
    }
    printf("--verbose v\n\tSet verbosity level to v\n");
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
        { "test",       required_argument, NULL, 't'},
        { NULL,           0,               NULL,  0  }
    };
    config * conf = calloc(1, sizeof(config));
    conf->verbose = 1;
    int ch;
    while((ch = getopt_long(argc, argv,
                            "1:2:dht:",
                            longopts, NULL)) != -1) {
        switch(ch) {
        case '1':
            conf->verbose = atoi(optarg);
            break;
        case '2':
            conf->benchmark = (u64) atol(optarg);
            break;
        case 'd':
            dummy();
            config_free(conf);
            exit(EXIT_SUCCESS);
        case 'h':
            usage();
            exit(EXIT_SUCCESS);
        case 't':
            conf->test = (u64) atol(optarg);
            break;
        default:
            break;
        }
    }
    return conf;
}

void bench_elias_gamma_array(u64 n){
    u64 t0, t1;
    u32 cpuid;
    printf("Elias gamma array, encoding 1, ... %lu\n", n);
    u64 * numbers = calloc(n, sizeof(u64));

    u64 req_bits = 0;
    for(u64 kk = 1; kk <= n; kk++)
    {
        req_bits += egamma_bits(kk);
    }
    u64 ne = req_bits/64 + 1;
    u64 * encoded = calloc(ne, sizeof(u64));

    for(u64 kk = 1; kk <= n; kk++)
    {
        numbers[kk-1] = kk;
    }

    t0 = __rdtscp(&cpuid);
    u64 nenc = egamma_encode_array(numbers, n, encoded, ne);
    t1 = __rdtscp(&cpuid);

    printf("   Encoded %lu numbers using %lu bits (as u64 %lu bits)\n",
           nenc, ne*8*sizeof(u64), nenc*sizeof(u64)*8);
    printf("   Took %lu, or %.1f per element\n", t1-t0,
           (double) (t1-t0) / (double) nenc);

    t0 = __rdtscp(&cpuid);
    nenc = egamma_encode_array(numbers, n, encoded, ne);
    t1 = __rdtscp(&cpuid);
    printf("   Decoded %lu numbers\n",
           nenc);
    printf("   Took %lu, or %.1f per element\n", t1-t0,
           (double) (t1-t0) / (double) nenc);

    free(encoded);
    free(numbers);
    return;
}

void bench_elias_delta_array(u64 n){
    u64 t0, t1;
    u32 cpuid;
    printf("Elias delta array, encoding 1, ... %lu\n", n);
    u64 * numbers = calloc(n, sizeof(u64));

    u64 req_bits = 0;
    for(u64 kk = 1; kk <= n; kk++)
    {
        req_bits += edelta_bits(kk);
    }
    u64 ne = req_bits/64 + 1;
    u64 * encoded = calloc(ne, sizeof(u64));

    for(u64 kk = 1; kk <= n; kk++)
    {
        numbers[kk-1] = kk;
    }

    t0 = __rdtscp(&cpuid);
    u64 nenc = edelta_encode_array(numbers, n, encoded, ne);
    t1 = __rdtscp(&cpuid);

    printf("   Encoded %lu numbers using %lu bits (as u64 %lu bits)\n",
           nenc, ne*8*sizeof(u64), nenc*sizeof(u64)*8);
    printf("   Took %lu, or %.1f per element\n", t1-t0,
           (double) (t1-t0) / (double) nenc);

    t0 = __rdtscp(&cpuid);
    nenc = edelta_encode_array(numbers, n, encoded, ne);
    t1 = __rdtscp(&cpuid);
    printf("   Decoded %lu numbers\n",
           nenc);
    printf("   Took %lu, or %.1f per element\n", t1-t0,
           (double) (t1-t0) / (double) nenc);

    free(encoded);
    free(numbers);
    return;
}


static void run_benchmark_elias_codes(){
    u32 cpuid;
    u64 n = 1000000;
    u64 dummy = 0;
    u64 t0, t1;

    printf("Elias gamma, encode 1, ..., %lu\n", n);
    dummy = 0;
    t0 = __rdtscp(&cpuid);
    for(u64 kk = 1; kk <= n; kk++){
        dummy += egamma_encode(kk);
    }
    t1 = __rdtscp(&cpuid);
    printf("%lu\r", dummy);
    printf("   Took %lu, or %.1f per element\n", t1-t0,
           (double) (t1-t0) / (double) n);

    printf("Elias gamma, encode and decode 1, ..., %lu\n", n);
    dummy = 0;
    t0 = __rdtscp(&cpuid);
    for(u64 kk = 1; kk <= n; kk++){
        dummy += egamma_decode(egamma_encode(kk));
    }
    t1 = __rdtscp(&cpuid);
    printf("%lu\r", dummy);
    printf("   Took %lu, or %.1f per element\n", t1-t0,
           (double) (t1-t0) / (double) n);


    printf("Elias delta, encode 1, ..., %lu\n", n);
    dummy = 0;
    t0 = __rdtscp(&cpuid);
    for(u64 kk = 1; kk <= n; kk++){
        dummy += edelta_encode(kk);
    }
    t1 = __rdtscp(&cpuid);
    printf("%lu\r", dummy);
    printf("   Took %lu, or %.1f per element\n", t1-t0,
           (double) (t1-t0) / (double) n);

    printf("Elias delta, encode and decode 1, ..., %lu\n", n);
    dummy = 0;
    t0 = __rdtscp(&cpuid);
    for(u64 kk = 1; kk <= n; kk++){
        dummy += edelta_decode(edelta_encode(kk));
    }
    t1 = __rdtscp(&cpuid);
    printf("%lu\r", dummy);
    printf("   Took %lu, or %.1f per element\n", t1-t0,
           (double) (t1-t0) / (double) n);

    bench_elias_gamma_array(n);
    bench_elias_delta_array(n);

    return;
}

static u64
select1_raw(const u64 * BA,
            u64 n)
{
    u64 i = 0;
    while(n > (u64) __builtin_popcountl(BA[i])){
        n -= (u64) __builtin_popcountl(BA[i++]);
    }

    return i*64 + bitpos(BA[i], n-1);
}

static u64 select1_word(u64 word, u64 i){

    return (u64) __builtin_ctzl(_pdep_u64(1LU << i, word));
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

void run_benchmark_linear_select1(void){

    bitarray * B1 = bitarray_new(64);
    bitarray_reset1(B1);
    for(u64 kk = 0; kk < 64; kk++){
        assert(bitpos(B1->data[0], kk) == select1_word(B1->data[0], kk));
    }
    for(u64 kk = 0; kk < 100000; kk++){
        u64 w = (u64) rand() + 1;
        w = w << 32;
        w += (u64) rand() + 1;
        u64 what = 0;
        if(bitpos(w, what) != select1_word(w, what)){
            print_binaryl("w", w); // printf("0x%016lx\n", w);
            printf("%lu vs %lu\n", bitpos(w, what), select1_word(w, what));
            exit(EXIT_FAILURE);
        }
    }
    bitarray_free(B1);


    u64 n = 100000000;
    bitarray * B = bitarray_new(n+64);
    bitarray_reset1(B); // set all to 1
    u32 cpuid;
    u64 t0, t1;
    u64 dummy = 0;
    volatile u64 spos = select1_raw(B->data, 1); // put the function in cache
    printf("| bit set | T_bitpos |\n");
    printf("| --:             | --:         |\n");
    for(u64 pos = 1; pos < n; pos*=2){
        t0 = __rdtscp(&cpuid);
        spos = select1_raw(B->data, pos);
        t1 = __rdtscp(&cpuid);
        assert(spos+1 == pos);
        //printf("ref:%lu ans:%lu\n", pos, spos);
        printf("| %lu  | %lu |\n", pos, t1 - t0);
        dummy += spos;
    }
    printf("%lu\r              \n", dummy);
    bitarray_free(B);
    return;
}



int main(int argc, char ** argv)
{
    pin_to_cpu(0);
    setlocale(LC_NUMERIC, "");

    while(unit_tests[n_ut].fun != NULL){n_ut++;};
    while(benchmarks[n_bm].fun != NULL){n_bm++;};

    config * conf = config_new(argc, argv);
    if(conf->benchmark > 0){
        if(conf->benchmark > n_bm){
            printf("There are only %lu benchmarks\n", n_bm);
            for(u64 kk = 0; kk < n_bm; kk++){
                printf("%lu %s\n", kk+1, benchmarks[kk].name);
            }
            exit(EXIT_FAILURE);
        }
        benchmarks[conf->benchmark-1].fun();
        free(conf);
        return EXIT_SUCCESS;
    }

    srand((u32) time(NULL));
#ifdef NDEBUG
    printf("Compiled with -DNDEBUG, not all tests are enabled\n");
#endif

    if(conf->test == 0){
        for(u64 kk = 0; kk < n_ut; kk++){
            printf("%lu/%lu\n", kk+1, n_ut);
            unit_tests[kk].fun(conf->verbose);
        }
        printf("All tests passed successfully, however, the tests are not tested.\n");
    } else {
        if(conf->test > n_ut){
            printf("There are only %lu unit tests\n", n_ut);
            for(u64 kk = 0; kk < n_ut; kk++){
                printf("%lu %s\n", kk+1, unit_tests[kk].name);
            }
            exit(EXIT_FAILURE);
        }
        unit_tests[conf->test-1].fun(conf->verbose);
    }

    config_free(conf);

    return EXIT_SUCCESS;
}
