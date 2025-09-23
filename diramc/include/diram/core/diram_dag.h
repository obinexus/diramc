// include/diram/core/diram_dag.h
// DAG (Directed Acyclic Graph) structures for OBINexus DIRAM phenomenological memory
#ifndef DIRAM_DAG_H
#define DIRAM_DAG_H

#include <stdint.h>
#include "diram/core/diram_phenomenological.h"

// DAG navigation states for memory phenomena tracking
typedef enum {
    DAG_STATE_IDLE,
    DAG_STATE_TRAVERSING,
    DAG_STATE_PREDICTING,
    DAG_STATE_BACKTRACKING,
    DAG_STATE_COMPLETE
} dag_state_t;

// DAG traversal context
typedef struct {
    dag_node_t* current_node;
    dag_node_t* root_node;
    uint32_t depth;
    uint32_t max_depth;
    dag_state_t state;
    float cumulative_probability;
} dag_traversal_context_t;

// DAG graph structure for phenomenological memory patterns
typedef struct {
    dag_node_t* root;
    uint32_t node_count;
    uint32_t edge_count;
    uint64_t generation;  // For versioning the DAG structure
    pthread_rwlock_t lock;
} diram_dag_t;

// DAG operations
diram_dag_t* diram_dag_create(void);
void diram_dag_destroy(diram_dag_t* dag);

// Node operations
dag_node_t* diram_dag_add_node(diram_dag_t* dag, phenotype_t pheno);
int diram_dag_connect_nodes(dag_node_t* from, dag_node_t* to, 
                            phenotype_t trigger, float probability);

// Traversal operations  
dag_traversal_context_t* diram_dag_begin_traversal(diram_dag_t* dag);
dag_node_t* diram_dag_traverse_next(dag_traversal_context_t* ctx, phenotype_t pheno);
void diram_dag_end_traversal(dag_traversal_context_t* ctx);

// Prediction operations
phenotype_t diram_dag_predict_next(diram_dag_t* dag, dag_node_t* current);
float diram_dag_get_transition_probability(dag_node_t* from, dag_node_t* to);

// Utility operations
void diram_dag_optimize(diram_dag_t* dag);  // Prune low-probability edges
void diram_dag_serialize(diram_dag_t* dag, const char* filepath);
diram_dag_t* diram_dag_deserialize(const char* filepath);

#endif // DIRAM_DAG_H
