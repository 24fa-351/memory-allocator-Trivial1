#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "heap_manager.h"

#ifdef USE_SYSTEM_MALLOC
    #define heap_malloc malloc
    #define heap_free free
    #define heap_realloc realloc
    #define heap_init(x) ((void)(x)) // No-op for system malloc
    #define heap_cleanup() ((void)0) // No-op for system malloc
#else
    #include "heap_manager.h"
#endif

#define NUM_THREADS 4



// Small Allocations Test
void test_small_allocations() {
    printf("Test: Small Allocations\n");
    void* ptr1 = heap_malloc(16);
    void* ptr2 = heap_malloc(32);
    void* ptr3 = heap_malloc(48);

    if (ptr1 && ptr2 && ptr3) {
        printf("Small allocations succeeded.\n");
    } else {
        printf("Small allocations failed.\n");
    }

    heap_free(ptr1);
    heap_free(ptr2);
    heap_free(ptr3);
}

// Realloc Test
void test_realloc() {
    printf("Test: Realloc\n");
    void* ptr = heap_malloc(64);
    if (!ptr) {
        printf("Initial allocation failed.\n");
        return;
    }

    ptr = heap_realloc(ptr, 128);
    if (ptr) {
        printf("Realloc succeeded.\n");
    } else {
        printf("Realloc failed.\n");
    }

    heap_free(ptr);
}

// Coalesce Test
void test_coalesce() {
    printf("Test: Coalesce Free Blocks\n");
    void* ptr1 = heap_malloc(64);
    void* ptr2 = heap_malloc(64);

    heap_free(ptr1);
    heap_free(ptr2);

    printf("Coalesce test completed.\n");
}

// Multi-threaded Test
void* thread_test(void* arg) {
    for (int i = 0; i < 100; i++) {
        void* ptr = heap_malloc(64);
        if (ptr) {
            memset(ptr, i, 64); // Fill memory with a test pattern
            heap_free(ptr);
        }
    }
    return NULL;
}

void test_multithreading() {
    printf("Test: Multi-Threading\n");

    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_test, NULL);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Multi-threading test completed.\n");
}

int main(int argc, char* argv[]) {
    size_t heap_size = 1024 * 1024; // 1 MB
    heap_init(heap_size);

    test_small_allocations();
    test_realloc();
    test_coalesce();
    test_multithreading();

    heap_cleanup();
    return 0;
}
