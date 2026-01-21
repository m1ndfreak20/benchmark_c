# C Benchmarks for Linux x64

This repository contains benchmark suites for common C operations on Linux x64.

## Available Benchmarks

| Benchmark | File | Description |
|-----------|------|-------------|
| [DateTime String](README_DATETIME.md) | `benchmark` | 40 methods for datetime string formatting |
| [Dictionary Operations](README_DICT.md) | `benchmark_dict` | Hash table implementations and operations |

## Quick Build & Run

```bash
# Build all
make all

# Run datetime benchmark
./bin/benchmark

# Run dictionary benchmark  
./bin/benchmark_dict
```

---

# DateTime String Benchmark (C/Linux x64)

Benchmark suite comparing 40 different methods for generating datetime strings in the format `[ HH:MM:SS:mmm.uuu ]` (hours:minutes:seconds:milliseconds.microseconds).

## Build

```bash
make datetime
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

### Full Results Table

| # | Benchmark | Total (ms) | Per call (ns) | Calls/sec | Description |
|---|-----------|------------|---------------|-----------|-------------|
| 1 | coarse_cached | 11.07 | 11.07 | 90,372,786 | CLOCK_REALTIME_COARSE + cached |
| 2 | minimal_gettimeofday | 37.02 | 37.02 | 27,014,621 | Minimal with full lookup (cached) |
| 3 | precomputed_memcpy | 40.01 | 40.01 | 24,990,779 | Precomputed string + memcpy (cached) |
| 4 | cached_localtime | 40.87 | 40.87 | 24,470,635 | Cached localtime_r() |
| 5 | clock_cached_gm | 41.10 | 41.10 | 24,332,081 | clock_gettime + cached gmtime |
| 6 | monotonic_relative | 41.39 | 41.39 | 24,161,213 | CLOCK_MONOTONIC relative |
| 7 | fully_cached | 42.61 | 42.61 | 23,469,500 | Fully cached (only us updated) |
| 8 | cached_full_lookup | 44.74 | 44.74 | 22,349,119 | Cached localtime + full lookup |
| 9 | coarse_nocache | 52.76 | 52.76 | 18,952,236 | CLOCK_REALTIME_COARSE + no cache |
| 10 | clock_nocache_gm | 61.42 | 61.42 | 16,280,748 | clock_gettime + gmtime (no cache) |
| 11 | minimal_nocache | 66.60 | 66.60 | 15,015,089 | Minimal with full lookup (no cache) |
| 12 | gmtime_manual | 67.73 | 67.73 | 14,764,420 | gmtime_r() + manual digits (UTC) |
| 13 | cached_gmtime | 69.47 | 69.47 | 14,394,227 | Cached gmtime_r() (UTC) |
| 14 | nocache_localtime | 70.35 | 70.35 | 14,214,344 | Non-cached localtime_r() + lookup |
| 15 | batch_read | 72.54 | 72.54 | 13,785,081 | Batched time read pattern |
| 16 | nocache_precomputed | 73.85 | 73.85 | 13,541,820 | Precomputed string + memcpy (no cache) |
| 17 | nocache_fully | 75.67 | 75.67 | 13,215,062 | Non-cached full string build |
| 18 | inline_all | 76.01 | 76.01 | 13,155,537 | All calculations inline |
| 19 | template_copy | 76.39 | 76.39 | 13,089,930 | Template memcpy + digit fill |
| 20 | manual_digits | 78.15 | 78.15 | 12,795,439 | Manual digit conversion |
| 21 | uint64_write | 78.88 | 78.88 | 12,677,967 | Direct byte-by-byte write |
| 22 | lookup_table | 79.10 | 79.10 | 12,642,698 | 2-digit lookup table |
| 23 | nocache_gmtime | 79.95 | 79.95 | 12,508,019 | Non-cached gmtime_r() + lookup (UTC) |
| 24 | nocache_full_lookup | 81.08 | 81.08 | 12,333,235 | Non-cached localtime + full lookup |
| 25 | clock_manual_digits | 84.91 | 84.91 | 11,777,279 | clock_gettime + manual digits |
| 26 | time_gettimeofday_hybrid | 85.42 | 85.42 | 11,707,081 | time() + gettimeofday() hybrid |
| 27 | full_lookup_tables | 86.57 | 86.57 | 11,551,313 | Full 2+3 digit lookup tables |
| 28 | div_optimization | 95.41 | 95.41 | 10,481,169 | Division optimization |
| 29 | time_only_snprintf | 195.98 | 195.98 | 5,102,442 | Time only (no sub-second) |
| 30 | strftime_clock | 225.27 | 225.27 | 4,439,177 | strftime() + clock_gettime() |
| 31 | clock_realtime_coarse | 227.42 | 227.42 | 4,397,238 | clock_gettime(CLOCK_REALTIME_COARSE) + snprintf() |
| 32 | gmtime_snprintf | 239.53 | 239.53 | 4,174,866 | gmtime_r() + snprintf() (UTC) |
| 33 | clock_realtime_snprintf | 249.54 | 249.54 | 4,007,324 | clock_gettime(CLOCK_REALTIME) + snprintf() |
| 34 | sprintf_gettimeofday | 265.48 | 265.48 | 3,766,743 | sprintf() + gettimeofday() (no size check) |
| 35 | multiple_snprintf | 285.53 | 285.53 | 3,502,263 | Multiple snprintf() calls |
| 36 | asprintf | 287.02 | 287.02 | 3,484,023 | asprintf() dynamic allocation |
| 37 | snprintf_gettimeofday | 295.47 | 295.47 | 3,384,478 | snprintf() + gettimeofday() |
| 38 | strftime_gettimeofday | 326.30 | 326.30 | 3,064,673 | strftime() + gettimeofday() basic |
| 39 | syscall_gettimeofday | 383.52 | 383.52 | 2,607,434 | Direct syscall(SYS_gettimeofday) |
| 40 | strcat_chain | 414.29 | 414.29 | 2,413,789 | strcat() chain |

---

## Cached vs Non-Cached Comparison

This table directly compares cached and non-cached versions of the same algorithm:

| Method | Cached (ns) | No Cache (ns) | Speedup | Cache Benefit |
|--------|-------------|---------------|---------|---------------|
| coarse (CLOCK_REALTIME_COARSE) | 11.07 | 52.76 | **4.8x** | 41.69 ns saved |
| minimal_gettimeofday | 37.02 | 66.60 | **1.8x** | 29.58 ns saved |
| precomputed_memcpy | 40.01 | 73.85 | **1.8x** | 33.84 ns saved |
| localtime + lookup | 40.87 | 70.35 | **1.7x** | 29.48 ns saved |
| clock + gmtime | 41.10 | 61.42 | **1.5x** | 20.32 ns saved |
| fully_cached | 42.61 | 75.67 | **1.8x** | 33.06 ns saved |
| full_lookup | 44.74 | 81.08 | **1.8x** | 36.34 ns saved |
| gmtime + lookup | 69.47 | 79.95 | **1.2x** | 10.48 ns saved |

### Key Insights:

1. **Caching provides 1.2x-4.8x speedup** depending on the time source
2. **CLOCK_REALTIME_COARSE benefits most from caching** (4.8x faster) because the clock read itself is very fast
3. **localtime_r() is expensive** (~30 ns overhead per call)
4. **gmtime_r() is cheaper than localtime_r()** due to no timezone conversion

---

## Performance Categories

### Ultra-Fast (< 50 ns/call) - Cached Methods

| Method | ns/call | Calls/sec | Notes |
|--------|---------|-----------|-------|
| coarse_cached | 11.07 | 90M | Fastest, but ~1-4ms resolution |
| minimal_gettimeofday | 37.02 | 27M | Best for high-precision logging |
| precomputed_memcpy | 40.01 | 25M | Good balance |
| cached_localtime | 40.87 | 24M | Simple and effective |

### Fast (50-100 ns/call) - Non-Cached Methods

| Method | ns/call | Calls/sec | Notes |
|--------|---------|-----------|-------|
| coarse_nocache | 52.76 | 19M | Fast even without cache |
| clock_nocache_gm | 61.42 | 16M | UTC, no cache needed |
| minimal_nocache | 66.60 | 15M | Good non-cached option |
| nocache_localtime | 70.35 | 14M | Standard approach |

### Standard (100-300 ns/call) - Library Functions

| Method | ns/call | Calls/sec | Notes |
|--------|---------|-----------|-------|
| snprintf_gettimeofday | 295.47 | 3.4M | Safe and portable |
| sprintf_gettimeofday | 265.48 | 3.8M | Slightly faster |
| strftime_gettimeofday | 326.30 | 3.1M | Most flexible |

### Slow (> 300 ns/call)

| Method | ns/call | Calls/sec | Notes |
|--------|---------|-----------|-------|
| syscall_gettimeofday | 383.52 | 2.6M | Direct syscall overhead |
| strcat_chain | 414.29 | 2.4M | Multiple string operations |

---

## Recommendations

| Use Case | Recommended Method | Performance | Accuracy |
|----------|-------------------|-------------|----------|
| High-frequency logging (millions/sec) | `coarse_cached` | ~90M calls/sec | ~1-4ms |
| Precise timestamps with caching | `minimal_gettimeofday` | ~27M calls/sec | ~1us |
| Precise timestamps without caching | `coarse_nocache` | ~19M calls/sec | ~1-4ms |
| UTC timestamps | `clock_cached_gm` | ~24M calls/sec | ~1us |
| Simple/portable code | `snprintf_gettimeofday` | ~3.4M calls/sec | ~1us |
| Maximum portability | `strftime_gettimeofday` | ~3.1M calls/sec | ~1us |

---

## Technical Notes

### Why Caching Works

- `localtime_r()` performs timezone conversion which requires reading `/etc/localtime` and computing DST
- Caching the `struct tm` result avoids this overhead when the second hasn't changed
- Most logging scenarios have many calls within the same second

### CLOCK_REALTIME_COARSE

- Returns cached kernel time (updated every timer tick, typically 1-4ms)
- Much faster than CLOCK_REALTIME (~5ns vs ~20ns for the clock read alone)
- Trade-off: lower time resolution

### vDSO Acceleration

- `gettimeofday()` and `clock_gettime()` use vDSO to avoid syscall overhead
- Direct `syscall(SYS_gettimeofday)` bypasses this, making it slower

### Lookup Tables

- Pre-computed digit strings avoid division operations
- 2-digit table: 200 bytes for "00"-"99"
- 3-digit table: 4000 bytes for "000"-"999"

---

## Benchmark Descriptions

1. **strftime_gettimeofday**: strftime() + gettimeofday() basic
2. **snprintf_gettimeofday**: snprintf() + gettimeofday()
3. **sprintf_gettimeofday**: sprintf() + gettimeofday() (no size check)
4. **clock_realtime_snprintf**: clock_gettime(CLOCK_REALTIME) + snprintf()
5. **clock_realtime_coarse**: clock_gettime(CLOCK_REALTIME_COARSE) + snprintf()
6. **manual_digits**: Manual digit conversion
7. **lookup_table**: 2-digit lookup table
8. **syscall_gettimeofday**: Direct syscall(SYS_gettimeofday)
9. **cached_localtime**: Cached localtime_r()
10. **nocache_localtime**: Non-cached localtime_r() + lookup
11. **template_copy**: Template memcpy + digit fill
12. **clock_manual_digits**: clock_gettime + manual digits
13. **gmtime_snprintf**: gmtime_r() + snprintf() (UTC)
14. **gmtime_manual**: gmtime_r() + manual digits (UTC)
15. **cached_gmtime**: Cached gmtime_r() (UTC)
16. **nocache_gmtime**: Non-cached gmtime_r() + lookup (UTC)
17. **time_gettimeofday_hybrid**: time() + gettimeofday() hybrid
18. **time_only_snprintf**: Time only (no sub-second)
19. **full_lookup_tables**: Full 2+3 digit lookup tables
20. **cached_full_lookup**: Cached localtime + full lookup
21. **nocache_full_lookup**: Non-cached localtime + full lookup
22. **coarse_cached**: CLOCK_REALTIME_COARSE + cached
23. **coarse_nocache**: CLOCK_REALTIME_COARSE + no cache
24. **asprintf**: asprintf() dynamic allocation
25. **strcat_chain**: strcat() chain
26. **multiple_snprintf**: Multiple snprintf() calls
27. **precomputed_memcpy**: Precomputed string + memcpy (cached)
28. **nocache_precomputed**: Precomputed string + memcpy (no cache)
29. **uint64_write**: Direct byte-by-byte write
30. **inline_all**: All calculations inline
31. **div_optimization**: Division optimization
32. **fully_cached**: Fully cached (only us updated)
33. **nocache_fully**: Non-cached full string build
34. **clock_cached_gm**: clock_gettime + cached gmtime
35. **clock_nocache_gm**: clock_gettime + gmtime (no cache)
36. **strftime_clock**: strftime() + clock_gettime()
37. **minimal_gettimeofday**: Minimal with full lookup (cached)
38. **minimal_nocache**: Minimal with full lookup (no cache)
39. **monotonic_relative**: CLOCK_MONOTONIC relative
40. **batch_read**: Batched time read pattern

## License

Public Domain / MIT
