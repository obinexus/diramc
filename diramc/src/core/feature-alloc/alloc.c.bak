#include "diram/core/diram.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
static int clock_gettime(int clk_id, struct timespec* ts) {
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    ts->tv_sec = (time_t)(count.QuadPart / freq.QuadPart);
    ts->tv_nsec = (long)(((count.QuadPart % freq.QuadPart) * 1000000000ULL) / freq.QuadPart);
    return 0;
}
#else
#include <time.h>
#endif

static __thread diram_heap_context_t heap_ctx = {0, 0};
static FILE* trace_log = NULL;
static pthread_mutex_t trace_mutex = PTHREAD_MUTEX_INITIALIZER;

static void sha256_hex(const void* data, size_t len, char* output) {
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
    } input;
    
    input.addr = alloc->base_addr;
    input.size = alloc->size;
    input.timestamp = alloc->timestamp;
    strncpy(input.tag, tag ? tag : "untagged", 63);
    sha256_hex(&input, sizeof(input), alloc->sha256_receipt);
}

int diram_init_trace_log(void) {
    pthread_mutex_lock(&trace_mutex);
    if (trace_log != NULL) {
        pthread_mutex_unlock(&trace_mutex);
        return 0;
    }
    
    trace_log = fopen(DIRAM_TRACE_LOG_PATH, "a");
    if (trace_log == NULL) {
        pthread_mutex_unlock(&trace_mutex);
        return -1;
    }
    
    setvbuf(trace_log, NULL, _IOLBF, 0);
    fprintf(trace_log, "# DIRAM Trace Log\n");
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
    struct timespec ts = {0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    
    if (heap_ctx.command_epoch != (uint64_t)ts.tv_sec) {
        heap_ctx.event_count = 0;
        heap_ctx.command_epoch = ts.tv_sec;
    }
    
    if (heap_ctx.event_count >= DIRAM_MAX_HEAP_EVENTS) {
        return NULL;
    }
    
    diram_allocation_t* alloc = calloc(1, sizeof(diram_allocation_t));
    if (!alloc) return NULL;
    
    alloc->base_addr = malloc(size);
    if (!alloc->base_addr) {
        free(alloc);
        return NULL;
    }
    
    heap_ctx.event_count++;
    alloc->size = size;
    alloc->timestamp = (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
    alloc->heap_events = heap_ctx.event_count;
    alloc->binding_pid = getpid();
    
    diram_compute_receipt(alloc, tag);
    
    pthread_mutex_lock(&trace_mutex);
    if (trace_log != NULL) {
        fprintf(trace_log, "%lu|%d|ALLOC|%p|%zu|%s|%s\n",
                alloc->timestamp, alloc->binding_pid,
                alloc->base_addr, alloc->size,
                alloc->sha256_receipt, tag ? tag : "untagged");
        fflush(trace_log);
    }
    pthread_mutex_unlock(&trace_mutex);
    
    return alloc;
}

void diram_free_traced(diram_allocation_t* alloc) {
    if (!alloc) return;
    if (alloc->binding_pid != getpid()) return;
    
    struct timespec ts = {0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    
    pthread_mutex_lock(&trace_mutex);
    if (trace_log != NULL) {
        fprintf(trace_log, "%lu|%d|FREE|%p|%zu|%s|traced\n",
                (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec,
                getpid(), alloc->base_addr, alloc->size,
                alloc->sha256_receipt);
        fflush(trace_log);
    }
    pthread_mutex_unlock(&trace_mutex);
    
    free(alloc->base_addr);
    free(alloc);
}
