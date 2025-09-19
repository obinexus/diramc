#include <stdio.h>
#include <assert.h>
#include "diram/core/bootstrap.h"

int main() {
    printf("Running DIRAMC allocation tests...\n");
    
    // Initialize bootstrap
    assert(diram_bootstrap_init() == 0);
    printf("✓ Bootstrap initialized\n");
    
    // Test allocation
    diram_allocation_t* alloc = diram_alloc_traced(1024, "test");
    assert(alloc != NULL);
    printf("✓ Allocation successful\n");
    
    // Test memory access
    memset(alloc->base_addr, 0x42, alloc->size);
    printf("✓ Memory accessible\n");
    
    // Test deallocation
    diram_free_traced(alloc);
    printf("✓ Deallocation successful\n");
    
    printf("\nAll tests passed!\n");
    return 0;
}
