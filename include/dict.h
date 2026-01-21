/*
 * dict.h - Generic Dictionary implementation (Robin Hood + DJB2)
 * 
 * Single-header library with support for custom key/value types.
 * 
 * Usage:
 *   // Define your dictionary type
 *   DICT_DEFINE_STR_INT(StrIntDict)    // Dict<string, int>
 *   DICT_DEFINE_INT_INT(IntIntDict)    // Dict<int, int>
 *   
 *   // Or custom types:
 *   DICT_DEFINE(MyDict, char*, double, dict_hash_str, dict_eq_str, dict_copy_str, dict_free_str)
 *   
 *   // Use it
 *   StrIntDict *dict = StrIntDict_create();
 *   StrIntDict_set(dict, "key", 42);
 *   int val = StrIntDict_get(dict, "key", -1);
 *   StrIntDict_destroy(dict);
 * 
 * License: Public Domain / MIT
 */

#ifndef DICT_H
#define DICT_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

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
// Hash Functions
// ============================================================================

// DJB2 hash for strings
static inline uint32_t dict_hash_str(const char *str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

// Hash for integers
static inline uint32_t dict_hash_int(int key) {
    uint32_t x = (uint32_t)key;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

// Hash for uint32
static inline uint32_t dict_hash_uint32(uint32_t key) {
    key = ((key >> 16) ^ key) * 0x45d9f3b;
    key = ((key >> 16) ^ key) * 0x45d9f3b;
    key = (key >> 16) ^ key;
    return key;
}

// Hash for uint64
static inline uint32_t dict_hash_uint64(uint64_t key) {
    key = (~key) + (key << 18);
    key = key ^ (key >> 31);
    key = key * 21;
    key = key ^ (key >> 11);
    key = key + (key << 6);
    key = key ^ (key >> 22);
    return (uint32_t)key;
}

// Hash for pointers
static inline uint32_t dict_hash_ptr(const void *ptr) {
    return dict_hash_uint64((uint64_t)(uintptr_t)ptr);
}

// ============================================================================
// Key Comparison Functions
// ============================================================================

static inline bool dict_eq_str(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}

static inline bool dict_eq_int(int a, int b) {
    return a == b;
}

static inline bool dict_eq_uint32(uint32_t a, uint32_t b) {
    return a == b;
}

static inline bool dict_eq_uint64(uint64_t a, uint64_t b) {
    return a == b;
}

static inline bool dict_eq_ptr(const void *a, const void *b) {
    return a == b;
}

// ============================================================================
// Key Copy/Free Functions
// ============================================================================

static inline char* dict_copy_str(const char *s) {
    return strdup(s);
}

static inline void dict_free_str(char *s) {
    free(s);
}

// For non-pointer types, no copy/free needed
#define dict_copy_val(x) (x)
#define dict_free_val(x) ((void)0)

// ============================================================================
// DICT_DEFINE macro - generates type-specific dictionary
// ============================================================================

#define DICT_DEFINE(NAME, KEY_TYPE, VALUE_TYPE, HASH_FN, EQ_FN, COPY_KEY_FN, FREE_KEY_FN) \
\
typedef struct { \
    KEY_TYPE key; \
    VALUE_TYPE value; \
    uint32_t hash; \
    int psl; \
} NAME##_Entry; \
\
typedef struct { \
    NAME##_Entry *entries; \
    size_t capacity; \
    size_t size; \
} NAME; \
\
typedef struct { \
    NAME *dict; \
    size_t index; \
} NAME##_Iterator; \
\
static inline NAME* NAME##_create_with_capacity(size_t capacity) { \
    NAME *dict = (NAME*)malloc(sizeof(NAME)); \
    if (!dict) return NULL; \
    dict->entries = (NAME##_Entry*)calloc(capacity, sizeof(NAME##_Entry)); \
    if (!dict->entries) { free(dict); return NULL; } \
    dict->capacity = capacity; \
    dict->size = 0; \
    for (size_t i = 0; i < capacity; i++) \
        dict->entries[i].psl = -1; \
    return dict; \
} \
\
static inline NAME* NAME##_create(void) { \
    return NAME##_create_with_capacity(DICT_INITIAL_CAPACITY); \
} \
\
static inline void NAME##_destroy(NAME *dict) { \
    if (!dict) return; \
    for (size_t i = 0; i < dict->capacity; i++) { \
        if (dict->entries[i].psl >= 0) { \
            FREE_KEY_FN(dict->entries[i].key); \
        } \
    } \
    free(dict->entries); \
    free(dict); \
} \
\
static inline void NAME##_resize(NAME *dict, size_t new_capacity) { \
    NAME##_Entry *old_entries = dict->entries; \
    size_t old_capacity = dict->capacity; \
    dict->entries = (NAME##_Entry*)calloc(new_capacity, sizeof(NAME##_Entry)); \
    dict->capacity = new_capacity; \
    dict->size = 0; \
    for (size_t i = 0; i < new_capacity; i++) \
        dict->entries[i].psl = -1; \
    for (size_t i = 0; i < old_capacity; i++) { \
        if (old_entries[i].psl >= 0) { \
            uint32_t hash = old_entries[i].hash; \
            size_t idx = hash % dict->capacity; \
            NAME##_Entry entry = old_entries[i]; \
            entry.psl = 0; \
            for (size_t j = 0; j < dict->capacity; j++) { \
                size_t probe = (idx + j) % dict->capacity; \
                if (dict->entries[probe].psl < 0) { \
                    dict->entries[probe] = entry; \
                    dict->size++; \
                    break; \
                } \
                if (entry.psl > dict->entries[probe].psl) { \
                    NAME##_Entry tmp = dict->entries[probe]; \
                    dict->entries[probe] = entry; \
                    entry = tmp; \
                } \
                entry.psl++; \
            } \
        } \
    } \
    free(old_entries); \
} \
\
static inline bool NAME##_set(NAME *dict, KEY_TYPE key, VALUE_TYPE value) { \
    if (!dict) return false; \
    if ((double)(dict->size + 1) / dict->capacity > DICT_LOAD_FACTOR) { \
        NAME##_resize(dict, dict->capacity * 2); \
    } \
    uint32_t hash = HASH_FN(key); \
    size_t idx = hash % dict->capacity; \
    NAME##_Entry entry; \
    entry.key = COPY_KEY_FN(key); \
    entry.value = value; \
    entry.hash = hash; \
    entry.psl = 0; \
    for (size_t i = 0; i < dict->capacity; i++) { \
        size_t probe = (idx + i) % dict->capacity; \
        if (dict->entries[probe].psl < 0) { \
            dict->entries[probe] = entry; \
            dict->size++; \
            return true; \
        } \
        if (dict->entries[probe].hash == hash && \
            EQ_FN(dict->entries[probe].key, key)) { \
            dict->entries[probe].value = value; \
            FREE_KEY_FN(entry.key); \
            return false; \
        } \
        if (entry.psl > dict->entries[probe].psl) { \
            NAME##_Entry tmp = dict->entries[probe]; \
            dict->entries[probe] = entry; \
            entry = tmp; \
        } \
        entry.psl++; \
    } \
    FREE_KEY_FN(entry.key); \
    return false; \
} \
\
static inline VALUE_TYPE NAME##_get(NAME *dict, KEY_TYPE key, VALUE_TYPE default_val) { \
    if (!dict) return default_val; \
    uint32_t hash = HASH_FN(key); \
    size_t idx = hash % dict->capacity; \
    for (int psl = 0; psl < (int)dict->capacity; psl++) { \
        size_t probe = (idx + psl) % dict->capacity; \
        if (dict->entries[probe].psl < 0 || dict->entries[probe].psl < psl) \
            return default_val; \
        if (dict->entries[probe].hash == hash && \
            EQ_FN(dict->entries[probe].key, key)) \
            return dict->entries[probe].value; \
    } \
    return default_val; \
} \
\
static inline VALUE_TYPE* NAME##_get_ptr(NAME *dict, KEY_TYPE key) { \
    if (!dict) return NULL; \
    uint32_t hash = HASH_FN(key); \
    size_t idx = hash % dict->capacity; \
    for (int psl = 0; psl < (int)dict->capacity; psl++) { \
        size_t probe = (idx + psl) % dict->capacity; \
        if (dict->entries[probe].psl < 0 || dict->entries[probe].psl < psl) \
            return NULL; \
        if (dict->entries[probe].hash == hash && \
            EQ_FN(dict->entries[probe].key, key)) \
            return &dict->entries[probe].value; \
    } \
    return NULL; \
} \
\
static inline bool NAME##_contains(NAME *dict, KEY_TYPE key) { \
    if (!dict) return false; \
    uint32_t hash = HASH_FN(key); \
    size_t idx = hash % dict->capacity; \
    for (int psl = 0; psl < (int)dict->capacity; psl++) { \
        size_t probe = (idx + psl) % dict->capacity; \
        if (dict->entries[probe].psl < 0 || dict->entries[probe].psl < psl) \
            return false; \
        if (dict->entries[probe].hash == hash && \
            EQ_FN(dict->entries[probe].key, key)) \
            return true; \
    } \
    return false; \
} \
\
static inline bool NAME##_remove(NAME *dict, KEY_TYPE key) { \
    if (!dict) return false; \
    uint32_t hash = HASH_FN(key); \
    size_t idx = hash % dict->capacity; \
    for (int psl = 0; psl < (int)dict->capacity; psl++) { \
        size_t probe = (idx + psl) % dict->capacity; \
        if (dict->entries[probe].psl < 0 || dict->entries[probe].psl < psl) \
            return false; \
        if (dict->entries[probe].hash == hash && \
            EQ_FN(dict->entries[probe].key, key)) { \
            FREE_KEY_FN(dict->entries[probe].key); \
            dict->entries[probe].psl = -1; \
            dict->size--; \
            size_t empty = probe; \
            for (size_t j = 1; j < dict->capacity; j++) { \
                size_t next = (probe + j) % dict->capacity; \
                if (dict->entries[next].psl <= 0) break; \
                dict->entries[empty] = dict->entries[next]; \
                dict->entries[empty].psl--; \
                dict->entries[next].psl = -1; \
                empty = next; \
            } \
            return true; \
        } \
    } \
    return false; \
} \
\
static inline size_t NAME##_size(NAME *dict) { \
    return dict ? dict->size : 0; \
} \
\
static inline size_t NAME##_capacity(NAME *dict) { \
    return dict ? dict->capacity : 0; \
} \
\
static inline bool NAME##_empty(NAME *dict) { \
    return !dict || dict->size == 0; \
} \
\
static inline void NAME##_clear(NAME *dict) { \
    if (!dict) return; \
    for (size_t i = 0; i < dict->capacity; i++) { \
        if (dict->entries[i].psl >= 0) { \
            FREE_KEY_FN(dict->entries[i].key); \
            dict->entries[i].psl = -1; \
        } \
    } \
    dict->size = 0; \
} \
\
static inline NAME##_Iterator NAME##_iter(NAME *dict) { \
    NAME##_Iterator iter = {dict, 0}; \
    return iter; \
} \
\
static inline bool NAME##_next(NAME##_Iterator *iter, KEY_TYPE *key, VALUE_TYPE *value) { \
    if (!iter || !iter->dict) return false; \
    while (iter->index < iter->dict->capacity) { \
        if (iter->dict->entries[iter->index].psl >= 0) { \
            if (key) *key = iter->dict->entries[iter->index].key; \
            if (value) *value = iter->dict->entries[iter->index].value; \
            iter->index++; \
            return true; \
        } \
        iter->index++; \
    } \
    return false; \
}

