#include "diram/core/diram.h"
#include <stdlib.h>
#include <string.h>

diram_memory_space_t* diram_space_create(const char* name, size_t limit) {
    diram_memory_space_t* space = calloc(1, sizeof(diram_memory_space_t));
    if (!space) return NULL;
    
    strncpy(space->space_name, name, 63);
    space->limit_bytes = limit;
    space->owner_pid = getpid();
    pthread_mutex_init(&space->lock, NULL);
    return space;
}

void diram_space_destroy(diram_memory_space_t* space) {
    if (!space) return;
    pthread_mutex_destroy(&space->lock);
    free(space);
}

int diram_space_check_limit(diram_memory_space_t* space, size_t requested) {
    if (!space) return 0;
    pthread_mutex_lock(&space->lock);
    int ok = (space->used_bytes + requested <= space->limit_bytes) ? 0 : -1;
    pthread_mutex_unlock(&space->lock);
    return ok;
}

void diram_error_index_init(void) {
    // Stub implementation
}

void diram_error_index_shutdown(void) {
    // Stub implementation
}
