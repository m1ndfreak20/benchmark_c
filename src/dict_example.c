/*
 * dict_example.c - Example usage of dict.h library
 * 
 * Compile: gcc -O3 -o dict_example dict_example.c
 * Run: ./dict_example
 */

#define _GNU_SOURCE
#define DICT_IMPLEMENTATION
#include "../include/dict.h"

#include <stdio.h>
#include <time.h>
#include <string.h>

// Helper to get current time in nanoseconds
static inline uint64_t get_nanos(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

void example_basic_usage(void) {
    printf("=== Basic Usage ===\n\n");
    
    // Create dictionary
    Dict *dict = dict_create();
    
    // Insert key-value pairs
    dict_set(dict, "apple", 10);
    dict_set(dict, "banana", 20);
    dict_set(dict, "cherry", 30);
    
    // Get values
    printf("apple  = %d\n", dict_get(dict, "apple", -1));
    printf("banana = %d\n", dict_get(dict, "banana", -1));
    printf("cherry = %d\n", dict_get(dict, "cherry", -1));
    printf("mango  = %d (not found, returns default)\n", dict_get(dict, "mango", -1));
    
    // Check if key exists
    printf("\nContains 'apple': %s\n", dict_contains(dict, "apple") ? "yes" : "no");
    printf("Contains 'mango': %s\n", dict_contains(dict, "mango") ? "yes" : "no");
    
    // Update value
    dict_set(dict, "apple", 100);
    printf("\nAfter update, apple = %d\n", dict_get(dict, "apple", -1));
    
    // Get size
    printf("\nDictionary size: %zu\n", dict_size(dict));
    
    // Remove key
    dict_remove(dict, "banana");
    printf("After removing 'banana', size: %zu\n", dict_size(dict));
    printf("Contains 'banana': %s\n", dict_contains(dict, "banana") ? "yes" : "no");
    
    // Cleanup
    dict_destroy(dict);
    printf("\n");
}

void example_iteration(void) {
    printf("=== Iteration ===\n\n");
    
    Dict *dict = dict_create();
    
    // Add some items
    dict_set(dict, "one", 1);
    dict_set(dict, "two", 2);
    dict_set(dict, "three", 3);
    dict_set(dict, "four", 4);
    dict_set(dict, "five", 5);
    
    // Iterate over all key-value pairs
    printf("All entries:\n");
    DictIterator iter = dict_iter(dict);
    const char *key;
    int value;
    while (dict_next(&iter, &key, &value)) {
        printf("  %s = %d\n", key, value);
    }
    
    dict_destroy(dict);
    printf("\n");
}

void example_word_count(void) {
    printf("=== Word Count Example ===\n\n");
    
    const char *text = "the quick brown fox jumps over the lazy dog "
                       "the fox is quick and the dog is lazy";
    
    Dict *word_count = dict_create();
    
    // Tokenize and count words
    char *text_copy = strdup(text);
    char *token = strtok(text_copy, " ");
    while (token != NULL) {
        int count = dict_get(word_count, token, 0);
        dict_set(word_count, token, count + 1);
        token = strtok(NULL, " ");
    }
    free(text_copy);
    
    // Print word counts
    printf("Word frequencies:\n");
    DictIterator iter = dict_iter(word_count);
    const char *word;
    int count;
    while (dict_next(&iter, &word, &count)) {
        printf("  %-10s: %d\n", word, count);
    }
    
    printf("\nMost common word 'the' appears %d times\n", 
           dict_get(word_count, "the", 0));
    
    dict_destroy(word_count);
    printf("\n");
}

void example_performance(void) {
    printf("=== Performance Test ===\n\n");
    
    const int N = 100000;
    char key[32];
    
    // Pre-create dictionary with capacity
    Dict *dict = dict_create_with_capacity(N * 2);
    
    // Benchmark insert
    uint64_t start = get_nanos();
    for (int i = 0; i < N; i++) {
        snprintf(key, sizeof(key), "key_%d", i);
        dict_set(dict, key, i);
    }
    uint64_t end = get_nanos();
    double insert_ns = (double)(end - start) / N;
    printf("Insert %d elements: %.2f ns/op\n", N, insert_ns);
    
    // Benchmark get (hit)
    start = get_nanos();
    volatile int sum = 0;
    for (int i = 0; i < N; i++) {
        snprintf(key, sizeof(key), "key_%d", i);
        sum += dict_get(dict, key, 0);
    }
    end = get_nanos();
    double get_ns = (double)(end - start) / N;
    printf("Get (hit) %d elements: %.2f ns/op\n", N, get_ns);
    
    // Benchmark contains (hit)
    start = get_nanos();
    volatile int found = 0;
    for (int i = 0; i < N; i++) {
        snprintf(key, sizeof(key), "key_%d", i);
        if (dict_contains(dict, key)) found++;
    }
    end = get_nanos();
    double contains_ns = (double)(end - start) / N;
    printf("Contains (hit) %d elements: %.2f ns/op\n", N, contains_ns);
    
    // Benchmark contains (miss)
    start = get_nanos();
    found = 0;
    for (int i = 0; i < N; i++) {
        snprintf(key, sizeof(key), "missing_%d", i);
        if (dict_contains(dict, key)) found++;
    }
    end = get_nanos();
    double miss_ns = (double)(end - start) / N;
    printf("Contains (miss) %d elements: %.2f ns/op\n", N, miss_ns);
    
    printf("\nDictionary stats:\n");
    printf("  Size: %zu\n", dict_size(dict));
    printf("  Capacity: %zu\n", dict_capacity(dict));
    printf("  Load factor: %.2f%%\n", dict_load_factor(dict) * 100);
    
    dict_destroy(dict);
    printf("\n");
}

void example_get_ptr(void) {
    printf("=== Modify Value via Pointer ===\n\n");
    
    Dict *dict = dict_create();
    
    dict_set(dict, "counter", 0);
    
    // Increment counter using pointer
    for (int i = 0; i < 5; i++) {
        int *ptr = dict_get_ptr(dict, "counter");
        if (ptr) {
            (*ptr)++;
        }
    }
    
    printf("Counter after 5 increments: %d\n", dict_get(dict, "counter", 0));
    
    dict_destroy(dict);
    printf("\n");
}

int main(void) {
    printf("dict.h - Dictionary<string, int> Examples\n");
    printf("==========================================\n\n");
    
    example_basic_usage();
    example_iteration();
    example_word_count();
    example_get_ptr();
    example_performance();
    
    printf("All examples completed successfully!\n");
    return 0;
}
