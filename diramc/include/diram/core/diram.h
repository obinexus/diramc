
// DIRAM Core Implementation Strategy
// Solving the phenomenological memory problem through directed axial DAG

// ============================================================================
// CORE PROBLEM: Memory as Observable Phenomena
// ============================================================================

// include/diram/core/diram.h - Primary phenomenological interface
#ifndef DIRAM_CORE_H
#define DIRAM_CORE_H

#include <stdint.h>
#include <stdbool.h>

// Bit field phenotype structure - observable memory behaviors
typedef union {
    struct {
        // Temporal phenomena (how memory changes over time)
        uint8_t age:        3;  // 0-7 age buckets
        uint8_t frequency:  3;  // 0-7 access frequency
        uint8_t volatility: 2;  // 0-3 change rate
        
        // Spatial phenomena (how memory relates in space)
        uint8_t locality:   4;  // 0-15 spatial proximity score
        uint8_t clustering: 2;  // 0-3 cluster density
        uint8_t spread:     2;  // 0-3 distribution pattern
        
        // Causal phenomena (why memory exists)
        uint8_t intent:     4;  // 0-15 intent strength
        uint8_t dependency: 3;  // 0-7 causal chain depth
        uint8_t necessity:  1;  // 0-1 required vs optional
        
        // Governance phenomena (how memory is controlled)
        uint8_t authority:  3;  // 0-7 permission level
        uint8_t compliance: 2;  // 0-3 governance state
        uint8_t audit:      3;  // 0-7 audit trail depth
    } fields;
    uint32_t raw;
} phenotype_t;

// Directed Axial State - 3D intent space navigation
typedef struct {
    uint16_t x_intent;      // Primary intent axis (what)
    uint16_t y_verify;      // Verification axis (how)
    uint16_t z_govern;      // Governance axis (why)
    uint32_t magnitude;     // Vector magnitude (strength)
} axial_state_t;

// DAG Node - Observable memory state
typedef struct dag_node {
    phenotype_t phenotype;          // Observable phenomena
    axial_state_t axial;            // Position in intent space
    uint64_t intent_region;         // Memory region identifier
    
    struct dag_edge** edges;       // State transitions
    uint32_t edge_count;
    uint32_t edge_capacity;
    
    // Phenomenological metrics
    float observation_confidence;   // How clearly we observe this state
    float stability_score;          // How stable this state is
    uint64_t observation_count;     // Times this state was observed
    
    // Triple-stream correlation
    uint8_t stream_a_state[3];     // Primary instruction stream
    uint8_t stream_b_state[3];     // Verification stream
    uint8_t stream_c_state[3];     // Governance stream
} dag_node_t;

// DAG Edge - State transition triggered by phenomena
typedef struct dag_edge {
    dag_node_t* from;               // Source state
    dag_node_t* to;                 // Target state
    phenotype_t trigger;            // Phenomenon that triggers transition
    float probability;              // Likelihood of transition
    uint64_t traversal_count;       // Times this edge was taken
} dag_edge_t;

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
