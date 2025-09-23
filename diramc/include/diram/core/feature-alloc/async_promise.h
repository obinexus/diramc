// include/diram/core/feature-alloc/async_promise.h
// Enhanced async promise system with JavaScript-inspired patterns
#ifndef DIRAM_ASYNC_PROMISE_H
#define DIRAM_ASYNC_PROMISE_H

#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <stdbool.h>

// Forward declarations for circular dependencies
typedef struct diram_memory_space diram_memory_space_t;
typedef struct diram_allocation diram_allocation_t;

// DIRAM allocation base structure - must be defined before enhanced
typedef struct diram_allocation {
    void* ptr;
    size_t size;
    char* tag;
    char sha256_receipt[65];  // SHA256 hex
    uint64_t timestamp;
    uint32_t flags;
} diram_allocation_t;

// Phenotype for allocation characteristics 
typedef struct {
    uint32_t memory_type;      // DRAM, HBM, NVM, etc
    uint32_t access_pattern;    // Sequential, random, strided
    uint32_t lifetime_hint;     // Short, medium, long
    uint32_t priority;          // 0-255 priority level
} phenotype_t;

// Axial state for multi-dimensional allocation
typedef struct {
    uint32_t axis_x;    // Spatial dimension X
    uint32_t axis_y;    // Spatial dimension Y  
    uint32_t axis_z;    // Spatial dimension Z
    uint32_t axis_t;    // Temporal dimension
} axial_state_t;

// Triple stream results for concurrent operations
typedef struct {
    void* stream_a;     // Primary data stream
    void* stream_b;     // Secondary/shadow stream
    void* stream_c;     // Tertiary/validation stream
    uint64_t sync_point;
} triple_stream_result_t;

// Enhanced allocation with advanced features
typedef struct diram_enhanced_allocation {
    diram_allocation_t base;
    phenotype_t phenotype;
    axial_state_t axial_state;
    triple_stream_result_t streams;
} diram_enhanced_allocation_t;

// Status structure
typedef struct diram_status {
    uint32_t err;
    int ok;
} diram_status_t;

// Promise states - JavaScript-inspired
typedef enum {
    PROMISE_STATE_PENDING,
    PROMISE_STATE_RESOLVED,
    PROMISE_STATE_REJECTED,
    PROMISE_STATE_SETTLED  // Either resolved or rejected
} diram_promise_state_t;

// Rejection reasons
typedef enum {
    REJECT_REASON_MEMORY_EXHAUSTED,
    REJECT_REASON_TIMEOUT,
    REJECT_REASON_CANCELLED,
    REJECT_REASON_FATAL_ERROR,
    REJECT_REASON_GOVERNANCE_VIOLATION,
    REJECT_REASON_LOOKAHEAD_MISS
} diram_reject_reason_t;

// Promise receipt for tracking
typedef struct {
    uint64_t promise_id;
    time_t creation_timestamp;
    diram_promise_state_t state;
    diram_reject_reason_t reject_reason;
    pid_t creator_pid;
    pthread_t creator_thread;
    char allocation_receipt[65];
} diram_promise_receipt_t;

// Forward declaration of promise type for callbacks
typedef struct diram_async_promise diram_async_promise_t;

// JavaScript-style thenable interface
typedef struct {
    diram_async_promise_t* (*then)(
        diram_async_promise_t* self,
        void (*onFulfilled)(diram_enhanced_allocation_t*),
        void (*onRejected)(diram_reject_reason_t, const char*)
    );
    diram_async_promise_t* (*catch)(
        diram_async_promise_t* self,
        void (*onRejected)(diram_reject_reason_t, const char*)
    );
    void (*finally)(
        diram_async_promise_t* self,
        void (*callback)(void)
    );
} diram_thenable_t;

// Promise chain node for .then() chaining
typedef struct diram_promise_chain {
    void (*onFulfilled)(diram_enhanced_allocation_t*);
    void (*onRejected)(diram_reject_reason_t, const char*);
    diram_async_promise_t* next_promise;
    struct diram_promise_chain* next;
} diram_promise_chain_t;

