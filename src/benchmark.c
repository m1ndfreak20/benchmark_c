#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>

#define ITERATIONS 1000000
#define WARMUP_ITERATIONS 10000

// Target format: [ HH:MM:SS:mmm.uuu ]
// Where mmm = milliseconds, uuu = microseconds

typedef void (*benchmark_func)(char *buf, size_t size);

// Get high-resolution timestamp
static inline uint64_t get_nanos(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

// ============================================================================
// Benchmark 1: strftime + gettimeofday (basic approach)
// ============================================================================
void bench_strftime_gettimeofday(char *buf, size_t size) {
    struct timeval tv;
    struct tm tm_info;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    char time_buf[16];
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", &tm_info);
    snprintf(buf, size, "[ %s:%03d.%03d ]", time_buf, ms, us);
}

// ============================================================================
// Benchmark 2: snprintf only with gettimeofday
// ============================================================================
void bench_snprintf_gettimeofday(char *buf, size_t size) {
    struct timeval tv;
    struct tm tm_info;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    snprintf(buf, size, "[ %02d:%02d:%02d:%03d.%03d ]",
             tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec, ms, us);
}

// ============================================================================
// Benchmark 3: sprintf (no size check)
// ============================================================================
void bench_sprintf_gettimeofday(char *buf, size_t size) {
    (void)size;
    struct timeval tv;
    struct tm tm_info;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    sprintf(buf, "[ %02d:%02d:%02d:%03d.%03d ]",
            tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec, ms, us);
}

// ============================================================================
// Benchmark 4: clock_gettime CLOCK_REALTIME + snprintf
// ============================================================================
void bench_clock_realtime_snprintf(char *buf, size_t size) {
    struct timespec ts;
    struct tm tm_info;
    clock_gettime(CLOCK_REALTIME, &ts);
    localtime_r(&ts.tv_sec, &tm_info);
    
    int ms = ts.tv_nsec / 1000000;
    int us = (ts.tv_nsec / 1000) % 1000;
    
    snprintf(buf, size, "[ %02d:%02d:%02d:%03d.%03d ]",
             tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec, ms, us);
}

// ============================================================================
// Benchmark 5: clock_gettime CLOCK_REALTIME_COARSE + snprintf
// ============================================================================
void bench_clock_realtime_coarse_snprintf(char *buf, size_t size) {
    struct timespec ts;
    struct tm tm_info;
    clock_gettime(CLOCK_REALTIME_COARSE, &ts);
    localtime_r(&ts.tv_sec, &tm_info);
    
    int ms = ts.tv_nsec / 1000000;
    int us = (ts.tv_nsec / 1000) % 1000;
    
    snprintf(buf, size, "[ %02d:%02d:%02d:%03d.%03d ]",
             tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec, ms, us);
}

// ============================================================================
// Benchmark 6: Manual digit conversion (optimized)
// ============================================================================
static inline void write_2digits(char *buf, int val) {
    buf[0] = '0' + val / 10;
    buf[1] = '0' + val % 10;
}

static inline void write_3digits(char *buf, int val) {
    buf[0] = '0' + val / 100;
    buf[1] = '0' + (val / 10) % 10;
    buf[2] = '0' + val % 10;
}

void bench_manual_digits_gettimeofday(char *buf, size_t size) {
    (void)size;
    struct timeval tv;
    struct tm tm_info;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    buf[0] = '[';
    buf[1] = ' ';
    write_2digits(buf + 2, tm_info.tm_hour);
    buf[4] = ':';
    write_2digits(buf + 5, tm_info.tm_min);
    buf[7] = ':';
    write_2digits(buf + 8, tm_info.tm_sec);
    buf[10] = ':';
    write_3digits(buf + 11, ms);
    buf[14] = '.';
    write_3digits(buf + 15, us);
    buf[18] = ' ';
    buf[19] = ']';
    buf[20] = '\0';
}

// ============================================================================
// Benchmark 7: Lookup table for 2-digit numbers
// ============================================================================
static const char digit_pairs[201] = 
    "00010203040506070809"
    "10111213141516171819"
    "20212223242526272829"
    "30313233343536373839"
    "40414243444546474849"
    "50515253545556575859"
    "60616263646566676869"
    "70717273747576777879"
    "80818283848586878889"
    "90919293949596979899";

void bench_lookup_table_gettimeofday(char *buf, size_t size) {
    (void)size;
    struct timeval tv;
    struct tm tm_info;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    buf[0] = '[';
    buf[1] = ' ';
    memcpy(buf + 2, digit_pairs + tm_info.tm_hour * 2, 2);
    buf[4] = ':';
    memcpy(buf + 5, digit_pairs + tm_info.tm_min * 2, 2);
    buf[7] = ':';
    memcpy(buf + 8, digit_pairs + tm_info.tm_sec * 2, 2);
    buf[10] = ':';
    buf[11] = '0' + ms / 100;
    memcpy(buf + 12, digit_pairs + (ms % 100) * 2, 2);
    buf[14] = '.';
    buf[15] = '0' + us / 100;
    memcpy(buf + 16, digit_pairs + (us % 100) * 2, 2);
    buf[18] = ' ';
    buf[19] = ']';
    buf[20] = '\0';
}

// ============================================================================
// Benchmark 8: syscall(SYS_gettimeofday) direct
// ============================================================================
void bench_syscall_gettimeofday(char *buf, size_t size) {
    (void)size;
    struct timeval tv;
    struct tm tm_info;
    syscall(SYS_gettimeofday, &tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    buf[0] = '[';
    buf[1] = ' ';
    memcpy(buf + 2, digit_pairs + tm_info.tm_hour * 2, 2);
    buf[4] = ':';
    memcpy(buf + 5, digit_pairs + tm_info.tm_min * 2, 2);
    buf[7] = ':';
    memcpy(buf + 8, digit_pairs + tm_info.tm_sec * 2, 2);
    buf[10] = ':';
    buf[11] = '0' + ms / 100;
    memcpy(buf + 12, digit_pairs + (ms % 100) * 2, 2);
    buf[14] = '.';
    buf[15] = '0' + us / 100;
    memcpy(buf + 16, digit_pairs + (us % 100) * 2, 2);
    buf[18] = ' ';
    buf[19] = ']';
    buf[20] = '\0';
}

// ============================================================================
// Benchmark 9: Cached localtime (update every second)
// ============================================================================
static struct tm cached_tm;
static time_t cached_sec = 0;

void bench_cached_localtime(char *buf, size_t size) {
    (void)size;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    if (tv.tv_sec != cached_sec) {
        cached_sec = tv.tv_sec;
        localtime_r(&tv.tv_sec, &cached_tm);
    }
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    buf[0] = '[';
    buf[1] = ' ';
    memcpy(buf + 2, digit_pairs + cached_tm.tm_hour * 2, 2);
    buf[4] = ':';
    memcpy(buf + 5, digit_pairs + cached_tm.tm_min * 2, 2);
    buf[7] = ':';
    memcpy(buf + 8, digit_pairs + cached_tm.tm_sec * 2, 2);
    buf[10] = ':';
    buf[11] = '0' + ms / 100;
    memcpy(buf + 12, digit_pairs + (ms % 100) * 2, 2);
    buf[14] = '.';
    buf[15] = '0' + us / 100;
    memcpy(buf + 16, digit_pairs + (us % 100) * 2, 2);
    buf[18] = ' ';
    buf[19] = ']';
    buf[20] = '\0';
}

// ============================================================================
// Benchmark 9b: Non-cached localtime (always call localtime_r)
// ============================================================================
void bench_nocache_localtime(char *buf, size_t size) {
    (void)size;
    struct timeval tv;
    struct tm tm_info;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    buf[0] = '[';
    buf[1] = ' ';
    memcpy(buf + 2, digit_pairs + tm_info.tm_hour * 2, 2);
    buf[4] = ':';
    memcpy(buf + 5, digit_pairs + tm_info.tm_min * 2, 2);
    buf[7] = ':';
    memcpy(buf + 8, digit_pairs + tm_info.tm_sec * 2, 2);
    buf[10] = ':';
    buf[11] = '0' + ms / 100;
    memcpy(buf + 12, digit_pairs + (ms % 100) * 2, 2);
    buf[14] = '.';
    buf[15] = '0' + us / 100;
    memcpy(buf + 16, digit_pairs + (us % 100) * 2, 2);
    buf[18] = ' ';
    buf[19] = ']';
    buf[20] = '\0';
}

// ============================================================================
// Benchmark 10: Pre-formatted template with direct copy
// ============================================================================
void bench_template_copy(char *buf, size_t size) {
    (void)size;
    struct timeval tv;
    struct tm tm_info;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    // Copy template: "[ 00:00:00:000.000 ]"
    memcpy(buf, "[ 00:00:00:000.000 ]", 21);
    
    buf[2] = '0' + tm_info.tm_hour / 10;
    buf[3] = '0' + tm_info.tm_hour % 10;
    buf[5] = '0' + tm_info.tm_min / 10;
    buf[6] = '0' + tm_info.tm_min % 10;
    buf[8] = '0' + tm_info.tm_sec / 10;
    buf[9] = '0' + tm_info.tm_sec % 10;
    buf[11] = '0' + ms / 100;
    buf[12] = '0' + (ms / 10) % 10;
    buf[13] = '0' + ms % 10;
    buf[15] = '0' + us / 100;
    buf[16] = '0' + (us / 10) % 10;
    buf[17] = '0' + us % 10;
}

// ============================================================================
// Benchmark 11: clock_gettime with manual digits
// ============================================================================
void bench_clock_manual_digits(char *buf, size_t size) {
    (void)size;
    struct timespec ts;
    struct tm tm_info;
    clock_gettime(CLOCK_REALTIME, &ts);
    localtime_r(&ts.tv_sec, &tm_info);
    
    int ms = ts.tv_nsec / 1000000;
    int us = (ts.tv_nsec / 1000) % 1000;
    
    buf[0] = '[';
    buf[1] = ' ';
    write_2digits(buf + 2, tm_info.tm_hour);
    buf[4] = ':';
    write_2digits(buf + 5, tm_info.tm_min);
    buf[7] = ':';
    write_2digits(buf + 8, tm_info.tm_sec);
    buf[10] = ':';
    write_3digits(buf + 11, ms);
    buf[14] = '.';
    write_3digits(buf + 15, us);
    buf[18] = ' ';
    buf[19] = ']';
    buf[20] = '\0';
}

// ============================================================================
// Benchmark 12: gmtime_r instead of localtime_r
// ============================================================================
void bench_gmtime_snprintf(char *buf, size_t size) {
    struct timeval tv;
    struct tm tm_info;
    gettimeofday(&tv, NULL);
    gmtime_r(&tv.tv_sec, &tm_info);
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    snprintf(buf, size, "[ %02d:%02d:%02d:%03d.%03d ]",
             tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec, ms, us);
}

// ============================================================================
// Benchmark 13: gmtime_r with manual digits
// ============================================================================
void bench_gmtime_manual(char *buf, size_t size) {
    (void)size;
    struct timeval tv;
    struct tm tm_info;
    gettimeofday(&tv, NULL);
    gmtime_r(&tv.tv_sec, &tm_info);
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    buf[0] = '[';
    buf[1] = ' ';
    memcpy(buf + 2, digit_pairs + tm_info.tm_hour * 2, 2);
    buf[4] = ':';
    memcpy(buf + 5, digit_pairs + tm_info.tm_min * 2, 2);
    buf[7] = ':';
    memcpy(buf + 8, digit_pairs + tm_info.tm_sec * 2, 2);
    buf[10] = ':';
    buf[11] = '0' + ms / 100;
    memcpy(buf + 12, digit_pairs + (ms % 100) * 2, 2);
    buf[14] = '.';
    buf[15] = '0' + us / 100;
    memcpy(buf + 16, digit_pairs + (us % 100) * 2, 2);
    buf[18] = ' ';
    buf[19] = ']';
    buf[20] = '\0';
}

// ============================================================================
// Benchmark 14: Cached gmtime
// ============================================================================
static struct tm cached_gm;
static time_t cached_gm_sec = 0;

void bench_cached_gmtime(char *buf, size_t size) {
    (void)size;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    if (tv.tv_sec != cached_gm_sec) {
        cached_gm_sec = tv.tv_sec;
        gmtime_r(&tv.tv_sec, &cached_gm);
    }
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    buf[0] = '[';
    buf[1] = ' ';
    memcpy(buf + 2, digit_pairs + cached_gm.tm_hour * 2, 2);
    buf[4] = ':';
    memcpy(buf + 5, digit_pairs + cached_gm.tm_min * 2, 2);
    buf[7] = ':';
    memcpy(buf + 8, digit_pairs + cached_gm.tm_sec * 2, 2);
    buf[10] = ':';
    buf[11] = '0' + ms / 100;
    memcpy(buf + 12, digit_pairs + (ms % 100) * 2, 2);
    buf[14] = '.';
    buf[15] = '0' + us / 100;
    memcpy(buf + 16, digit_pairs + (us % 100) * 2, 2);
    buf[18] = ' ';
    buf[19] = ']';
    buf[20] = '\0';
}

// ============================================================================
// Benchmark 14b: Non-cached gmtime (always call gmtime_r)
// ============================================================================
void bench_nocache_gmtime(char *buf, size_t size) {
    (void)size;
    struct timeval tv;
    struct tm tm_info;
    gettimeofday(&tv, NULL);
    gmtime_r(&tv.tv_sec, &tm_info);
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    buf[0] = '[';
    buf[1] = ' ';
    memcpy(buf + 2, digit_pairs + tm_info.tm_hour * 2, 2);
    buf[4] = ':';
    memcpy(buf + 5, digit_pairs + tm_info.tm_min * 2, 2);
    buf[7] = ':';
    memcpy(buf + 8, digit_pairs + tm_info.tm_sec * 2, 2);
    buf[10] = ':';
    buf[11] = '0' + ms / 100;
    memcpy(buf + 12, digit_pairs + (ms % 100) * 2, 2);
    buf[14] = '.';
    buf[15] = '0' + us / 100;
    memcpy(buf + 16, digit_pairs + (us % 100) * 2, 2);
    buf[18] = ' ';
    buf[19] = ']';
    buf[20] = '\0';
}

// ============================================================================
// Benchmark 15: time() + gettimeofday for microseconds only
// ============================================================================
void bench_time_gettimeofday_hybrid(char *buf, size_t size) {
    (void)size;
    struct timeval tv;
    struct tm tm_info;
    time_t t = time(NULL);
    gettimeofday(&tv, NULL);
    localtime_r(&t, &tm_info);
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    buf[0] = '[';
    buf[1] = ' ';
    memcpy(buf + 2, digit_pairs + tm_info.tm_hour * 2, 2);
    buf[4] = ':';
    memcpy(buf + 5, digit_pairs + tm_info.tm_min * 2, 2);
    buf[7] = ':';
    memcpy(buf + 8, digit_pairs + tm_info.tm_sec * 2, 2);
    buf[10] = ':';
    buf[11] = '0' + ms / 100;
    memcpy(buf + 12, digit_pairs + (ms % 100) * 2, 2);
    buf[14] = '.';
    buf[15] = '0' + us / 100;
    memcpy(buf + 16, digit_pairs + (us % 100) * 2, 2);
    buf[18] = ' ';
    buf[19] = ']';
    buf[20] = '\0';
}

// ============================================================================
// Benchmark 16: Only time formatting (no sub-second precision)
// ============================================================================
void bench_time_only_snprintf(char *buf, size_t size) {
    struct timeval tv;
    struct tm tm_info;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);
    
    snprintf(buf, size, "[ %02d:%02d:%02d:000.000 ]",
             tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec);
}

// ============================================================================
// Benchmark 17: Full lookup tables for 3-digit numbers
// ============================================================================
static char digit_triples[4000]; // 000-999 * 4 bytes each
static int triples_initialized = 0;

static void init_triples(void) {
    if (triples_initialized) return;
    for (int i = 0; i < 1000; i++) {
        digit_triples[i * 4] = '0' + i / 100;
        digit_triples[i * 4 + 1] = '0' + (i / 10) % 10;
        digit_triples[i * 4 + 2] = '0' + i % 10;
        digit_triples[i * 4 + 3] = '\0';
    }
    triples_initialized = 1;
}

void bench_full_lookup_tables(char *buf, size_t size) {
    (void)size;
    init_triples();
    
    struct timeval tv;
    struct tm tm_info;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    buf[0] = '[';
    buf[1] = ' ';
    memcpy(buf + 2, digit_pairs + tm_info.tm_hour * 2, 2);
    buf[4] = ':';
    memcpy(buf + 5, digit_pairs + tm_info.tm_min * 2, 2);
    buf[7] = ':';
    memcpy(buf + 8, digit_pairs + tm_info.tm_sec * 2, 2);
    buf[10] = ':';
    memcpy(buf + 11, digit_triples + ms * 4, 3);
    buf[14] = '.';
    memcpy(buf + 15, digit_triples + us * 4, 3);
    buf[18] = ' ';
    buf[19] = ']';
    buf[20] = '\0';
}

// ============================================================================
// Benchmark 18: Cached localtime with full lookup tables
// ============================================================================
void bench_cached_full_lookup(char *buf, size_t size) {
    (void)size;
    init_triples();
    
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    if (tv.tv_sec != cached_sec) {
        cached_sec = tv.tv_sec;
        localtime_r(&tv.tv_sec, &cached_tm);
    }
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    buf[0] = '[';
    buf[1] = ' ';
    memcpy(buf + 2, digit_pairs + cached_tm.tm_hour * 2, 2);
    buf[4] = ':';
    memcpy(buf + 5, digit_pairs + cached_tm.tm_min * 2, 2);
    buf[7] = ':';
    memcpy(buf + 8, digit_pairs + cached_tm.tm_sec * 2, 2);
    buf[10] = ':';
    memcpy(buf + 11, digit_triples + ms * 4, 3);
    buf[14] = '.';
    memcpy(buf + 15, digit_triples + us * 4, 3);
    buf[18] = ' ';
    buf[19] = ']';
    buf[20] = '\0';
}

// ============================================================================
// Benchmark 18b: Non-cached localtime with full lookup tables
// ============================================================================
void bench_nocache_full_lookup(char *buf, size_t size) {
    (void)size;
    init_triples();
    
    struct timeval tv;
    struct tm tm_info;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    buf[0] = '[';
    buf[1] = ' ';
    memcpy(buf + 2, digit_pairs + tm_info.tm_hour * 2, 2);
    buf[4] = ':';
    memcpy(buf + 5, digit_pairs + tm_info.tm_min * 2, 2);
    buf[7] = ':';
    memcpy(buf + 8, digit_pairs + tm_info.tm_sec * 2, 2);
    buf[10] = ':';
    memcpy(buf + 11, digit_triples + ms * 4, 3);
    buf[14] = '.';
    memcpy(buf + 15, digit_triples + us * 4, 3);
    buf[18] = ' ';
    buf[19] = ']';
    buf[20] = '\0';
}

// ============================================================================
// Benchmark 19: CLOCK_REALTIME_COARSE with cached localtime
// ============================================================================
void bench_coarse_cached(char *buf, size_t size) {
    (void)size;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME_COARSE, &ts);
    
    if (ts.tv_sec != cached_sec) {
        cached_sec = ts.tv_sec;
        localtime_r(&ts.tv_sec, &cached_tm);
    }
    
    int ms = ts.tv_nsec / 1000000;
    int us = (ts.tv_nsec / 1000) % 1000;
    
    buf[0] = '[';
    buf[1] = ' ';
    memcpy(buf + 2, digit_pairs + cached_tm.tm_hour * 2, 2);
    buf[4] = ':';
    memcpy(buf + 5, digit_pairs + cached_tm.tm_min * 2, 2);
    buf[7] = ':';
    memcpy(buf + 8, digit_pairs + cached_tm.tm_sec * 2, 2);
    buf[10] = ':';
    buf[11] = '0' + ms / 100;
    memcpy(buf + 12, digit_pairs + (ms % 100) * 2, 2);
    buf[14] = '.';
    buf[15] = '0' + us / 100;
    memcpy(buf + 16, digit_pairs + (us % 100) * 2, 2);
    buf[18] = ' ';
    buf[19] = ']';
    buf[20] = '\0';
}

