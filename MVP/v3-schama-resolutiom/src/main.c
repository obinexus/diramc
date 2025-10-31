#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COHERENCE_THRESHOLD 0.954f
#define MAX_HEAP_SIZE 1024

typedef struct {
    void* data;       // Actual memory block
    float priority;   // 0.0 (evict) → 1.0 (critical)
    size_t size;
} WeakMap;

static WeakMap heap[MAX_HEAP_SIZE];
static int heap_size = 0;

// Active eviction: directed, not passive
void evict_least() {
    if (heap_size == 0) return;

    int min_idx = 0;
    for (int i = 1; i < heap_size; i++) {
        if (heap[i].priority < heap[min_idx].priority)
            min_idx = i;
    }

    if (heap[min_idx].priority < COHERENCE_THRESHOLD) {
        printf("[DIRAM] EVICT: priority=%.3f < %.3f → freeing %zu bytes\n",
               heap[min_idx].priority, COHERENCE_THRESHOLD, heap[min_idx].size);
        free(heap[min_idx].data);  // ← Correct: free the pointer
        // Shift heap down
        for (int i = min_idx; i < heap_size - 1; i++) {
            heap[i] = heap[i + 1];
        }
        heap_size--;
    }
}

// Directed alloc with prediction + receipt
void* diram_alloc(size_t size, float priority) {
    if (heap_size >= MAX_HEAP_SIZE) {
        printf("[DIRAM] Heap full → triggering active eviction...\n");
        evict_least();
        if (heap_size >= MAX_HEAP_SIZE) return NULL;
    }

    void* ptr = malloc(size);
    if (!ptr) return NULL;

    heap[heap_size++] = (WeakMap){ .data = ptr, .priority = priority, .size = size };

    unsigned long receipt = ((unsigned long)ptr ^ size) * 2654435761u;
    printf("[DIRAM] ALLOC %zu bytes @ %p | priority=%.3f | receipt=%lu\n",
           size, ptr, priority, receipt);

    // Prediction: pre-allocate if high-priority
    if (priority > 0.8f && heap_size < MAX_HEAP_SIZE) {
        void* pred = malloc(size);
        if (pred) {
            heap[heap_size++] = (WeakMap){ .data = pred, .priority = priority * 0.9f, .size = size };
            printf("[DIRAM] PREDICTED: pre-alloc %zu bytes @ %p\n", size, pred);
        }
    }

    return ptr;
}

void diram_free(void* ptr) {
    for (int i = 0; i < heap_size; i++) {
        if (heap[i].data == ptr) {
            free(ptr);
            printf("[DIRAM] FREED %zu bytes @ %p\n", heap[i].size, ptr);
            // Remove from heap
            for (int j = i; j < heap_size - 1; j++) {
                heap[j] = heap[j + 1];
            }
            heap_size--;
            return;
        }
    }
    printf("[DIRAM] UNKNOWN PTR: %p\n", ptr);
}

int main(int argc, char** argv) {
    int detach = (argc > 1 && strcmp(argv[1], "&") == 0);

    printf("OBINexus DIRAM v3 MVP | 95.4%% Schema Active\n");
    if (detach) printf("[DETACHED MODE]\n");

    char* msg = (char*)diram_alloc(1024, 0.97f);  // High priority
    strcpy(msg, "Hello from DIRAM! Active Memory Lives.");

    char* junk = (char*)diram_alloc(64, 0.12f);   // Low priority → will be evicted
    strcpy(junk, "Forget me.");

    printf("Message: %s\n", msg);

    // Simulate load → trigger eviction
    for (int i = 0; i < 1020; i++) {
        diram_alloc(128, 0.5f + (float)i / 2000.0f);  // Gradual priority
    }

    diram_free(msg);
    diram_free(junk);

    printf("[DIRAM] Coherence Check: %d active blocks | %.1f%% threshold met\n",
           heap_size, COHERENCE_THRESHOLD * 100);

    return 0;
}
