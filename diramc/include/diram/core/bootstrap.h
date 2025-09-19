#ifndef DIRAM_BOOTSTRAP_H
#define DIRAM_BOOTSTRAP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#ifdef _WIN32
    #include <windows.h>
    #define getpid _getpid
    typedef CRITICAL_SECTION pthread_mutex_t;
    #define PTHREAD_MUTEX_INITIALIZER {0}
    static inline int pthread_mutex_init(pthread_mutex_t *m, void *a) {
        InitializeCriticalSection(m);
        return 0;
    }
    static inline int pthread_mutex_lock(pthread_mutex_t *m) {
        EnterCriticalSection(m);
        return 0;
    }
    static inline int pthread_mutex_unlock(pthread_mutex_t *m) {
        LeaveCriticalSection(m);
        return 0;
    }
#else
    #include <pthread.h>
#endif

#define DIRAM_MAX_HEAP_EVENTS 3
#define DIRAM_TRACE_LOG_PATH "logs/diram_trace.log"

typedef struct {
    uint64_t command_epoch;
    uint32_t event_count;
} diram_heap_context_t;

typedef struct {
    void* base_addr;
    size_t size;
    uint64_t timestamp;
    uint32_t heap_events;
    pid_t binding_pid;
    char sha256_receipt[65];
} diram_allocation_t;

// Function declarations
int diram_bootstrap_init(void);
diram_allocation_t* diram_alloc_traced(size_t size, const char* tag);
void diram_free_traced(diram_allocation_t* alloc);
void diram_compute_receipt(diram_allocation_t* alloc, const char* tag);

#endif /* DIRAM_BOOTSTRAP_H */
