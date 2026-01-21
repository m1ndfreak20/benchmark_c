# dict.h - Generic Dictionary Library for C

**Single-header** generic dictionary implementation using Robin Hood hashing with DJB2 hash function.

## Features

- **Generic** - supports any key/value types
- **Header-only** - just include one file
- **Fast** - Robin Hood hashing with ~200-350 ns lookups
- **Type-safe** - macros generate typed functions
- **Auto-resize** - grows automatically
- **Iterator support** - iterate over all entries
- **No dependencies** - pure C, works with C11+

## Installation

Copy `include/dict.h` to your project.

## Quick Start

```c
#define _GNU_SOURCE
#include "dict.h"

// Define your dictionary type
DICT_DEFINE_STR_INT(MyDict)  // Dict<string, int>

int main() {
    MyDict *dict = MyDict_create();
    
    MyDict_set(dict, "apple", 10);
    MyDict_set(dict, "banana", 20);
    
    int val = MyDict_get(dict, "apple", -1);  // Returns 10
    
    if (MyDict_contains(dict, "banana")) {
        printf("Found!\n");
    }
    
    MyDict_remove(dict, "banana");
    MyDict_destroy(dict);
    return 0;
}
```

## Available Type Macros

```c
// String key
DICT_DEFINE_STR_INT(Name)      // Dict<string, int>
DICT_DEFINE_STR_DOUBLE(Name)   // Dict<string, double>
DICT_DEFINE_STR_FLOAT(Name)    // Dict<string, float>
DICT_DEFINE_STR_STR(Name)      // Dict<string, string>
DICT_DEFINE_STR_PTR(Name)      // Dict<string, void*>

// Integer key
DICT_DEFINE_INT_INT(Name)      // Dict<int, int>
DICT_DEFINE_INT_DOUBLE(Name)   // Dict<int, double>
DICT_DEFINE_INT_STR(Name)      // Dict<int, string>
DICT_DEFINE_INT_PTR(Name)      // Dict<int, void*>

// Other keys
DICT_DEFINE_UINT32_INT(Name)   // Dict<uint32_t, int>
DICT_DEFINE_UINT32_PTR(Name)   // Dict<uint32_t, void*>
DICT_DEFINE_UINT64_INT(Name)   // Dict<uint64_t, int>
DICT_DEFINE_UINT64_PTR(Name)   // Dict<uint64_t, void*>
DICT_DEFINE_PTR_INT(Name)      // Dict<void*, int>
DICT_DEFINE_PTR_PTR(Name)      // Dict<void*, void*>
```

## Custom Types

For custom types, use the full macro:

```c
DICT_DEFINE(Name, KEY_TYPE, VALUE_TYPE, HASH_FN, EQ_FN, COPY_KEY_FN, FREE_KEY_FN)
```

Example with custom struct key:

```c
// Your key type
typedef struct { int x, y; } Point;

// Hash function
uint32_t hash_point(Point p) {
    return dict_hash_int(p.x) ^ dict_hash_int(p.y);
}

// Equality function
bool eq_point(Point a, Point b) {
    return a.x == b.x && a.y == b.y;
}

// Copy/free for value types (no allocation needed)
#define copy_point(p) (p)
#define free_point(p) ((void)0)

// Define the dictionary
DICT_DEFINE(PointDict, Point, int, hash_point, eq_point, copy_point, free_point)

// Use it
PointDict *dict = PointDict_create();
PointDict_set(dict, (Point){1, 2}, 100);
int val = PointDict_get(dict, (Point){1, 2}, -1);
PointDict_destroy(dict);
```

## API Reference

All functions are prefixed with your dictionary name. Example for `DICT_DEFINE_STR_INT(MyDict)`:

### Creation & Destruction

```c
MyDict* MyDict_create(void);                        // Create with default capacity
MyDict* MyDict_create_with_capacity(size_t cap);    // Create with specific capacity
void MyDict_destroy(MyDict *dict);                  // Free all memory
```

### Basic Operations

```c
bool MyDict_set(MyDict *dict, char *key, int value);     // Insert/update, returns true if new
int MyDict_get(MyDict *dict, char *key, int default_val); // Get or default
int* MyDict_get_ptr(MyDict *dict, char *key);            // Get pointer to value (or NULL)
bool MyDict_contains(MyDict *dict, char *key);           // Check if key exists
bool MyDict_remove(MyDict *dict, char *key);             // Remove key
```

### Utility

