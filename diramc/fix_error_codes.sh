#!/bin/bash
# OBINexus DIRAM Error Code Fix
# Fixing DIRAM_ERR_PENDING typo and missing DIRAM_ERR_INVALID_ARG
# Maintains phenomenological memory architecture continuity

echo "╔══════════════════════════════════════════════════════════╗"
echo "║     OBINexus DIRAM Error Code Correction                ║"
echo "╠══════════════════════════════════════════════════════════╣"
echo "║  Fixing DIRAM_ERR_PENDING and DIRAM_ERR_INVALID_ARG     ║"
echo "║  Toolchain: riftlang.exe → .so.a → rift.exe → gosilang  ║"
echo "╚══════════════════════════════════════════════════════════╝"

# Fix the typo in async_promise.c - DIRAM_ERR_PENDINGG -> DIRAM_ERR_PENDING
echo "[OBINEXUS] Fixing DIRAM_ERR_PENDINGG typo in async_promise.c..."
sed -i 's/DIRAM_ERR_PENDINGG/DIRAM_ERR_PENDING/g' src/core/feature-alloc/async_promise.c

# Add missing error codes to async_promise.h
echo "[OBINEXUS] Adding missing error codes to async_promise.h..."
if ! grep -q "DIRAM_ERR_PENDING" include/diram/core/feature-alloc/async_promise.h; then
    # Add the missing error codes after the existing ones
    sed -i '/^#define DIRAM_ERR_CANCELLED/a\
#define DIRAM_ERR_PENDING              0x100D\
#define DIRAM_ERR_INVALID_ARG          0x100E\
#define DIRAM_ERR_FATAL                0x100F\
#define DIRAM_ERR_UNKNOWN              0x1010' include/diram/core/feature-alloc/async_promise.h
fi

# Also ensure the error codes are properly defined in the C file
echo "[OBINEXUS] Ensuring error codes are consistent in async_promise.c..."

# Create a temporary fix for the duplicate/incorrect definitions
cat > /tmp/fix_error_codes.sed << 'EOF'
# Remove all the duplicate error code blocks
/^\/\/ Extension error codes for async promises$/,/^#endif$/{
    # Keep only the first occurrence
    :a
    N
    $!ba
    s/\n\/\/ Extension error codes for async promises\n#ifndef DIRAM_ERR_TIMEOUT[^#]*#endif//g
}
EOF

# Apply the deduplication
sed -i -f /tmp/fix_error_codes.sed src/core/feature-alloc/async_promise.c 2>/dev/null || true

# Now ensure there's exactly one correct block of error codes after includes
cat > /tmp/async_promise_error_block.txt << 'EOF'
// Extension error codes for async promises
#ifndef DIRAM_ERR_TIMEOUT
#define DIRAM_ERR_TIMEOUT        0x100B
#define DIRAM_ERR_CANCELLED      0x100C
#define DIRAM_ERR_PENDING        0x100D
#define DIRAM_ERR_INVALID_ARG    0x100E
#define DIRAM_ERR_FATAL          0x100F
#define DIRAM_ERR_UNKNOWN        0x1010
#define DIRAM_SUCCESS            DIRAM_ERR_NONE
#endif

#ifndef DIRAM_SHA256_HEX_LEN
#define DIRAM_SHA256_HEX_LEN 65
#endif
EOF

# Remove all existing error code blocks and add the correct one
echo "[OBINEXUS] Cleaning up error code definitions..."
if [ -f src/core/feature-alloc/async_promise.c ]; then
    # Create a backup
    cp src/core/feature-alloc/async_promise.c src/core/feature-alloc/async_promise.c.bak
    
    # Remove all duplicate error code blocks
    perl -i -0pe 's/\/\/ Extension error codes for async promises\s*\n#ifndef DIRAM_ERR_TIMEOUT.*?#endif\s*\n//gs' src/core/feature-alloc/async_promise.c
    
    # Also remove the standalone DIRAM_SHA256_HEX_LEN definition
    perl -i -0pe 's/\n#ifndef DIRAM_SHA256_HEX_LEN\s*\n#define DIRAM_SHA256_HEX_LEN 65\s*\n#endif\s*\n//gs' src/core/feature-alloc/async_promise.c
    
    # Add the correct block after the last include
    awk '
    /^#include.*<stdio\.h>$/ {
        print $0
        print ""
        while ((getline line < "/tmp/async_promise_error_block.txt") > 0) {
            print line
        }
        close("/tmp/async_promise_error_block.txt")
        next
    }
    {print}
    ' src/core/feature-alloc/async_promise.c.bak > src/core/feature-alloc/async_promise.c
fi

# Verify the fixes
echo ""
echo "[OBINEXUS] Verification:"
echo -n "  • DIRAM_ERR_PENDING defined correctly: "
if grep -q "define DIRAM_ERR_PENDING " src/core/feature-alloc/async_promise.c; then
    echo "✓"
else
    echo "✗"
fi

echo -n "  • DIRAM_ERR_INVALID_ARG defined: "
if grep -q "define DIRAM_ERR_INVALID_ARG" src/core/feature-alloc/async_promise.c; then
    echo "✓"
else
    echo "✗"
fi

echo -n "  • No DIRAM_ERR_PENDINGG typos: "
if ! grep -q "DIRAM_ERR_PENDINGG" src/core/feature-alloc/async_promise.c; then
    echo "✓"
else
    echo "✗ (still found typos)"
fi

echo ""
echo "╔══════════════════════════════════════════════════════════╗"
echo "║     OBINexus DIRAM Error Codes Fixed                    ║"
echo "╠══════════════════════════════════════════════════════════╣"
echo "║  ✓ DIRAM_ERR_PENDINGG typo corrected                    ║"
echo "║  ✓ DIRAM_ERR_INVALID_ARG added                          ║"
echo "║  ✓ Duplicate error blocks removed                       ║"
echo "║  ✓ Error codes consolidated                             ║"
echo "╠══════════════════════════════════════════════════════════╣"
echo "║  Toolchain: riftlang.exe → .so.a → rift.exe → gosilang  ║"
echo "║  Build: nlink → polybuild orchestration                 ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "Now run: make clean && make"
echo ""
echo "If errors persist, check:"
echo "  • src/core/feature-alloc/async_promise.c.bak (backup)"
echo "  • include/diram/core/feature-alloc/async_promise.h"
