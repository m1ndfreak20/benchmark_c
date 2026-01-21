/*
 * dict.h - Fast Dictionary<string, int> implementation
 * 
 * Robin Hood hashing with DJB2 hash function
 * Single-header library for C projects
 * 
 * Usage:
 *   #define DICT_IMPLEMENTATION
 *   #include "dict.h"
 * 
 * License: Public Domain / MIT
 */

#ifndef DICT_H
#define DICT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// ============================================================================
// Configuration
// ============================================================================

#ifndef DICT_INITIAL_CAPACITY
#define DICT_INITIAL_CAPACITY 16
#endif

#ifndef DICT_LOAD_FACTOR
#define DICT_LOAD_FACTOR 0.75
#endif

// ============================================================================
// Types
// ============================================================================

typedef struct {
    char *key;
    int value;
    uint32_t hash;
    int psl;  // probe sequence length (-1 = empty)
} DictEntry;

typedef struct {
    DictEntry *entries;
    size_t capacity;
    size_t size;
} Dict;

// ============================================================================
// API Functions
// ============================================================================

// Create a new dictionary with default capacity
Dict* dict_create(void);

// Create a new dictionary with specified initial capacity
Dict* dict_create_with_capacity(size_t capacity);

// Destroy dictionary and free all memory
void dict_destroy(Dict *dict);

// Insert or update key-value pair
// Returns true if new key was inserted, false if existing key was updated
bool dict_set(Dict *dict, const char *key, int value);

// Get value by key
// Returns the value if found, or default_value if not found
int dict_get(Dict *dict, const char *key, int default_value);

// Get pointer to value by key (allows modification)
// Returns NULL if key not found
int* dict_get_ptr(Dict *dict, const char *key);

// Check if key exists
bool dict_contains(Dict *dict, const char *key);

// Remove key from dictionary
// Returns true if key was found and removed, false if not found
bool dict_remove(Dict *dict, const char *key);

// Get number of elements
size_t dict_size(Dict *dict);

// Get current capacity
size_t dict_capacity(Dict *dict);

// Check if dictionary is empty
bool dict_empty(Dict *dict);

// Clear all elements (keeps capacity)
void dict_clear(Dict *dict);

// ============================================================================
// Iterator API
// ============================================================================

typedef struct {
    Dict *dict;
    size_t index;
} DictIterator;

// Initialize iterator
DictIterator dict_iter(Dict *dict);

// Get next key-value pair
// Returns true if there is a next element, false if iteration is complete
// If returns true, key and value pointers are set
bool dict_next(DictIterator *iter, const char **key, int *value);

// ============================================================================
// Utility Functions
// ============================================================================

// Reserve capacity for at least n elements
void dict_reserve(Dict *dict, size_t n);

// Get load factor (size / capacity)
double dict_load_factor(Dict *dict);

// ============================================================================
// Implementation
// ============================================================================

#ifdef DICT_IMPLEMENTATION

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <string.h>

// DJB2 hash function - fast and good distribution for strings
static inline uint32_t dict_hash(const char *str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;  // hash * 33 + c
    return hash;
}

// Internal: resize the dictionary
static void dict_resize(Dict *dict, size_t new_capacity) {
    DictEntry *old_entries = dict->entries;
    size_t old_capacity = dict->capacity;
    
    dict->entries = (DictEntry*)calloc(new_capacity, sizeof(DictEntry));
    dict->capacity = new_capacity;
    dict->size = 0;
    
    // Mark all as empty
    for (size_t i = 0; i < new_capacity; i++) {
        dict->entries[i].psl = -1;
    }
    
    // Reinsert all elements
    for (size_t i = 0; i < old_capacity; i++) {
        if (old_entries[i].psl >= 0) {
            dict_set(dict, old_entries[i].key, old_entries[i].value);
            free(old_entries[i].key);
        }
    }
    
    free(old_entries);
}

Dict* dict_create(void) {
    return dict_create_with_capacity(DICT_INITIAL_CAPACITY);
}

