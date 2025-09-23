// include/diram/core/diram.h
#ifndef DIRAM_CORE_H
#define DIRAM_CORE_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

// Error codes aligned with OBINexus governance
#define DIRAM_ERR_NONE                 0x0000
#define DIRAM_ERR_MEMORY_EXHAUSTED     0x1001
#define DIRAM_ERR_INVALID_ARG          0x1002
#define DIRAM_ERR_TIMEOUT              0x100B
#define DIRAM_ERR_CANCELLED            0x100C
#define DIRAM_ERR_PENDING              0x100D
#define DIRAM_ERR_FATAL                0x100F
#define DIRAM_ERR_UNKNOWN              0x1010
#define DIRAM_ERR_GOVERNANCE_FAIL      0x1011

#define DIRAM_SHA256_HEX_LEN           65

// Status structure
typedef struct {
    uint32_t err;
    int ok;
} diram_status_t;

// Base allocation structure
typedef struct {
    void* ptr;
    size_t size;
    char sha256_receipt[DIRAM_SHA256_HEX_LEN];
} diram_allocation_t;

// Enhanced allocation with metadata - COMPLETE DEFINITION
typedef struct diram_enhanced_allocation {
    diram_allocation_t base;
    time_t timestamp;
    pid_t pid;
    char tag[128];
    uint32_t flags;
} diram_enhanced_allocation_t;

// Memory space structure
typedef struct diram_memory_space {
    char space_name[64];
    size_t limit_bytes;
    size_t used_bytes;
    pid_t owner_pid;
    pthread_mutex_t lock;
    void* base;
    uint32_t flags;
} diram_memory_space_t;

// Core allocation functions
diram_enhanced_allocation_t* diram_alloc_enhanced(
    size_t size,
    const char* tag,
    diram_memory_space_t* space
);

// Space management
diram_memory_space_t* diram_space_create(const char* name, size_t limit);
void diram_space_destroy(diram_memory_space_t* space);
int diram_space_check_limit(diram_memory_space_t* space, size_t requested);
void diram_error_index_init(void);
void diram_error_index_shutdown(void);

#endif // DIRAM_CORE_H