// ============================================================================
// Convenience macros for common types
// ============================================================================

// Dict<string, int>
#define DICT_DEFINE_STR_INT(NAME) \
    DICT_DEFINE(NAME, char*, int, dict_hash_str, dict_eq_str, dict_copy_str, dict_free_str)

// Dict<string, string>
#define DICT_DEFINE_STR_STR(NAME) \
    DICT_DEFINE(NAME, char*, char*, dict_hash_str, dict_eq_str, dict_copy_str, dict_free_str)

// Dict<string, double>
#define DICT_DEFINE_STR_DOUBLE(NAME) \
    DICT_DEFINE(NAME, char*, double, dict_hash_str, dict_eq_str, dict_copy_str, dict_free_str)

// Dict<string, float>
#define DICT_DEFINE_STR_FLOAT(NAME) \
    DICT_DEFINE(NAME, char*, float, dict_hash_str, dict_eq_str, dict_copy_str, dict_free_str)

// Dict<string, void*>
#define DICT_DEFINE_STR_PTR(NAME) \
    DICT_DEFINE(NAME, char*, void*, dict_hash_str, dict_eq_str, dict_copy_str, dict_free_str)

// Dict<int, int>
#define DICT_DEFINE_INT_INT(NAME) \
    DICT_DEFINE(NAME, int, int, dict_hash_int, dict_eq_int, dict_copy_val, dict_free_val)

