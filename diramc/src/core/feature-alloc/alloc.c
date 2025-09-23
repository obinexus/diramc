// src/core/feature-alloc/alloc.c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>  // For PRIu64 and other portable format specifiers
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "diram/core/feature-alloc/alloc.h"
#include "diram/core/feature-alloc/async_promise.h"

#define MAX_ALLOCATIONS 1024
#define TRACE_BUFFER_SIZE 4096

typedef struct allocation_entry {
    void* address;
    size_t size;
    char tag[256];
    uint64_t timestamp;
    uint8_t sha256[32];  // SHA-256 receipt
    int is_traced;
} allocation_entry_t;

typedef struct {
    allocation_entry_t entries[MAX_ALLOCATIONS];
    int count;
    int trace_enabled;
    FILE* trace_file;
    pthread_mutex_t mutex;
    uint64_t total_allocated;
    uint64_t total_freed;
} allocation_manager_t;

static allocation_manager_t g_alloc_mgr = {
    .count = 0,
    .trace_enabled = 0,
    .trace_file = NULL,
    .total_allocated = 0,
    .total_freed = 0
};

// Initialize allocation manager
void diram_alloc_init(int enable_trace) {
    pthread_mutex_init(&g_alloc_mgr.mutex, NULL);
    g_alloc_mgr.trace_enabled = enable_trace;
    
    if (enable_trace) {
        g_alloc_mgr.trace_file = fopen("alloc_trace.log", "w");
        if (g_alloc_mgr.trace_file) {
            fprintf(g_alloc_mgr.trace_file, "# DIRAM Allocation Trace Log\n");
            fprintf(g_alloc_mgr.trace_file, "# Timestamp, Operation, Address, Size, Tag\n");
        }
    }
}

// Generate SHA-256 receipt (simplified for example)
void generate_receipt(void* addr, size_t size, uint8_t* receipt) {
    // Simplified hash - in production use proper SHA-256
    uint64_t hash_input = (uint64_t)addr ^ size ^ time(NULL);
    memcpy(receipt, &hash_input, sizeof(hash_input));
    // Fill rest with pattern
    for (int i = sizeof(hash_input); i < 32; i++) {
        receipt[i] = (uint8_t)(hash_input >> (i % 8));
    }
}

// Thread-safe allocation with tracing
void* diram_alloc(size_t size, const char* tag) {
    pthread_mutex_lock(&g_alloc_mgr.mutex);
    
    if (g_alloc_mgr.count >= MAX_ALLOCATIONS) {
        fprintf(stderr, "Error: Maximum allocations reached\n");
        pthread_mutex_unlock(&g_alloc_mgr.mutex);
        return NULL;
    }
    
    void* ptr = malloc(size);
    if (!ptr) {
        pthread_mutex_unlock(&g_alloc_mgr.mutex);
        return NULL;
    }
    
    // Register allocation
    allocation_entry_t* entry = &g_alloc_mgr.entries[g_alloc_mgr.count];
    entry->address = ptr;
    entry->size = size;
    entry->timestamp = (uint64_t)time(NULL);
    strncpy(entry->tag, tag ? tag : "unnamed", 255);
    entry->is_traced = g_alloc_mgr.trace_enabled;
    generate_receipt(ptr, size, entry->sha256);
    
    g_alloc_mgr.count++;
    g_alloc_mgr.total_allocated += size;
    
    // Trace if enabled
    if (g_alloc_mgr.trace_enabled && g_alloc_mgr.trace_file) {
        fprintf(g_alloc_mgr.trace_file, 
                "%" PRIu64 ", ALLOC, %p, %zu, %s\n",
                entry->timestamp, ptr, size, entry->tag);
        fflush(g_alloc_mgr.trace_file);
    }
    
    pthread_mutex_unlock(&g_alloc_mgr.mutex);
    return ptr;
}

