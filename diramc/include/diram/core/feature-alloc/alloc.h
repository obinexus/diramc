// include/diram/core/feature-alloc/alloc.h
// OBINexus DIRAM Feature Allocation Header
#ifndef DIRAM_FEATURE_ALLOC_H
#define DIRAM_FEATURE_ALLOC_H

#include "diram/core/diram.h"
#include "diram/core/diram_phenomenological.h"

// The diram_enhanced_allocation_t is already defined in diram.h
// This header extends functionality without redefinition

// Feature allocation specific functions
diram_enhanced_allocation_t* diram_alloc_enhanced(
    size_t size,
    const char* tag,
    diram_memory_space_t* space
);

// Phenotype-based allocation
void* diram_alloc_by_phenotype(
    diram_context_t* ctx,
    size_t size,
    phenotype_t pheno,
    const char* tag
);

// Triple-stream verified allocation
void* diram_alloc_verified(
    diram_context_t* ctx,
    size_t size,
    triple_stream_result_t* verification
);

// Axial state allocation
void* diram_alloc_axial(
    diram_context_t* ctx,
    size_t size,
    axial_state_t axial
);

// Feature allocation utilities
int diram_validate_allocation(diram_enhanced_allocation_t* alloc);
void diram_release_enhanced(diram_enhanced_allocation_t* alloc);
phenotype_t diram_extract_phenotype(void* memory);

#endif // DIRAM_FEATURE_ALLOC_H
