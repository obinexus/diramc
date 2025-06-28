
// core/feature-alloc/alloc.c
#include "alloc.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/file.h>

// Thread-local storage for heap event constraints
static __thread diram_heap_context_t heap_ctx = {0, 0};
static FILE* trace_log = NULL;
static pthread_mutex_t trace_mutex = PTHREAD_MUTEX_INITIALIZER;

// SHA-256 implementation (simplified for demonstration)
static void sha256_hex(const void* data, size_t len, char* output) {
    // In production, link against OpenSSL or similar
    // This is a placeholder implementation
    const uint8_t* bytes = (const uint8_t*)data;
    for (size_t i = 0; i < 32; i++) {
        sprintf(output + (i * 2), "%02x", bytes[i % len]);
    }
    output[64] = '\0';
}

void diram_compute_receipt(diram_allocation_t* alloc, const char* tag) {
    struct {
        void* addr;
        size_t size;
        uint64_t timestamp;
        char tag[64];
    } receipt_input;
    
    receipt_input.addr = alloc->base_addr;
    receipt_input.size = alloc->size;
    receipt_input.timestamp = alloc->timestamp;
    strncpy(receipt_input.tag, tag ? tag : "untagged", 63);
    receipt_input.tag[63] = '\0';
    
    sha256_hex(&receipt_input, sizeof(receipt_input), alloc->sha256_receipt);
}

static int check_heap_constraint(uint64_t current_epoch) {
    // Reset counter if we're in a new command epoch
    if (heap_ctx.command_epoch != current_epoch) {
        heap_ctx.event_count = 0;
        heap_ctx.command_epoch = current_epoch;
    }
    
    // Enforce e(x) = 0.6 constraint (max 3 events)
    if (heap_ctx.event_count >= DIRAM_MAX_HEAP_EVENTS) {
        return -1;  // Constraint violation
    }
    
    heap_ctx.event_count++;
    return 0;
}

int diram_init_trace_log(void) {
    pthread_mutex_lock(&trace_mutex);
    
    if (trace_log != NULL) {
        pthread_mutex_unlock(&trace_mutex);
        return 0;  // Already initialized
    }
    
    trace_log = fopen(DIRAM_TRACE_LOG_PATH, "a");
    if (trace_log == NULL) {
        pthread_mutex_unlock(&trace_mutex);
        return -1;
    }
    
    // Enable line buffering for immediate trace visibility
    setvbuf(trace_log, NULL, _IOLBF, 0);
    
    // Write header
    fprintf(trace_log, "# DIRAM Allocation Trace Log\n");
    fprintf(trace_log, "# Format: TIMESTAMP|PID|OPERATION|ADDRESS|SIZE|SHA256|TAG\n");
    fflush(trace_log);
    
    pthread_mutex_unlock(&trace_mutex);
    return 0;
}

void diram_close_trace_log(void) {
    pthread_mutex_lock(&trace_mutex);
    if (trace_log != NULL) {
        fclose(trace_log);
        trace_log = NULL;
    }
    pthread_mutex_unlock(&trace_mutex);
}

diram_allocation_t* diram_alloc_traced(size_t size, const char* tag) {
    // Get current timestamp for epoch checking
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t current_epoch = ts.tv_sec;
    
    // Check heap event constraint
    if (check_heap_constraint(current_epoch) < 0) {
        // Constraint violation - defer allocation
        return NULL;
    }
    
    // Allocate tracking structure
    diram_allocation_t* alloc = malloc(sizeof(diram_allocation_t));
    if (alloc == NULL) {
        heap_ctx.event_count--;  // Rollback counter
        return NULL;
    }
    
    // Perform actual memory allocation
    alloc->base_addr = malloc(size);
    if (alloc->base_addr == NULL) {
        free(alloc);
        heap_ctx.event_count--;  // Rollback counter
        return NULL;
    }
    
    // Initialize allocation metadata
    alloc->size = size;
    alloc->timestamp = (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
    alloc->heap_events = heap_ctx.event_count;
    alloc->binding_pid = getpid();
    
    // Generate SHA-256 receipt
    diram_compute_receipt(alloc, tag);
    
    // Write trace entry (no runtime reflection)
    pthread_mutex_lock(&trace_mutex);
    if (trace_log != NULL) {
        fprintf(trace_log, "%lu|%d|ALLOC|%p|%zu|%s|%s\n",
                alloc->timestamp,
                alloc->binding_pid,
                alloc->base_addr,
                alloc->size,
                alloc->sha256_receipt,
                tag ? tag : "untagged");
        fflush(trace_log);
    }
    pthread_mutex_unlock(&trace_mutex);
    
    return alloc;
}

void diram_free_traced(diram_allocation_t* alloc) {
    if (alloc == NULL) {
        return;
    }
    
    // Verify PID binding for fork safety
    if (alloc->binding_pid != getpid()) {
        // Fork detected - this allocation belongs to parent process
        // In production, implement proper fork handling
        return;
    }
    
    // Get timestamp for trace
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t timestamp = (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
    
    // Write trace entry before freeing
    pthread_mutex_lock(&trace_mutex);
    if (trace_log != NULL) {
        fprintf(trace_log, "%lu|%d|FREE|%p|%zu|%s|traced\n",
                timestamp,
                getpid(),
                alloc->base_addr,
                alloc->size,
                alloc->sha256_receipt);
        fflush(trace_log);
    }
    pthread_mutex_unlock(&trace_mutex);
    
    // Free memory
    free(alloc->base_addr);
    
    // Clear sensitive data before freeing tracker
    memset(alloc, 0, sizeof(diram_allocation_t));
    free(alloc);
}
