#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "heap_manager.h"

// Memory block header structure
typedef struct block_header {
    size_t size;
    struct block_header* next;
    int free;
} block_header_t;

// Global variables
static void* heap_start = NULL;
static size_t heap_size = 0;
static block_header_t* free_list = NULL;
static pthread_mutex_t heap_lock = PTHREAD_MUTEX_INITIALIZER; // Thread safety lock

// Helper function to align size to 8 bytes
static size_t align_size(size_t size) {
    return (size + 7) & ~7;
}

// Initialize heap
void heap_init(size_t total_size) {
    heap_start = sbrk(total_size);
    if (heap_start == (void*)-1) {
        fprintf(stderr, "Failed to allocate heap memory.\n");
        exit(1);
    }
    heap_size = total_size;

    // Initialize free list with one large free block
    free_list = (block_header_t*)heap_start;
    free_list->size = total_size - sizeof(block_header_t);
    free_list->next = NULL;
    free_list->free = 1;
}

// Find a suitable block for allocation
static block_header_t* find_free_block(size_t size) {
    block_header_t* current = free_list;
    while (current) {
        if (current->free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Split a block into two if it's large enough
static void split_block(block_header_t* block, size_t size) {
    if (block->size >= size + sizeof(block_header_t) + 8) {
        block_header_t* new_block = (block_header_t*)((char*)block + sizeof(block_header_t) + size);
        new_block->size = block->size - size - sizeof(block_header_t);
        new_block->next = block->next;
        new_block->free = 1;

        block->size = size;
        block->next = new_block;
    }
}

// Allocate memory
void* heap_malloc(size_t size) {
    pthread_mutex_lock(&heap_lock);

    size = align_size(size);
    block_header_t* block = find_free_block(size);
    if (!block) {
        fprintf(stderr, "Out of memory.\n");
        pthread_mutex_unlock(&heap_lock);
        return NULL;
    }

    block->free = 0;
    split_block(block, size);

    pthread_mutex_unlock(&heap_lock);
    return (void*)((char*)block + sizeof(block_header_t));
}

// Coalesce adjacent free blocks
static void coalesce() {
    block_header_t* current = free_list;
    while (current && current->next) {
        if (current->free && current->next->free) {
            current->size += sizeof(block_header_t) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

// Free memory
void heap_free(void* ptr) {
    if (!ptr) return;

    pthread_mutex_lock(&heap_lock);
    block_header_t* block = (block_header_t*)((char*)ptr - sizeof(block_header_t));
    block->free = 1;

    coalesce();
    pthread_mutex_unlock(&heap_lock);
}

// Reallocate memory
void* heap_realloc(void* ptr, size_t size) {
    pthread_mutex_lock(&heap_lock);

    if (!ptr) {
        pthread_mutex_unlock(&heap_lock);
        return heap_malloc(size);
    }

    block_header_t* block = (block_header_t*)((char*)ptr - sizeof(block_header_t));
    if (block->size >= size) {
        pthread_mutex_unlock(&heap_lock);
        return ptr; // The current block is already large enough
    }

    void* new_ptr = heap_malloc(size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, block->size);
        heap_free(ptr);
    }

    pthread_mutex_unlock(&heap_lock);
    return new_ptr;
}

// Cleanup heap
void heap_cleanup() {
    brk(heap_start);
    heap_start = NULL;
    heap_size = 0;
    free_list = NULL;
}
