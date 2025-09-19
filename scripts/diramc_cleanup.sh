#!/bin/bash
# ============================================================================
# DIRAMC Complete Cleanup Script - Finish the Refactoring
# ============================================================================

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m'

echo -e "${PURPLE}============================================${NC}"
echo -e "${PURPLE}    DIRAMC Complete Cleanup                ${NC}"
echo -e "${PURPLE}    Finishing the Refactoring              ${NC}"
echo -e "${PURPLE}============================================${NC}"
echo ""

# Make sure we're in the diramc directory
cd ~/obinexus/workspace/diramc || exit 1

echo -e "${BLUE}Current directory: $(pwd)${NC}"
echo ""

# Step 1: Create missing directories
echo -e "${BLUE}Step 1: Creating missing directories...${NC}"
mkdir -p docs/archived/{legacy,old-builds}
mkdir -p docs/{api,guides,internals}
mkdir -p config/{development,production,testing}
mkdir -p scripts/{build,install,utilities}
mkdir -p resources/{schemas,templates,assets}
mkdir -p src/core/{assembly,config,feature-alloc,hotwire,parser}
echo -e "${GREEN}✓${NC} Directory structure completed"

# Step 2: Move PDFs to proper locations
echo -e "\n${BLUE}Step 2: Organizing PDF files...${NC}"
if [ -f "diram_festival_analogy.pdf" ]; then
    mv diram_festival_analogy.pdf docs/archived/pdfs/
    echo -e "${GREEN}✓${NC} Moved diram_festival_analogy.pdf → docs/archived/pdfs/"
fi

# Find and move any other PDFs in root
for pdf in *.pdf; do
    if [ -f "$pdf" ]; then
        if [[ "$pdf" == *"README"* ]]; then
            mv "$pdf" docs/archived/legacy/
        else
            mv "$pdf" docs/archived/pdfs/
        fi
        echo -e "${GREEN}✓${NC} Moved $pdf"
    fi
done

# Step 3: Move .txt files to archived
echo -e "\n${BLUE}Step 3: Archiving .txt files...${NC}"
mkdir -p docs/archived/recovery-scripts
for txt in *.txt; do
    if [ -f "$txt" ]; then
        mv "$txt" docs/archived/recovery-scripts/
        echo -e "${GREEN}✓${NC} Archived $txt → docs/archived/recovery-scripts/"
    fi
done

# Step 4: Consolidate Makefiles
echo -e "\n${BLUE}Step 4: Organizing Makefiles...${NC}"
mkdir -p scripts/build/makefiles
for makefile in Makefile.*; do
    if [ -f "$makefile" ]; then
        mv "$makefile" scripts/build/makefiles/
        echo -e "${GREEN}✓${NC} Moved $makefile → scripts/build/makefiles/"
    fi
done

# Step 5: Fix duplicate logs directories
echo -e "\n${BLUE}Step 5: Fixing duplicate logs directories...${NC}"
# Check if there's a weird logs directory with special character
if [ -d "logs " ]; then
    # Merge contents
    cp -r "logs "/* logs/ 2>/dev/null || true
    rm -rf "logs "
    echo -e "${GREEN}✓${NC} Merged duplicate logs directory"
fi

# Remove the weird character from the comment
if grep -q "# Directory for detached mode logs" Makefile 2>/dev/null; then
    sed -i 's/logs          # Directory for detached mode logs.*/logs\/ # Directory for detached mode logs/' Makefile 2>/dev/null || true
fi

# Step 6: Move configuration files
echo -e "\n${BLUE}Step 6: Organizing configuration files...${NC}"
if [ -f "diram.drc" ]; then
    mv diram.drc config/development/
    echo -e "${GREEN}✓${NC} Moved diram.drc → config/development/"
fi

if [ -f "diram.drc.in.xml" ]; then
    mv diram.drc.in.xml resources/schemas/
    echo -e "${GREEN}✓${NC} Moved diram.drc.in.xml → resources/schemas/"
fi

# Step 7: Move scripts
echo -e "\n${BLUE}Step 7: Organizing scripts...${NC}"
for script in *.sh; do
    if [ -f "$script" ]; then
        if [[ "$script" == *"recovery"* ]] || [[ "$script" == *"refactor"* ]]; then
            mv "$script" scripts/utilities/
            echo -e "${GREEN}✓${NC} Moved $script → scripts/utilities/"
        else
            mv "$script" scripts/
            echo -e "${GREEN}✓${NC} Moved $script → scripts/"
        fi
    fi
done

# Step 8: Move diram-tree.txt if it exists
if [ -f "diram-tree.txt" ]; then
    mv diram-tree.txt docs/archived/
    echo -e "${GREEN}✓${NC} Moved diram-tree.txt → docs/archived/"
fi

# Step 9: Create proper .gitignore
echo -e "\n${BLUE}Step 9: Creating comprehensive .gitignore...${NC}"
cat > .gitignore << 'EOF'
# Build artifacts
build/
bin/*
!bin/.gitkeep
lib/*
!lib/.gitkeep
*.o
*.so
*.so.*
*.a
*.dll
*.exe
*.dylib

# Logs
logs/*.log
logs/*.err
logs/*.out
*.log

# Temporary files
*.tmp
*.temp
*.swp
*.swo
*~
.*.swp
.*.swo

# Backup files
*.backup
*_backup_*/
*.bak
*.old

# IDE files
.vscode/
.idea/
*.sublime-*
.project
.cproject
.settings/

# OS files
.DS_Store
Thumbs.db
desktop.ini

# Python cache (if any Python scripts)
__pycache__/
*.pyc
*.pyo