// ============================================================================
// Benchmark 19b: CLOCK_REALTIME_COARSE without cache (always call localtime_r)
// ============================================================================
void bench_coarse_nocache(char *buf, size_t size) {
    (void)size;
    struct timespec ts;
    struct tm tm_info;
    clock_gettime(CLOCK_REALTIME_COARSE, &ts);
    localtime_r(&ts.tv_sec, &tm_info);
    
    int ms = ts.tv_nsec / 1000000;
    int us = (ts.tv_nsec / 1000) % 1000;
    
    buf[0] = '[';
    buf[1] = ' ';
    memcpy(buf + 2, digit_pairs + tm_info.tm_hour * 2, 2);
    buf[4] = ':';
    memcpy(buf + 5, digit_pairs + tm_info.tm_min * 2, 2);
    buf[7] = ':';
    memcpy(buf + 8, digit_pairs + tm_info.tm_sec * 2, 2);
    buf[10] = ':';
    buf[11] = '0' + ms / 100;
    memcpy(buf + 12, digit_pairs + (ms % 100) * 2, 2);
    buf[14] = '.';
    buf[15] = '0' + us / 100;
    memcpy(buf + 16, digit_pairs + (us % 100) * 2, 2);
    buf[18] = ' ';
    buf[19] = ']';
    buf[20] = '\0';
}

