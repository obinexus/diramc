#ifndef DIRAM_PHENOMENOLOGICAL_H
#define DIRAM_PHENOMENOLOGICAL_H

#include <stdint.h>
#include <stdint.h>
#include <stddef.h>  // For size_t
#include <stdbool.h> // For bool type

// Phenomenological type capturing observable memory phenomena
typedef union {
    struct {
        // Temporal phenomena (8 bits)
        uint32_t age : 3;           // Age bucket (0-7)
        uint32_t frequency : 3;     // Access frequency level
        uint32_t volatility : 2;    // Change rate
        
        // Spatial phenomena (8 bits)
        uint32_t locality : 3;      // Spatial locality strength
        uint32_t clustering : 3;    // Cluster density
        uint32_t spread : 2;        // Distribution pattern
        
        // Causal phenomena (8 bits)
        uint32_t intent : 3;        // Intent strength
        uint32_t dependency : 3;    // Causal chain depth
        uint32_t necessity : 2;     // Necessity level
        
        // Governance phenomena (8 bits) - MISSING FIELDS
        uint32_t authority : 3;     // Permission level
        uint32_t compliance : 3;    // Governance state compliance
        uint32_t audit : 2;         // Audit trail depth
    } fields;
    uint32_t raw;                   // Raw 32-bit representation
} phenotype_t;

// Axial state representing 3D intent space
typedef struct {
    uint16_t x_intent;    // Primary intent axis
    uint16_t y_verify;    // Verification axis
    uint16_t z_govern;    // Governance axis
    uint16_t magnitude;   // Vector magnitude
} axial_state_t;

// DAG node for state navigation
typedef struct dag_node {
    phenotype_t phenotype;
    axial_state_t axial;
    struct dag_edge** edges;
    uint32_t edge_count;
    uint32_t edge_capacity;
    uint32_t observation_count;
    float observation_confidence; 
    float stability_score;         
} dag_node_t;


// DAG edge with probabilistic transition
typedef struct dag_edge {
    dag_node_t* from;
    dag_node_t* to;
    phenotype_t trigger;
    float probability;
    uint32_t traversal_count;
} dag_edge_t;

// Triple-stream processing result

typedef struct {
    uint64_t stream_a;  // Primary intent stream
    uint64_t stream_b;  // Verification stream
    uint64_t stream_c;  // Governance stream
    bool verified;      // Add verification status
} triple_stream_result_t;

// Triple-stream processor
typedef struct {
    triple_stream_result_t current;
    triple_stream_result_t previous;
    uint32_t epoch;
} triple_stream_t;

// Main context structure
typedef struct {
    dag_node_t* dag_root;
    dag_node_t* current_state;
    phenotype_t* observation_buffer;
    uint32_t observation_count;
    uint32_t observation_capacity;
    float phenomenon_threshold;
    uint32_t max_dag_depth;
    triple_stream_t* streams;
} diram_context_t;

// Function prototypes for phenomenological operations
dag_node_t* create_dag_node(phenotype_t pheno, axial_state_t axial);
void add_dag_edge(dag_node_t* from, dag_node_t* to, phenotype_t trigger, float probability);
triple_stream_t* init_triple_streams(void);
triple_stream_result_t query_triple_streams(triple_stream_t* streams);

// Helper functions referenced in diram.c
uint64_t get_memory_access_time(void* memory);
uint8_t compute_age_bucket(uint64_t time);
uint8_t compute_access_frequency(void* memory);
uint8_t measure_change_rate(void* memory, size_t size);
uint8_t compute_spatial_locality(void* memory);
uint8_t measure_cluster_density(void* memory, size_t size);
uint8_t analyze_distribution_pattern(void* memory, size_t size);
uint8_t extract_intent_strength(triple_stream_result_t result);
uint8_t trace_causal_chain_depth(void* memory);
uint8_t determine_necessity(triple_stream_result_t result);
uint8_t check_permission_level(void* memory);
uint8_t verify_governance_state(diram_context_t* ctx, void* memory);
uint8_t get_audit_trail_depth(void* memory);
axial_state_t compute_axial_intent(phenotype_t current, phenotype_t intent, dag_node_t* target);
uint64_t encode_primary_intent(uint16_t intent);
uint64_t encode_verification(uint16_t verify);
uint64_t encode_governance(uint16_t govern);
int verify_triple_stream(triple_stream_t* streams, triple_stream_result_t* verification);
void* perform_raw_allocation(size_t size);
void tag_memory_with_phenotype(void* memory, size_t size, phenotype_t pheno);
float compute_phenotype_similarity(phenotype_t a, phenotype_t b);
axial_state_t compute_axial_state(phenotype_t pheno, axial_state_t previous);

#endif // DIRAM_PHENOMENOLOGICAL_H