// Thread-safe deallocation with verification
int diram_free(void* ptr) {
    if (!ptr) return 0;
    
    pthread_mutex_lock(&g_alloc_mgr.mutex);
    
    // Find and verify allocation
    int found = -1;
    for (int i = 0; i < g_alloc_mgr.count; i++) {
        if (g_alloc_mgr.entries[i].address == ptr) {
            found = i;
            break;
        }
    }
    
    if (found == -1) {
        fprintf(stderr, "Error: Attempted to free untracked pointer %p\n", ptr);
        pthread_mutex_unlock(&g_alloc_mgr.mutex);
        return -1;
    }
    
    allocation_entry_t* entry = &g_alloc_mgr.entries[found];
    g_alloc_mgr.total_freed += entry->size;
    
    // Trace deallocation
    if (g_alloc_mgr.trace_enabled && g_alloc_mgr.trace_file) {
        // FIX: Use PRIu64 for portable uint64_t formatting
        fprintf(g_alloc_mgr.trace_file, 
                "%" PRIu64 ", FREE, %p, %zu, %s\n",
                (uint64_t)time(NULL), ptr, entry->size, entry->tag);
        fflush(g_alloc_mgr.trace_file);
    }
    
    // Free memory
    free(ptr);
    
    // Remove from tracking (compact array)
    for (int i = found; i < g_alloc_mgr.count - 1; i++) {
        g_alloc_mgr.entries[i] = g_alloc_mgr.entries[i + 1];
    }
    g_alloc_mgr.count--;
    
    pthread_mutex_unlock(&g_alloc_mgr.mutex);
    return 0;
}

// Trace a specific allocation
void diram_trace(void* ptr) {
    pthread_mutex_lock(&g_alloc_mgr.mutex);
    
    for (int i = 0; i < g_alloc_mgr.count; i++) {
        if (g_alloc_mgr.entries[i].address == ptr) {
            allocation_entry_t* entry = &g_alloc_mgr.entries[i];
            
            printf("[TRACE] Address: %p\n", entry->address);
            printf("[TRACE] Size: %zu bytes\n", entry->size);
            printf("[TRACE] Tag: %s\n", entry->tag);
            // FIX: Use PRIu64 for uint64_t
            printf("[TRACE] Timestamp: %" PRIu64 "\n", entry->timestamp);
            printf("[TRACE] SHA-256: ");
            for (int j = 0; j < 32; j++) {
                printf("%02x", entry->sha256[j]);
            }
            printf("\n");
            
            pthread_mutex_unlock(&g_alloc_mgr.mutex);
            return;
        }
    }
    
    printf("[TRACE] Pointer %p not found in allocations\n", ptr);
    pthread_mutex_unlock(&g_alloc_mgr.mutex);
}

// Get allocation statistics
void diram_get_stats(diram_stats_t* stats) {
    pthread_mutex_lock(&g_alloc_mgr.mutex);
    
    stats->total_allocated = g_alloc_mgr.total_allocated;
    stats->total_freed = g_alloc_mgr.total_freed;
    stats->current_allocated = g_alloc_mgr.total_allocated - g_alloc_mgr.total_freed;
    stats->allocation_count = g_alloc_mgr.count;
    stats->trace_enabled = g_alloc_mgr.trace_enabled;
    
    pthread_mutex_unlock(&g_alloc_mgr.mutex);
}

// Cleanup
void diram_alloc_cleanup() {
    pthread_mutex_lock(&g_alloc_mgr.mutex);
    
    if (g_alloc_mgr.trace_file) {
        // FIX: Use PRIu64 for final stats
        fprintf(g_alloc_mgr.trace_file, 
                "# Final Stats: Allocated=%" PRIu64 ", Freed=%" PRIu64 ", Leaked=%" PRIu64 "\n",
                g_alloc_mgr.total_allocated, 
                g_alloc_mgr.total_freed,
                g_alloc_mgr.total_allocated - g_alloc_mgr.total_freed);
        fclose(g_alloc_mgr.trace_file);
    }
    
    // Warn about leaks
    if (g_alloc_mgr.count > 0) {
        fprintf(stderr, "Warning: %d allocations not freed\n", g_alloc_mgr.count);
        for (int i = 0; i < g_alloc_mgr.count; i++) {
            fprintf(stderr, "  Leak: %p (%zu bytes, tag: %s)\n",
                    g_alloc_mgr.entries[i].address,
                    g_alloc_mgr.entries[i].size,
                    g_alloc_mgr.entries[i].tag);
        }
    }
    
    pthread_mutex_unlock(&g_alloc_mgr.mutex);
    pthread_mutex_destroy(&g_alloc_mgr.mutex);
}
