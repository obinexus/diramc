#!/bin/bash
# diram_trace.sh - Trace library with detached daemon

./bin/diram \
    --trace \
    --lib /path/to/libcustom.so \
    --detach \
    --log-dir ./logs \
    --pid-file /var/run/diram.pid \
    --config diram.drc \
    --script trace_custom.dr

# The detached process will:
# 1. Fork twice (Unix daemon pattern)
# 2. Hook into libcustom.so functions
# 3. Log all allocations to ./logs/trace_YYYY-MM-DD.log
# 4. Maintain coherence checking in background
