#ifndef DIRAM_H
#define DIRAM_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>

#ifdef _WIN32
    #include <windows.h>
    #define getpid _getpid
    typedef CRITICAL_SECTION pthread_mutex_t;
    #define PTHREAD_MUTEX_INITIALIZER {0}
#else
    #include <pthread.h>
#endif

#define DIRAM_MAX_HEAP_EVENTS 3
#define DIRAM_SHA256_HEX_LEN 65
#define DIRAM_TRACE_LOG_PATH "logs/diram_trace.log"

// Core types
typedef struct {
    void* base_addr;
    size_t size;
    uint64_t timestamp;
    uint32_t heap_events;
    pid_t binding_pid;
    char sha256_receipt[65];
} diram_allocation_t;

typedef struct {
    uint64_t command_epoch;
    uint32_t event_count;
} diram_heap_context_t;

// Memory space management
typedef struct {
    char space_name[64];
    size_t limit_bytes;
    size_t used_bytes;
    uint32_t allocation_count;
    pid_t owner_pid;
    int isolation_active;
    pthread_mutex_t lock;
} diram_memory_space_t;

// Error codes
typedef enum {
    DIRAM_ERR_NONE = 0,
    DIRAM_ERR_HEAP_CONSTRAINT = 0x1001,
    DIRAM_ERR_MEMORY_EXHAUSTED = 0x1002,
    DIRAM_ERR_BOUNDARY_VIOLATION = 0x1003,
    DIRAM_ERR_PID_MISMATCH = 0x1004
} diram_error_code_t;

// Core allocation API
diram_allocation_t* diram_alloc_traced(size_t size, const char* tag);
void diram_free_traced(diram_allocation_t* alloc);
int diram_init_trace_log(void);
void diram_close_trace_log(void);
void diram_compute_receipt(diram_allocation_t* alloc, const char* tag);

// Memory space API
diram_memory_space_t* diram_space_create(const char* name, size_t limit);
void diram_space_destroy(diram_memory_space_t* space);
int diram_space_check_limit(diram_memory_space_t* space, size_t requested);

// Error management
void diram_error_index_init(void);
void diram_error_index_shutdown(void);

#endif /* DIRAM_H */
