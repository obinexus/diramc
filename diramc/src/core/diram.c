
// ============================================================================
// src/core/diram.c - Core implementation
// ============================================================================

#include "diram/core/diram.h"
#include "diram/core/diram_phenomenological.h"  // Add this first
#include "diram/core/diram.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// Initialize phenomenological observer
diram_context_t* diram_init(void) {
    diram_context_t* ctx = calloc(1, sizeof(diram_context_t));
    
    // Create root phenomenon - the null state
    ctx->dag_root = create_dag_node((phenotype_t){.raw = 0}, 
                                    (axial_state_t){0, 0, 0, 0});
    ctx->current_state = ctx->dag_root;
    
    // Initialize observation apparatus
    ctx->observation_capacity = 1024;
    ctx->observation_buffer = calloc(ctx->observation_capacity, 
                                     sizeof(phenotype_t));
    
    // Set phenomenological thresholds
    ctx->phenomenon_threshold = 0.6f;  // 60% confidence required
    ctx->max_dag_depth = 32;           // Maximum state depth
    
    // Initialize triple-stream processor
    ctx->streams = init_triple_streams();
    
    return ctx;
}

// Observe current memory phenomena
phenotype_t diram_observe(diram_context_t* ctx, void* memory, size_t size) {
    phenotype_t observed = {.raw = 0};
    
    // Extract temporal phenomena
    uint64_t access_time = get_memory_access_time(memory);
    observed.fields.age = compute_age_bucket(access_time);
    observed.fields.frequency = compute_access_frequency(memory);
    observed.fields.volatility = measure_change_rate(memory, size);
    
    // Extract spatial phenomena
    observed.fields.locality = compute_spatial_locality(memory);
    observed.fields.clustering = measure_cluster_density(memory, size);
    observed.fields.spread = analyze_distribution_pattern(memory, size);
    
    // Extract causal phenomena (from triple-stream)
    triple_stream_result_t stream_result = query_triple_streams(ctx->streams);
    observed.fields.intent = extract_intent_strength(stream_result);
    observed.fields.dependency = trace_causal_chain_depth(memory);
    observed.fields.necessity = determine_necessity(stream_result);
    
    // Extract governance phenomena
    observed.fields.authority = check_permission_level(memory);
    observed.fields.compliance = verify_governance_state(ctx, memory);
    observed.fields.audit = get_audit_trail_depth(memory);
    
    // Record observation
    if (ctx->observation_count < ctx->observation_capacity) {
        ctx->observation_buffer[ctx->observation_count++] = observed;
    }
    
    return observed;
}

// Navigate DAG based on observed phenomena
dag_node_t* diram_navigate_dag(diram_context_t* ctx, phenotype_t target) {
    dag_node_t* current = ctx->current_state;
    uint32_t depth = 0;
    
    while (depth < ctx->max_dag_depth) {
        // Find best matching edge based on phenomenological distance
        dag_edge_t* best_edge = NULL;
        float best_score = 0.0f;
        
        for (uint32_t i = 0; i < current->edge_count; i++) {
            dag_edge_t* edge = current->edges[i];
            
            // Calculate phenomenological similarity
            float similarity = compute_phenotype_similarity(edge->trigger, target);
            float score = similarity * edge->probability;
            
            if (score > best_score && score > ctx->phenomenon_threshold) {
                best_score = score;
                best_edge = edge;
            }
        }
        
        if (!best_edge) {
            // No suitable transition found - create new state
            dag_node_t* new_state = create_dag_node(target, 
                compute_axial_state(target, current->axial));
            add_dag_edge(current, new_state, target, 0.5f);
            return new_state;
        }
        
        // Traverse edge
        current = best_edge->to;
        best_edge->traversal_count++;
        depth++;
        
        // Check if we've reached target phenomena
        if (compute_phenotype_similarity(current->phenotype, target) > 0.95f) {
            return current;
        }
    }
    
    return current;  // Return best state found within depth limit
}

// Allocate memory based on phenomena
void* diram_alloc(diram_context_t* ctx, size_t size, phenotype_t intent) {
    // 1. Observe current phenomena
    phenotype_t current = diram_observe(ctx, NULL, 0);
    
    // 2. Navigate DAG to find/create target state
    dag_node_t* target_state = diram_navigate_dag(ctx, intent);
    
    // 3. Compute axial intent vector
    axial_state_t axial = compute_axial_intent(current, intent, target_state);
    
    // 4. Query triple-stream for verification
    triple_stream_result_t verification = {
        .stream_a = encode_primary_intent(axial.x_intent),
        .stream_b = encode_verification(axial.y_verify),
        .stream_c = encode_governance(axial.z_govern)
    };
    
    if (!verify_triple_stream(ctx->streams, &verification)) {
        return NULL;  // Triple-stream verification failed
    }
    
    // 5. Perform actual allocation with phenomena tracking
    void* memory = perform_raw_allocation(size);
    if (!memory) return NULL;
    
    // 6. Tag memory with phenomena
    tag_memory_with_phenotype(memory, size, intent);
    
    // 7. Update DAG with observation
    ctx->current_state = target_state;
    target_state->observation_count++;
    
    return memory;
}

// Compute phenomenological similarity (0.0 to 1.0)
float compute_phenotype_similarity(phenotype_t a, phenotype_t b) {
    uint32_t diff = a.raw ^ b.raw;  // XOR to find differences
    uint32_t bit_distance = __builtin_popcount(diff);  // Count differing bits
    return 1.0f - (float)bit_distance / 32.0f;
}

// Compute axial state from phenomena
axial_state_t compute_axial_state(phenotype_t pheno, axial_state_t previous) {
    axial_state_t state;
    
    // Map phenomena to 3D intent space
    state.x_intent = (pheno.fields.intent << 8) | 
                     (pheno.fields.frequency << 4) | 
                     pheno.fields.age;
    
    state.y_verify = (pheno.fields.locality << 8) | 
                     (pheno.fields.clustering << 6) | 
                     (pheno.fields.dependency << 2) | 
                     pheno.fields.necessity;
    
    state.z_govern = (pheno.fields.authority << 8) | 
                     (pheno.fields.compliance << 6) | 
                     (pheno.fields.audit << 3) | 
                     pheno.fields.volatility;
    
    // Compute magnitude as distance from previous state
    uint32_t dx = abs((int)state.x_intent - (int)previous.x_intent);
    uint32_t dy = abs((int)state.y_verify - (int)previous.y_verify);
    uint32_t dz = abs((int)state.z_govern - (int)previous.z_govern);
    
    state.magnitude = (uint32_t)sqrt(dx*dx + dy*dy + dz*dz);
    
    return state;
}
