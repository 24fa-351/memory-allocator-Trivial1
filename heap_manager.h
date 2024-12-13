#ifndef HEAP_MANAGER_H
#define HEAP_MANAGER_H

#include <stddef.h> // For size_t

void* heap_malloc(size_t size);
void heap_free(void* ptr);
void* heap_realloc(void* ptr, size_t size);
void heap_init(size_t total_size);
void heap_cleanup();

#endif