// ============================================================================
// Benchmark 20: Using asprintf (dynamic allocation)
// ============================================================================
void bench_asprintf(char *buf, size_t size) {
    struct timeval tv;
    struct tm tm_info;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    char *tmp;
    asprintf(&tmp, "[ %02d:%02d:%02d:%03d.%03d ]",
             tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec, ms, us);
    strncpy(buf, tmp, size - 1);
    buf[size - 1] = '\0';
    free(tmp);
}

// ============================================================================
// Benchmark 21: Using strcat chain
// ============================================================================
void bench_strcat_chain(char *buf, size_t size) {
    (void)size;
    struct timeval tv;
    struct tm tm_info;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    char tmp[8];
    buf[0] = '\0';
    strcat(buf, "[ ");
    sprintf(tmp, "%02d:", tm_info.tm_hour);
    strcat(buf, tmp);
    sprintf(tmp, "%02d:", tm_info.tm_min);
    strcat(buf, tmp);
    sprintf(tmp, "%02d:", tm_info.tm_sec);
    strcat(buf, tmp);
    sprintf(tmp, "%03d.", ms);
    strcat(buf, tmp);
    sprintf(tmp, "%03d", us);
    strcat(buf, tmp);
    strcat(buf, " ]");
}

// ============================================================================
// Benchmark 22: Multiple snprintf calls
// ============================================================================
void bench_multiple_snprintf(char *buf, size_t size) {
    struct timeval tv;
    struct tm tm_info;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    int pos = 0;
    pos += snprintf(buf + pos, size - pos, "[ %02d:", tm_info.tm_hour);
    pos += snprintf(buf + pos, size - pos, "%02d:", tm_info.tm_min);
    pos += snprintf(buf + pos, size - pos, "%02d:", tm_info.tm_sec);
    pos += snprintf(buf + pos, size - pos, "%03d.", ms);
    pos += snprintf(buf + pos, size - pos, "%03d ]", us);
}

