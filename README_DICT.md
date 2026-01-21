# Dictionary<string, int> Benchmark (C/Linux x64)

Benchmark suite comparing different hash table implementations and hash functions for `Dictionary<string, int>` operations.

## Build

```bash
make dict
```

## Run

```bash
./bin/benchmark_dict
```

Or:
```bash
make run-dict
```

## Operations Tested

- **Insert** - Add a new key-value pair
- **Contains (hit)** - Check if an existing key is present
- **Contains (miss)** - Check if a non-existing key is present
- **Get (hit)** - Retrieve value for an existing key
- **Get (miss)** - Retrieve value for a non-existing key
- **Remove** - Delete a key-value pair

---

## Benchmark Results

**System:** AMD EPYC-Rome Processor  
**Iterations:** 65,535  
**Capacity:** 131,070 (load factor ~50%)

### Hash Table Implementation Comparison

| Implementation | Insert | Contains Hit | Contains Miss | Get Hit | Get Miss | Remove |
|----------------|--------|--------------|---------------|---------|----------|--------|
| chain_linked_list | 172.21 | 30.96 | 188.84 | 16.37 | 123.08 | 69.78 |
| open_linear_probe | 131.82 | 132.83 | 291.23 | 92.25 | 311.32 | 50.30 |
| **robin_hood** | **66.78** | **4.47** | **10.65** | **3.49** | **11.28** | **61.49** |

*All times in nanoseconds per operation*

#### Winner: Robin Hood Hashing

Robin Hood hashing is the clear winner for most operations:
- **7x faster** for Contains Hit (4.47 ns vs 30.96 ns)
- **18x faster** for Contains Miss (10.65 ns vs 188.84 ns)
- **5x faster** for Get Hit (3.49 ns vs 16.37 ns)
- **11x faster** for Get Miss (11.28 ns vs 123.08 ns)
- **2.6x faster** for Insert (66.78 ns vs 172.21 ns)

---

### Hash Function Comparison (using chaining)

| Hash Function | Insert | Contains Hit | Contains Miss | Get Hit | Get Miss | Remove |
|---------------|--------|--------------|---------------|---------|----------|--------|
| **hash_djb2** | **45.35** | **14.91** | 69.89 | 29.60 | 49.51 | **32.20** |
| hash_fnv1a | 85.58 | 28.50 | 63.00 | 31.98 | 84.95 | 47.90 |
| hash_sdbm | 43.35 | 17.91 | **42.16** | **16.28** | **36.85** | 35.89 |
| hash_murmur3_simple | 62.81 | 34.94 | 73.89 | 42.79 | 69.58 | 58.69 |

*All times in nanoseconds per operation*

#### Winner: DJB2 / SDBM

DJB2 and SDBM hash functions show the best performance:
- DJB2: Best for Insert and Contains Hit
- SDBM: Best for Contains Miss, Get Hit, Get Miss
- Both are simple and fast implementations

---

### Load Factor Impact (Chain with DJB2)

| Load Factor | Insert | Contains Hit | Contains Miss | Get Hit | Get Miss | Remove |
|-------------|--------|--------------|---------------|---------|----------|--------|
| ~10% | 117.44 | 14.29 | 78.85 | 12.77 | 40.37 | 28.55 |
| ~25% | 80.81 | 18.85 | 52.34 | 13.54 | 55.13 | 49.49 |
| **~50%** | **61.36** | **15.02** | **47.93** | **15.63** | **41.93** | **32.42** |
| ~75% | 41.53 | 16.84 | 49.84 | 14.53 | 59.03 | 38.40 |
| ~90% | 45.92 | 11.87 | 50.11 | 16.19 | 85.04 | 31.00 |

*All times in nanoseconds per operation*

#### Key Insights:
- **~50% load factor** provides the best balance
- Higher load factors increase collision chain length
- Lower load factors waste memory

---

### Key Length Impact (Chain with DJB2, 50% load)

| Key Length | Insert | Contains Hit | Get Hit |
|------------|--------|--------------|---------|
| 8 chars | 69.34 | 14.43 | 15.62 |
| 16 chars | 61.81 | 18.37 | 20.12 |
| 32 chars | 87.16 | 35.05 | 33.66 |
| 64 chars | 121.75 | 92.63 | 63.60 |
| 128 chars | 279.61 | 142.75 | 140.68 |
| 256 chars | 1000.25 | 429.52 | 292.74 |

*All times in nanoseconds per operation*

#### Key Insights:
- Performance degrades linearly with key length
- Hash computation and string comparison dominate for long keys
- Consider using hash caching for long keys

---

## Implementation Details

### Hash Table Implementations

1. **chain_linked_list** - Separate chaining with linked lists
   - Simple and reliable
   - No clustering issues
   - Memory overhead for pointers
   - Poor cache locality

2. **open_linear_probe** - Open addressing with linear probing
   - Better cache locality
   - Clustering can occur
   - Tombstone handling for deletion

3. **robin_hood** - Robin Hood hashing
   - Minimizes variance in probe sequence lengths
   - "Steal from the rich, give to the poor"
   - Backward shift deletion
   - Best overall performance

### Hash Functions

1. **DJB2** - Dan Bernstein's hash
   ```c
   hash = ((hash << 5) + hash) + c  // hash * 33 + c
   ```

2. **FNV-1a** - Fowler-Noll-Vo
   ```c
   hash ^= byte; hash *= FNV_prime;
   ```

3. **SDBM** - From SDBM database
   ```c
   hash = c + (hash << 6) + (hash << 16) - hash
   ```

4. **MurmurHash3 (simplified)** - Simplified version
   ```c
   h ^= *str; h *= 0x5bd1e995; h ^= h >> 15;
   ```

---

## Recommendations

| Use Case | Recommended | Notes |
|----------|-------------|-------|
| General purpose | Robin Hood + DJB2 | Best overall performance |
| Memory constrained | Open addressing | No linked list overhead |
| High load factor | Separate chaining | Graceful degradation |
| Long keys | Cache hash values | Avoid recomputation |
| Read-heavy | Robin Hood | Excellent lookup performance |
| Write-heavy | Chaining | Simpler insert/delete |

---

## Memory Usage

| Implementation | Per Entry Overhead | Notes |
|----------------|-------------------|-------|
| chain_linked_list | ~24 bytes | key ptr + value + next ptr |
| open_linear_probe | ~16 bytes | key ptr + value + flags |
| robin_hood | ~20 bytes | key ptr + value + hash + psl |

---

## Comparison with Other Languages

Typical dictionary performance in other languages (approximate):

| Language | Contains (ns) | Insert (ns) | Notes |
|----------|--------------|-------------|-------|
| **C (Robin Hood)** | **4-11** | **67** | This benchmark |
| C++ std::unordered_map | 30-50 | 100-150 | Depends on implementation |
| Rust HashMap | 20-40 | 80-120 | FxHash is faster |
| Go map | 40-80 | 100-200 | Runtime overhead |
| Python dict | 50-100 | 100-300 | Dynamic typing overhead |
| Java HashMap | 50-150 | 100-400 | JIT warmup dependent |

---

## License

Public Domain / MIT
