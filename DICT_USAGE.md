# dict.h - Fast Dictionary Library for C

**Single-header** implementation of `Dictionary<string, int>` using Robin Hood hashing with DJB2 hash function.

## Features

- **Header-only** - just include one file
- **Fast** - Robin Hood hashing with ~4 ns lookups
- **Simple API** - intuitive functions
- **Auto-resize** - grows automatically
- **Iterator support** - iterate over all entries
- **No dependencies** - pure C, works with C11+

## Installation

Copy `include/dict.h` to your project.

## Quick Start

```c
#define DICT_IMPLEMENTATION
#include "dict.h"

int main() {
    // Create dictionary
    Dict *dict = dict_create();
    
    // Insert
    dict_set(dict, "apple", 10);
    dict_set(dict, "banana", 20);
    
    // Get
    int value = dict_get(dict, "apple", -1);  // Returns 10
    
    // Check existence
    if (dict_contains(dict, "apple")) {
        printf("Found!\n");
    }
    
    // Remove
    dict_remove(dict, "banana");
    
    // Cleanup
    dict_destroy(dict);
    return 0;
}
```

## Compilation

```bash
gcc -O3 -o myprogram myprogram.c
```

Or use the provided Makefile:

```bash
make dict-example
./bin/dict_example
```

## API Reference

### Creation & Destruction

```c
// Create with default capacity (16)
Dict* dict_create(void);

// Create with specific capacity
Dict* dict_create_with_capacity(size_t capacity);

// Destroy and free all memory
void dict_destroy(Dict *dict);
```

### Basic Operations

```c
// Insert or update key-value pair
// Returns true if new key inserted, false if updated
bool dict_set(Dict *dict, const char *key, int value);

// Get value by key (returns default_value if not found)
int dict_get(Dict *dict, const char *key, int default_value);

// Get pointer to value (returns NULL if not found)
// Allows direct modification of value
int* dict_get_ptr(Dict *dict, const char *key);

// Check if key exists
bool dict_contains(Dict *dict, const char *key);

// Remove key (returns true if found and removed)
bool dict_remove(Dict *dict, const char *key);
```

### Utility Functions

```c
// Get number of elements
size_t dict_size(Dict *dict);

// Get current capacity
size_t dict_capacity(Dict *dict);

// Check if empty
bool dict_empty(Dict *dict);

// Remove all elements (keeps capacity)
void dict_clear(Dict *dict);

// Reserve capacity for at least n elements
void dict_reserve(Dict *dict, size_t n);

// Get current load factor (size / capacity)
double dict_load_factor(Dict *dict);
```

### Iteration

```c
Dict *dict = dict_create();
dict_set(dict, "a", 1);
dict_set(dict, "b", 2);

DictIterator iter = dict_iter(dict);
const char *key;
int value;
while (dict_next(&iter, &key, &value)) {
    printf("%s = %d\n", key, value);
}
```

## Examples

### Word Counter

```c
#define DICT_IMPLEMENTATION
#include "dict.h"
#include <stdio.h>
#include <string.h>

int main() {
    Dict *wc = dict_create();
    char *text = strdup("the quick brown fox jumps over the lazy dog");
    
    char *word = strtok(text, " ");
    while (word) {
        int count = dict_get(wc, word, 0);
        dict_set(wc, word, count + 1);
        word = strtok(NULL, " ");
    }
    
    DictIterator iter = dict_iter(wc);
    const char *key;
    int value;
    while (dict_next(&iter, &key, &value)) {
        printf("%s: %d\n", key, value);
    }
    
    free(text);
    dict_destroy(wc);
    return 0;
}
```

### Increment Counter via Pointer

```c
Dict *dict = dict_create();
dict_set(dict, "counter", 0);

// Increment without re-hashing the key
int *ptr = dict_get_ptr(dict, "counter");
if (ptr) {
    (*ptr)++;
}

printf("Counter: %d\n", dict_get(dict, "counter", 0));  // Output: 1
```

### Pre-allocate Capacity

```c
// When you know approximate size
Dict *dict = dict_create_with_capacity(10000);

// Or reserve later
dict_reserve(dict, 50000);
```

## Performance

Benchmark results (100,000 elements):

| Operation | Time (ns/op) |
|-----------|-------------|
| Insert | ~500 |
| Get (hit) | ~165 |
| Contains (hit) | ~190 |
| Contains (miss) | ~140 |

Robin Hood hashing provides:
- **Low variance** in lookup times
- **Efficient** deletions (backward shift)
- **Good cache** locality

## Configuration

Define before including `dict.h`:

```c
// Initial capacity (default: 16)
#define DICT_INITIAL_CAPACITY 1024

// Load factor threshold for resize (default: 0.75)
#define DICT_LOAD_FACTOR 0.5

#define DICT_IMPLEMENTATION
#include "dict.h"
```

## Thread Safety

This library is **NOT thread-safe**. For multi-threaded use, add your own synchronization.

## Memory Management

- Keys are **copied** (via `strdup`) when inserted
- Keys are **freed** when removed or dictionary is destroyed
- Values are stored by value (no allocation)

## License

Public Domain / MIT

---

## Files

```
include/
  dict.h           # Header-only library (copy this to your project)

src/
  dict_example.c   # Usage examples

bin/
  dict_example     # Compiled example (Linux x64)
```