// ============================================================================
// Benchmark 23: Single memcpy with pre-computed string (cached)
// ============================================================================
static char precomputed_time[32] = "[ 00:00:00:000.000 ]";
static time_t precomputed_sec = 0;

void bench_precomputed_memcpy(char *buf, size_t size) {
    (void)size;
    struct timeval tv;
    struct tm tm_info;
    gettimeofday(&tv, NULL);
    
    if (tv.tv_sec != precomputed_sec) {
        precomputed_sec = tv.tv_sec;
        localtime_r(&tv.tv_sec, &tm_info);
        precomputed_time[2] = '0' + tm_info.tm_hour / 10;
        precomputed_time[3] = '0' + tm_info.tm_hour % 10;
        precomputed_time[5] = '0' + tm_info.tm_min / 10;
        precomputed_time[6] = '0' + tm_info.tm_min % 10;
        precomputed_time[8] = '0' + tm_info.tm_sec / 10;
        precomputed_time[9] = '0' + tm_info.tm_sec % 10;
    }
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    memcpy(buf, precomputed_time, 21);
    buf[11] = '0' + ms / 100;
    buf[12] = '0' + (ms / 10) % 10;
    buf[13] = '0' + ms % 10;
    buf[15] = '0' + us / 100;
    buf[16] = '0' + (us / 10) % 10;
    buf[17] = '0' + us % 10;
}