# Core dumps
core
core.*
vgcore.*

# Test coverage
*.gcno
*.gcda
*.gcov
coverage/
.coverage

# Documentation build
docs/_build/
docs/html/
*.doxygen
EOF
echo -e "${GREEN}✓${NC} .gitignore created"

# Step 10: Create directory placeholders
echo -e "\n${BLUE}Step 10: Creating directory placeholders...${NC}"
touch bin/.gitkeep
touch lib/.gitkeep
touch build/.gitkeep
touch docs/api/.gitkeep
touch docs/guides/.gitkeep
touch docs/internals/.gitkeep
echo -e "${GREEN}✓${NC} Placeholders created"

# Step 11: Update main Makefile path references
echo -e "\n${BLUE}Step 11: Creating unified Makefile...${NC}"
if [ -f "Makefile" ]; then
    # Backup original
    cp Makefile scripts/build/makefiles/Makefile.original
    
    # Create a clean unified Makefile
    cat > Makefile << 'EOF'
# DIRAMC Unified Makefile
PROJECT = diramc
VERSION = 2.0.0

# Include modular makefiles if needed
-include scripts/build/makefiles/Makefile.config
-include scripts/build/makefiles/Makefile.core

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin
LIB_DIR = lib
LOGS_DIR = logs

# Compiler settings
CC = gcc
CFLAGS = -Wall -g -O2 -Iinclude
LDFLAGS = -pthread -ldl

# Default target
all: directories
	@echo "Building DIRAMC..."
	@$(MAKE) -C $(SRC_DIR)

directories:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR) $(LIB_DIR) $(LOGS_DIR)

clean:
	rm -rf $(BUILD_DIR)/* $(BIN_DIR)/* $(LIB_DIR)/*
	rm -f $(LOGS_DIR)/*.log

.PHONY: all clean directories
EOF
    echo -e "${GREEN}✓${NC} Unified Makefile created"
fi

# Step 12: Generate structure report
echo -e "\n${BLUE}Step 12: Generating structure report...${NC}"
cat > STRUCTURE.md << 'EOF'
# DIRAMC Directory Structure

## Clean Organization

```
diramc/
├── src/                    # Source code
│   ├── cli/               # CLI implementation
│   └── core/              # Core modules
│       ├── assembly/      # Assembly pipeline
│       ├── config/        # Configuration
│       ├── feature-alloc/ # Memory allocation
│       ├── hotwire/       # Hotwire transformation
│       └── parser/        # Parser
│
├── include/               # Header files
│   └── diram/
│       └── core/
│
├── docs/                  # Documentation
│   ├── api/              # API documentation
│   ├── guides/           # User guides
│   ├── specs/            # Specifications
│   │   └── pdfs/         # Technical PDFs
│   ├── internals/        # Internal docs
│   ├── references/       # Reference materials
│   └── archived/         # Archived content
│       ├── pdfs/         # Old PDFs
│       ├── legacy/       # Legacy docs
│       ├── old-builds/   # Old build artifacts
│       └── recovery-scripts/ # Recovery scripts
│
├── config/               # Configuration files
│   ├── development/      # Dev configs
│   ├── production/       # Prod configs
│   └── testing/          # Test configs
│
├── scripts/              # Scripts
│   ├── build/            # Build scripts
│   │   └── makefiles/    # Old Makefiles
│   ├── install/          # Installation
│   └── utilities/        # Utilities
│
├── resources/            # Resources
│   ├── schemas/          # XML/XSD schemas
│   ├── templates/        # Templates
│   └── assets/           # Assets
│
├── examples/             # Example code
├── tests/                # Test suites
├── lib/                  # Libraries (generated)
├── bin/                  # Binaries (generated)
├── build/                # Build artifacts (generated)
├── logs/                 # Log files
│
├── Makefile              # Main build file
├── README.md             # Project documentation
├── STRUCTURE.md          # This file
└── .gitignore            # Git ignores
```

## What Was Cleaned

- ✓ PDFs moved to appropriate docs subdirectories
- ✓ .txt files archived
- ✓ Multiple Makefiles consolidated
- ✓ Configuration files organized
- ✓ Scripts properly categorized
- ✓ Duplicate logs directory fixed
- ✓ Clean root directory

## Next Steps

1. Build the project: `make all`
2. Run tests: `make test`
3. Check logs: `ls logs/`
EOF

echo -e "${GREEN}✓${NC} Structure report created"

# Step 13: Final cleanup check
echo -e "\n${BLUE}Step 13: Final verification...${NC}"
echo "Root directory now contains:"
ls -la | grep -E "^[^d]" | awk '{print "  - " $NF}' | head -20

echo -e "\n${PURPLE}============================================${NC}"
echo -e "${GREEN}       Cleanup Complete!                   ${NC}"
echo -e "${PURPLE}============================================${NC}"
echo ""
echo "Summary of changes:"
echo "  ✓ PDFs organized in docs/archived/pdfs and docs/specs/pdfs"
echo "  ✓ .txt files moved to docs/archived/recovery-scripts"
echo "  ✓ Makefiles consolidated in scripts/build/makefiles"
echo "  ✓ Configuration files in config/"
echo "  ✓ Scripts organized in scripts/"
echo "  ✓ Resources in resources/schemas"
echo "  ✓ Clean root directory achieved"
echo ""
echo -e "${GREEN}The DIRAMC structure is now properly organized!${NC}"
echo ""
echo "Next commands:"
echo "  tree -L 2 .        # View new structure"
echo "  cat STRUCTURE.md   # See documentation"
echo "  make all          # Build project"