Dict* dict_create_with_capacity(size_t capacity) {
    Dict *dict = (Dict*)malloc(sizeof(Dict));
    if (!dict) return NULL;
    
    dict->entries = (DictEntry*)calloc(capacity, sizeof(DictEntry));
    if (!dict->entries) {
        free(dict);
        return NULL;
    }
    
    dict->capacity = capacity;
    dict->size = 0;
    
    // Mark all entries as empty
    for (size_t i = 0; i < capacity; i++) {
        dict->entries[i].psl = -1;
    }
    
    return dict;
}

void dict_destroy(Dict *dict) {
    if (!dict) return;
    
    for (size_t i = 0; i < dict->capacity; i++) {
        if (dict->entries[i].psl >= 0 && dict->entries[i].key) {
            free(dict->entries[i].key);
        }
    }
    free(dict->entries);
    free(dict);
}

bool dict_set(Dict *dict, const char *key, int value) {
    if (!dict || !key) return false;
    
    // Check if resize needed
    if ((double)(dict->size + 1) / dict->capacity > DICT_LOAD_FACTOR) {
        dict_resize(dict, dict->capacity * 2);
    }
    
    uint32_t hash = dict_hash(key);
    size_t idx = hash % dict->capacity;
    
    // Create entry to insert
    DictEntry entry;
    entry.key = NULL;  // Will be set if we need to insert
    entry.value = value;
    entry.hash = hash;
    entry.psl = 0;
    
    bool is_new = true;
    (void)is_new; // unused for now
    
    for (size_t i = 0; i < dict->capacity; i++) {
        size_t probe = (idx + i) % dict->capacity;
        
        // Empty slot - insert here
        if (dict->entries[probe].psl < 0) {
            if (!entry.key) {
                entry.key = strdup(key);
                if (!entry.key) return false;
            }
            dict->entries[probe] = entry;
            dict->size++;
            return true;
        }
        
        // Key already exists - update value
        if (dict->entries[probe].hash == hash && 
            strcmp(dict->entries[probe].key, key) == 0) {
            dict->entries[probe].value = value;
            if (entry.key) free(entry.key);
            return false;  // Not a new key
        }
        
        // Robin Hood: steal from the rich
        if (entry.psl > dict->entries[probe].psl) {
            if (!entry.key) {
                entry.key = strdup(key);
                if (!entry.key) return false;
            }
            DictEntry tmp = dict->entries[probe];
            dict->entries[probe] = entry;
            entry = tmp;
            is_new = false;  // We're now moving an existing entry
        }
        
        entry.psl++;
    }
    
    // Should never reach here if load factor is maintained
    if (entry.key) free(entry.key);
    return false;
}

int dict_get(Dict *dict, const char *key, int default_value) {
    if (!dict || !key) return default_value;
    
    uint32_t hash = dict_hash(key);
    size_t idx = hash % dict->capacity;
    
    for (int psl = 0; psl < (int)dict->capacity; psl++) {
        size_t probe = (idx + psl) % dict->capacity;
        
        // Empty slot or would have been inserted here
        if (dict->entries[probe].psl < 0 || dict->entries[probe].psl < psl) {
            return default_value;
        }
        
        // Found key
        if (dict->entries[probe].hash == hash &&
            strcmp(dict->entries[probe].key, key) == 0) {
            return dict->entries[probe].value;
        }
    }
    
    return default_value;
}

int* dict_get_ptr(Dict *dict, const char *key) {
    if (!dict || !key) return NULL;
    
    uint32_t hash = dict_hash(key);
    size_t idx = hash % dict->capacity;
    
    for (int psl = 0; psl < (int)dict->capacity; psl++) {
        size_t probe = (idx + psl) % dict->capacity;
        
        if (dict->entries[probe].psl < 0 || dict->entries[probe].psl < psl) {
            return NULL;
        }
        
        if (dict->entries[probe].hash == hash &&
            strcmp(dict->entries[probe].key, key) == 0) {
            return &dict->entries[probe].value;
        }
    }
    
    return NULL;
}

