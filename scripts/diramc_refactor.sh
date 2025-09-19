#!/bin/bash
# ============================================================================
# DIRAMC Directory Structure Refactoring Script
# OBINexus Project - Proper organization before wrapper implementation
# ============================================================================

set -e

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m'

echo -e "${PURPLE}============================================${NC}"
echo -e "${PURPLE}    DIRAMC Directory Structure Refactor    ${NC}"
echo -e "${PURPLE}    OBINexus LIBPOLYCALL2DIRAM Project    ${NC}"
echo -e "${PURPLE}============================================${NC}"
echo ""

# Configuration
WORKSPACE_ROOT="/obinexus/workspace"
DIRAMC_ROOT="${WORKSPACE_ROOT}/diramc"
BACKUP_DIR="${WORKSPACE_ROOT}/diramc_backup_$(date +%Y%m%d_%H%M%S)"

# Function to print status
print_status() {
    echo -e "${GREEN}[✓]${NC} $1"
}

print_error() {
    echo -e "${RED}[✗]${NC} $1"
}

print_info() {
    echo -e "${BLUE}[i]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

# Step 1: Create comprehensive backup
echo -e "${BLUE}Step 1: Creating comprehensive backup...${NC}"
if [ -d "$DIRAMC_ROOT" ]; then
    cp -r "$DIRAMC_ROOT" "$BACKUP_DIR"
    print_status "Backup created at: $BACKUP_DIR"
else
    mkdir -p "$DIRAMC_ROOT"
    print_warning "Created new diramc directory"
fi

cd "$WORKSPACE_ROOT" || exit 1

# Step 2: Create new organized structure
echo -e "\n${BLUE}Step 2: Creating organized directory structure...${NC}"

# Create the new structure
mkdir -p diramc/{src,include,lib,bin,build,tests,examples}
mkdir -p diramc/docs/{api,guides,specs,internals,archived}
mkdir -p diramc/docs/archived/{legacy,pdfs,old-builds}
mkdir -p diramc/config/{development,production,testing}
mkdir -p diramc/scripts/{build,install,utilities}
mkdir -p diramc/resources/{schemas,templates,assets}
mkdir -p diramc/.github/workflows

print_status "Created new directory structure"

# Step 3: Move and organize source code
echo -e "\n${BLUE}Step 3: Organizing source code...${NC}"

# Move core source files
if [ -d "diramc/src/core" ]; then
    print_info "Source structure already exists, preserving..."
else
    mkdir -p diramc/src/core/{feature-alloc,hotwire,parser,assembly,config}
    mkdir -p diramc/src/cli
    
    # Move existing source files if they exist
    [ -d "diramc_backup_*/src" ] && cp -r diramc_backup_*/src/* diramc/src/ 2>/dev/null || true
fi

# Move include files
mkdir -p diramc/include/diram/core/{feature-alloc,hotwire,parser,config}
[ -d "diramc_backup_*/include" ] && cp -r diramc_backup_*/include/* diramc/include/ 2>/dev/null || true

print_status "Source code organized"

# Step 4: Organize documentation
echo -e "\n${BLUE}Step 4: Reorganizing documentation...${NC}"

# Move PDFs to archived/pdfs
print_info "Moving PDF documents to docs/archived/pdfs/..."
find . -maxdepth 3 -name "*.pdf" -type f 2>/dev/null | while read -r pdf; do
    filename=$(basename "$pdf")
    target_dir="diramc/docs/archived/pdfs"
    
    # Categorize PDFs
    if [[ "$filename" == *"Technical"* ]] || [[ "$filename" == *"Specification"* ]]; then
        target_dir="diramc/docs/specs"
    elif [[ "$filename" == *"DIRAM"* ]] && [[ "$filename" != *"backup"* ]]; then
        target_dir="diramc/docs/specs"
    fi
    
    cp "$pdf" "$target_dir/$filename" 2>/dev/null && \
        print_info "  Moved: $filename → $target_dir/"
done

# Move markdown documentation
print_info "Organizing markdown documentation..."
find . -maxdepth 4 -name "*.md" -type f 2>/dev/null | while read -r md; do
    filename=$(basename "$md")
    target_dir="diramc/docs/guides"
    
    # Skip README files in root
    if [[ "$filename" == "README.md" ]] && [[ "$(dirname "$md")" == "." ]]; then
        cp "$md" "diramc/README.md"
        continue
    fi
    
    # Categorize markdown files
    if [[ "$filename" == *"LEGACY"* ]] || [[ "$filename" == *"OLD"* ]]; then
        target_dir="diramc/docs/archived/legacy"
    elif [[ "$filename" == *"PLAN"* ]] || [[ "$filename" == *"spec"* ]]; then
        target_dir="diramc/docs/specs"
    elif [[ "$filename" == *"api"* ]] || [[ "$filename" == *"API"* ]]; then
        target_dir="diramc/docs/api"
    elif [[ "$filename" == *"truth_table"* ]] || [[ "$filename" == *"boolean"* ]]; then
        target_dir="diramc/docs/internals"
    fi
    
    cp "$md" "$target_dir/$filename" 2>/dev/null && \
        print_info "  Organized: $filename → $target_dir/"
done

print_status "Documentation reorganized"

# Step 5: Move old builds to archived
echo -e "\n${BLUE}Step 5: Archiving old builds...${NC}"

# Archive old build directories
for build_dir in build.backup build-universal diramc_backup_*; do
    if [ -d "$build_dir" ]; then
        mv "$build_dir" "diramc/docs/archived/old-builds/" 2>/dev/null && \
            print_info "  Archived: $build_dir"
    fi
done

print_status "Old builds archived"

# Step 6: Organize configuration files
echo -e "\n${BLUE}Step 6: Organizing configuration files...${NC}"

# Move config files
[ -f "diram.drc" ] && mv diram.drc diramc/config/development/
[ -f "diram.drc.in.xml" ] && mv diram.drc.in.xml diramc/resources/schemas/

# Move Makefiles
[ -f "Makefile" ] && cp Makefile diramc/Makefile
for makefile in Makefile.*; do
    [ -f "$makefile" ] && mv "$makefile" diramc/scripts/build/
done

print_status "Configuration files organized"

# Step 7: Create new project documentation structure
echo -e "\n${BLUE}Step 7: Creating project documentation...${NC}"

# Create main README
cat > diramc/README.md << 'EOF'
# DIRAMC - Directed Instruction Random Access Memory

## Overview
DIRAMC is a library wrapper and fault tracer for LibPolyCall and other C libraries, part of the OBINexus ecosystem.

## Directory Structure

```
diramc/
├── src/                    # Source code (polyglot/modular)
│   ├── core/              # Core functionality
│   │   ├── feature-alloc/ # Memory allocation features
│   │   ├── hotwire/       # Hotwire transformation
│   │   ├── parser/        # Parser implementation
│   │   ├── assembly/      # Assembly pipeline
│   │   └── config/        # Configuration management
│   └── cli/               # Command-line interface
├── include/               # Header files
│   └── diram/
│       └── core/         # Core headers
├── lib/                  # Compiled libraries
├── bin/                  # Executables
├── build/                # Build artifacts
├── tests/                # Test suites
├── examples/             # Example code
├── docs/                 # Documentation
│   ├── api/             # API documentation
│   ├── guides/          # User guides
│   ├── specs/           # Technical specifications
│   ├── internals/       # Internal documentation
│   └── archived/        # Archived content
│       ├── legacy/      # Legacy documentation
│       ├── pdfs/        # PDF documents
│       └── old-builds/  # Previous build artifacts
├── config/              # Configuration files
│   ├── development/     # Dev configs
│   ├── production/      # Prod configs
│   └── testing/         # Test configs
├── scripts/             # Utility scripts
│   ├── build/          # Build scripts
│   ├── install/        # Installation scripts
│   └── utilities/      # Helper utilities
└── resources/          # Resources
    ├── schemas/        # XML/JSON schemas
    ├── templates/      # Code templates
    └── assets/         # Project assets
```

## Quick Start

```bash
# Build DIRAMC
make all

# Run tests
make test

# Install
make install
```

## Integration with LibPolyCall

DIRAMC acts as a wrapper for LibPolyCall to trace memory faults:

```bash
./diram -l libpolycall.so <your_program>
```

## Documentation

- [API Reference](docs/api/)
- [User Guides](docs/guides/)
- [Technical Specifications](docs/specs/)
- [Internal Architecture](docs/internals/)

## License

Part of the OBINexus Constitutional Ecosystem
EOF

# Create documentation index
cat > diramc/docs/README.md << 'EOF'
# DIRAMC Documentation

## Documentation Structure

### Active Documentation

- **[API](./api/)** - API reference and function documentation
- **[Guides](./guides/)** - User guides and tutorials
- **[Specs](./specs/)** - Technical specifications and RFCs
- **[Internals](./internals/)** - Internal architecture and design docs

### Archived Content

- **[Legacy](./archived/legacy/)** - Old documentation versions
- **[PDFs](./archived/pdfs/)** - PDF documents and papers
- **[Old Builds](./archived/old-builds/)** - Previous build artifacts

## Key Documents

### Specifications
- [DIRAM Technical Specification](specs/OBINexus_DIRAM_Technical_Specification.pdf)
- [DIRAM Boolean Logic Truth Table](internals/diram_boolean_truth_table.md)

### Architecture
- [Memory Management Gates](specs/DIRAM_Boolean_Logic_Truth_Table_Memory_Management_Gates.pdf)
- [Hotwire Architecture](internals/hotwire_architecture.md)

### Guides
- [Installation Guide](guides/INSTALLATION.md)
- [Integration with LibPolyCall](guides/libpolycall_integration.md)
EOF

print_status "Documentation structure created"

# Step 8: Create gitignore
echo -e "\n${BLUE}Step 8: Creating .gitignore...${NC}"

cat > diramc/.gitignore << 'EOF'
# Build artifacts
build/
*.o
*.so
*.a
*.dll
*.exe

# Logs
logs/
*.log

# Temporary files
*.tmp
*.swp
*~

# IDE files
.vscode/
.idea/
*.sublime-*

# OS files
.DS_Store
Thumbs.db

# Backup directories
*_backup_*/
*.backup
EOF

print_status ".gitignore created"

# Step 9: Move scripts
echo -e "\n${BLUE}Step 9: Organizing scripts...${NC}"

[ -d "scripts" ] && cp -r scripts/* diramc/scripts/utilities/ 2>/dev/null || true
[ -d "workflows" ] && cp -r workflows/* diramc/.github/workflows/ 2>/dev/null || true

print_status "Scripts organized"

# Step 10: Summary and cleanup recommendations
echo -e "\n${BLUE}Step 10: Generating cleanup report...${NC}"

# Count files in each category
pdf_count=$(find diramc/docs -name "*.pdf" 2>/dev/null | wc -l)
md_count=$(find diramc/docs -name "*.md" 2>/dev/null | wc -l)
src_count=$(find diramc/src -name "*.c" -o -name "*.h" 2>/dev/null | wc -l)

cat > diramc/REFACTOR_REPORT.md << EOF
# DIRAMC Refactoring Report
Generated: $(date)

## Statistics
- PDF documents organized: $pdf_count
- Markdown files organized: $md_count  
- Source files: $src_count

## New Structure Benefits
- Clear separation of concerns
- Polyglot codebase support in /src
- Organized documentation in /docs
- Archived old content for reference
- Proper build isolation

## Next Steps
1. Review archived content in docs/archived/
2. Update Makefiles for new structure
3. Add wrapper implementation
4. Update CI/CD workflows

## Backup Location
$BACKUP_DIR
EOF

print_status "Refactor report generated"

# Final summary
echo -e "\n${PURPLE}============================================${NC}"
echo -e "${GREEN}    Directory Refactoring Complete!        ${NC}"
echo -e "${PURPLE}============================================${NC}"
echo ""
echo "Summary:"
echo "  ✓ Created organized directory structure"
echo "  ✓ Moved PDFs to docs/archived/pdfs/ and docs/specs/"
echo "  ✓ Organized markdown documentation"
echo "  ✓ Archived old builds"
echo "  ✓ Created proper project documentation"
echo ""
echo -e "${GREEN}New structure:${NC}"
echo "  diramc/              # Root for polyglot codebase"
echo "    ├── src/           # Source code (modular)"
echo "    ├── docs/          # All documentation"
echo "    │   ├── specs/     # Technical specifications"
echo "    │   ├── guides/    # User guides"
echo "    │   └── archived/  # Old/legacy content"
echo "    ├── config/        # Configuration files"
echo "    └── scripts/       # Build and utility scripts"
echo ""
echo -e "${BLUE}Backup saved to: $BACKUP_DIR${NC}"
echo -e "${YELLOW}Review REFACTOR_REPORT.md for details${NC}"
echo ""
echo -e "${GREEN}Ready to add DIRAM wrapper implementation!${NC}"