// ============================================================================
// Benchmark 23b: Single memcpy without cache (always localtime_r)
// ============================================================================
void bench_nocache_precomputed_memcpy(char *buf, size_t size) {
    (void)size;
    struct timeval tv;
    struct tm tm_info;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    memcpy(buf, "[ 00:00:00:000.000 ]", 21);
    buf[2] = '0' + tm_info.tm_hour / 10;
    buf[3] = '0' + tm_info.tm_hour % 10;
    buf[5] = '0' + tm_info.tm_min / 10;
    buf[6] = '0' + tm_info.tm_min % 10;
    buf[8] = '0' + tm_info.tm_sec / 10;
    buf[9] = '0' + tm_info.tm_sec % 10;
    buf[11] = '0' + ms / 100;
    buf[12] = '0' + (ms / 10) % 10;
    buf[13] = '0' + ms % 10;
    buf[15] = '0' + us / 100;
    buf[16] = '0' + (us / 10) % 10;
    buf[17] = '0' + us % 10;
}

// ============================================================================
// Benchmark 24: uint64_t pointer write (8 bytes at once)
// ============================================================================
void bench_uint64_write(char *buf, size_t size) {
    (void)size;
    struct timeval tv;
    struct tm tm_info;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    // Build string manually
    buf[0] = '[';
    buf[1] = ' ';
    buf[2] = '0' + tm_info.tm_hour / 10;
    buf[3] = '0' + tm_info.tm_hour % 10;
    buf[4] = ':';
    buf[5] = '0' + tm_info.tm_min / 10;
    buf[6] = '0' + tm_info.tm_min % 10;
    buf[7] = ':';
    buf[8] = '0' + tm_info.tm_sec / 10;
    buf[9] = '0' + tm_info.tm_sec % 10;
    buf[10] = ':';
    buf[11] = '0' + ms / 100;
    buf[12] = '0' + (ms / 10) % 10;
    buf[13] = '0' + ms % 10;
    buf[14] = '.';
    buf[15] = '0' + us / 100;
    buf[16] = '0' + (us / 10) % 10;
    buf[17] = '0' + us % 10;
    buf[18] = ' ';
    buf[19] = ']';
    buf[20] = '\0';
}

// ============================================================================
// Benchmark 25: Inline all calculations (no function calls for digits)
// ============================================================================
void bench_inline_all(char *buf, size_t size) {
    (void)size;
    struct timeval tv;
    struct tm tm_info;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);
    
    long us_total = tv.tv_usec;
    
    *buf++ = '[';
    *buf++ = ' ';
    *buf++ = '0' + tm_info.tm_hour / 10;
    *buf++ = '0' + tm_info.tm_hour % 10;
    *buf++ = ':';
    *buf++ = '0' + tm_info.tm_min / 10;
    *buf++ = '0' + tm_info.tm_min % 10;
    *buf++ = ':';
    *buf++ = '0' + tm_info.tm_sec / 10;
    *buf++ = '0' + tm_info.tm_sec % 10;
    *buf++ = ':';
    *buf++ = '0' + (us_total / 100000);
    *buf++ = '0' + (us_total / 10000) % 10;
    *buf++ = '0' + (us_total / 1000) % 10;
    *buf++ = '.';
    *buf++ = '0' + (us_total / 100) % 10;
    *buf++ = '0' + (us_total / 10) % 10;
    *buf++ = '0' + us_total % 10;
    *buf++ = ' ';
    *buf++ = ']';
    *buf = '\0';
}

// ============================================================================
// Benchmark 26: Division optimization using multiplication
// ============================================================================
void bench_div_optimization(char *buf, size_t size) {
    (void)size;
    struct timeval tv;
    struct tm tm_info;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);
    
    unsigned int us_total = (unsigned int)tv.tv_usec;
    unsigned int ms = us_total / 1000;
    unsigned int us = us_total - ms * 1000;
    
    buf[0] = '[';
    buf[1] = ' ';
    buf[2] = '0' + tm_info.tm_hour / 10;
    buf[3] = '0' + tm_info.tm_hour % 10;
    buf[4] = ':';
    buf[5] = '0' + tm_info.tm_min / 10;
    buf[6] = '0' + tm_info.tm_min % 10;
    buf[7] = ':';
    buf[8] = '0' + tm_info.tm_sec / 10;
    buf[9] = '0' + tm_info.tm_sec % 10;
    buf[10] = ':';
    
    unsigned int ms_q = ms / 100;
    unsigned int ms_r = ms - ms_q * 100;
    buf[11] = '0' + ms_q;
    buf[12] = '0' + ms_r / 10;
    buf[13] = '0' + ms_r % 10;
    buf[14] = '.';
    
    unsigned int us_q = us / 100;
    unsigned int us_r = us - us_q * 100;
    buf[15] = '0' + us_q;
    buf[16] = '0' + us_r / 10;
    buf[17] = '0' + us_r % 10;
    buf[18] = ' ';
    buf[19] = ']';
    buf[20] = '\0';
}

