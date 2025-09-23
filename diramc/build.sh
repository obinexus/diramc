#!/bin/bash
# OBINexus DIRAMC Build Script
# Implements riftlang.exe → .so.a → rift.exe → gosilang toolchain

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║     OBINexus DIRAMC Build System       ║${NC}"
echo -e "${BLUE}║     Milestone-Based Architecture       ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════╝${NC}"
echo

# Configuration
PROJECT_ROOT="$(pwd)"
BUILD_MODE="${1:-debug}"

# Validate environment
if [ ! -f "Makefile.core" ]; then
    echo -e "${RED}Error: Not in diramc directory${NC}"
    exit 1
fi

# Create Makefile.config if missing
if [ ! -f "Makefile.config" ]; then
    echo -e "${YELLOW}Creating Makefile.config...${NC}"
    cat > Makefile.config << 'EOF'
CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -g -fPIC -pthread -D_GNU_SOURCE
LDFLAGS = -ldl -lpthread -lm
ARFLAGS = rcs

SRC_DIR = src
OBJ_DIR = build
LIB_DIR = lib
BIN_DIR = bin
INCLUDE_DIR = include

INCLUDES = -I$(INCLUDE_DIR) -I.

export CC AR CFLAGS LDFLAGS ARFLAGS
export SRC_DIR OBJ_DIR LIB_DIR BIN_DIR INCLUDE_DIR
export INCLUDES
EOF
    echo -e "${GREEN}✓${NC} Makefile.config created"
fi

# Create missing directories
echo -e "${BLUE}Setting up directory structure...${NC}"
mkdir -p include/diram/core/{feature-alloc,config,hotwire,monitor,parser}
mkdir -p src/core/{feature-alloc,config,hotwire,monitor,parser,assembly}
mkdir -p build/core/{feature-alloc,config}
mkdir -p lib bin logs

# Create stub config.c if missing
if [ ! -f "src/core/config/config.c" ]; then
    echo -e "${YELLOW}Creating config.c stub...${NC}"
    cat > src/core/config/config.c << 'EOF'
#include <stdio.h>
void diram_config_init(void) {
    // OBINexus config initialization
}
EOF
fi

# Clean and build core
echo -e "\n${BLUE}Building core components...${NC}"
make -f Makefile.core clean >/dev/null 2>&1
if make -f Makefile.core core; then
    echo -e "${GREEN}✓${NC} Core build successful"
else
    echo -e "${RED}✗${NC} Core build failed"
    exit 1
fi

# Build shared library
echo -e "\n${BLUE}Building shared library...${NC}"
gcc -shared build/core/feature-alloc/*.o -pthread -o lib/libdiram.so 2>/dev/null
if [ -f "lib/libdiram.so" ]; then
    echo -e "${GREEN}✓${NC} libdiram.so created"
fi

# Build static library
ar rcs lib/libdiram.a build/core/feature-alloc/*.o 2>/dev/null
if [ -f "lib/libdiram.a" ]; then
    echo -e "${GREEN}✓${NC} libdiram.a created"
fi

# Build CLI if main.c exists
if [ -f "src/cli/main.c" ]; then
    echo -e "\n${BLUE}Building CLI...${NC}"
    mkdir -p build/cli
    gcc -Wall -g -Iinclude -c src/cli/main.c -o build/cli/main.o
    gcc build/cli/main.o -Llib -ldiram -ldl -pthread -o bin/diram
    if [ -f "bin/diram" ]; then
        echo -e "${GREEN}✓${NC} diram executable created"
    fi
fi

# Summary
echo
echo -e "${GREEN}════════════════════════════════${NC}"
echo -e "${GREEN}Build Complete!${NC}"
echo -e "${GREEN}════════════════════════════════${NC}"
echo
echo "Artifacts:"
ls -la lib/*.{so,a} 2>/dev/null | awk '{print "  " $9}'
[ -f "bin/diram" ] && ls -la bin/diram | awk '{print "  " $9}'
echo
echo "Next steps:"
echo "  ./bin/diram --help    # Show help"
echo "  ./bin/diram --trace   # Enable tracing"