```c
size_t MyDict_size(MyDict *dict);      // Number of elements
size_t MyDict_capacity(MyDict *dict);  // Current capacity
bool MyDict_empty(MyDict *dict);       // Is empty?
void MyDict_clear(MyDict *dict);       // Remove all elements
```

### Iteration

```c
MyDict_Iterator iter = MyDict_iter(dict);
char *key;
int value;
while (MyDict_next(&iter, &key, &value)) {
    printf("%s = %d\n", key, value);
}
```

## Examples

### Dict<string, int> - Word Counter

```c
#define _GNU_SOURCE
#include "dict.h"
#include <string.h>

DICT_DEFINE_STR_INT(WordCount)

int main() {
    WordCount *wc = WordCount_create();
    char *text = strdup("the quick brown fox the lazy dog the fox");
    
    char *word = strtok(text, " ");
    while (word) {
        int count = WordCount_get(wc, word, 0);
        WordCount_set(wc, word, count + 1);
        word = strtok(NULL, " ");
    }
    
    WordCount_Iterator iter = WordCount_iter(wc);
    char *w; int c;
    while (WordCount_next(&iter, &w, &c)) {
        printf("%s: %d\n", w, c);
    }
    
    free(text);
    WordCount_destroy(wc);
}
```

### Dict<string, double> - Prices

```c
DICT_DEFINE_STR_DOUBLE(PriceDict)

PriceDict *prices = PriceDict_create();
PriceDict_set(prices, "BTC", 45000.50);
PriceDict_set(prices, "ETH", 2500.75);

printf("BTC: $%.2f\n", PriceDict_get(prices, "BTC", 0.0));
PriceDict_destroy(prices);
```

### Dict<int, int> - Squares

```c
DICT_DEFINE_INT_INT(Squares)

Squares *sq = Squares_create();
for (int i = 1; i <= 100; i++) {
    Squares_set(sq, i, i * i);
}

printf("50^2 = %d\n", Squares_get(sq, 50, 0));  // 2500
Squares_destroy(sq);
```

### Dict<int, string> - HTTP Codes

```c
DICT_DEFINE_INT_STR(HttpCodes)

HttpCodes *codes = HttpCodes_create();
HttpCodes_set(codes, 200, "OK");
HttpCodes_set(codes, 404, "Not Found");
HttpCodes_set(codes, 500, "Internal Server Error");

printf("HTTP 404: %s\n", HttpCodes_get(codes, 404, "Unknown"));
HttpCodes_destroy(codes);
```

### Dict<string, void*> - Object Storage

```c
DICT_DEFINE_STR_PTR(ObjectStore)

typedef struct { int id; char name[32]; } User;
User alice = {1, "Alice"};
User bob = {2, "Bob"};

ObjectStore *users = ObjectStore_create();
ObjectStore_set(users, "alice", &alice);
ObjectStore_set(users, "bob", &bob);

User *u = (User*)ObjectStore_get(users, "alice", NULL);
if (u) printf("Found: %s\n", u->name);

ObjectStore_destroy(users);
```

## Performance

Benchmark results (100,000 string keys):

| Operation | Time (ns/op) |
|-----------|-------------|
| Insert | ~1000 |
| Get (hit) | ~350 |
| Contains (miss) | ~200 |

## Built-in Hash Functions

```c
dict_hash_str(const char *s)   // DJB2 for strings
dict_hash_int(int key)         // Integer hash
dict_hash_uint32(uint32_t key) // uint32 hash
dict_hash_uint64(uint64_t key) // uint64 hash
dict_hash_ptr(const void *ptr) // Pointer hash
```

## Built-in Comparison Functions

```c
dict_eq_str(a, b)     // strcmp based
dict_eq_int(a, b)     // a == b
dict_eq_uint32(a, b)  // a == b
dict_eq_uint64(a, b)  // a == b
dict_eq_ptr(a, b)     // a == b
```

## Configuration

Define before including `dict.h`:

```c
#define DICT_INITIAL_CAPACITY 1024  // Default: 16
#define DICT_LOAD_FACTOR 0.5        // Default: 0.75
#include "dict.h"
```

## Thread Safety

This library is **NOT thread-safe**. Add your own synchronization for multi-threaded use.

## Memory Management

- **String keys** are copied via `strdup()` and freed on removal
- **Value types** (int, double, pointers) are stored by value
- **Pointer values** are NOT freed - you manage their lifetime

## License

Public Domain / MIT
