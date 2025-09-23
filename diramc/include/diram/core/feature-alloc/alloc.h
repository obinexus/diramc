// include/diram/core/feature-alloc/alloc.h
#ifndef DIRAM_ALLOC_H
#define DIRAM_ALLOC_H

#include <stddef.h>
#include <stdint.h>

// Statistics structure
typedef struct {
    uint64_t total_allocated;
    uint64_t total_freed;
    uint64_t current_allocated;
    int allocation_count;
    int trace_enabled;
} diram_stats_t;

// Initialize allocation system
void diram_alloc_init(int enable_trace);

// Allocate memory with tagging and tracing
void* diram_alloc(size_t size, const char* tag);

// Free memory with verification
int diram_free(void* ptr);

// Trace specific allocation
void diram_trace(void* ptr);

// Get allocation statistics
void diram_get_stats(diram_stats_t* stats);

// Cleanup and report leaks
void diram_alloc_cleanup(void);

// Advanced features for async operations
typedef struct {
    void* (*allocator)(size_t);
    void (*deallocator)(void*);
    int (*tracer)(void*, const char*);
} diram_alloc_ops_t;

// Set custom allocator operations
void diram_set_alloc_ops(const diram_alloc_ops_t* ops);

#endif // DIRAM_ALLOC_H
