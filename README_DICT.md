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
**Iterations:** 1,000,000  
**Capacity:** 2,000,000 (load factor ~50%)

### Hash Table Implementation Comparison

| Implementation | Insert | Contains Hit | Contains Miss | Get Hit | Get Miss | Remove |
|----------------|--------|--------------|---------------|---------|----------|--------|
| chain_linked_list | 208.70 | 62.10 | 195.54 | 20.55 | 173.65 | 40.20 |
| open_linear_probe | 93.76 | 49.67 | 339.78 | 45.31 | 301.50 | 32.34 |
| **robin_hood** | **68.76** | **4.77** | **15.77** | **3.93** | **12.39** | 45.94 |

*All times in nanoseconds per operation*

#### Winner: Robin Hood Hashing

Robin Hood hashing is the clear winner for most operations:
- **13x faster** for Contains Hit (4.77 ns vs 62.10 ns)
- **12x faster** for Contains Miss (15.77 ns vs 195.54 ns)
- **5x faster** for Get Hit (3.93 ns vs 20.55 ns)
- **14x faster** for Get Miss (12.39 ns vs 173.65 ns)
- **3x faster** for Insert (68.76 ns vs 208.70 ns)

---

### Hash Function Comparison (using chaining)

| Hash Function | Insert | Contains Hit | Contains Miss | Get Hit | Get Miss | Remove |
|---------------|--------|--------------|---------------|---------|----------|--------|
| **hash_djb2** | **62.48** | **20.54** | 154.25 | **21.72** | 169.95 | **41.42** |
| hash_fnv1a | 214.39 | 77.28 | 155.82 | 77.88 | **148.36** | 196.17 |
| hash_sdbm | 81.08 | 46.92 | 204.87 | 44.43 | 191.53 | 69.04 |
| hash_murmur3_simple | 223.45 | 117.31 | 239.45 | 120.82 | 260.51 | 218.75 |

*All times in nanoseconds per operation*

#### Winner: DJB2

DJB2 hash function is fastest for most operations:
- Simple implementation
- Good distribution for string keys
- Widely used and battle-tested

---

### Load Factor Impact (Chain with DJB2)

| Load Factor | Insert | Contains Hit | Contains Miss | Get Hit | Get Miss | Remove |
|-------------|--------|--------------|---------------|---------|----------|--------|
| ~10% | 35.38 | 12.97 | 96.93 | 17.72 | 56.73 | 29.80 |
| ~25% | 48.76 | 13.27 | 72.80 | 13.17 | 39.83 | 32.69 |
| **~50%** | **61.28** | **27.00** | **57.44** | **14.87** | **47.73** | **31.79** |
| ~75% | 51.93 | 14.70 | 48.17 | 15.10 | 51.14 | 64.74 |
| ~90% | 45.67 | 12.44 | 56.22 | 11.96 | 92.43 | 28.94 |

*All times in nanoseconds per operation*

#### Key Insights:
- **~50% load factor** provides the best balance
- Higher load factors increase collision chain length
- Lower load factors waste memory

---

### Key Length Impact (Chain with DJB2, 50% load)

| Key Length | Insert | Contains Hit | Get Hit |
|------------|--------|--------------|---------|
| 8 chars | 42.81 | 14.04 | 14.52 |
| 16 chars | 43.20 | 20.06 | 20.78 |
| 32 chars | 56.86 | 35.87 | 32.47 |
| 64 chars | 102.10 | 90.95 | 73.20 |
| 128 chars | 181.37 | 126.06 | 134.28 |
| 256 chars | 300.45 | 314.50 | 295.86 |

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
| **C (Robin Hood)** | **4-16** | **69** | This benchmark |
| C++ std::unordered_map | 30-50 | 100-150 | Depends on implementation |
| Rust HashMap | 20-40 | 80-120 | FxHash is faster |
| Go map | 40-80 | 100-200 | Runtime overhead |
| Python dict | 50-100 | 100-300 | Dynamic typing overhead |
| Java HashMap | 50-150 | 100-400 | JIT warmup dependent |

---

## License

Public Domain / MIT
