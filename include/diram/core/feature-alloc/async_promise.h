// include/diram/core/feature-alloc/async_promise.h
#ifndef DIRAM_ASYNC_PROMISE_H
#define DIRAM_ASYNC_PROMISE_H

#include <stdint.h>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <synchapi.h>
#include <process.h>
#include <sys/types.h>

typedef HANDLE pthread_mutex_t;
typedef CONDITION_VARIABLE pthread_cond_t;
typedef DWORD pthread_t;
#else
#include <pthread.h>
#endif
#include "feature_alloc.h"

typedef enum {
    PROMISE_STATE_PENDING = 0,
    PROMISE_STATE_RESOLVED,
    PROMISE_STATE_REJECTED,
    PROMISE_STATE_CANCELLED
} diram_promise_state_t;

typedef enum {
    REJECT_REASON_NONE = 0,
    REJECT_REASON_FATAL_ERROR,      // Process-killing error
    REJECT_REASON_UNCAUGHT_EXCEPTION,
    REJECT_REASON_GOVERNANCE_VIOLATION,
    REJECT_REASON_MEMORY_EXHAUSTED,
    REJECT_REASON_TIMEOUT,
    REJECT_REASON_CANCELLED
} diram_reject_reason_t;

// Promise receipt for resilience tracking
typedef struct {
    uint64_t promise_id;
    uint64_t creation_timestamp;
    char allocation_receipt[DIRAM_SHA256_HEX_LEN];
    diram_promise_state_t state;
    diram_reject_reason_t reject_reason;
    pid_t creator_pid;
    pthread_t creator_thread;
} diram_promise_receipt_t;

// Async promise structure
typedef struct diram_async_promise {
    diram_promise_receipt_t receipt;
    
    // State management
    pthread_mutex_t state_mutex;
    pthread_cond_t state_cond;
    
    // Result storage
    union {
        diram_enhanced_allocation_t* resolved_allocation;
        diram_error_context_t rejection_context;
    } result;
    
    // Callback functions
    void (*on_resolve)(struct diram_async_promise*, diram_enhanced_allocation_t*);
    void (*on_reject)(struct diram_async_promise*, diram_reject_reason_t, const char*);
    void* callback_context;
    
    // Lookahead cache hint
    size_t lookahead_size;
    uint32_t cache_priority;
} diram_async_promise_t;

// Async allocation API
diram_async_promise_t* diram_alloc_async(
    size_t size,
    const char* tag,
    diram_memory_space_t* space,
    size_t lookahead_hint
);

// Promise lifecycle
int diram_promise_await(diram_async_promise_t* promise, uint64_t timeout_ms);
int diram_promise_resolve(diram_async_promise_t* promise, diram_enhanced_allocation_t* alloc);
int diram_promise_reject(diram_async_promise_t* promise, diram_reject_reason_t reason, const char* msg);
void diram_promise_destroy(diram_async_promise_t* promise);

// Status check (err, ok) pattern
typedef struct {
    diram_error_code_t err;
    int ok;  // 1 = success, 0 = failure
} diram_status_t;

diram_status_t diram_promise_get_status(diram_async_promise_t* promise);

#endif // DIRAM_ASYNC_PROMISE_H