// ============================================================================
// Benchmark 27: Fully cached (only microseconds updated)
// ============================================================================
static char fully_cached[32] = "[ 00:00:00:000.000 ]";
static time_t fully_cached_sec = 0;
static int fully_cached_initialized = 0;

void bench_fully_cached(char *buf, size_t size) {
    (void)size;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    if (!fully_cached_initialized || tv.tv_sec != fully_cached_sec) {
        struct tm tm_info;
        fully_cached_sec = tv.tv_sec;
        localtime_r(&tv.tv_sec, &tm_info);
        fully_cached[2] = '0' + tm_info.tm_hour / 10;
        fully_cached[3] = '0' + tm_info.tm_hour % 10;
        fully_cached[5] = '0' + tm_info.tm_min / 10;
        fully_cached[6] = '0' + tm_info.tm_min % 10;
        fully_cached[8] = '0' + tm_info.tm_sec / 10;
        fully_cached[9] = '0' + tm_info.tm_sec % 10;
        fully_cached_initialized = 1;
    }
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    memcpy(buf, fully_cached, 11);
    buf[11] = '0' + ms / 100;
    buf[12] = '0' + (ms / 10) % 10;
    buf[13] = '0' + ms % 10;
    buf[14] = '.';
    buf[15] = '0' + us / 100;
    buf[16] = '0' + (us / 10) % 10;
    buf[17] = '0' + us % 10;
    buf[18] = ' ';
    buf[19] = ']';
    buf[20] = '\0';
}

// ============================================================================
// Benchmark 27b: Non-cached full string build
// ============================================================================
void bench_nocache_fully(char *buf, size_t size) {
    (void)size;
    struct timeval tv;
    struct tm tm_info;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    buf[0] = '[';
    buf[1] = ' ';
    buf[2] = '0' + tm_info.tm_hour / 10;
    buf[3] = '0' + tm_info.tm_hour % 10;
    buf[4] = ':';
    buf[5] = '0' + tm_info.tm_min / 10;
    buf[6] = '0' + tm_info.tm_min % 10;
    buf[7] = ':';
    buf[8] = '0' + tm_info.tm_sec / 10;
    buf[9] = '0' + tm_info.tm_sec % 10;
    buf[10] = ':';
    buf[11] = '0' + ms / 100;
    buf[12] = '0' + (ms / 10) % 10;
    buf[13] = '0' + ms % 10;
    buf[14] = '.';
    buf[15] = '0' + us / 100;
    buf[16] = '0' + (us / 10) % 10;
    buf[17] = '0' + us % 10;
    buf[18] = ' ';
    buf[19] = ']';
    buf[20] = '\0';
}

// ============================================================================
// Benchmark 28: clock_gettime + cached gmtime
// ============================================================================
void bench_clock_cached_gm(char *buf, size_t size) {
    (void)size;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    
    if (ts.tv_sec != cached_gm_sec) {
        cached_gm_sec = ts.tv_sec;
        gmtime_r(&ts.tv_sec, &cached_gm);
    }
    
    int ms = ts.tv_nsec / 1000000;
    int us = (ts.tv_nsec / 1000) % 1000;
    
    buf[0] = '[';
    buf[1] = ' ';
    memcpy(buf + 2, digit_pairs + cached_gm.tm_hour * 2, 2);
    buf[4] = ':';
    memcpy(buf + 5, digit_pairs + cached_gm.tm_min * 2, 2);
    buf[7] = ':';
    memcpy(buf + 8, digit_pairs + cached_gm.tm_sec * 2, 2);
    buf[10] = ':';
    buf[11] = '0' + ms / 100;
    memcpy(buf + 12, digit_pairs + (ms % 100) * 2, 2);
    buf[14] = '.';
    buf[15] = '0' + us / 100;
    memcpy(buf + 16, digit_pairs + (us % 100) * 2, 2);
    buf[18] = ' ';
    buf[19] = ']';
    buf[20] = '\0';
}

// ============================================================================
// Benchmark 28b: clock_gettime + gmtime without cache
// ============================================================================
void bench_clock_nocache_gm(char *buf, size_t size) {
    (void)size;
    struct timespec ts;
    struct tm tm_info;
    clock_gettime(CLOCK_REALTIME, &ts);
    gmtime_r(&ts.tv_sec, &tm_info);
    
    int ms = ts.tv_nsec / 1000000;
    int us = (ts.tv_nsec / 1000) % 1000;
    
    buf[0] = '[';
    buf[1] = ' ';
    memcpy(buf + 2, digit_pairs + tm_info.tm_hour * 2, 2);
    buf[4] = ':';
    memcpy(buf + 5, digit_pairs + tm_info.tm_min * 2, 2);
    buf[7] = ':';
    memcpy(buf + 8, digit_pairs + tm_info.tm_sec * 2, 2);
    buf[10] = ':';
    buf[11] = '0' + ms / 100;
    memcpy(buf + 12, digit_pairs + (ms % 100) * 2, 2);
    buf[14] = '.';
    buf[15] = '0' + us / 100;
    memcpy(buf + 16, digit_pairs + (us % 100) * 2, 2);
    buf[18] = ' ';
    buf[19] = ']';
    buf[20] = '\0';
}

// ============================================================================
// Benchmark 29: strftime with CLOCK_REALTIME
// ============================================================================
void bench_strftime_clock(char *buf, size_t size) {
    struct timespec ts;
    struct tm tm_info;
    clock_gettime(CLOCK_REALTIME, &ts);
    localtime_r(&ts.tv_sec, &tm_info);
    
    int ms = ts.tv_nsec / 1000000;
    int us = (ts.tv_nsec / 1000) % 1000;
    
    char time_buf[16];
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", &tm_info);
    snprintf(buf, size, "[ %s:%03d.%03d ]", time_buf, ms, us);
}

