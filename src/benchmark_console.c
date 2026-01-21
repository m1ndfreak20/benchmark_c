#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/uio.h>

#define ITERATIONS 10000
#define WARMUP_ITERATIONS 1000

typedef void (*benchmark_func)(void);

// Get high-resolution timestamp
static inline uint64_t get_nanos(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

// Test strings
static const char *test_string_short = "Hello, C!!!!\n";
static const char *test_string_medium = "Hello, C!!!! This is a medium length string for benchmarking.\n";
static const char *test_string_long = "Hello, C!!!! This is a much longer string that contains more data for testing console output performance with various methods available in C.\n";

// ============================================================================
// Benchmark 1: printf - standard formatted output
// ============================================================================
void bench_printf_short(void) {
    printf("%s", test_string_short);
}

void bench_printf_medium(void) {
    printf("%s", test_string_medium);
}

void bench_printf_long(void) {
    printf("%s", test_string_long);
}

// ============================================================================
// Benchmark 2: printf with literal string
// ============================================================================
void bench_printf_literal(void) {
    printf("Hello, C!!!!\n");
}

// ============================================================================
// Benchmark 3: puts - simple string output
// ============================================================================
void bench_puts_short(void) {
    puts("Hello, C!!!!");
}

// ============================================================================
// Benchmark 4: fputs - string to stream
// ============================================================================
void bench_fputs_short(void) {
    fputs("Hello, C!!!!\n", stdout);
}

void bench_fputs_medium(void) {
    fputs(test_string_medium, stdout);
}

void bench_fputs_long(void) {
    fputs(test_string_long, stdout);
}

// ============================================================================
// Benchmark 5: fwrite - binary write
// ============================================================================
void bench_fwrite_short(void) {
    fwrite(test_string_short, 1, 14, stdout);
}

void bench_fwrite_medium(void) {
    fwrite(test_string_medium, 1, 63, stdout);
}

void bench_fwrite_long(void) {
    fwrite(test_string_long, 1, 142, stdout);
}

// ============================================================================
// Benchmark 6: write - direct syscall
// ============================================================================
void bench_write_short(void) {
    (void)write(STDOUT_FILENO, test_string_short, 14);
}

void bench_write_medium(void) {
    (void)write(STDOUT_FILENO, test_string_medium, 63);
}

void bench_write_long(void) {
    (void)write(STDOUT_FILENO, test_string_long, 142);
}

// ============================================================================
// Benchmark 7: putchar - single character output
// ============================================================================
void bench_putchar_loop(void) {
    const char *s = test_string_short;
    while (*s) {
        putchar(*s++);
    }
}

// ============================================================================
// Benchmark 8: fputc - single character to stream
// ============================================================================
void bench_fputc_loop(void) {
    const char *s = test_string_short;
    while (*s) {
        fputc(*s++, stdout);
    }
}

// ============================================================================
// Benchmark 9: printf with formatting
// ============================================================================
void bench_printf_formatted_int(void) {
    printf("Value: %d\n", 12345);
}

void bench_printf_formatted_float(void) {
    printf("Value: %.2f\n", 123.45);
}

void bench_printf_formatted_string(void) {
    printf("Message: %s, Count: %d\n", "Hello", 42);
}

void bench_printf_formatted_complex(void) {
    printf("[%04d-%02d-%02d %02d:%02d:%02d] %s: %d (%.2f%%)\n", 
           2024, 1, 15, 10, 30, 45, "Status", 100, 99.5);
}

// ============================================================================
// Benchmark 10: fprintf - to stdout
// ============================================================================
void bench_fprintf_short(void) {
    fprintf(stdout, "%s", test_string_short);
}

void bench_fprintf_formatted(void) {
    fprintf(stdout, "Value: %d, String: %s\n", 42, "test");
}

// ============================================================================
// Benchmark 11: snprintf + write (pre-format then write)
// ============================================================================
void bench_snprintf_write(void) {
    char buf[256];
    int len = snprintf(buf, sizeof(buf), "Value: %d, String: %s\n", 42, "test");
    (void)write(STDOUT_FILENO, buf, len);
}

// ============================================================================
// Benchmark 12: sprintf + write (no size check)
// ============================================================================
void bench_sprintf_write(void) {
    char buf[256];
    int len = sprintf(buf, "Value: %d, String: %s\n", 42, "test");
    (void)write(STDOUT_FILENO, buf, len);
}

// ============================================================================
// Benchmark 13: writev - scatter/gather I/O
// ============================================================================
void bench_writev_multi(void) {
    struct iovec iov[3];
    iov[0].iov_base = (void*)"[INFO] ";
    iov[0].iov_len = 7;
    iov[1].iov_base = (void*)"Hello, C!!!!";
    iov[1].iov_len = 12;
    iov[2].iov_base = (void*)"\n";
    iov[2].iov_len = 1;
    (void)writev(STDOUT_FILENO, iov, 3);
}

// ============================================================================
// Benchmark 14: setvbuf - unbuffered vs line buffered vs fully buffered
// ============================================================================
void bench_unbuffered_printf(void) {
    printf("%s", test_string_short);
}

void bench_linebuffered_printf(void) {
    printf("%s", test_string_short);
}

void bench_fullbuffered_printf(void) {
    printf("%s", test_string_short);
}

// ============================================================================
// Benchmark 15: dprintf - direct file descriptor printf
// ============================================================================
void bench_dprintf_short(void) {
    dprintf(STDOUT_FILENO, "%s", test_string_short);
}

void bench_dprintf_formatted(void) {
    dprintf(STDOUT_FILENO, "Value: %d, String: %s\n", 42, "test");
}

// ============================================================================
// Run benchmark and measure time
// ============================================================================
typedef struct {
    const char *name;
    benchmark_func func;
    const char *description;
    int special_setup; // 0=none, 1=unbuffered, 2=line buffered, 3=full buffered
} Benchmark;

double run_benchmark(Benchmark *bench, int silent __attribute__((unused))) {
    // Save original stdout
    int stdout_copy = dup(STDOUT_FILENO);
    
    // Redirect stdout to /dev/null during benchmark
    int devnull = open("/dev/null", O_WRONLY);
    if (devnull == -1) {
        perror("open /dev/null");
        return -1;
    }
    
    // Handle special buffer modes
    if (bench->special_setup == 1) {
        setvbuf(stdout, NULL, _IONBF, 0);
    } else if (bench->special_setup == 2) {
        setvbuf(stdout, NULL, _IOLBF, 0);
    } else if (bench->special_setup == 3) {
        static char buf[BUFSIZ];
        setvbuf(stdout, buf, _IOFBF, BUFSIZ);
    }
    
    dup2(devnull, STDOUT_FILENO);
    close(devnull);
    
    // Warmup
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        bench->func();
    }
    fflush(stdout);
    
    // Benchmark
    uint64_t start = get_nanos();
    for (int i = 0; i < ITERATIONS; i++) {
        bench->func();
    }
    fflush(stdout);
    uint64_t end = get_nanos();
    
    // Restore stdout
    dup2(stdout_copy, STDOUT_FILENO);
    close(stdout_copy);
    
    // Reset buffer mode to line buffered
    setvbuf(stdout, NULL, _IOLBF, 0);
    
    double total_ns = (double)(end - start);
    double per_op_ns = total_ns / ITERATIONS;
    
    return per_op_ns;
}

