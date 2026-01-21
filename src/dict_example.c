/*
 * dict_example.c - Example usage of generic dict.h library
 * 
 * Compile: gcc -O3 -o dict_example dict_example.c
 * Run: ./dict_example
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "../include/dict.h"

// ============================================================================
// Define dictionary types
// ============================================================================

// Dict<string, int>
DICT_DEFINE_STR_INT(StrIntDict)

// Dict<string, double>
DICT_DEFINE_STR_DOUBLE(StrDoubleDict)

// Dict<int, int>
DICT_DEFINE_INT_INT(IntIntDict)

// Dict<int, string> - value is char* but NOT freed automatically
DICT_DEFINE_INT_STR(IntStrDict)

// Dict<string, void*> - for storing any pointers
DICT_DEFINE_STR_PTR(StrPtrDict)

// ============================================================================
// Helper
// ============================================================================

static inline uint64_t get_nanos(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

// ============================================================================
// Examples
// ============================================================================

void example_str_int(void) {
    printf("=== Dict<string, int> ===\n\n");
    
    StrIntDict *dict = StrIntDict_create();
    
    // Insert
    StrIntDict_set(dict, "apple", 10);
    StrIntDict_set(dict, "banana", 20);
    StrIntDict_set(dict, "cherry", 30);
    
    // Get
    printf("apple  = %d\n", StrIntDict_get(dict, "apple", -1));
    printf("banana = %d\n", StrIntDict_get(dict, "banana", -1));
    printf("mango  = %d (default)\n", StrIntDict_get(dict, "mango", -1));
    
    // Contains
    printf("\nContains 'apple': %s\n", StrIntDict_contains(dict, "apple") ? "yes" : "no");
    printf("Contains 'mango': %s\n", StrIntDict_contains(dict, "mango") ? "yes" : "no");
    
    // Iterate
    printf("\nAll entries:\n");
    StrIntDict_Iterator iter = StrIntDict_iter(dict);
    char *key;
    int value;
    while (StrIntDict_next(&iter, &key, &value)) {
        printf("  %s = %d\n", key, value);
    }
    
    // Size
    printf("\nSize: %zu\n", StrIntDict_size(dict));
    
    // Remove
    StrIntDict_remove(dict, "banana");
    printf("After remove 'banana', size: %zu\n", StrIntDict_size(dict));
    
    StrIntDict_destroy(dict);
    printf("\n");
}

void example_str_double(void) {
    printf("=== Dict<string, double> ===\n\n");
    
    StrDoubleDict *prices = StrDoubleDict_create();
    
    StrDoubleDict_set(prices, "BTC", 45000.50);
    StrDoubleDict_set(prices, "ETH", 2500.75);
    StrDoubleDict_set(prices, "SOL", 98.25);
    
    printf("BTC: $%.2f\n", StrDoubleDict_get(prices, "BTC", 0.0));
    printf("ETH: $%.2f\n", StrDoubleDict_get(prices, "ETH", 0.0));
    printf("SOL: $%.2f\n", StrDoubleDict_get(prices, "SOL", 0.0));
    printf("XRP: $%.2f (default)\n", StrDoubleDict_get(prices, "XRP", 0.0));
    
    StrDoubleDict_destroy(prices);
    printf("\n");
}

void example_int_int(void) {
    printf("=== Dict<int, int> ===\n\n");
    
    IntIntDict *squares = IntIntDict_create();
    
    // Store squares of numbers
    for (int i = 1; i <= 10; i++) {
        IntIntDict_set(squares, i, i * i);
    }
    
    printf("Squares:\n");
    for (int i = 1; i <= 10; i++) {
        printf("  %d^2 = %d\n", i, IntIntDict_get(squares, i, 0));
    }
    
    printf("\n15^2 = %d (not stored, default 0)\n", IntIntDict_get(squares, 15, 0));
    
    IntIntDict_destroy(squares);
    printf("\n");
}

void example_int_str(void) {
    printf("=== Dict<int, string> ===\n\n");
    
    IntStrDict *errors = IntStrDict_create();
    
    // HTTP status codes
    IntStrDict_set(errors, 200, "OK");
    IntStrDict_set(errors, 201, "Created");
    IntStrDict_set(errors, 400, "Bad Request");
    IntStrDict_set(errors, 401, "Unauthorized");
    IntStrDict_set(errors, 403, "Forbidden");
    IntStrDict_set(errors, 404, "Not Found");
    IntStrDict_set(errors, 500, "Internal Server Error");
    
    int codes[] = {200, 201, 404, 500, 999};
    for (int i = 0; i < 5; i++) {
        int code = codes[i];
        char *msg = IntStrDict_get(errors, code, "Unknown");
        printf("HTTP %d: %s\n", code, msg);
    }
    
    IntStrDict_destroy(errors);
    printf("\n");
}

void example_str_ptr(void) {
    printf("=== Dict<string, void*> ===\n\n");
    
    // Example: storing structs
    typedef struct {
        int id;
        char name[32];
        double balance;
    } User;
    
    User users[3] = {
        {1, "Alice", 1000.50},
        {2, "Bob", 2500.00},
        {3, "Charlie", 750.25}
    };
    
    StrPtrDict *user_db = StrPtrDict_create();
    
    // Store pointers to users
    StrPtrDict_set(user_db, "alice", &users[0]);
    StrPtrDict_set(user_db, "bob", &users[1]);
    StrPtrDict_set(user_db, "charlie", &users[2]);
    
    // Retrieve
    User *u = (User*)StrPtrDict_get(user_db, "bob", NULL);
    if (u) {
        printf("Found user: id=%d, name=%s, balance=%.2f\n", 
               u->id, u->name, u->balance);
    }
    
    // Modify via pointer
    u->balance += 100.0;
    printf("After deposit: balance=%.2f\n", u->balance);
    
    // Check non-existent
    User *unknown = (User*)StrPtrDict_get(user_db, "unknown", NULL);
    printf("Unknown user: %s\n", unknown ? unknown->name : "(NULL)");
    
    StrPtrDict_destroy(user_db);
    printf("\n");
}

void example_performance(void) {
    printf("=== Performance (Dict<string, int>) ===\n\n");
    
    const int N = 100000;
    char key[32];
    
    StrIntDict *dict = StrIntDict_create_with_capacity(N * 2);
    
    // Insert
    uint64_t start = get_nanos();
    for (int i = 0; i < N; i++) {
        snprintf(key, sizeof(key), "key_%d", i);
        StrIntDict_set(dict, key, i);
    }
    uint64_t end = get_nanos();
    printf("Insert %d: %.2f ns/op\n", N, (double)(end - start) / N);
    
    // Get (hit)
    start = get_nanos();
    volatile int sum = 0;
    for (int i = 0; i < N; i++) {
        snprintf(key, sizeof(key), "key_%d", i);
        sum += StrIntDict_get(dict, key, 0);
    }
    end = get_nanos();
    printf("Get (hit) %d: %.2f ns/op\n", N, (double)(end - start) / N);
    
    // Contains (miss)
    start = get_nanos();
    volatile int found = 0;
    for (int i = 0; i < N; i++) {
        snprintf(key, sizeof(key), "miss_%d", i);
        if (StrIntDict_contains(dict, key)) found++;
    }
    end = get_nanos();
    printf("Contains (miss) %d: %.2f ns/op\n", N, (double)(end - start) / N);
    
    printf("\nSize: %zu, Capacity: %zu\n", 
           StrIntDict_size(dict), StrIntDict_capacity(dict));
    
    StrIntDict_destroy(dict);
    printf("\n");
}

void example_word_count(void) {
    printf("=== Word Count Example ===\n\n");
    
    const char *text = "the quick brown fox jumps over the lazy dog "
                       "the fox is quick and the dog is lazy";
    
    StrIntDict *wc = StrIntDict_create();
    
    char *copy = strdup(text);
    char *word = strtok(copy, " ");
    while (word) {
        int count = StrIntDict_get(wc, word, 0);
        StrIntDict_set(wc, word, count + 1);
        word = strtok(NULL, " ");
    }
    free(copy);
    
    printf("Word frequencies:\n");
    StrIntDict_Iterator iter = StrIntDict_iter(wc);
    char *w;
    int c;
    while (StrIntDict_next(&iter, &w, &c)) {
        printf("  %-10s: %d\n", w, c);
    }
    
    StrIntDict_destroy(wc);
    printf("\n");
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
    printf("dict.h - Generic Dictionary Examples\n");
    printf("=====================================\n\n");
    
    example_str_int();
    example_str_double();
    example_int_int();
    example_int_str();
    example_str_ptr();
    example_word_count();
    example_performance();
    
    printf("All examples completed!\n");
    return 0;
}