// Dict<int, string>
#define DICT_DEFINE_INT_STR(NAME) \
    DICT_DEFINE(NAME, int, char*, dict_hash_int, dict_eq_int, dict_copy_val, dict_free_val)

// Dict<int, double>
#define DICT_DEFINE_INT_DOUBLE(NAME) \
    DICT_DEFINE(NAME, int, double, dict_hash_int, dict_eq_int, dict_copy_val, dict_free_val)

// Dict<int, void*>
#define DICT_DEFINE_INT_PTR(NAME) \
    DICT_DEFINE(NAME, int, void*, dict_hash_int, dict_eq_int, dict_copy_val, dict_free_val)

// Dict<uint32, int>
#define DICT_DEFINE_UINT32_INT(NAME) \
    DICT_DEFINE(NAME, uint32_t, int, dict_hash_uint32, dict_eq_uint32, dict_copy_val, dict_free_val)

// Dict<uint32, void*>
#define DICT_DEFINE_UINT32_PTR(NAME) \
    DICT_DEFINE(NAME, uint32_t, void*, dict_hash_uint32, dict_eq_uint32, dict_copy_val, dict_free_val)

// Dict<uint64, int>
#define DICT_DEFINE_UINT64_INT(NAME) \
    DICT_DEFINE(NAME, uint64_t, int, dict_hash_uint64, dict_eq_uint64, dict_copy_val, dict_free_val)

// Dict<uint64, void*>
#define DICT_DEFINE_UINT64_PTR(NAME) \
    DICT_DEFINE(NAME, uint64_t, void*, dict_hash_uint64, dict_eq_uint64, dict_copy_val, dict_free_val)

// Dict<ptr, int>
#define DICT_DEFINE_PTR_INT(NAME) \
    DICT_DEFINE(NAME, void*, int, dict_hash_ptr, dict_eq_ptr, dict_copy_val, dict_free_val)

// Dict<ptr, void*>
#define DICT_DEFINE_PTR_PTR(NAME) \
    DICT_DEFINE(NAME, void*, void*, dict_hash_ptr, dict_eq_ptr, dict_copy_val, dict_free_val)

#ifdef __cplusplus
}
#endif

#endif // DICT_H