bool dict_contains(Dict *dict, const char *key) {
    if (!dict || !key) return false;
    
    uint32_t hash = dict_hash(key);
    size_t idx = hash % dict->capacity;
    
    for (int psl = 0; psl < (int)dict->capacity; psl++) {
        size_t probe = (idx + psl) % dict->capacity;
        
        if (dict->entries[probe].psl < 0 || dict->entries[probe].psl < psl) {
            return false;
        }
        
        if (dict->entries[probe].hash == hash &&
            strcmp(dict->entries[probe].key, key) == 0) {
            return true;
        }
    }
    
    return false;
}

bool dict_remove(Dict *dict, const char *key) {
    if (!dict || !key) return false;
    
    uint32_t hash = dict_hash(key);
    size_t idx = hash % dict->capacity;
    
    for (int psl = 0; psl < (int)dict->capacity; psl++) {
        size_t probe = (idx + psl) % dict->capacity;
        
        if (dict->entries[probe].psl < 0 || dict->entries[probe].psl < psl) {
            return false;
        }
        
        if (dict->entries[probe].hash == hash &&
            strcmp(dict->entries[probe].key, key) == 0) {
            // Free key
            free(dict->entries[probe].key);
            dict->entries[probe].key = NULL;
            dict->entries[probe].psl = -1;
            dict->size--;
            
            // Backward shift deletion to maintain Robin Hood invariant
            size_t empty = probe;
            for (size_t j = 1; j < dict->capacity; j++) {
                size_t next = (probe + j) % dict->capacity;
                
                // Stop if next slot is empty or at its ideal position
                if (dict->entries[next].psl <= 0) {
                    break;
                }
                
                // Shift entry back
                dict->entries[empty] = dict->entries[next];
                dict->entries[empty].psl--;
                dict->entries[next].psl = -1;
                dict->entries[next].key = NULL;
                empty = next;
            }
            
            return true;
        }
    }
    
    return false;
}

size_t dict_size(Dict *dict) {
    return dict ? dict->size : 0;
}

size_t dict_capacity(Dict *dict) {
    return dict ? dict->capacity : 0;
}

bool dict_empty(Dict *dict) {
    return !dict || dict->size == 0;
}

void dict_clear(Dict *dict) {
    if (!dict) return;
    
    for (size_t i = 0; i < dict->capacity; i++) {
        if (dict->entries[i].psl >= 0 && dict->entries[i].key) {
            free(dict->entries[i].key);
            dict->entries[i].key = NULL;
        }
        dict->entries[i].psl = -1;
    }
    dict->size = 0;
}

DictIterator dict_iter(Dict *dict) {
    DictIterator iter = {dict, 0};
    return iter;
}

bool dict_next(DictIterator *iter, const char **key, int *value) {
    if (!iter || !iter->dict) return false;
    
    while (iter->index < iter->dict->capacity) {
        if (iter->dict->entries[iter->index].psl >= 0) {
            if (key) *key = iter->dict->entries[iter->index].key;
            if (value) *value = iter->dict->entries[iter->index].value;
            iter->index++;
            return true;
        }
        iter->index++;
    }
    
    return false;
}

void dict_reserve(Dict *dict, size_t n) {
    if (!dict) return;
    
    size_t required = (size_t)(n / DICT_LOAD_FACTOR) + 1;
    if (required > dict->capacity) {
        // Round up to next power of 2
        size_t new_cap = dict->capacity;
        while (new_cap < required) {
            new_cap *= 2;
        }
        dict_resize(dict, new_cap);
    }
}

double dict_load_factor(Dict *dict) {
    if (!dict || dict->capacity == 0) return 0.0;
    return (double)dict->size / dict->capacity;
}

#endif // DICT_IMPLEMENTATION

#ifdef __cplusplus
}
#endif

#endif // DICT_H
