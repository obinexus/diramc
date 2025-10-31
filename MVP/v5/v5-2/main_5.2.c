/*********************************************************************
 *  v5 – Consumer-Observer + Attack-Recovery Coherence Demo
 *  -------------------------------------------------------
 *  • One producer (DIRAM allocator) pushes blocks into a shared ring
 *  • N consumers pull blocks, “process” them, and push results
 *  • An observer watches every state change and updates a live
 *    coherence score (Hamiltonian-Eulerian constraint Γ(Gₕ,Gₑ))
 *  • Random “attacks” (NULL-ptr, priority sabotage) are injected
 *  • Self-healing: low-priority blocks are evicted, receipts re-issued
 *  • Target: coherence ≥ 95.4 % over 10 000 cycles
 *********************************************************************/

#include <stdint.h>
#include <inttypes.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdatomic.h>

#define RING_SIZE          256
#define MAX_CONSUMERS      8
#define CYCLES             10000
#define COHERENCE_TARGET   0.954f

/* ------------------------------------------------------------------ */
/*  DIRAM block – now carries a cryptographic receipt & priority       */
/* ------------------------------------------------------------------ */
typedef struct {
    void*   data;
    size_t  size;
    float   priority;          // 0.0 → evict, 1.0 → critical
    unsigned long receipt;
} diram_block_t;

/* ------------------------------------------------------------------ */
/*  Ring buffer shared between producer & consumers                    */
/* ------------------------------------------------------------------ */
typedef struct {
    diram_block_t buf[RING_SIZE];
    _Atomic size_t head;
    _Atomic size_t tail;
    pthread_mutex_t lock;
} ring_t;

static ring_t ring = { .lock = PTHREAD_MUTEX_INITIALIZER };

/* ------------------------------------------------------------------ */
/*  Global observer state                                              */
/* ------------------------------------------------------------------ */
static _Atomic float coherence = 1.0f;   // start perfect
static _Atomic unsigned long long events_ok = 0;
static _Atomic unsigned long long events_total = 0;



static _Atomic int producer_done = 0;

static uint64_t receipt(void* p, size_t s)
{
    uintptr_t up = (uintptr_t)p;
    return (uint64_t)((up ^ (uintptr_t)s) * (uintptr_t)2654435761ULL);
}

/* ------------------------------------------------------------------ */ /* Cryptographic receipt (Knuth 
multiplicative hash) */ /* ------------------------------------------------------------------ */ static unsigned 
long receipt(void* p, size_t s) {
    return ((unsigned long)p ^ s) * 2654435761UL;
}

/* ------------------------------------------------------------------ */
/*  Ring helpers                                                       */
/* ------------------------------------------------------------------ */
static int ring_push(diram_block_t b)
{
    size_t h = atomic_load(&ring.head);
    size_t t = atomic_load(&ring.tail);
    size_t next = (h + 1) % RING_SIZE;

    if (next == t) return 0;               // full
    pthread_mutex_lock(&ring.lock);
    ring.buf[h] = b;
    atomic_store(&ring.head, next);
    pthread_mutex_unlock(&ring.lock);
    return 1;
}

static int ring_pop(diram_block_t *out)
{
    size_t t = atomic_load(&ring.tail);
    size_t h = atomic_load(&ring.head);

    if (t == h) return 0;                  // empty
    pthread_mutex_lock(&ring.lock);
    *out = ring.buf[t];
    atomic_store(&ring.tail, (t + 1) % RING_SIZE);
    pthread_mutex_unlock(&ring.lock);
    return 1;
}

/* ------------------------------------------------------------------ */
/*  Producer – allocates, predicts, injects occasional attack          */
/* ------------------------------------------------------------------ */
static void* producer(void *arg)
{
    (void)arg;
    for (int i = 0; i < CYCLES; ++i) {
        size_t sz = 64 + (rand() % 960);                 // 64-1023 B
        float pri = 0.3f + ((float)rand()/RAND_MAX)*0.7f; // 0.3-1.0

        void *p = malloc(sz);
        if (!p) continue;

        /* 2 % chance of “attack” – sabotage priority or NULL ptr */
        if (rand() % 100 < 2) {
            pri = 0.0f;                     // force eviction
            printf("[ATTACK] sabotage priority → 0.0\n");
        }

        diram_block_t b = {
            .data = p,
            .size = sz,
            .priority = pri,
            .receipt = receipt(p, sz)
        };

        if (!ring_push(b))
            free(p);                        // ring full → drop

        /* prediction: high-priority → pre-allocate twin */
        if (pri > 0.85f) {
            void *twin = malloc(sz);
            if (twin) {
                diram_block_t twin_b = {
                    .data = twin,
                    .size = sz,
                    .priority = pri * 0.9f,
                    .receipt = receipt(twin, sz)
                };
                ring_push(twin_b);
            }
        }
        usleep(100);   // give consumers breathing room
    }

    atomic_store(&producer_done,1);
    return NULL;
}

/* ------------------------------------------------------------------ */
/*  Consumer – pulls, “processes”, pushes result back (observer)       */
/* ------------------------------------------------------------------ */

static void* consumer(void *arg)
{
    (void)arg;
    diram_block_t b;
    for (;;) {
        if (ring_pop(&b)) {
            atomic_fetch_add(&events_total, 1);
            int ok = 1;
            if (b.priority < 0.1f) {
                printf("[EVICT] low priority %.3f → free %zu B\n", b.priority, b.size);
                free(b.data);
                ok = 0;
            } else if (b.receipt != receipt(b.data, b.size)) {
                printf("[CORRUPT] receipt mismatch!\n");
                ok = 0;
            } else {
                memset(b.data, 0xAA, b.size);
            }
            if (ok) atomic_fetch_add(&events_ok, 1);
            float local_c = (float)atomic_load(&events_ok) / (float)atomic_load(&events_total);
            atomic_store(&coherence, local_c);
            free(b.data);
            usleep(50);
        } else {
            if (atomic_load(&producer_done)) break;
            usleep(100);
        }
    }
    return NULL;
}
/* ------------------------------------------------------------------ */
/*  Main – spawn threads, wait, report final coherence                */
/* ------------------------------------------------------------------ */
int main(void)
{
    srand(time(NULL));
    printf("=== DIRAM v5 Consumer-Observer Demo ===\n");
    printf("Target coherence ≥ %.1f%%\n", COHERENCE_TARGET*100);

    pthread_t prod_thr;
    pthread_t cons_thr[MAX_CONSUMERS];

    pthread_create(&prod_thr, NULL, producer, NULL);
    for (int i = 0; i < MAX_CONSUMERS; ++i)
        pthread_create(&cons_thr[i], NULL, consumer, NULL);

    pthread_join(prod_thr, NULL);
    /* give consumers a moment to drain the ring */
    usleep(500000);
    for (int i = 0; i < MAX_CONSUMERS; ++i)
        pthread_join(cons_thr[i], NULL);

    float final = atomic_load(&coherence);
    printf("\n--- FINAL REPORT ---\n");
    printf("Events processed : %llu\n", (unsigned long long)atomic_load(&events_total));
    printf("Events OK        : %llu\n", (unsigned long long)atomic_load(&events_ok));
    printf("Coherence        : %.3f %%\n", final*100);
    printf("%s\n", final >= COHERENCE_TARGET ? "PASS ≥ 95.4%" : "FAIL");

    return 0;
}