// ============================================================================
// Benchmark 30: Minimal - only gettimeofday + lookup (cached)
// ============================================================================
void bench_minimal_gettimeofday(char *buf, size_t size) {
    (void)size;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    if (tv.tv_sec != cached_sec) {
        cached_sec = tv.tv_sec;
        localtime_r(&tv.tv_sec, &cached_tm);
    }
    
    memcpy(buf, "[ ", 2);
    memcpy(buf + 2, digit_pairs + cached_tm.tm_hour * 2, 2);
    buf[4] = ':';
    memcpy(buf + 5, digit_pairs + cached_tm.tm_min * 2, 2);
    buf[7] = ':';
    memcpy(buf + 8, digit_pairs + cached_tm.tm_sec * 2, 2);
    buf[10] = ':';
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    memcpy(buf + 11, digit_triples + ms * 4, 3);
    buf[14] = '.';
    memcpy(buf + 15, digit_triples + us * 4, 3);
    memcpy(buf + 18, " ]", 3);
}

// ============================================================================
// Benchmark 30b: Minimal - gettimeofday + lookup without cache
// ============================================================================
void bench_minimal_nocache(char *buf, size_t size) {
    (void)size;
    struct timeval tv;
    struct tm tm_info;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm_info);
    
    memcpy(buf, "[ ", 2);
    memcpy(buf + 2, digit_pairs + tm_info.tm_hour * 2, 2);
    buf[4] = ':';
    memcpy(buf + 5, digit_pairs + tm_info.tm_min * 2, 2);
    buf[7] = ':';
    memcpy(buf + 8, digit_pairs + tm_info.tm_sec * 2, 2);
    buf[10] = ':';
    
    int ms = tv.tv_usec / 1000;
    int us = tv.tv_usec % 1000;
    
    memcpy(buf + 11, digit_triples + ms * 4, 3);
    buf[14] = '.';
    memcpy(buf + 15, digit_triples + us * 4, 3);
    memcpy(buf + 18, " ]", 3);
}

// ============================================================================
// Benchmark 31: CLOCK_MONOTONIC based (relative time)
// ============================================================================
static time_t monotonic_start_sec = 0;
static struct tm monotonic_base_tm;
static int monotonic_initialized = 0;

void bench_monotonic_relative(char *buf, size_t size) {
    (void)size;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    
    if (!monotonic_initialized) {
        struct timespec real_ts;
        clock_gettime(CLOCK_REALTIME, &real_ts);
        localtime_r(&real_ts.tv_sec, &monotonic_base_tm);
        monotonic_start_sec = ts.tv_sec;
        monotonic_initialized = 1;
    }
    
    // Calculate elapsed seconds and update time
    time_t elapsed = ts.tv_sec - monotonic_start_sec;
    int total_sec = monotonic_base_tm.tm_sec + (int)(elapsed % 60);
    int total_min = monotonic_base_tm.tm_min + (int)((elapsed / 60) % 60);
    int total_hour = monotonic_base_tm.tm_hour + (int)((elapsed / 3600) % 24);
    
    if (total_sec >= 60) { total_sec -= 60; total_min++; }
    if (total_min >= 60) { total_min -= 60; total_hour++; }
    if (total_hour >= 24) { total_hour -= 24; }
    
    int ms = ts.tv_nsec / 1000000;
    int us = (ts.tv_nsec / 1000) % 1000;
    
    buf[0] = '[';
    buf[1] = ' ';
    memcpy(buf + 2, digit_pairs + total_hour * 2, 2);
    buf[4] = ':';
    memcpy(buf + 5, digit_pairs + total_min * 2, 2);
    buf[7] = ':';
    memcpy(buf + 8, digit_pairs + total_sec * 2, 2);
    buf[10] = ':';
    buf[11] = '0' + ms / 100;
    memcpy(buf + 12, digit_pairs + (ms % 100) * 2, 2);
    buf[14] = '.';
    buf[15] = '0' + us / 100;
    memcpy(buf + 16, digit_pairs + (us % 100) * 2, 2);
    buf[18] = ' ';
    buf[19] = ']';
    buf[20] = '\0';
}

// ============================================================================
// Benchmark 32: Batched time read (read once, format multiple)
// ============================================================================
static struct timeval batch_tv;
static struct tm batch_tm;
static int batch_ms, batch_us;

void bench_batch_read(char *buf, size_t size) {
    (void)size;
    // In real usage, you'd call this once per batch
    gettimeofday(&batch_tv, NULL);
    localtime_r(&batch_tv.tv_sec, &batch_tm);
    batch_ms = batch_tv.tv_usec / 1000;
    batch_us = batch_tv.tv_usec % 1000;
    
    buf[0] = '[';
    buf[1] = ' ';
    memcpy(buf + 2, digit_pairs + batch_tm.tm_hour * 2, 2);
    buf[4] = ':';
    memcpy(buf + 5, digit_pairs + batch_tm.tm_min * 2, 2);
    buf[7] = ':';
    memcpy(buf + 8, digit_pairs + batch_tm.tm_sec * 2, 2);
    buf[10] = ':';
    buf[11] = '0' + batch_ms / 100;
    memcpy(buf + 12, digit_pairs + (batch_ms % 100) * 2, 2);
    buf[14] = '.';
    buf[15] = '0' + batch_us / 100;
    memcpy(buf + 16, digit_pairs + (batch_us % 100) * 2, 2);
    buf[18] = ' ';
    buf[19] = ']';
    buf[20] = '\0';
}

// ============================================================================
// Benchmark runner
// ============================================================================
typedef struct {
    const char *name;
    benchmark_func func;
    const char *description;
} benchmark_t;

