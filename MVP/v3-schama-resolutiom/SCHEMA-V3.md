┌─────────────────────────────────────────────────────────────┐
│ ABSTRACT LAYER: Ontological Bijian (Human Interpretation)   │
│ "Who am I? How do I know you are you?" – Epistemic Custom    │
│ ↓ (Filter: Objective Reasoning – K-Clustering Hypotheses)    │
├─────────────────────────────────────────────────────────────┤
│ PROTOCOL LAYER: Active Caching Medium (Communication)        │
│ Passive RAM → Active DARM: Directed Eviction Strategies      │
│                                                              │
│   ┌──────────────┐    ┌──────────────┐    ┌──────────────┐  │
│   │ LRU Heap     │    │ MU Approx.   │    │ TTL Timeout  │  │
│   │ (Evict Least │    │ (Most Unused │    │ (Time-to-Live│  │
│   │  Used)       │    │  Frequency)  │    │  – 30 days?) │  │
│   └──────┬───────┘    └──────┬───────┘    └──────┬───────┘  │
│          │                   │                   │           │
│          └──────────┬────────┼───────────┬──────┘           │
│                     │        │           │                   │
│                     └────────┼───────────┘                   │
│                              │                                │
│   Cache Hit ────────────────▶┼─── Update Policy ──▶ Persist   │
│   (Need-Based: >95.4% Coherence)                          │
│                              │                                │
│   Cache Miss ────────────────▶└─── Exception (Bit Error) ─▶ Evict/Retry
│   (Overlook → Directed Evolution)                           │
│                                                              │
│ ↓ (Flash: Store Objective Knowledge – Vector Embeddings)     │
├─────────────────────────────────────────────────────────────┤
│ EXPRESSION LAYER: System Manifestation (Silicon/Hardware)    │
│ DARM.exe Hooks: XOR + CNOT Gates for Quantum-Resilient States│
│                                                              │
│ Inputs (2 Channels: 00, 01, 10, 11) ──▶ [XOR Gate: Flip Bits] ──▶ Output States
│   - 00 → 0 (Retain Zero – Passive Hold)                      │
│   - 01 → 1 (Directed Flip – Evict Weak)                      │
│   - 10 → 1 (Shadow Heap – Prioritize Need)                   │
│   - 11 → 0 (CNOT w/o NOT: Superposition Retain – Quantum)    │
│                                                              │
│ C Impl: #include <darm.h> // Linked List + Flags             │
│   int main() { darm_init(&); /* Run Detached */ return 0; }  │
│   // Evict: priority_queue<weak_map> evict_least_needed();   │
│                                                              │
│ ↓ (Persist: Assembly Artifacts – Type-Token Memory Trace)    │
└──────────────────────────────────────────────────────────────┘
