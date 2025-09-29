#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    void* ptr;
    size_t size;
    int used;
} diram_block_t;

#define MAX_BLOCKS 4
static diram_block_t pool[MAX_BLOCKS];

// Cryptographic “receipt” placeholder
unsigned long fake_receipt(void* p, size_t size) {
    return ((unsigned long)p ^ size) * 2654435761u;
}

// Directed alloc: predict, enforce, log
void* diram_alloc(size_t size) {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (!pool[i].used) {
            pool[i].ptr = malloc(size);
            pool[i].size = size;
            pool[i].used = 1;
            printf("[DIRAM] Alloc %zu bytes -> receipt %lu\n",
                   size, fake_receipt(pool[i].ptr, size));
            // Prediction gimmick: if block is “large,” pre-allocate another
            if (size > 512 && i + 1 < MAX_BLOCKS && !pool[i+1].used) {
                pool[i+1].ptr = malloc(size);
                pool[i+1].size = size;
                pool[i+1].used = 1;
                printf("[DIRAM] Predicted future need, pre-allocated %zu bytes\n", size);
            }
            return pool[i].ptr;
        }
    }
    printf("[DIRAM] No blocks available!\n");
    return NULL;
}

void diram_free(void* ptr) {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (pool[i].used && pool[i].ptr == ptr) {
            free(pool[i].ptr);
            pool[i].used = 0;
            printf("[DIRAM] Freed block of %zu bytes\n", pool[i].size);
            return;
        }
    }
    printf("[DIRAM] Unknown pointer!\n");
}

int main() {
    char* msg = (char*)diram_alloc(1024);
    strcpy(msg, "Hello from DIRAM!");
    printf("%s\n", msg);

    diram_free(msg);
    return 0;
}