int main(void) {
    // Use stderr for all output to avoid conflicts with benchmark redirections
    fprintf(stderr, "# Console Output Benchmark Results\n\n");
    fprintf(stderr, "Benchmarking various methods of writing to console in C.\n");
    fprintf(stderr, "Output redirected to /dev/null during benchmarks to measure pure overhead.\n\n");
    fprintf(stderr, "Iterations: %d\n\n", ITERATIONS);
    
    // System info
    fprintf(stderr, "## System Info\n");
    FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
    if (cpuinfo) {
        char line[256];
        while (fgets(line, sizeof(line), cpuinfo)) {
            if (strncmp(line, "model name", 10) == 0) {
                char *colon = strchr(line, ':');
                if (colon) {
                    fprintf(stderr, "CPU:%s", colon + 1);
                }
                break;
            }
        }
        fclose(cpuinfo);
    }
    fprintf(stderr, "\n");
    
    // ========================================================================
    // Basic Output Methods
    // ========================================================================
    fprintf(stderr, "## Basic Output Methods (Short String: 14 bytes)\n\n");
    fprintf(stderr, "| Method | Time (ns) | Throughput | Description |\n");
    fprintf(stderr, "|--------|-----------|------------|-------------|\n");
    
    Benchmark basic_benchmarks[] = {
        {"printf(\"%s\")", bench_printf_short, "Format string with %%s", 0},
        {"printf(literal)", bench_printf_literal, "Literal string in printf", 0},
        {"puts()", bench_puts_short, "Simple string output (adds newline)", 0},
        {"fputs()", bench_fputs_short, "String to stdout stream", 0},
        {"fwrite()", bench_fwrite_short, "Binary write to stdout", 0},
        {"write()", bench_write_short, "Direct syscall to fd 1", 0},
        {"putchar loop", bench_putchar_loop, "Character by character", 0},
        {"fputc loop", bench_fputc_loop, "fputc for each char", 0},
    };
    
    int n_basic = sizeof(basic_benchmarks) / sizeof(basic_benchmarks[0]);
    for (int i = 0; i < n_basic; i++) {
        double ns = run_benchmark(&basic_benchmarks[i], 1);
        double ops_per_sec = 1000000000.0 / ns;
        char throughput[32];
        if (ops_per_sec >= 1000000) {
            snprintf(throughput, sizeof(throughput), "%.2fM/s", ops_per_sec / 1000000);
        } else if (ops_per_sec >= 1000) {
            snprintf(throughput, sizeof(throughput), "%.2fK/s", ops_per_sec / 1000);
        } else {
            snprintf(throughput, sizeof(throughput), "%.0f/s", ops_per_sec);
        }
        fprintf(stderr, "| %-16s | %9.2f | %10s | %s |\n", 
               basic_benchmarks[i].name, ns, throughput, basic_benchmarks[i].description);
    }
    fprintf(stderr, "\n");
    
    // ========================================================================
    // String Length Impact
    // ========================================================================
    fprintf(stderr, "## String Length Impact\n\n");
    fprintf(stderr, "| Method | Short (14B) | Medium (63B) | Long (142B) |\n");
    fprintf(stderr, "|--------|-------------|--------------|-------------|\n");
    
    // printf
    Benchmark printf_short = {"printf short", bench_printf_short, "", 0};
    Benchmark printf_medium = {"printf medium", bench_printf_medium, "", 0};
    Benchmark printf_long = {"printf long", bench_printf_long, "", 0};
    fprintf(stderr, "| printf | %.2f ns | %.2f ns | %.2f ns |\n",
           run_benchmark(&printf_short, 1),
           run_benchmark(&printf_medium, 1),
           run_benchmark(&printf_long, 1));
    
    // fputs
    Benchmark fputs_short = {"fputs short", bench_fputs_short, "", 0};
    Benchmark fputs_medium = {"fputs medium", bench_fputs_medium, "", 0};
    Benchmark fputs_long = {"fputs long", bench_fputs_long, "", 0};
    fprintf(stderr, "| fputs | %.2f ns | %.2f ns | %.2f ns |\n",
           run_benchmark(&fputs_short, 1),
           run_benchmark(&fputs_medium, 1),
           run_benchmark(&fputs_long, 1));
    
    // fwrite
    Benchmark fwrite_short = {"fwrite short", bench_fwrite_short, "", 0};
    Benchmark fwrite_medium = {"fwrite medium", bench_fwrite_medium, "", 0};
    Benchmark fwrite_long = {"fwrite long", bench_fwrite_long, "", 0};
    fprintf(stderr, "| fwrite | %.2f ns | %.2f ns | %.2f ns |\n",
           run_benchmark(&fwrite_short, 1),
           run_benchmark(&fwrite_medium, 1),
           run_benchmark(&fwrite_long, 1));
    
    // write
    Benchmark write_short = {"write short", bench_write_short, "", 0};
    Benchmark write_medium = {"write medium", bench_write_medium, "", 0};
    Benchmark write_long = {"write long", bench_write_long, "", 0};
    fprintf(stderr, "| write | %.2f ns | %.2f ns | %.2f ns |\n",
           run_benchmark(&write_short, 1),
           run_benchmark(&write_medium, 1),
           run_benchmark(&write_long, 1));
    fprintf(stderr, "\n");
    
    // ========================================================================
    // Formatted Output
    // ========================================================================
    fprintf(stderr, "## Formatted Output Comparison\n\n");
    fprintf(stderr, "| Method | Time (ns) | Description |\n");
    fprintf(stderr, "|--------|-----------|-------------|\n");
    
    Benchmark formatted_benchmarks[] = {
        {"printf %%d", bench_printf_formatted_int, "Single integer: \"Value: 12345\"", 0},
        {"printf %%.2f", bench_printf_formatted_float, "Single float: \"Value: 123.45\"", 0},
        {"printf %%s %%d", bench_printf_formatted_string, "String + int", 0},
        {"printf complex", bench_printf_formatted_complex, "Date/time + string + int + float", 0},
        {"fprintf", bench_fprintf_formatted, "fprintf to stdout", 0},
        {"dprintf", bench_dprintf_formatted, "dprintf to fd (bypasses buffer)", 0},
        {"snprintf+write", bench_snprintf_write, "Pre-format then syscall", 0},
        {"sprintf+write", bench_sprintf_write, "sprintf (no size check) + syscall", 0},
    };
    
    int n_formatted = sizeof(formatted_benchmarks) / sizeof(formatted_benchmarks[0]);
    for (int i = 0; i < n_formatted; i++) {
        double ns = run_benchmark(&formatted_benchmarks[i], 1);
        fprintf(stderr, "| %-16s | %9.2f | %s |\n", 
               formatted_benchmarks[i].name, ns, formatted_benchmarks[i].description);
    }
    fprintf(stderr, "\n");
    
    // ========================================================================
    // Buffer Mode Impact
    // ========================================================================
    fprintf(stderr, "## Buffer Mode Impact\n\n");
    fprintf(stderr, "| Buffer Mode | Time (ns) | Description |\n");
    fprintf(stderr, "|-------------|-----------|-------------|\n");
    
    Benchmark buffer_benchmarks[] = {
        {"Unbuffered", bench_unbuffered_printf, "_IONBF - Each write goes to kernel", 1},
        {"Line buffered", bench_linebuffered_printf, "_IOLBF - Flush on newline", 2},
        {"Fully buffered", bench_fullbuffered_printf, "_IOFBF - Flush when buffer full", 3},
    };
    
    int n_buffer = sizeof(buffer_benchmarks) / sizeof(buffer_benchmarks[0]);
    for (int i = 0; i < n_buffer; i++) {
        double ns = run_benchmark(&buffer_benchmarks[i], 1);
        fprintf(stderr, "| %-13s | %9.2f | %s |\n", 
               buffer_benchmarks[i].name, ns, buffer_benchmarks[i].description);
    }
    fprintf(stderr, "\n");
    
    // ========================================================================
    // Advanced Methods
    // ========================================================================
    fprintf(stderr, "## Advanced Methods\n\n");
    fprintf(stderr, "| Method | Time (ns) | Description |\n");
    fprintf(stderr, "|--------|-----------|-------------|\n");
    
    Benchmark advanced_benchmarks[] = {
        {"dprintf short", bench_dprintf_short, "Direct fd printf (short string)", 0},
        {"writev", bench_writev_multi, "Scatter/gather I/O (3 segments)", 0},
    };
    
    int n_advanced = sizeof(advanced_benchmarks) / sizeof(advanced_benchmarks[0]);
    for (int i = 0; i < n_advanced; i++) {
        double ns = run_benchmark(&advanced_benchmarks[i], 1);
        fprintf(stderr, "| %-14s | %9.2f | %s |\n", 
               advanced_benchmarks[i].name, ns, advanced_benchmarks[i].description);
    }
    fprintf(stderr, "\n");
    
    // ========================================================================
    // Summary
    // ========================================================================
    fprintf(stderr, "## Summary\n\n");
    fprintf(stderr, "### Fastest Methods by Use Case:\n\n");
    fprintf(stderr, "1. **Simple string output**: `fwrite()` or `fputs()` - minimal overhead\n");
    fprintf(stderr, "2. **Formatted output**: `printf()` - convenience vs small overhead\n");
    fprintf(stderr, "3. **High performance**: `write()` syscall - bypasses stdio buffering\n");
    fprintf(stderr, "4. **Multiple segments**: `writev()` - single syscall for multiple buffers\n");
    fprintf(stderr, "5. **Logging with format**: `snprintf()` + `write()` - pre-format for consistency\n\n");
    
    fprintf(stderr, "### Key Insights:\n\n");
    fprintf(stderr, "- `puts()` adds a newline automatically, slightly slower than `fputs()`\n");
    fprintf(stderr, "- Character-by-character output (`putchar`, `fputc`) is **very slow**\n");
    fprintf(stderr, "- Buffering mode significantly affects performance\n");
    fprintf(stderr, "- `dprintf()` bypasses stdio buffer, directly writes to fd\n");
    fprintf(stderr, "- String length has minimal impact for buffered output\n");
    
    return 0;
}
