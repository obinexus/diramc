#!/bin/bash
# fix_build_errors.sh - OBINexus diramc build fixes

echo "[FIX] Resolving cache_lookahead.c dependencies..."
# Add missing includes to cache_lookahead.c
cat > /tmp/cache_fix.tmp << 'EOF'
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "diram/core/diram.h"
#include "diram/core/diram_phenomenological.h"
#include "diram/core/feature-alloc/alloc.h"

// Forward declarations if DAG not yet defined
typedef struct dag_node {
    uint32_t edge_count;
    struct dag_edge** edges;
} dag_node_t;

typedef struct dag_edge {
    phenotype_t trigger;
    float probability;
} dag_edge_t;

// Helper functions
static uint32_t detect_pattern_length(phenotype_t* seq, uint32_t len) {
    // Simple pattern detection stub
    return len > 4 ? 2 : 0;
}

static dag_node_t* diram_navigate_dag(diram_context_t* ctx, phenotype_t pheno) {
    // Navigation stub
    return NULL;
}

static void mark_memory_speculative(void* ptr, size_t size) {
    // Speculation marker stub
    (void)ptr; (void)size;
}

EOF

# Prepend to cache_lookahead.c
if [ -f src/core/feature-alloc/cache_lookahead.c ]; then
    cp src/core/feature-alloc/cache_lookahead.c src/core/feature-alloc/cache_lookahead.c.orig
    cat /tmp/cache_fix.tmp src/core/feature-alloc/cache_lookahead.c.orig > src/core/feature-alloc/cache_lookahead.c
fi

echo "[FIX] Fixing triple_stream_t redefinition..."
# Fix the header file
sed -i.bak '75s/typedef struct triple_stream triple_stream_t;//' include/diram/core/diram_phenomenological.h

echo "[FIX] Fixing format specifier in alloc.c..."
sed -i.bak '129s/%lu/%llu/' src/core/feature-alloc/alloc.c

echo "[FIX] Silencing unused parameters..."
# Add void casts for unused params in async_promise.c
sed -i.bak '/diram_memory_space_t\* space/a\    (void)space;' src/core/feature-alloc/async_promise.c

echo "[✓] Build fixes applied"
