#ifndef DIRAM_ALLOC_H
#define DIRAM_ALLOC_H

// Only include this header if bootstrap.h hasn't been included
#ifndef DIRAM_BOOTSTRAP_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <unistd.h>

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

// Function declarations
diram_allocation_t* diram_alloc_traced(size_t size, const char* tag);
void diram_free_traced(diram_allocation_t* alloc);
int diram_init_trace_log(void);
void diram_close_trace_log(void);
void diram_compute_receipt(diram_allocation_t* alloc, const char* tag);

#else
// If bootstrap.h is included, just add the missing functions
int diram_init_trace_log(void);
void diram_close_trace_log(void);
#endif /* DIRAM_BOOTSTRAP_H */

#endif /* DIRAM_ALLOC_H */
