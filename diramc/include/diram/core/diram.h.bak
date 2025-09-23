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
#define DIRAM_TRACE_LOG_PATH "/var/log/diram/trace.log"
#define DIRAM_MAX_HEAP_EVENTS 1000

// Status structure
typedef struct {
    uint32_t err;
    int ok;
} diram_status_t;

// Base allocation structure (simplified, used in enhanced)
typedef struct {
    void* ptr;
    size_t size;
    char sha256_receipt[DIRAM_SHA256_HEX_LEN];
} diram_base_allocation_t;

// Traced allocation structure (for legacy trace functions)
typedef struct {
    void* base_addr;
    size_t size;
    uint64_t timestamp;
    uint32_t heap_events;
    pid_t binding_pid;
    char sha256_receipt[DIRAM_SHA256_HEX_LEN];
} diram_allocation_t;

// Enhanced allocation with metadata
typedef struct diram_enhanced_allocation {
    diram_base_allocation_t base;  // Changed from diram_allocation_t
    time_t timestamp;
    pid_t pid;
    char tag[128];
    uint32_t flags;
} diram_enhanced_allocation_t;

// Thread-local heap context
typedef struct {
    uint64_t command_epoch;
    uint32_t event_count;
} diram_heap_context_t;

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
diram_allocation_t* diram_alloc_traced(size_t size, const char* tag);
void diram_free_traced(diram_allocation_t* alloc);
void diram_compute_receipt(diram_allocation_t* alloc, const char* tag);
int diram_init_trace_log(void);
void diram_close_trace_log(void);

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


// Core DIRAM allocation functions
void* diram_alloc(diram_context_t* ctx, size_t size, phenotype_t intent);
void diram_free(diram_context_t* ctx, void* memory);

// Phenomenological operations
phenotype_t diram_observe(diram_context_t* ctx, void* memory, size_t size);
dag_node_t* diram_navigate_dag(diram_context_t* ctx, phenotype_t target);

// Similarity and state computation
float compute_phenotype_similarity(phenotype_t a, phenotype_t b);
axial_state_t compute_axial_state(phenotype_t pheno, axial_state_t previous);
axial_state_t compute_axial_intent(phenotype_t current, phenotype_t intent, dag_node_t* target);

// Context management
diram_context_t* diram_init(void);
void diram_destroy(diram_context_t* ctx);
#endif // DIRAM_CORE_H
