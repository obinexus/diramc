// include/diram/core/feature-alloc/async_promise.h
#ifndef DIRAM_ASYNC_PROMISE_H
#define DIRAM_ASYNC_PROMISE_H

#include <pthread.h>
#include <stdint.h>
#include <time.h>

// Forward declarations
typedef struct diram_memory_space diram_memory_space_t;
typedef struct diram_enhanced_allocation diram_enhanced_allocation_t;
// Result union - FIX THIS SECTION
    union {
        diram_enhanced_allocation_t* resolved_allocation;
        struct {
            int code;                  // Error code
            time_t timestamp;          // When rejection occurred  
            pid_t pid;                 // Process ID
            const char* file;          // Source file
            int line;                  // Source line
            char context[256];         // Error message
            int severity;              // Severity level
        } rejection_context;
    } result;
    
// ... rest of file ...

// Add missing rejection reason
typedef enum {
    REJECT_REASON_MEMORY_EXHAUSTED,
    REJECT_REASON_TIMEOUT,
    REJECT_REASON_CANCELLED,
    REJECT_REASON_FATAL_ERROR,
    REJECT_REASON_GOVERNANCE_VIOLATION  // Add this
} diram_reject_reason_t;

// Add function declarations that are missing
int diram_promise_resolve(diram_async_promise_t* promise, 
                         diram_enhanced_allocation_t* alloc);
int diram_promise_reject(diram_async_promise_t* promise, 
                        diram_reject_reason_t reason, 
                        const char* msg);
diram_status_t diram_promise_get_status(diram_async_promise_t* promise);
// Promise states
typedef enum {
    PROMISE_STATE_PENDING,
    PROMISE_STATE_RESOLVED,
    PROMISE_STATE_REJECTED
} diram_promise_state_t;

// Rejection reasons
typedef enum {
    REJECT_REASON_MEMORY_EXHAUSTED,
    REJECT_REASON_TIMEOUT,
    REJECT_REASON_CANCELLED,
    REJECT_REASON_FATAL_ERROR
} diram_reject_reason_t;

// Promise receipt
typedef struct {
    uint64_t promise_id;
    time_t creation_timestamp;
    diram_promise_state_t state;
    diram_reject_reason_t reject_reason;
    pid_t creator_pid;
    pthread_t creator_thread;
    char allocation_receipt[65];  // SHA256 hex
} diram_promise_receipt_t;

// Main promise structure
typedef struct diram_async_promise {
    diram_promise_receipt_t receipt;
    pthread_mutex_t state_mutex;
    pthread_cond_t state_cond;
    
    // Callbacks
    void (*on_resolve)(struct diram_async_promise*, diram_enhanced_allocation_t*);
    void (*on_reject)(struct diram_async_promise*, diram_reject_reason_t, const char*);
    
    void* callback_context;
    size_t lookahead_size;
    uint32_t cache_priority;
    
    // Result union
    union {
        diram_enhanced_allocation_t* resolved_allocation;
        struct {
            int errno_code;
        } rejection_context;
    } result;
} diram_async_promise_t;

// API functions
diram_async_promise_t* diram_alloc_async(size_t size, const char* tag, 
                                          diram_memory_space_t* space, 
                                          size_t lookahead_hint);
int diram_promise_await(diram_async_promise_t* promise, uint64_t timeout_ms);
void diram_promise_destroy(diram_async_promise_t* promise);

#endif // DIRAM_ASYNC_PROMISE_H