// Main async promise structure - JavaScript Promise-inspired
typedef struct diram_async_promise {
    // State management
    diram_promise_receipt_t receipt;
    pthread_mutex_t state_mutex;
    pthread_cond_t state_cond;
    
    // Thenable interface for JavaScript-style chaining
    diram_thenable_t thenable;
    
    // Promise chain for multiple then() calls
    diram_promise_chain_t* chain_head;
    diram_promise_chain_t* chain_tail;
    
    // Primary callbacks (JavaScript executor pattern)
    void (*on_resolve)(struct diram_async_promise*, diram_enhanced_allocation_t*);
    void (*on_reject)(struct diram_async_promise*, diram_reject_reason_t, const char*);
    void (*on_finally)(struct diram_async_promise*);
    
    // Context and configuration
    void* callback_context;
    size_t lookahead_size;
    uint32_t cache_priority;
    bool is_chained;
    
    // Lookahead computation parameters
    struct {
        uint64_t prediction_confidence;  // 0-100 percentage
        size_t predicted_next_size;
        uint32_t access_pattern_hint;
        bool prefetch_enabled;
    } lookahead;
    
    // Result union
    union {
        diram_enhanced_allocation_t* resolved_allocation;
        struct {
            int code;
            time_t timestamp;
            pid_t pid;
            const char* file;
            int line;
            char context[256];
            int severity;
        } rejection_context;
    } result;
} diram_async_promise_t;

// Promise.all() implementation
typedef struct {
    diram_async_promise_t** promises;
    size_t count;
    diram_async_promise_t* aggregate_promise;
} diram_promise_all_t;

// Promise.race() implementation  
typedef struct {
    diram_async_promise_t** promises;
    size_t count;
    diram_async_promise_t* winner_promise;
} diram_promise_race_t;

// Core API functions
diram_async_promise_t* diram_promise_create(
    void (*executor)(
        void (*resolve)(diram_enhanced_allocation_t*),
        void (*reject)(diram_reject_reason_t, const char*)
    )
);

diram_async_promise_t* diram_alloc_async(
    size_t size, 
    const char* tag,
    diram_memory_space_t* space,
    size_t lookahead_hint
);

diram_async_promise_t* diram_alloc_with_lookahead(
    size_t size,
    const char* tag,
    diram_memory_space_t* space,
    uint32_t access_pattern_hint
);

// JavaScript-style static methods
diram_async_promise_t* diram_promise_resolve(diram_enhanced_allocation_t* value);
diram_async_promise_t* diram_promise_reject(diram_reject_reason_t reason, const char* msg);
diram_promise_all_t* diram_promise_all(diram_async_promise_t** promises, size_t count);
diram_promise_race_t* diram_promise_race(diram_async_promise_t** promises, size_t count);

// Promise operations
int diram_promise_await(diram_async_promise_t* promise, uint64_t timeout_ms);
int diram_promise_resolve_internal(diram_async_promise_t* promise, diram_enhanced_allocation_t* alloc);
int diram_promise_reject_internal(diram_async_promise_t* promise, diram_reject_reason_t reason, const char* msg);
void diram_promise_destroy(diram_async_promise_t* promise);
diram_status_t diram_promise_get_status(diram_async_promise_t* promise);

// Enhanced allocation function declaration
diram_enhanced_allocation_t* diram_alloc_enhanced(
    size_t size,
    const char* tag,
    diram_memory_space_t* space
);

// Constants
#define DIRAM_SHA256_HEX_LEN           65
#define DIRAM_ERR_NONE                 0x0000
#define DIRAM_ERR_MEMORY_EXHAUSTED     0x1001
#define DIRAM_ERR_GOVERNANCE_FAIL      0x1011
#define DIRAM_ERR_TIMEOUT              0x100B
#define DIRAM_ERR_CANCELLED            0x100C
#define DIRAM_ERR_PENDING              0x100D
#define DIRAM_ERR_INVALID_ARG          0x100E
#define DIRAM_ERR_FATAL                0x100F
#define DIRAM_ERR_UNKNOWN              0x1010

#endif // DIRAM_ASYNC_PROMISE_H
