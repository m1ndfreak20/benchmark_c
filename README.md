# DateTime String Benchmark (C/Linux x64)

Benchmark suite comparing 32 different methods for generating datetime strings in the format `[ HH:MM:SS:mmm.uuu ]` (hours:minutes:seconds:milliseconds.microseconds).

## Build

```bash
make
```

## Run

```bash
./bin/benchmark
```

Or:
```bash
make run
```

## Output Format

```
[ 18:16:57:335.435 ]
```

Where:
- `HH` - hours (00-23)
- `MM` - minutes (00-59)
- `SS` - seconds (00-59)
- `mmm` - milliseconds (000-999)
- `uuu` - microseconds (000-999)

## Benchmark Results

**System:** AMD EPYC-Rome Processor  
**Iterations:** 1,000,000  
**Warmup:** 10,000

| # | Benchmark | Total (ms) | Per call (ns) | Calls/sec | Description |
|---|-----------|------------|---------------|-----------|-------------|
| 1 | coarse_cached | 9.98 | 9.98 | 100,234,448 | CLOCK_REALTIME_COARSE + cached |
| 2 | cached_full_lookup | 38.57 | 38.57 | 25,926,658 | Cached localtime + full lookup |
| 3 | cached_gmtime | 40.30 | 40.30 | 24,811,628 | Cached gmtime_r() (UTC) |
| 4 | precomputed_memcpy | 40.82 | 40.82 | 24,495,960 | Precomputed string + memcpy |
| 5 | fully_cached | 41.69 | 41.69 | 23,989,180 | Fully cached (only us updated) |
| 6 | minimal_gettimeofday | 41.75 | 41.75 | 23,954,728 | Minimal with full lookup |
| 7 | monotonic_relative | 42.16 | 42.16 | 23,717,158 | CLOCK_MONOTONIC relative |
| 8 | clock_cached_gm | 42.15 | 42.15 | 23,725,767 | clock_gettime + cached gmtime |
| 9 | cached_localtime | 43.27 | 43.27 | 23,110,537 | Cached localtime_r() |
| 10 | gmtime_manual | 64.27 | 64.27 | 15,560,087 | gmtime_r() + manual digits (UTC) |
| 11 | time_gettimeofday_hybrid | 69.06 | 69.06 | 14,479,402 | time() + gettimeofday() hybrid |
| 12 | batch_read | 69.77 | 69.77 | 14,332,978 | Batched time read pattern |
| 13 | lookup_table | 75.57 | 75.57 | 13,232,381 | 2-digit lookup table |
| 14 | template_copy | 76.50 | 76.50 | 13,072,271 | Template memcpy + digit fill |
| 15 | clock_manual_digits | 78.24 | 78.24 | 12,781,853 | clock_gettime + manual digits |
| 16 | manual_digits | 79.03 | 79.03 | 12,653,412 | Manual digit conversion |
| 17 | uint64_write | 81.26 | 81.26 | 12,306,232 | Direct byte-by-byte write |
| 18 | full_lookup_tables | 84.33 | 84.33 | 11,857,631 | Full 2+3 digit lookup tables |
| 19 | inline_all | 85.77 | 85.77 | 11,659,749 | All calculations inline |
| 20 | div_optimization | 93.64 | 93.64 | 10,678,688 | Division optimization |
| 21 | time_only_snprintf | 180.22 | 180.22 | 5,548,921 | Time only (no sub-second) |
| 22 | strftime_clock | 229.30 | 229.30 | 4,361,153 | strftime() + clock_gettime() |
| 23 | clock_realtime_coarse | 239.78 | 239.78 | 4,170,437 | clock_gettime(CLOCK_REALTIME_COARSE) + snprintf() |
| 24 | snprintf_gettimeofday | 245.73 | 245.73 | 4,069,442 | snprintf() + gettimeofday() |
| 25 | sprintf_gettimeofday | 245.10 | 245.10 | 4,080,017 | sprintf() + gettimeofday() (no size check) |
| 26 | gmtime_snprintf | 251.14 | 251.14 | 3,981,898 | gmtime_r() + snprintf() (UTC) |
| 27 | clock_realtime_snprintf | 255.19 | 255.19 | 3,918,580 | clock_gettime(CLOCK_REALTIME) + snprintf() |
| 28 | asprintf | 292.10 | 292.10 | 3,423,445 | asprintf() dynamic allocation |
| 29 | multiple_snprintf | 335.89 | 335.89 | 2,977,175 | Multiple snprintf() calls |
| 30 | strftime_gettimeofday | 365.53 | 365.53 | 2,735,774 | strftime() + gettimeofday() basic |
| 31 | strcat_chain | 370.24 | 370.24 | 2,700,933 | strcat() chain |
| 32 | syscall_gettimeofday | 374.14 | 374.14 | 2,672,778 | Direct syscall(SYS_gettimeofday) |

## Key Findings

### Fastest Methods (< 50 ns/call)

1. **coarse_cached** (~10 ns) - Uses `CLOCK_REALTIME_COARSE` with cached `localtime`. Fastest but with ~1-4ms time resolution.
2. **cached_full_lookup** (~39 ns) - Caches `localtime_r()` result and uses lookup tables for digit conversion.
3. **cached_gmtime** (~40 ns) - Similar to above but uses UTC time.

### Balanced Performance (50-100 ns/call)

- **gmtime_manual** (~64 ns) - Good for UTC timestamps without caching
- **lookup_table** (~76 ns) - No caching, uses 2-digit lookup table
- **manual_digits** (~79 ns) - Simple manual digit conversion

### Standard Library Methods (> 200 ns/call)

- **snprintf** methods (~245 ns) - Standard and safe
- **strftime** methods (~365 ns) - Most flexible but slowest
- **asprintf** (~292 ns) - Dynamic allocation overhead

## Recommendations

| Use Case | Recommended Method | Performance |
|----------|-------------------|-------------|
| High-frequency logging | `coarse_cached` | ~100M calls/sec |
| Precise timestamps | `cached_localtime` | ~23M calls/sec |
| UTC timestamps | `cached_gmtime` | ~25M calls/sec |
| Simple/portable code | `snprintf_gettimeofday` | ~4M calls/sec |
| Maximum portability | `strftime_gettimeofday` | ~2.7M calls/sec |

## Notes

- Cached methods update the time structure once per second, reducing `localtime_r()`/`gmtime_r()` calls
- `CLOCK_REALTIME_COARSE` has lower resolution (~1-4ms) but is faster
- Direct syscall is slower than vDSO-accelerated `gettimeofday()`
- Lookup tables provide consistent speedup over manual division

## License

Public Domain / MIT
