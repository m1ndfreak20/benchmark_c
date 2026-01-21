# Console Output Benchmark (C/Linux x64)

Benchmark suite comparing different methods of writing to console (stdout) in C.

## Build

```bash
make console
```

## Run

```bash
./bin/benchmark_console
```

Or:
```bash
make run-console
```

## Test String

The benchmark uses `printf("Hello, C!!!!\n");` as the base test case.

---

## Benchmark Results

**System:** AMD EPYC-Rome Processor  
**Iterations:** 10,000  
**Note:** Output redirected to `/dev/null` during benchmarks to measure pure overhead.

### Basic Output Methods (Short String: 14 bytes)

| Method | Time (ns) | Throughput | Description |
|--------|-----------|------------|-------------|
| printf("%s") | 179.09 | 5.58M/s | Format string with %s |
| printf(literal) | 2657.33 | 376.32K/s | Literal string in printf |
| puts() | 1424.55 | 701.98K/s | Simple string output (adds newline) |
| fputs() | 1734.07 | 576.68K/s | String to stdout stream |
| fwrite() | 1829.57 | 546.58K/s | Binary write to stdout |
| write() | 2281.14 | 438.38K/s | Direct syscall to fd 1 |
| putchar loop | 2300.61 | 434.67K/s | Character by character |
| fputc loop | 1662.79 | 601.40K/s | fputc for each char |

---

### String Length Impact

| Method | Short (14B) | Medium (63B) | Long (142B) |
|--------|-------------|--------------|-------------|
| printf | 1355.55 ns | 1378.81 ns | 2439.28 ns |
| fputs | 1054.18 ns | 1171.00 ns | 1715.52 ns |
| fwrite | 990.51 ns | 1012.50 ns | 1569.91 ns |
| write | 1247.49 ns | 890.06 ns | 1823.15 ns |

**Key Insight:** String length has moderate impact on performance. Longer strings take more time but the overhead per byte decreases.

---

### Formatted Output Comparison

| Method | Time (ns) | Description |
|--------|-----------|-------------|
| printf %d | 1829.71 | Single integer: "Value: 12345" |
| printf %.2f | 2927.55 | Single float: "Value: 123.45" |
| printf %s %d | 2386.51 | String + int |
| printf complex | 5252.02 | Date/time + string + int + float |
| fprintf | 4221.72 | fprintf to stdout |
| **dprintf** | **1586.01** | dprintf to fd (bypasses buffer) |
| snprintf+write | 1873.51 | Pre-format then syscall |
| **sprintf+write** | **1463.28** | sprintf (no size check) + syscall |

**Winner:** `sprintf` + `write` combo for formatted output with known buffer size.

---

### Buffer Mode Impact

| Buffer Mode | Time (ns) | Description |
|-------------|-----------|-------------|
| Unbuffered | 1905.92 | _IONBF - Each write goes to kernel |
| Line buffered | 2811.78 | _IOLBF - Flush on newline |
| **Fully buffered** | **28.99** | _IOFBF - Flush when buffer full |

**Key Insight:** Fully buffered mode is **~97x faster** than line buffered! This is because data accumulates in the buffer and only syscalls when the buffer is full.

---

### Advanced Methods

| Method | Time (ns) | Description |
|--------|-----------|-------------|
| dprintf | 1752.39 | Direct fd printf (short string) |
| writev | 1803.12 | Scatter/gather I/O (3 segments) |

---

## Summary

### Fastest Methods by Use Case

| Use Case | Recommended | Time (ns) | Notes |
|----------|-------------|-----------|-------|
| Simple string | `fputs()` / `fwrite()` | ~1000 | Minimal parsing overhead |
| Formatted output | `sprintf()` + `write()` | ~1500 | Pre-format, then single syscall |
| High throughput | Fully buffered | ~30 | Set `setvbuf(stdout, buf, _IOFBF, BUFSIZ)` |
| Real-time logging | `dprintf()` | ~1600 | Bypasses stdio buffer |
| Multiple segments | `writev()` | ~1800 | Single syscall for multiple buffers |

### Performance Tips

1. **Use full buffering for batch output**
   ```c
   char buf[BUFSIZ];
   setvbuf(stdout, buf, _IOFBF, BUFSIZ);
   ```
   This provides ~97x speedup over line buffering.

2. **Avoid character-by-character output**
   ```c
   // SLOW (~2300 ns)
   for (char *p = str; *p; p++) putchar(*p);
   
   // FAST (~1000 ns)
   fputs(str, stdout);
   ```

3. **For formatted strings, pre-format then write**
   ```c
   char buf[256];
   int len = sprintf(buf, "Value: %d\n", value);
   write(STDOUT_FILENO, buf, len);
   ```

4. **Use `dprintf()` for direct file descriptor output**
   ```c
   dprintf(STDOUT_FILENO, "Value: %d\n", value);
   ```

5. **For multiple related writes, use `writev()`**
   ```c
   struct iovec iov[3];
   iov[0] = (struct iovec){"[INFO] ", 7};
   iov[1] = (struct iovec){message, msg_len};
   iov[2] = (struct iovec){"\n", 1};
   writev(STDOUT_FILENO, iov, 3);
   ```

---

## Function Reference

| Function | Header | Description |
|----------|--------|-------------|
| `printf()` | stdio.h | Formatted output to stdout |
| `fprintf()` | stdio.h | Formatted output to stream |
| `dprintf()` | stdio.h | Formatted output to file descriptor |
| `puts()` | stdio.h | Write string + newline to stdout |
| `fputs()` | stdio.h | Write string to stream |
| `fwrite()` | stdio.h | Write binary data to stream |
| `write()` | unistd.h | Write to file descriptor (syscall) |
| `writev()` | sys/uio.h | Scatter/gather write (syscall) |
| `putchar()` | stdio.h | Write single character to stdout |
| `fputc()` | stdio.h | Write single character to stream |

---

## License

Public Domain / MIT
