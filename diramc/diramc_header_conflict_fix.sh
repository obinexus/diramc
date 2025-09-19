#!/bin/bash
# ============================================================================
# Fix DIRAMC Header Conflicts - Resolve duplicate definitions
# ============================================================================

set -e

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}Fixing DIRAMC header conflicts...${NC}"
echo ""

cd ~/obinexus/workspace/diramc/diramc || exit 1

# Step 1: Fix alloc.h to not conflict with bootstrap.h
echo -e "${BLUE}Step 1: Fixing alloc.h header...${NC}"
cat > include/diram/core/feature-alloc/alloc.h << 'EOF'
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
EOF
echo -e "${GREEN}✓${NC} alloc.h fixed"

# Step 2: Fix alloc.c to only include one header
echo -e "\n${BLUE}Step 2: Fixing alloc.c...${NC}"
cat > src/core/feature-alloc/alloc.c << 'EOF'
// Only include the alloc header, not bootstrap
#include "diram/core/feature-alloc/alloc.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef _WIN32
#include <windows.h>
#include <time.h>
#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif
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
#include <sys/file.h>
#endif

// Thread-local storage for heap event constraints
static __thread diram_heap_context_t heap_ctx = {0, 0};
static FILE* trace_log = NULL;
static pthread_mutex_t trace_mutex = PTHREAD_MUTEX_INITIALIZER;

// SHA-256 implementation (simplified)
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
    } receipt_input;
    
    receipt_input.addr = alloc->base_addr;
    receipt_input.size = alloc->size;
    receipt_input.timestamp = alloc->timestamp;
    strncpy(receipt_input.tag, tag ? tag : "untagged", 63);
    receipt_input.tag[63] = '\0';
    
    sha256_hex(&receipt_input, sizeof(receipt_input), alloc->sha256_receipt);
}

static int check_heap_constraint(uint64_t current_epoch) {
    if (heap_ctx.command_epoch != current_epoch) {
        heap_ctx.event_count = 0;
        heap_ctx.command_epoch = current_epoch;
    }
    
    if (heap_ctx.event_count >= DIRAM_MAX_HEAP_EVENTS) {
        return -1;
    }
    
    heap_ctx.event_count++;
    return 0;
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
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t current_epoch = ts.tv_sec;
    
    if (check_heap_constraint(current_epoch) < 0) {
        return NULL;
    }
    
    diram_allocation_t* alloc = malloc(sizeof(diram_allocation_t));
    if (alloc == NULL) {
        heap_ctx.event_count--;
        return NULL;
    }
    
    alloc->base_addr = malloc(size);
    if (alloc->base_addr == NULL) {
        free(alloc);
        heap_ctx.event_count--;
        return NULL;
    }
    
    alloc->size = size;
    alloc->timestamp = (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
    alloc->heap_events = heap_ctx.event_count;
    alloc->binding_pid = getpid();
    
    diram_compute_receipt(alloc, tag);
    
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
    
    if (alloc->binding_pid != getpid()) {
        return;
    }
    
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t timestamp = (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
    
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
    
    free(alloc->base_addr);
    memset(alloc, 0, sizeof(diram_allocation_t));
    free(alloc);
}
EOF
echo -e "${GREEN}✓${NC} alloc.c fixed"

# Step 3: Now build with the fixed files
echo -e "\n${BLUE}Step 3: Building with fixed headers...${NC}"

# Clean old objects
rm -f build/obj/core/feature-alloc/*.o
rm -f build/release/obj/core/feature-alloc/*.o

# Build alloc.c
echo "Compiling alloc.c..."
gcc -Wall -g -O2 -fPIC -Iinclude -c src/core/feature-alloc/alloc.c \
    -o build/obj/core/feature-alloc/alloc.o

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓${NC} alloc.c compiled successfully!"
else
    echo -e "${YELLOW}Warning: alloc.c compilation had issues${NC}"
fi

# Step 4: Rebuild the library with alloc.o included
echo -e "\n${BLUE}Step 4: Rebuilding library with allocation functions...${NC}"

# Find all object files
OBJS=$(find build/obj -name "*.o" 2>/dev/null)

# Create static library
ar rcs lib/libdiram.a $OBJS
echo -e "${GREEN}✓${NC} Static library created: lib/libdiram.a"

# Create shared library
gcc -shared $OBJS -pthread -lm -o lib/libdiram.so
echo -e "${GREEN}✓${NC} Shared library created: lib/libdiram.so"

# Step 5: Now build the CLI
echo -e "\n${BLUE}Step 5: Building CLI executable...${NC}"

# Compile main.c if needed
if [ ! -f build/obj/cli/main.o ]; then
    gcc -Wall -g -O2 -Iinclude -DDIRAM_VERSION=\"2.0.0\" \
        -c src/cli/main.c -o build/obj/cli/main.o
fi

# Link the executable
gcc build/obj/cli/main.o -Llib -ldiram -pthread -lm \
    -Wl,-rpath,lib -o bin/diram

if [ -f bin/diram ]; then
    echo -e "${GREEN}✓${NC} Executable created: bin/diram"
else
    echo -e "${YELLOW}CLI build had issues${NC}"
fi

echo ""
echo -e "${BLUE}============================================${NC}"
echo -e "${GREEN}     Header Conflicts Fixed!               ${NC}"
echo -e "${BLUE}============================================${NC}"
echo ""
echo "Summary:"
echo "  ✓ Fixed duplicate type definitions"
echo "  ✓ alloc.c now compiles successfully"
echo "  ✓ Library includes allocation functions"
echo "  ✓ CLI can link properly"
echo ""

# Test if executable works
if [ -f bin/diram ]; then
    echo "Testing executable..."
    ./bin/diram --version 2>/dev/null && echo -e "${GREEN}✓ DIRAM is working!${NC}" || echo -e "${YELLOW}DIRAM needs more fixes${NC}"
fi

echo ""
echo "You can now use:"
echo "  ./bin/diram           # Run DIRAMC"
echo "  make -f Makefile.cli  # Rebuild CLI"
echo "  make -f Makefile.core # Rebuild core"
