#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>

#define ITERATIONS 65535
#define WARMUP_ITERATIONS 10000
#define INITIAL_CAPACITY 16
#define LOAD_FACTOR 0.75

// ============================================================================
// High-resolution timing
// ============================================================================
static inline uint64_t get_nanos(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

// ============================================================================
// Hash Table Implementation 1: Simple chaining with linked list
// ============================================================================
typedef struct ChainNode {
    char *key;
    int value;
    struct ChainNode *next;
} ChainNode;

typedef struct {
    ChainNode **buckets;
    size_t capacity;
    size_t size;
} ChainHashTable;

static uint32_t hash_djb2(const char *str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

static uint32_t hash_fnv1a(const char *str) {
    uint32_t hash = 2166136261u;
    while (*str) {
        hash ^= (uint8_t)*str++;
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t hash_sdbm(const char *str) {
    uint32_t hash = 0;
    int c;
    while ((c = *str++))
        hash = c + (hash << 6) + (hash << 16) - hash;
    return hash;
}

static uint32_t hash_murmur3_simple(const char *str) {
    uint32_t h = 0;
    for (; *str; ++str) {
        h ^= *str;
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }
    return h;
}

// Chain hash table functions
ChainHashTable* chain_create(size_t capacity) {
    ChainHashTable *ht = malloc(sizeof(ChainHashTable));
    ht->buckets = calloc(capacity, sizeof(ChainNode*));
    ht->capacity = capacity;
    ht->size = 0;
    return ht;
}

void chain_destroy(ChainHashTable *ht) {
    for (size_t i = 0; i < ht->capacity; i++) {
        ChainNode *node = ht->buckets[i];
        while (node) {
            ChainNode *next = node->next;
            free(node->key);
            free(node);
            node = next;
        }
    }
    free(ht->buckets);
    free(ht);
}

void chain_insert(ChainHashTable *ht, const char *key, int value) {
    uint32_t hash = hash_djb2(key);
    size_t idx = hash % ht->capacity;
    
    // Check if key exists
    ChainNode *node = ht->buckets[idx];
    while (node) {
        if (strcmp(node->key, key) == 0) {
            node->value = value;
            return;
        }
        node = node->next;
    }
    
    // Insert new node
    ChainNode *new_node = malloc(sizeof(ChainNode));
    new_node->key = strdup(key);
    new_node->value = value;
    new_node->next = ht->buckets[idx];
    ht->buckets[idx] = new_node;
    ht->size++;
}

bool chain_contains(ChainHashTable *ht, const char *key) {
    uint32_t hash = hash_djb2(key);
    size_t idx = hash % ht->capacity;
    
    ChainNode *node = ht->buckets[idx];
    while (node) {
        if (strcmp(node->key, key) == 0)
            return true;
        node = node->next;
    }
    return false;
}

int chain_get(ChainHashTable *ht, const char *key, int default_val) {
    uint32_t hash = hash_djb2(key);
    size_t idx = hash % ht->capacity;
    
    ChainNode *node = ht->buckets[idx];
    while (node) {
        if (strcmp(node->key, key) == 0)
            return node->value;
        node = node->next;
    }
    return default_val;
}

bool chain_remove(ChainHashTable *ht, const char *key) {
    uint32_t hash = hash_djb2(key);
    size_t idx = hash % ht->capacity;
    
    ChainNode *prev = NULL;
    ChainNode *node = ht->buckets[idx];
    while (node) {
        if (strcmp(node->key, key) == 0) {
            if (prev)
                prev->next = node->next;
            else
                ht->buckets[idx] = node->next;
            free(node->key);
            free(node);
            ht->size--;
            return true;
        }
        prev = node;
        node = node->next;
    }
    return false;
}

// ============================================================================
// Hash Table Implementation 2: Open addressing with linear probing
// ============================================================================
typedef struct {
    char *key;
    int value;
    bool occupied;
    bool deleted;
} OpenEntry;

typedef struct {
    OpenEntry *entries;
    size_t capacity;
    size_t size;
} OpenHashTable;

OpenHashTable* open_create(size_t capacity) {
    OpenHashTable *ht = malloc(sizeof(OpenHashTable));
    ht->entries = calloc(capacity, sizeof(OpenEntry));
    ht->capacity = capacity;
    ht->size = 0;
    return ht;
}

void open_destroy(OpenHashTable *ht) {
    for (size_t i = 0; i < ht->capacity; i++) {
        if (ht->entries[i].key)
            free(ht->entries[i].key);
    }
    free(ht->entries);
    free(ht);
}

void open_insert(OpenHashTable *ht, const char *key, int value) {
    uint32_t hash = hash_djb2(key);
    size_t idx = hash % ht->capacity;
    
    for (size_t i = 0; i < ht->capacity; i++) {
        size_t probe = (idx + i) % ht->capacity;
        if (!ht->entries[probe].occupied || ht->entries[probe].deleted) {
            if (ht->entries[probe].key)
                free(ht->entries[probe].key);
            ht->entries[probe].key = strdup(key);
            ht->entries[probe].value = value;
            ht->entries[probe].occupied = true;
            ht->entries[probe].deleted = false;
            ht->size++;
            return;
        }
        if (strcmp(ht->entries[probe].key, key) == 0) {
            ht->entries[probe].value = value;
            return;
        }
    }
}

bool open_contains(OpenHashTable *ht, const char *key) {
    uint32_t hash = hash_djb2(key);
    size_t idx = hash % ht->capacity;
    
    for (size_t i = 0; i < ht->capacity; i++) {
        size_t probe = (idx + i) % ht->capacity;
        if (!ht->entries[probe].occupied && !ht->entries[probe].deleted)
            return false;
        if (ht->entries[probe].occupied && !ht->entries[probe].deleted &&
            strcmp(ht->entries[probe].key, key) == 0)
            return true;
    }
    return false;
}

int open_get(OpenHashTable *ht, const char *key, int default_val) {
    uint32_t hash = hash_djb2(key);
    size_t idx = hash % ht->capacity;
    
    for (size_t i = 0; i < ht->capacity; i++) {
        size_t probe = (idx + i) % ht->capacity;
        if (!ht->entries[probe].occupied && !ht->entries[probe].deleted)
            return default_val;
        if (ht->entries[probe].occupied && !ht->entries[probe].deleted &&
            strcmp(ht->entries[probe].key, key) == 0)
            return ht->entries[probe].value;
    }
    return default_val;
}

bool open_remove(OpenHashTable *ht, const char *key) {
    uint32_t hash = hash_djb2(key);
    size_t idx = hash % ht->capacity;
    
    for (size_t i = 0; i < ht->capacity; i++) {
        size_t probe = (idx + i) % ht->capacity;
        if (!ht->entries[probe].occupied && !ht->entries[probe].deleted)
            return false;
        if (ht->entries[probe].occupied && !ht->entries[probe].deleted &&
            strcmp(ht->entries[probe].key, key) == 0) {
            ht->entries[probe].deleted = true;
            ht->size--;
            return true;
        }
    }
    return false;
}

// ============================================================================
// Hash Table Implementation 3: Robin Hood hashing
// ============================================================================
typedef struct {
    char *key;
    int value;
    uint32_t hash;
    int psl; // probe sequence length
} RobinEntry;

typedef struct {
    RobinEntry *entries;
    size_t capacity;
    size_t size;
} RobinHashTable;

RobinHashTable* robin_create(size_t capacity) {
    RobinHashTable *ht = malloc(sizeof(RobinHashTable));
    ht->entries = calloc(capacity, sizeof(RobinEntry));
    for (size_t i = 0; i < capacity; i++)
        ht->entries[i].psl = -1;
    ht->capacity = capacity;
    ht->size = 0;
    return ht;
}

void robin_destroy(RobinHashTable *ht) {
    for (size_t i = 0; i < ht->capacity; i++) {
        if (ht->entries[i].psl >= 0 && ht->entries[i].key)
            free(ht->entries[i].key);
    }
    free(ht->entries);
    free(ht);
}

void robin_insert(RobinHashTable *ht, const char *key, int value) {
    uint32_t hash = hash_djb2(key);
    size_t idx = hash % ht->capacity;
    
    RobinEntry entry = {strdup(key), value, hash, 0};
    
    for (size_t i = 0; i < ht->capacity; i++) {
        size_t probe = (idx + i) % ht->capacity;
        
        if (ht->entries[probe].psl < 0) {
            ht->entries[probe] = entry;
            ht->size++;
            return;
        }
        
        if (ht->entries[probe].hash == hash && 
            strcmp(ht->entries[probe].key, key) == 0) {
            ht->entries[probe].value = value;
            free(entry.key);
            return;
        }
        
        if (entry.psl > ht->entries[probe].psl) {
            RobinEntry tmp = ht->entries[probe];
            ht->entries[probe] = entry;
            entry = tmp;
        }
        entry.psl++;
    }
}

bool robin_contains(RobinHashTable *ht, const char *key) {
    uint32_t hash = hash_djb2(key);
    size_t idx = hash % ht->capacity;
    
    for (int psl = 0; psl < (int)ht->capacity; psl++) {
        size_t probe = (idx + psl) % ht->capacity;
        if (ht->entries[probe].psl < 0 || ht->entries[probe].psl < psl)
            return false;
        if (ht->entries[probe].hash == hash &&
            strcmp(ht->entries[probe].key, key) == 0)
            return true;
    }
    return false;
}

int robin_get(RobinHashTable *ht, const char *key, int default_val) {
    uint32_t hash = hash_djb2(key);
    size_t idx = hash % ht->capacity;
    
    for (int psl = 0; psl < (int)ht->capacity; psl++) {
        size_t probe = (idx + psl) % ht->capacity;
        if (ht->entries[probe].psl < 0 || ht->entries[probe].psl < psl)
            return default_val;
        if (ht->entries[probe].hash == hash &&
            strcmp(ht->entries[probe].key, key) == 0)
            return ht->entries[probe].value;
    }
    return default_val;
}

bool robin_remove(RobinHashTable *ht, const char *key) {
    uint32_t hash = hash_djb2(key);
    size_t idx = hash % ht->capacity;
    
    for (int psl = 0; psl < (int)ht->capacity; psl++) {
        size_t probe = (idx + psl) % ht->capacity;
        if (ht->entries[probe].psl < 0 || ht->entries[probe].psl < psl)
            return false;
        if (ht->entries[probe].hash == hash &&
            strcmp(ht->entries[probe].key, key) == 0) {
            free(ht->entries[probe].key);
            ht->entries[probe].key = NULL;
            ht->entries[probe].psl = -1;
            ht->size--;
            
            // Backward shift deletion
            size_t empty = probe;
            for (size_t j = 1; j < ht->capacity; j++) {
                size_t next = (probe + j) % ht->capacity;
                if (ht->entries[next].psl <= 0)
                    break;
                ht->entries[empty] = ht->entries[next];
                ht->entries[empty].psl--;
                ht->entries[next].psl = -1;
                ht->entries[next].key = NULL;
                empty = next;
            }
            return true;
        }
    }
    return false;
}

// ============================================================================
// Hash function comparison table using chain hash table
// ============================================================================
typedef struct {
    ChainNode **buckets;
    size_t capacity;
    size_t size;
    uint32_t (*hash_func)(const char*);
} ChainHashTableWithFunc;

ChainHashTableWithFunc* chain_create_with_func(size_t capacity, uint32_t (*hash_func)(const char*)) {
    ChainHashTableWithFunc *ht = malloc(sizeof(ChainHashTableWithFunc));
    ht->buckets = calloc(capacity, sizeof(ChainNode*));
    ht->capacity = capacity;
    ht->size = 0;
    ht->hash_func = hash_func;
    return ht;
}

void chain_destroy_with_func(ChainHashTableWithFunc *ht) {
    for (size_t i = 0; i < ht->capacity; i++) {
        ChainNode *node = ht->buckets[i];
        while (node) {
            ChainNode *next = node->next;
            free(node->key);
            free(node);
            node = next;
        }
    }
    free(ht->buckets);
    free(ht);
}

void chain_insert_with_func(ChainHashTableWithFunc *ht, const char *key, int value) {
    uint32_t hash = ht->hash_func(key);
    size_t idx = hash % ht->capacity;
    
    ChainNode *node = ht->buckets[idx];
    while (node) {
        if (strcmp(node->key, key) == 0) {
            node->value = value;
            return;
        }
        node = node->next;
    }
    
    ChainNode *new_node = malloc(sizeof(ChainNode));
    new_node->key = strdup(key);
    new_node->value = value;
    new_node->next = ht->buckets[idx];
    ht->buckets[idx] = new_node;
    ht->size++;
}

bool chain_contains_with_func(ChainHashTableWithFunc *ht, const char *key) {
    uint32_t hash = ht->hash_func(key);
    size_t idx = hash % ht->capacity;
    
    ChainNode *node = ht->buckets[idx];
    while (node) {
        if (strcmp(node->key, key) == 0)
            return true;
        node = node->next;
    }
    return false;
}

int chain_get_with_func(ChainHashTableWithFunc *ht, const char *key, int default_val) {
    uint32_t hash = ht->hash_func(key);
    size_t idx = hash % ht->capacity;
    
    ChainNode *node = ht->buckets[idx];
    while (node) {
        if (strcmp(node->key, key) == 0)
            return node->value;
        node = node->next;
    }
    return default_val;
}

bool chain_remove_with_func(ChainHashTableWithFunc *ht, const char *key) {
    uint32_t hash = ht->hash_func(key);
    size_t idx = hash % ht->capacity;
    
    ChainNode *prev = NULL;
    ChainNode *node = ht->buckets[idx];
    while (node) {
        if (strcmp(node->key, key) == 0) {
            if (prev)
                prev->next = node->next;
            else
                ht->buckets[idx] = node->next;
            free(node->key);
            free(node);
            ht->size--;
            return true;
        }
        prev = node;
        node = node->next;
    }
    return false;
}

// ============================================================================
// Test data generation
// ============================================================================
char** generate_keys(int count) {
    char **keys = malloc(count * sizeof(char*));
    for (int i = 0; i < count; i++) {
        keys[i] = malloc(32);
        snprintf(keys[i], 32, "key_%d", i);
    }
    return keys;
}

char** generate_random_keys(int count) {
    char **keys = malloc(count * sizeof(char*));
    for (int i = 0; i < count; i++) {
        keys[i] = malloc(32);
        snprintf(keys[i], 32, "rnd_%d_%d", rand(), i);
    }
    return keys;
}

void free_keys(char **keys, int count) {
    for (int i = 0; i < count; i++)
        free(keys[i]);
    free(keys);
}

// ============================================================================
// Benchmark functions
// ============================================================================
typedef struct {
    const char *name;
    const char *description;
    double insert_ns;
    double contains_hit_ns;
    double contains_miss_ns;
    double get_hit_ns;
    double get_miss_ns;
    double remove_ns;
} BenchmarkResult;

// Benchmark chain hash table
BenchmarkResult bench_chain(size_t capacity, int iterations) {
    BenchmarkResult result = {"chain_linked_list", "Chaining with linked lists (DJB2)", 0, 0, 0, 0, 0, 0};
    char **keys = generate_keys(iterations);
    char **miss_keys = generate_random_keys(iterations);
    uint64_t start, end;
    
    // Warmup
    ChainHashTable *ht = chain_create(capacity);
    for (int i = 0; i < WARMUP_ITERATIONS && i < iterations; i++)
        chain_insert(ht, keys[i], i);
    chain_destroy(ht);
    
    // Insert benchmark
    ht = chain_create(capacity);
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        chain_insert(ht, keys[i], i);
    end = get_nanos();
    result.insert_ns = (double)(end - start) / iterations;
    
    // Contains hit benchmark
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        chain_contains(ht, keys[i]);
    end = get_nanos();
    result.contains_hit_ns = (double)(end - start) / iterations;
    
    // Contains miss benchmark
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        chain_contains(ht, miss_keys[i]);
    end = get_nanos();
    result.contains_miss_ns = (double)(end - start) / iterations;
    
    // Get hit benchmark
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        chain_get(ht, keys[i], -1);
    end = get_nanos();
    result.get_hit_ns = (double)(end - start) / iterations;
    
    // Get miss benchmark
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        chain_get(ht, miss_keys[i], -1);
    end = get_nanos();
    result.get_miss_ns = (double)(end - start) / iterations;
    
    // Remove benchmark
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        chain_remove(ht, keys[i]);
    end = get_nanos();
    result.remove_ns = (double)(end - start) / iterations;
    
    chain_destroy(ht);
    free_keys(keys, iterations);
    free_keys(miss_keys, iterations);
    
    return result;
}

// Benchmark open addressing hash table
BenchmarkResult bench_open(size_t capacity, int iterations) {
    BenchmarkResult result = {"open_linear_probe", "Open addressing with linear probing", 0, 0, 0, 0, 0, 0};
    char **keys = generate_keys(iterations);
    char **miss_keys = generate_random_keys(iterations);
    uint64_t start, end;
    
    // Warmup
    OpenHashTable *ht = open_create(capacity);
    for (int i = 0; i < WARMUP_ITERATIONS && i < iterations; i++)
        open_insert(ht, keys[i], i);
    open_destroy(ht);
    
    // Insert benchmark
    ht = open_create(capacity);
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        open_insert(ht, keys[i], i);
    end = get_nanos();
    result.insert_ns = (double)(end - start) / iterations;
    
    // Contains hit benchmark
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        open_contains(ht, keys[i]);
    end = get_nanos();
    result.contains_hit_ns = (double)(end - start) / iterations;
    
    // Contains miss benchmark
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        open_contains(ht, miss_keys[i]);
    end = get_nanos();
    result.contains_miss_ns = (double)(end - start) / iterations;
    
    // Get hit benchmark
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        open_get(ht, keys[i], -1);
    end = get_nanos();
    result.get_hit_ns = (double)(end - start) / iterations;
    
    // Get miss benchmark
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        open_get(ht, miss_keys[i], -1);
    end = get_nanos();
    result.get_miss_ns = (double)(end - start) / iterations;
    
    // Remove benchmark
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        open_remove(ht, keys[i]);
    end = get_nanos();
    result.remove_ns = (double)(end - start) / iterations;
    
    open_destroy(ht);
    free_keys(keys, iterations);
    free_keys(miss_keys, iterations);
    
    return result;
}

// Benchmark Robin Hood hash table
BenchmarkResult bench_robin(size_t capacity, int iterations) {
    BenchmarkResult result = {"robin_hood", "Robin Hood hashing", 0, 0, 0, 0, 0, 0};
    char **keys = generate_keys(iterations);
    char **miss_keys = generate_random_keys(iterations);
    uint64_t start, end;
    
    // Warmup
    RobinHashTable *ht = robin_create(capacity);
    for (int i = 0; i < WARMUP_ITERATIONS && i < iterations; i++)
        robin_insert(ht, keys[i], i);
    robin_destroy(ht);
    
    // Insert benchmark
    ht = robin_create(capacity);
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        robin_insert(ht, keys[i], i);
    end = get_nanos();
    result.insert_ns = (double)(end - start) / iterations;
    
    // Contains hit benchmark
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        robin_contains(ht, keys[i]);
    end = get_nanos();
    result.contains_hit_ns = (double)(end - start) / iterations;
    
    // Contains miss benchmark
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        robin_contains(ht, miss_keys[i]);
    end = get_nanos();
    result.contains_miss_ns = (double)(end - start) / iterations;
    
    // Get hit benchmark
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        robin_get(ht, keys[i], -1);
    end = get_nanos();
    result.get_hit_ns = (double)(end - start) / iterations;
    
    // Get miss benchmark
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        robin_get(ht, miss_keys[i], -1);
    end = get_nanos();
    result.get_miss_ns = (double)(end - start) / iterations;
    
    // Remove benchmark
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        robin_remove(ht, keys[i]);
    end = get_nanos();
    result.remove_ns = (double)(end - start) / iterations;
    
    robin_destroy(ht);
    free_keys(keys, iterations);
    free_keys(miss_keys, iterations);
    
    return result;
}

// Benchmark hash functions
BenchmarkResult bench_hash_func(const char *name, const char *desc, 
                                uint32_t (*hash_func)(const char*),
                                size_t capacity, int iterations) {
    BenchmarkResult result = {name, desc, 0, 0, 0, 0, 0, 0};
    char **keys = generate_keys(iterations);
    char **miss_keys = generate_random_keys(iterations);
    uint64_t start, end;
    
    ChainHashTableWithFunc *ht = chain_create_with_func(capacity, hash_func);
    
    // Insert benchmark
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        chain_insert_with_func(ht, keys[i], i);
    end = get_nanos();
    result.insert_ns = (double)(end - start) / iterations;
    
    // Contains hit benchmark
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        chain_contains_with_func(ht, keys[i]);
    end = get_nanos();
    result.contains_hit_ns = (double)(end - start) / iterations;
    
    // Contains miss benchmark
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        chain_contains_with_func(ht, miss_keys[i]);
    end = get_nanos();
    result.contains_miss_ns = (double)(end - start) / iterations;
    
    // Get hit benchmark
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        chain_get_with_func(ht, keys[i], -1);
    end = get_nanos();
    result.get_hit_ns = (double)(end - start) / iterations;
    
    // Get miss benchmark
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        chain_get_with_func(ht, miss_keys[i], -1);
    end = get_nanos();
    result.get_miss_ns = (double)(end - start) / iterations;
    
    // Remove benchmark
    start = get_nanos();
    for (int i = 0; i < iterations; i++)
        chain_remove_with_func(ht, keys[i]);
    end = get_nanos();
    result.remove_ns = (double)(end - start) / iterations;
    
    chain_destroy_with_func(ht);
    free_keys(keys, iterations);
    free_keys(miss_keys, iterations);
    
    return result;
}

void print_result(BenchmarkResult *r) {
    printf("| %-25s | %8.2f | %12.2f | %13.2f | %8.2f | %9.2f | %8.2f |\n",
           r->name, r->insert_ns, r->contains_hit_ns, r->contains_miss_ns,
           r->get_hit_ns, r->get_miss_ns, r->remove_ns);
}

int main(void) {
    srand(42);
    
    int iterations = ITERATIONS;
    size_t capacity = iterations * 2; // ~50% load factor
    
    printf("# Dictionary<string, int> Benchmark Results\n\n");
    printf("Operations: Insert, Contains (hit/miss), Get (hit/miss), Remove\n");
    printf("Iterations: %d\n", iterations);
    printf("Capacity: %zu (load factor ~50%%)\n\n", capacity);
    
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
    
    printf("## Hash Table Implementation Comparison\n\n");
    printf("| %-25s | %8s | %12s | %13s | %8s | %9s | %8s |\n",
           "Implementation", "Insert", "Contains Hit", "Contains Miss", "Get Hit", "Get Miss", "Remove");
    printf("|%-27s|%10s|%14s|%15s|%10s|%11s|%10s|\n",
           "---------------------------", "----------", "--------------", "---------------", 
           "----------", "-----------", "----------");
    
    BenchmarkResult r;
    
    r = bench_chain(capacity, iterations);
    print_result(&r);
    
    r = bench_open(capacity, iterations);
    print_result(&r);
    
    r = bench_robin(capacity, iterations);
    print_result(&r);
    
    printf("\n*All times in nanoseconds per operation*\n");
    
    printf("\n## Hash Function Comparison (using chaining)\n\n");
    printf("| %-25s | %8s | %12s | %13s | %8s | %9s | %8s |\n",
           "Hash Function", "Insert", "Contains Hit", "Contains Miss", "Get Hit", "Get Miss", "Remove");
    printf("|%-27s|%10s|%14s|%15s|%10s|%11s|%10s|\n",
           "---------------------------", "----------", "--------------", "---------------", 
           "----------", "-----------", "----------");
    
    r = bench_hash_func("hash_djb2", "DJB2 hash function", hash_djb2, capacity, iterations);
    print_result(&r);
    
    r = bench_hash_func("hash_fnv1a", "FNV-1a hash function", hash_fnv1a, capacity, iterations);
    print_result(&r);
    
    r = bench_hash_func("hash_sdbm", "SDBM hash function", hash_sdbm, capacity, iterations);
    print_result(&r);
    
    r = bench_hash_func("hash_murmur3_simple", "Simplified MurmurHash3", hash_murmur3_simple, capacity, iterations);
    print_result(&r);
    
    printf("\n*All times in nanoseconds per operation*\n");
    
    // Different capacity tests
    printf("\n## Load Factor Impact (Chain with DJB2)\n\n");
    printf("| %-15s | %8s | %12s | %13s | %8s | %9s | %8s |\n",
           "Load Factor", "Insert", "Contains Hit", "Contains Miss", "Get Hit", "Get Miss", "Remove");
    printf("|%-17s|%10s|%14s|%15s|%10s|%11s|%10s|\n",
           "-----------------", "----------", "--------------", "---------------", 
           "----------", "-----------", "----------");
    
    int test_iters = 100000;
    size_t capacities[] = {test_iters * 10, test_iters * 4, test_iters * 2, 
                           (size_t)(test_iters * 1.33), test_iters + test_iters/10};
    const char *load_names[] = {"~10%", "~25%", "~50%", "~75%", "~90%"};
    
    for (int i = 0; i < 5; i++) {
        char **keys = generate_keys(test_iters);
        char **miss_keys = generate_random_keys(test_iters);
        uint64_t start, end;
        
        ChainHashTable *ht = chain_create(capacities[i]);
        
        start = get_nanos();
        for (int j = 0; j < test_iters; j++)
            chain_insert(ht, keys[j], j);
        end = get_nanos();
        double insert_ns = (double)(end - start) / test_iters;
        
        start = get_nanos();
        for (int j = 0; j < test_iters; j++)
            chain_contains(ht, keys[j]);
        end = get_nanos();
        double contains_hit_ns = (double)(end - start) / test_iters;
        
        start = get_nanos();
        for (int j = 0; j < test_iters; j++)
            chain_contains(ht, miss_keys[j]);
        end = get_nanos();
        double contains_miss_ns = (double)(end - start) / test_iters;
        
        start = get_nanos();
        for (int j = 0; j < test_iters; j++)
            chain_get(ht, keys[j], -1);
        end = get_nanos();
        double get_hit_ns = (double)(end - start) / test_iters;
        
        start = get_nanos();
        for (int j = 0; j < test_iters; j++)
            chain_get(ht, miss_keys[j], -1);
        end = get_nanos();
        double get_miss_ns = (double)(end - start) / test_iters;
        
        start = get_nanos();
        for (int j = 0; j < test_iters; j++)
            chain_remove(ht, keys[j]);
        end = get_nanos();
        double remove_ns = (double)(end - start) / test_iters;
        
        printf("| %-15s | %8.2f | %12.2f | %13.2f | %8.2f | %9.2f | %8.2f |\n",
               load_names[i], insert_ns, contains_hit_ns, contains_miss_ns,
               get_hit_ns, get_miss_ns, remove_ns);
        
        chain_destroy(ht);
        free_keys(keys, test_iters);
        free_keys(miss_keys, test_iters);
    }
    
    printf("\n*All times in nanoseconds per operation*\n");
    
    // Key length impact
    printf("\n## Key Length Impact (Chain with DJB2, 50%% load)\n\n");
    printf("| %-15s | %8s | %12s | %8s |\n",
           "Key Length", "Insert", "Contains Hit", "Get Hit");
    printf("|%-17s|%10s|%14s|%10s|\n",
           "-----------------", "----------", "--------------", "----------");
    
    int key_lengths[] = {8, 16, 32, 64, 128, 256};
    test_iters = 100000;
    
    for (int k = 0; k < 6; k++) {
        int key_len = key_lengths[k];
        char **keys = malloc(test_iters * sizeof(char*));
        for (int i = 0; i < test_iters; i++) {
            keys[i] = malloc(key_len + 16);
            snprintf(keys[i], key_len + 16, "key_%0*d", key_len - 5, i);
        }
        
        ChainHashTable *ht = chain_create(test_iters * 2);
        uint64_t start, end;
        
        start = get_nanos();
        for (int i = 0; i < test_iters; i++)
            chain_insert(ht, keys[i], i);
        end = get_nanos();
        double insert_ns = (double)(end - start) / test_iters;
        
        start = get_nanos();
        for (int i = 0; i < test_iters; i++)
            chain_contains(ht, keys[i]);
        end = get_nanos();
        double contains_ns = (double)(end - start) / test_iters;
        
        start = get_nanos();
        for (int i = 0; i < test_iters; i++)
            chain_get(ht, keys[i], -1);
        end = get_nanos();
        double get_ns = (double)(end - start) / test_iters;
        
        printf("| %3d chars       | %8.2f | %12.2f | %8.2f |\n",
               key_len, insert_ns, contains_ns, get_ns);
        
        chain_destroy(ht);
        for (int i = 0; i < test_iters; i++)
            free(keys[i]);
        free(keys);
    }
    
    printf("\n*All times in nanoseconds per operation*\n");
    
    printf("\n## Implementation Descriptions\n\n");
    printf("1. **chain_linked_list**: Separate chaining using linked lists. Simple and reliable.\n");
    printf("2. **open_linear_probe**: Open addressing with linear probing. Better cache locality.\n");
    printf("3. **robin_hood**: Robin Hood hashing with backward shift deletion. Lower variance.\n");
    printf("\n## Hash Function Descriptions\n\n");
    printf("1. **DJB2**: Dan Bernstein's hash. Simple and fast.\n");
    printf("2. **FNV-1a**: Fowler-Noll-Vo hash. Good distribution.\n");
    printf("3. **SDBM**: From SDBM database. Similar to DJB2.\n");
    printf("4. **MurmurHash3 (simplified)**: Simplified version of MurmurHash3.\n");
    
    return 0;
}