static benchmark_t benchmarks[] = {
    {"strftime_gettimeofday", bench_strftime_gettimeofday, "strftime() + gettimeofday() basic"},
    {"snprintf_gettimeofday", bench_snprintf_gettimeofday, "snprintf() + gettimeofday()"},
    {"sprintf_gettimeofday", bench_sprintf_gettimeofday, "sprintf() + gettimeofday() (no size check)"},
    {"clock_realtime_snprintf", bench_clock_realtime_snprintf, "clock_gettime(CLOCK_REALTIME) + snprintf()"},
    {"clock_realtime_coarse", bench_clock_realtime_coarse_snprintf, "clock_gettime(CLOCK_REALTIME_COARSE) + snprintf()"},
    {"manual_digits", bench_manual_digits_gettimeofday, "Manual digit conversion"},
    {"lookup_table", bench_lookup_table_gettimeofday, "2-digit lookup table"},
    {"syscall_gettimeofday", bench_syscall_gettimeofday, "Direct syscall(SYS_gettimeofday)"},
    {"cached_localtime", bench_cached_localtime, "Cached localtime_r()"},
    {"nocache_localtime", bench_nocache_localtime, "Non-cached localtime_r() + lookup"},
    {"template_copy", bench_template_copy, "Template memcpy + digit fill"},
    {"clock_manual_digits", bench_clock_manual_digits, "clock_gettime + manual digits"},
    {"gmtime_snprintf", bench_gmtime_snprintf, "gmtime_r() + snprintf() (UTC)"},
    {"gmtime_manual", bench_gmtime_manual, "gmtime_r() + manual digits (UTC)"},
    {"cached_gmtime", bench_cached_gmtime, "Cached gmtime_r() (UTC)"},
    {"nocache_gmtime", bench_nocache_gmtime, "Non-cached gmtime_r() + lookup (UTC)"},
    {"time_gettimeofday_hybrid", bench_time_gettimeofday_hybrid, "time() + gettimeofday() hybrid"},
    {"time_only_snprintf", bench_time_only_snprintf, "Time only (no sub-second)"},
    {"full_lookup_tables", bench_full_lookup_tables, "Full 2+3 digit lookup tables"},
    {"cached_full_lookup", bench_cached_full_lookup, "Cached localtime + full lookup"},
    {"nocache_full_lookup", bench_nocache_full_lookup, "Non-cached localtime + full lookup"},
    {"coarse_cached", bench_coarse_cached, "CLOCK_REALTIME_COARSE + cached"},
    {"coarse_nocache", bench_coarse_nocache, "CLOCK_REALTIME_COARSE + no cache"},
    {"asprintf", bench_asprintf, "asprintf() dynamic allocation"},
    {"strcat_chain", bench_strcat_chain, "strcat() chain"},
    {"multiple_snprintf", bench_multiple_snprintf, "Multiple snprintf() calls"},
    {"precomputed_memcpy", bench_precomputed_memcpy, "Precomputed string + memcpy (cached)"},
    {"nocache_precomputed", bench_nocache_precomputed_memcpy, "Precomputed string + memcpy (no cache)"},
    {"uint64_write", bench_uint64_write, "Direct byte-by-byte write"},
    {"inline_all", bench_inline_all, "All calculations inline"},
    {"div_optimization", bench_div_optimization, "Division optimization"},
    {"fully_cached", bench_fully_cached, "Fully cached (only us updated)"},
    {"nocache_fully", bench_nocache_fully, "Non-cached full string build"},
    {"clock_cached_gm", bench_clock_cached_gm, "clock_gettime + cached gmtime"},
    {"clock_nocache_gm", bench_clock_nocache_gm, "clock_gettime + gmtime (no cache)"},
    {"strftime_clock", bench_strftime_clock, "strftime() + clock_gettime()"},
    {"minimal_gettimeofday", bench_minimal_gettimeofday, "Minimal with full lookup (cached)"},
    {"minimal_nocache", bench_minimal_nocache, "Minimal with full lookup (no cache)"},
    {"monotonic_relative", bench_monotonic_relative, "CLOCK_MONOTONIC relative"},
    {"batch_read", bench_batch_read, "Batched time read pattern"},
};

#define NUM_BENCHMARKS (sizeof(benchmarks) / sizeof(benchmarks[0]))

void run_benchmark(benchmark_t *bench) {
    char buf[64];
    uint64_t start, end;
    
    // Warmup
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        bench->func(buf, sizeof(buf));
    }
    
    // Benchmark
    start = get_nanos();
    for (int i = 0; i < ITERATIONS; i++) {
        bench->func(buf, sizeof(buf));
    }
    end = get_nanos();
    
    double total_ms = (double)(end - start) / 1000000.0;
    double per_call_ns = (double)(end - start) / ITERATIONS;
    double calls_per_sec = 1000000000.0 / per_call_ns;
    
    // Get sample output
    bench->func(buf, sizeof(buf));
    
    printf("| %-28s | %10.2f | %8.2f | %12.0f | %s |\n",
           bench->name, total_ms, per_call_ns, calls_per_sec, buf);
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    
    // Initialize lookup tables
    init_triples();
    
    printf("# DateTime String Benchmark Results\n\n");
    printf("Format: [ HH:MM:SS:mmm.uuu ]\n");
    printf("Iterations: %d\n", ITERATIONS);
    printf("Warmup: %d\n\n", WARMUP_ITERATIONS);
    
    // System info
    printf("## System Info\n");
    FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
    if (cpuinfo) {
        char line[256];
        while (fgets(line, sizeof(line), cpuinfo)) {
            if (strncmp(line, "model name", 10) == 0) {
                printf("CPU: %s", strchr(line, ':') + 2);
                break;
            }
        }
        fclose(cpuinfo);
    }
    printf("\n");
    
    printf("## Results\n\n");
    printf("| %-28s | %10s | %8s | %12s | %s |\n",
           "Benchmark", "Total (ms)", "Per call (ns)", "Calls/sec", "Sample Output");
    printf("|%-30s|%12s|%10s|%14s|%22s|\n",
           "------------------------------", "------------", "----------", "--------------", "----------------------");
    
    for (size_t i = 0; i < NUM_BENCHMARKS; i++) {
        run_benchmark(&benchmarks[i]);
    }
    
    printf("\n## Benchmark Descriptions\n\n");
    for (size_t i = 0; i < NUM_BENCHMARKS; i++) {
        printf("%zu. **%s**: %s\n", i + 1, benchmarks[i].name, benchmarks[i].description);
    }
    
    return 0;
}
