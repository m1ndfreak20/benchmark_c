/*
 * benchmark_dict_generic.c - Benchmarks for generic dict.h
 * 
 * Tests all dictionary types: string, int, uint32, uint64, pointer keys
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include "../include/dict.h"

#define ITERATIONS 100000
#define WARMUP 10000

// ============================================================================
// Define all dictionary types for benchmarking
// ============================================================================

DICT_DEFINE_STR_INT(StrInt)
DICT_DEFINE_STR_DOUBLE(StrDouble)
DICT_DEFINE_STR_PTR(StrPtr)
DICT_DEFINE_INT_INT(IntInt)
DICT_DEFINE_INT_DOUBLE(IntDouble)
DICT_DEFINE_INT_PTR(IntPtr)
DICT_DEFINE_UINT32_INT(U32Int)
DICT_DEFINE_UINT64_INT(U64Int)
DICT_DEFINE_PTR_INT(PtrInt)

// ============================================================================
// Timing
// ============================================================================

static inline uint64_t get_nanos(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

// ============================================================================
// Benchmark: Dict<string, int>
// ============================================================================

void bench_str_int(void) {
    fprintf(stderr, "### Dict<string, int>\n\n");
    
    char **keys = malloc(ITERATIONS * sizeof(char*));
    for (int i = 0; i < ITERATIONS; i++) {
        keys[i] = malloc(32);
        snprintf(keys[i], 32, "key_%d", i);
    }
    
    StrInt *dict = StrInt_create_with_capacity(ITERATIONS * 2);
    
    // Warmup
    for (int i = 0; i < WARMUP; i++) {
        StrInt_set(dict, keys[i], i);
    }
    StrInt_clear(dict);
    
    // Insert
    uint64_t start = get_nanos();
    for (int i = 0; i < ITERATIONS; i++) {
        StrInt_set(dict, keys[i], i);
    }
    uint64_t end = get_nanos();
    double insert_ns = (double)(end - start) / ITERATIONS;
    
    // Get (hit)
    start = get_nanos();
    volatile int sum = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        sum += StrInt_get(dict, keys[i], 0);
    }
    end = get_nanos();
    double get_hit_ns = (double)(end - start) / ITERATIONS;
    
    // Contains (hit)
    start = get_nanos();
    volatile int found = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        if (StrInt_contains(dict, keys[i])) found++;
    }
    end = get_nanos();
    double contains_hit_ns = (double)(end - start) / ITERATIONS;
    
    // Contains (miss)
    char miss_key[32];
    start = get_nanos();
    found = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        snprintf(miss_key, 32, "miss_%d", i);
        if (StrInt_contains(dict, miss_key)) found++;
    }
    end = get_nanos();
    double contains_miss_ns = (double)(end - start) / ITERATIONS;
    
    // Remove
    start = get_nanos();
    for (int i = 0; i < ITERATIONS; i++) {
        StrInt_remove(dict, keys[i]);
    }
    end = get_nanos();
    double remove_ns = (double)(end - start) / ITERATIONS;
    
    fprintf(stderr, "| Operation | Time (ns) |\n");
    fprintf(stderr, "|-----------|----------:|\n");
    fprintf(stderr, "| Insert | %.2f |\n", insert_ns);
    fprintf(stderr, "| Get (hit) | %.2f |\n", get_hit_ns);
    fprintf(stderr, "| Contains (hit) | %.2f |\n", contains_hit_ns);
    fprintf(stderr, "| Contains (miss) | %.2f |\n", contains_miss_ns);
    fprintf(stderr, "| Remove | %.2f |\n", remove_ns);
    fprintf(stderr, "\n");
    
    StrInt_destroy(dict);
    for (int i = 0; i < ITERATIONS; i++) free(keys[i]);
    free(keys);
}

// ============================================================================
// Benchmark: Dict<string, double>
// ============================================================================

void bench_str_double(void) {
    fprintf(stderr, "### Dict<string, double>\n\n");
    
    char **keys = malloc(ITERATIONS * sizeof(char*));
    for (int i = 0; i < ITERATIONS; i++) {
        keys[i] = malloc(32);
        snprintf(keys[i], 32, "key_%d", i);
    }
    
    StrDouble *dict = StrDouble_create_with_capacity(ITERATIONS * 2);
    
    // Warmup
    for (int i = 0; i < WARMUP; i++) {
        StrDouble_set(dict, keys[i], (double)i * 1.5);
    }
    StrDouble_clear(dict);
    
    // Insert
    uint64_t start = get_nanos();
    for (int i = 0; i < ITERATIONS; i++) {
        StrDouble_set(dict, keys[i], (double)i * 1.5);
    }
    uint64_t end = get_nanos();
    double insert_ns = (double)(end - start) / ITERATIONS;
    
    // Get (hit)
    start = get_nanos();
    volatile double sum = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        sum += StrDouble_get(dict, keys[i], 0.0);
    }
    end = get_nanos();
    double get_hit_ns = (double)(end - start) / ITERATIONS;
    
    // Contains (hit)
    start = get_nanos();
    volatile int found = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        if (StrDouble_contains(dict, keys[i])) found++;
    }
    end = get_nanos();
    double contains_hit_ns = (double)(end - start) / ITERATIONS;
    
    fprintf(stderr, "| Operation | Time (ns) |\n");
    fprintf(stderr, "|-----------|----------:|\n");
    fprintf(stderr, "| Insert | %.2f |\n", insert_ns);
    fprintf(stderr, "| Get (hit) | %.2f |\n", get_hit_ns);
    fprintf(stderr, "| Contains (hit) | %.2f |\n", contains_hit_ns);
    fprintf(stderr, "\n");
    
    StrDouble_destroy(dict);
    for (int i = 0; i < ITERATIONS; i++) free(keys[i]);
    free(keys);
}

// ============================================================================
// Benchmark: Dict<int, int>
// ============================================================================

void bench_int_int(void) {
    fprintf(stderr, "### Dict<int, int>\n\n");
    
    IntInt *dict = IntInt_create_with_capacity(ITERATIONS * 2);
    
    // Warmup
    for (int i = 0; i < WARMUP; i++) {
        IntInt_set(dict, i, i * i);
    }
    IntInt_clear(dict);
    
    // Insert
    uint64_t start = get_nanos();
    for (int i = 0; i < ITERATIONS; i++) {
        IntInt_set(dict, i, i * i);
    }
    uint64_t end = get_nanos();
    double insert_ns = (double)(end - start) / ITERATIONS;
    
    // Get (hit)
    start = get_nanos();
    volatile int sum = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        sum += IntInt_get(dict, i, 0);
    }
    end = get_nanos();
    double get_hit_ns = (double)(end - start) / ITERATIONS;
    
    // Contains (hit)
    start = get_nanos();
    volatile int found = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        if (IntInt_contains(dict, i)) found++;
    }
    end = get_nanos();
    double contains_hit_ns = (double)(end - start) / ITERATIONS;
    
    // Contains (miss)
    start = get_nanos();
    found = 0;
    for (int i = ITERATIONS; i < ITERATIONS * 2; i++) {
        if (IntInt_contains(dict, i)) found++;
    }
    end = get_nanos();
    double contains_miss_ns = (double)(end - start) / ITERATIONS;
    
    // Remove
    start = get_nanos();
    for (int i = 0; i < ITERATIONS; i++) {
        IntInt_remove(dict, i);
    }
    end = get_nanos();
    double remove_ns = (double)(end - start) / ITERATIONS;
    
    fprintf(stderr, "| Operation | Time (ns) |\n");
    fprintf(stderr, "|-----------|----------:|\n");
    fprintf(stderr, "| Insert | %.2f |\n", insert_ns);
    fprintf(stderr, "| Get (hit) | %.2f |\n", get_hit_ns);
    fprintf(stderr, "| Contains (hit) | %.2f |\n", contains_hit_ns);
    fprintf(stderr, "| Contains (miss) | %.2f |\n", contains_miss_ns);
    fprintf(stderr, "| Remove | %.2f |\n", remove_ns);
    fprintf(stderr, "\n");
    
    IntInt_destroy(dict);
}

// ============================================================================
// Benchmark: Dict<int, double>
// ============================================================================

void bench_int_double(void) {
    fprintf(stderr, "### Dict<int, double>\n\n");
    
    IntDouble *dict = IntDouble_create_with_capacity(ITERATIONS * 2);
    
    // Warmup
    for (int i = 0; i < WARMUP; i++) {
        IntDouble_set(dict, i, (double)i * 3.14);
    }
    IntDouble_clear(dict);
    
    // Insert
    uint64_t start = get_nanos();
    for (int i = 0; i < ITERATIONS; i++) {
        IntDouble_set(dict, i, (double)i * 3.14);
    }
    uint64_t end = get_nanos();
    double insert_ns = (double)(end - start) / ITERATIONS;
    
    // Get (hit)
    start = get_nanos();
    volatile double sum = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        sum += IntDouble_get(dict, i, 0.0);
    }
    end = get_nanos();
    double get_hit_ns = (double)(end - start) / ITERATIONS;
    
    // Contains (hit)
    start = get_nanos();
    volatile int found = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        if (IntDouble_contains(dict, i)) found++;
    }
    end = get_nanos();
    double contains_hit_ns = (double)(end - start) / ITERATIONS;
    
    fprintf(stderr, "| Operation | Time (ns) |\n");
    fprintf(stderr, "|-----------|----------:|\n");
    fprintf(stderr, "| Insert | %.2f |\n", insert_ns);
    fprintf(stderr, "| Get (hit) | %.2f |\n", get_hit_ns);
    fprintf(stderr, "| Contains (hit) | %.2f |\n", contains_hit_ns);
    fprintf(stderr, "\n");
    
    IntDouble_destroy(dict);
}

// ============================================================================
// Benchmark: Dict<uint32_t, int>
// ============================================================================

void bench_u32_int(void) {
    fprintf(stderr, "### Dict<uint32_t, int>\n\n");
    
    U32Int *dict = U32Int_create_with_capacity(ITERATIONS * 2);
    
    // Warmup
    for (uint32_t i = 0; i < WARMUP; i++) {
        U32Int_set(dict, i * 7919, (int)i);  // Use prime multiplier for spread
    }
    U32Int_clear(dict);
    
    // Insert
    uint64_t start = get_nanos();
    for (uint32_t i = 0; i < ITERATIONS; i++) {
        U32Int_set(dict, i * 7919, (int)i);
    }
    uint64_t end = get_nanos();
    double insert_ns = (double)(end - start) / ITERATIONS;
    
    // Get (hit)
    start = get_nanos();
    volatile int sum = 0;
    for (uint32_t i = 0; i < ITERATIONS; i++) {
        sum += U32Int_get(dict, i * 7919, 0);
    }
    end = get_nanos();
    double get_hit_ns = (double)(end - start) / ITERATIONS;
    
    // Contains (hit)
    start = get_nanos();
    volatile int found = 0;
    for (uint32_t i = 0; i < ITERATIONS; i++) {
        if (U32Int_contains(dict, i * 7919)) found++;
    }
    end = get_nanos();
    double contains_hit_ns = (double)(end - start) / ITERATIONS;
    
    // Contains (miss)
    start = get_nanos();
    found = 0;
    for (uint32_t i = 0; i < ITERATIONS; i++) {
        if (U32Int_contains(dict, i * 7919 + 1)) found++;
    }
    end = get_nanos();
    double contains_miss_ns = (double)(end - start) / ITERATIONS;
    
    fprintf(stderr, "| Operation | Time (ns) |\n");
    fprintf(stderr, "|-----------|----------:|\n");
    fprintf(stderr, "| Insert | %.2f |\n", insert_ns);
    fprintf(stderr, "| Get (hit) | %.2f |\n", get_hit_ns);
    fprintf(stderr, "| Contains (hit) | %.2f |\n", contains_hit_ns);
    fprintf(stderr, "| Contains (miss) | %.2f |\n", contains_miss_ns);
    fprintf(stderr, "\n");
    
    U32Int_destroy(dict);
}

// ============================================================================
// Benchmark: Dict<uint64_t, int>
// ============================================================================

void bench_u64_int(void) {
    fprintf(stderr, "### Dict<uint64_t, int>\n\n");
    
    U64Int *dict = U64Int_create_with_capacity(ITERATIONS * 2);
    
    // Warmup
    for (uint64_t i = 0; i < WARMUP; i++) {
        U64Int_set(dict, i * 1000000007ULL, (int)i);
    }
    U64Int_clear(dict);
    
    // Insert
    uint64_t start = get_nanos();
    for (uint64_t i = 0; i < ITERATIONS; i++) {
        U64Int_set(dict, i * 1000000007ULL, (int)i);
    }
    uint64_t end = get_nanos();
    double insert_ns = (double)(end - start) / ITERATIONS;
    
    // Get (hit)
    start = get_nanos();
    volatile int sum = 0;
    for (uint64_t i = 0; i < ITERATIONS; i++) {
        sum += U64Int_get(dict, i * 1000000007ULL, 0);
    }
    end = get_nanos();
    double get_hit_ns = (double)(end - start) / ITERATIONS;
    
    // Contains (hit)
    start = get_nanos();
    volatile int found = 0;
    for (uint64_t i = 0; i < ITERATIONS; i++) {
        if (U64Int_contains(dict, i * 1000000007ULL)) found++;
    }
    end = get_nanos();
    double contains_hit_ns = (double)(end - start) / ITERATIONS;
    
    // Contains (miss)
    start = get_nanos();
    found = 0;
    for (uint64_t i = 0; i < ITERATIONS; i++) {
        if (U64Int_contains(dict, i * 1000000007ULL + 1)) found++;
    }
    end = get_nanos();
    double contains_miss_ns = (double)(end - start) / ITERATIONS;
    
    fprintf(stderr, "| Operation | Time (ns) |\n");
    fprintf(stderr, "|-----------|----------:|\n");
    fprintf(stderr, "| Insert | %.2f |\n", insert_ns);
    fprintf(stderr, "| Get (hit) | %.2f |\n", get_hit_ns);
    fprintf(stderr, "| Contains (hit) | %.2f |\n", contains_hit_ns);
    fprintf(stderr, "| Contains (miss) | %.2f |\n", contains_miss_ns);
    fprintf(stderr, "\n");
    
    U64Int_destroy(dict);
}

// ============================================================================
// Benchmark: Dict<void*, int>
// ============================================================================

void bench_ptr_int(void) {
    fprintf(stderr, "### Dict<void*, int>\n\n");
    
    // Create array of fake pointers
    void **ptrs = malloc(ITERATIONS * sizeof(void*));
    for (int i = 0; i < ITERATIONS; i++) {
        ptrs[i] = (void*)(uintptr_t)(0x10000 + i * 64);  // Fake aligned addresses
    }
    
    PtrInt *dict = PtrInt_create_with_capacity(ITERATIONS * 2);
    
    // Warmup
    for (int i = 0; i < WARMUP; i++) {
        PtrInt_set(dict, ptrs[i], i);
    }
    PtrInt_clear(dict);
    
    // Insert
    uint64_t start = get_nanos();
    for (int i = 0; i < ITERATIONS; i++) {
        PtrInt_set(dict, ptrs[i], i);
    }
    uint64_t end = get_nanos();
    double insert_ns = (double)(end - start) / ITERATIONS;
    
    // Get (hit)
    start = get_nanos();
    volatile int sum = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        sum += PtrInt_get(dict, ptrs[i], 0);
    }
    end = get_nanos();
    double get_hit_ns = (double)(end - start) / ITERATIONS;
    
    // Contains (hit)
    start = get_nanos();
    volatile int found = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        if (PtrInt_contains(dict, ptrs[i])) found++;
    }
    end = get_nanos();
    double contains_hit_ns = (double)(end - start) / ITERATIONS;
    
    // Contains (miss)
    start = get_nanos();
    found = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        void *miss = (void*)(uintptr_t)(0x90000000 + i);
        if (PtrInt_contains(dict, miss)) found++;
    }
    end = get_nanos();
    double contains_miss_ns = (double)(end - start) / ITERATIONS;
    
    fprintf(stderr, "| Operation | Time (ns) |\n");
    fprintf(stderr, "|-----------|----------:|\n");
    fprintf(stderr, "| Insert | %.2f |\n", insert_ns);
    fprintf(stderr, "| Get (hit) | %.2f |\n", get_hit_ns);
    fprintf(stderr, "| Contains (hit) | %.2f |\n", contains_hit_ns);
    fprintf(stderr, "| Contains (miss) | %.2f |\n", contains_miss_ns);
    fprintf(stderr, "\n");
    
    PtrInt_destroy(dict);
    free(ptrs);
}

// ============================================================================
// Summary table
// ============================================================================

typedef struct {
    const char *name;
    double insert;
    double get_hit;
    double contains_hit;
    double contains_miss;
} BenchResult;

void run_all_and_summary(void) {
    fprintf(stderr, "# Generic Dict Benchmark Results\n\n");
    fprintf(stderr, "**Iterations:** %d\n", ITERATIONS);
    fprintf(stderr, "**Algorithm:** Robin Hood hashing + DJB2/integer hash\n\n");
    
    // Get CPU info
    FILE *f = fopen("/proc/cpuinfo", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, "model name", 10) == 0) {
                char *colon = strchr(line, ':');
                if (colon) fprintf(stderr, "**CPU:**%s\n", colon + 1);
                break;
            }
        }
        fclose(f);
    }
    
    fprintf(stderr, "---\n\n");
    
    // Run individual benchmarks
    bench_str_int();
    bench_str_double();
    bench_int_int();
    bench_int_double();
    bench_u32_int();
    bench_u64_int();
    bench_ptr_int();
    
    fprintf(stderr, "---\n\n");
    fprintf(stderr, "## Summary Table\n\n");
    fprintf(stderr, "| Type | Insert | Get | Contains (hit) | Contains (miss) |\n");
    fprintf(stderr, "|------|-------:|----:|---------------:|----------------:|\n");
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    run_all_and_summary();
    
    // Run quick benchmarks again for summary table
    BenchResult results[7];
    
    // Str->Int
    {
        char **keys = malloc(ITERATIONS * sizeof(char*));
        for (int i = 0; i < ITERATIONS; i++) {
            keys[i] = malloc(32);
            snprintf(keys[i], 32, "key_%d", i);
        }
        StrInt *d = StrInt_create_with_capacity(ITERATIONS * 2);
        
        uint64_t s = get_nanos();
        for (int i = 0; i < ITERATIONS; i++) StrInt_set(d, keys[i], i);
        results[0].insert = (double)(get_nanos() - s) / ITERATIONS;
        
        s = get_nanos();
        volatile int sum = 0;
        for (int i = 0; i < ITERATIONS; i++) sum += StrInt_get(d, keys[i], 0);
        results[0].get_hit = (double)(get_nanos() - s) / ITERATIONS;
        
        s = get_nanos();
        volatile int f = 0;
        for (int i = 0; i < ITERATIONS; i++) if (StrInt_contains(d, keys[i])) f++;
        results[0].contains_hit = (double)(get_nanos() - s) / ITERATIONS;
        
        char miss[32];
        s = get_nanos();
        f = 0;
        for (int i = 0; i < ITERATIONS; i++) {
            snprintf(miss, 32, "m_%d", i);
            if (StrInt_contains(d, miss)) f++;
        }
        results[0].contains_miss = (double)(get_nanos() - s) / ITERATIONS;
        results[0].name = "string → int";
        
        StrInt_destroy(d);
        for (int i = 0; i < ITERATIONS; i++) free(keys[i]);
        free(keys);
    }
    
    // Str->Double
    {
        char **keys = malloc(ITERATIONS * sizeof(char*));
        for (int i = 0; i < ITERATIONS; i++) {
            keys[i] = malloc(32);
            snprintf(keys[i], 32, "key_%d", i);
        }
        StrDouble *d = StrDouble_create_with_capacity(ITERATIONS * 2);
        
        uint64_t s = get_nanos();
        for (int i = 0; i < ITERATIONS; i++) StrDouble_set(d, keys[i], i * 1.5);
        results[1].insert = (double)(get_nanos() - s) / ITERATIONS;
        
        s = get_nanos();
        volatile double sum = 0;
        for (int i = 0; i < ITERATIONS; i++) sum += StrDouble_get(d, keys[i], 0);
        results[1].get_hit = (double)(get_nanos() - s) / ITERATIONS;
        
        s = get_nanos();
        volatile int f = 0;
        for (int i = 0; i < ITERATIONS; i++) if (StrDouble_contains(d, keys[i])) f++;
        results[1].contains_hit = (double)(get_nanos() - s) / ITERATIONS;
        
        char miss[32];
        s = get_nanos();
        f = 0;
        for (int i = 0; i < ITERATIONS; i++) {
            snprintf(miss, 32, "m_%d", i);
            if (StrDouble_contains(d, miss)) f++;
        }
        results[1].contains_miss = (double)(get_nanos() - s) / ITERATIONS;
        results[1].name = "string → double";
        
        StrDouble_destroy(d);
        for (int i = 0; i < ITERATIONS; i++) free(keys[i]);
        free(keys);
    }
    
    // Int->Int
    {
        IntInt *d = IntInt_create_with_capacity(ITERATIONS * 2);
        
        uint64_t s = get_nanos();
        for (int i = 0; i < ITERATIONS; i++) IntInt_set(d, i, i * i);
        results[2].insert = (double)(get_nanos() - s) / ITERATIONS;
        
        s = get_nanos();
        volatile int sum = 0;
        for (int i = 0; i < ITERATIONS; i++) sum += IntInt_get(d, i, 0);
        results[2].get_hit = (double)(get_nanos() - s) / ITERATIONS;
        
        s = get_nanos();
        volatile int f = 0;
        for (int i = 0; i < ITERATIONS; i++) if (IntInt_contains(d, i)) f++;
        results[2].contains_hit = (double)(get_nanos() - s) / ITERATIONS;
        
        s = get_nanos();
        f = 0;
        for (int i = ITERATIONS; i < ITERATIONS*2; i++) if (IntInt_contains(d, i)) f++;
        results[2].contains_miss = (double)(get_nanos() - s) / ITERATIONS;
        results[2].name = "int → int";
        
        IntInt_destroy(d);
    }
    
    // Int->Double
    {
        IntDouble *d = IntDouble_create_with_capacity(ITERATIONS * 2);
        
        uint64_t s = get_nanos();
        for (int i = 0; i < ITERATIONS; i++) IntDouble_set(d, i, i * 3.14);
        results[3].insert = (double)(get_nanos() - s) / ITERATIONS;
        
        s = get_nanos();
        volatile double sum = 0;
        for (int i = 0; i < ITERATIONS; i++) sum += IntDouble_get(d, i, 0);
        results[3].get_hit = (double)(get_nanos() - s) / ITERATIONS;
        
        s = get_nanos();
        volatile int f = 0;
        for (int i = 0; i < ITERATIONS; i++) if (IntDouble_contains(d, i)) f++;
        results[3].contains_hit = (double)(get_nanos() - s) / ITERATIONS;
        
        s = get_nanos();
        f = 0;
        for (int i = ITERATIONS; i < ITERATIONS*2; i++) if (IntDouble_contains(d, i)) f++;
        results[3].contains_miss = (double)(get_nanos() - s) / ITERATIONS;
        results[3].name = "int → double";
        
        IntDouble_destroy(d);
    }
    
    // U32->Int
    {
        U32Int *d = U32Int_create_with_capacity(ITERATIONS * 2);
        
        uint64_t s = get_nanos();
        for (uint32_t i = 0; i < ITERATIONS; i++) U32Int_set(d, i * 7919, (int)i);
        results[4].insert = (double)(get_nanos() - s) / ITERATIONS;
        
        s = get_nanos();
        volatile int sum = 0;
        for (uint32_t i = 0; i < ITERATIONS; i++) sum += U32Int_get(d, i * 7919, 0);
        results[4].get_hit = (double)(get_nanos() - s) / ITERATIONS;
        
        s = get_nanos();
        volatile int f = 0;
        for (uint32_t i = 0; i < ITERATIONS; i++) if (U32Int_contains(d, i * 7919)) f++;
        results[4].contains_hit = (double)(get_nanos() - s) / ITERATIONS;
        
        s = get_nanos();
        f = 0;
        for (uint32_t i = 0; i < ITERATIONS; i++) if (U32Int_contains(d, i * 7919 + 1)) f++;
        results[4].contains_miss = (double)(get_nanos() - s) / ITERATIONS;
        results[4].name = "uint32 → int";
        
        U32Int_destroy(d);
    }
    
    // U64->Int
    {
        U64Int *d = U64Int_create_with_capacity(ITERATIONS * 2);
        
        uint64_t s = get_nanos();
        for (uint64_t i = 0; i < ITERATIONS; i++) U64Int_set(d, i * 1000000007ULL, (int)i);
        results[5].insert = (double)(get_nanos() - s) / ITERATIONS;
        
        s = get_nanos();
        volatile int sum = 0;
        for (uint64_t i = 0; i < ITERATIONS; i++) sum += U64Int_get(d, i * 1000000007ULL, 0);
        results[5].get_hit = (double)(get_nanos() - s) / ITERATIONS;
        
        s = get_nanos();
        volatile int f = 0;
        for (uint64_t i = 0; i < ITERATIONS; i++) if (U64Int_contains(d, i * 1000000007ULL)) f++;
        results[5].contains_hit = (double)(get_nanos() - s) / ITERATIONS;
        
        s = get_nanos();
        f = 0;
        for (uint64_t i = 0; i < ITERATIONS; i++) if (U64Int_contains(d, i * 1000000007ULL + 1)) f++;
        results[5].contains_miss = (double)(get_nanos() - s) / ITERATIONS;
        results[5].name = "uint64 → int";
        
        U64Int_destroy(d);
    }
    
    // Ptr->Int
    {
        void **ptrs = malloc(ITERATIONS * sizeof(void*));
        for (int i = 0; i < ITERATIONS; i++) ptrs[i] = (void*)(uintptr_t)(0x10000 + i * 64);
        
        PtrInt *d = PtrInt_create_with_capacity(ITERATIONS * 2);
        
        uint64_t s = get_nanos();
        for (int i = 0; i < ITERATIONS; i++) PtrInt_set(d, ptrs[i], i);
        results[6].insert = (double)(get_nanos() - s) / ITERATIONS;
        
        s = get_nanos();
        volatile int sum = 0;
        for (int i = 0; i < ITERATIONS; i++) sum += PtrInt_get(d, ptrs[i], 0);
        results[6].get_hit = (double)(get_nanos() - s) / ITERATIONS;
        
        s = get_nanos();
        volatile int f = 0;
        for (int i = 0; i < ITERATIONS; i++) if (PtrInt_contains(d, ptrs[i])) f++;
        results[6].contains_hit = (double)(get_nanos() - s) / ITERATIONS;
        
        s = get_nanos();
        f = 0;
        for (int i = 0; i < ITERATIONS; i++) {
            void *m = (void*)(uintptr_t)(0x90000000 + i);
            if (PtrInt_contains(d, m)) f++;
        }
        results[6].contains_miss = (double)(get_nanos() - s) / ITERATIONS;
        results[6].name = "void* → int";
        
        PtrInt_destroy(d);
        free(ptrs);
    }
    
    // Print summary
    for (int i = 0; i < 7; i++) {
        fprintf(stderr, "| %s | %.2f | %.2f | %.2f | %.2f |\n",
                results[i].name,
                results[i].insert,
                results[i].get_hit,
                results[i].contains_hit,
                results[i].contains_miss);
    }
    
    fprintf(stderr, "\n*All times in nanoseconds per operation*\n");
    
    return 0;
}
