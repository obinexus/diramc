// include/diram/core/diram_phenomenological.h
#ifndef DIRAM_PHENOMENOLOGICAL_H
#define DIRAM_PHENOMENOLOGICAL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>  // For calloc

// ============================================================================
// PHENOMENOLOGICAL TYPE DEFINITIONS
// ============================================================================

// Bit field phenotype - encodes observable memory phenomena
typedef union {
    struct {
        // Temporal phenomena (8 bits each)
        uint32_t age:         4;  // Age bucket (0-15)
        uint32_t frequency:   4;  // Access frequency (0-15)
        uint32_t volatility:  4;  // Rate of change (0-15)
        
        // Spatial phenomena (8 bits each)
        uint32_t locality:    4;  // Spatial locality (0-15)
        uint32_t clustering:  4;  // Cluster density (0-15)
        uint32_t spread:      4;  // Distribution pattern (0-15)
        
        // Causal phenomena (8 bits each)
        uint32_t intent:      3;  // Intent strength (0-7)
        uint32_t dependency:  3;  // Causal chain depth (0-7)
        uint32_t necessity:   2;  // Necessity level (0-3)
    } fields;
    uint32_t raw;  // Raw bit representation
} phenotype_t;

// Axial state - 3D intent space representation
typedef struct {
    uint16_t x_intent;      // Primary intent axis
    uint16_t y_verify;      // Verification axis  
    uint16_t z_govern;      // Governance axis
    uint16_t magnitude;     // Distance from previous state
} axial_state_t;

// Triple-stream result - parallel verification streams
typedef struct {
    uint64_t stream_a;      // Primary instruction stream
    uint64_t stream_b;      // Verification stream
    uint64_t stream_c;      // Governance stream
    bool verified;          // Overall verification result
} triple_stream_result_t;

// Intent region - memory region with intent
typedef struct intent_region {
    uint64_t base_addr;     // Base address
    size_t size;            // Region size
    phenotype_t intent;     // Intent phenomena
    uint32_t flags;         // Region flags
} intent_region_t;

// Forward declarations
struct dag_node;
struct dag_edge;
struct triple_stream;

// DAG Edge - State transition triggered by phenomena
typedef struct dag_edge {
    struct dag_node* from;       // Source state
    struct dag_node* to;         // Target state
    phenotype_t trigger;         // Phenomenon that triggers transition
    float probability;           // Likelihood of transition
    uint64_t traversal_count;    // Times this edge was taken
} dag_edge_t;

// DAG Node - Observable memory state
typedef struct dag_node {
    phenotype_t phenotype;       // Observable phenomena
    axial_state_t axial;         // Position in 3D intent space
    uint64_t intent_region;      // Memory region identifier
    
    dag_edge_t** edges;          // State transitions
    uint32_t edge_count;
    uint32_t edge_capacity;
    
    // Phenomenological metrics
    float observation_confidence; // How clearly we observe this state
    float stability_score;        // How stable this state is
    uint64_t observation_count;   // Times this state was observed
    
    // Triple-stream correlation
    uint8_t stream_a_state[3];   // Primary instruction stream
    uint8_t stream_b_state[3];   // Verification stream
    uint8_t stream_c_state[3];   // Governance stream
} dag_node_t;

// Core DIRAM context - the phenomenological observer
typedef struct {
    dag_node_t* dag_root;           // Root of state DAG
    dag_node_t* current_state;      // Current observed state
    
    // Observation apparatus
    phenotype_t* observation_buffer;
    size_t observation_capacity;
    size_t observation_count;
    
    // Intent region map
    struct intent_region* regions;
    size_t region_count;
    
    // Triple-stream processor
    struct triple_stream* streams;
    
    // Phenomenological configuration
    float phenomenon_threshold;     // Minimum observation confidence
    uint32_t max_dag_depth;         // Maximum DAG traversal depth
    bool enable_hotwire;            // Allow emergency bypass
} diram_context_t;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

// Core initialization and observation
diram_context_t* diram_init(void);
phenotype_t diram_observe(diram_context_t* ctx, void* memory, size_t size);
dag_node_t* diram_navigate_dag(diram_context_t* ctx, phenotype_t target);
void* diram_alloc(diram_context_t* ctx, size_t size, phenotype_t intent);

// Phenomenological computations
float compute_phenotype_similarity(phenotype_t a, phenotype_t b);
axial_state_t compute_axial_state(phenotype_t pheno, axial_state_t previous);
axial_state_t compute_axial_intent(phenotype_t current, phenotype_t intent, dag_node_t* target);

// DAG operations
dag_node_t* create_dag_node(phenotype_t pheno, axial_state_t axial);
void add_dag_edge(dag_node_t* from, dag_node_t* to, phenotype_t trigger, float probability);

// Triple-stream operations
struct triple_stream* init_triple_streams(void);
triple_stream_result_t query_triple_streams(struct triple_stream* streams);
bool verify_triple_stream(struct triple_stream* streams, triple_stream_result_t* result);

// Encoding functions
uint64_t encode_primary_intent(uint16_t x_intent);
uint64_t encode_verification(uint16_t y_verify);
uint64_t encode_governance(uint16_t z_govern);

// Memory phenomena extraction functions
uint64_t get_memory_access_time(void* memory);
uint8_t compute_age_bucket(uint64_t access_time);
uint8_t compute_access_frequency(void* memory);
uint8_t measure_change_rate(void* memory, size_t size);
uint8_t compute_spatial_locality(void* memory);
uint8_t measure_cluster_density(void* memory, size_t size);
uint8_t analyze_distribution_pattern(void* memory, size_t size);

// Causal phenomena extraction
uint8_t extract_intent_strength(triple_stream_result_t result);
uint8_t trace_causal_chain_depth(void* memory);
uint8_t determine_necessity(triple_stream_result_t result);

// Governance phenomena extraction
uint8_t check_permission_level(void* memory);
uint8_t verify_governance_state(diram_context_t* ctx, void* memory);
uint8_t get_audit_trail_depth(void* memory);

// Raw memory operations
void* perform_raw_allocation(size_t size);
void tag_memory_with_phenotype(void* memory, size_t size, phenotype_t pheno);
void mark_memory_speculative(void* memory, size_t size);

// Predictive phenomena
phenotype_t predict_next_phenomenon(diram_context_t* ctx, dag_node_t* current);
int prefetch_by_phenomenon(diram_context_t* ctx, phenotype_t predicted);

#endif // DIRAM_PHENOMENOLOGICAL_H
