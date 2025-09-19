#!/bin/bash
# ============================================================================
# DIRAMC PDF Document Organizer
# Specifically handles PDF organization for the OBINexus project
# ============================================================================

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m'

echo -e "${PURPLE}PDF Document Organization for DIRAMC${NC}"
echo "======================================"
echo ""

# Create PDF organization structure
echo -e "${BLUE}Creating PDF organization directories...${NC}"
mkdir -p diramc/docs/specs/pdfs
mkdir -p diramc/docs/archived/pdfs/{technical,design,legacy}
mkdir -p diramc/docs/references

# Function to categorize and move PDFs
organize_pdf() {
    local pdf_path="$1"
    local pdf_name=$(basename "$pdf_path")
    local target_dir=""
    
    # Categorize based on filename
    case "$pdf_name" in
        *"Technical Specification"*|*"DIRAM Technical"*)
            target_dir="diramc/docs/specs/pdfs"
            echo -e "  ${GREEN}[SPEC]${NC} $pdf_name"
            ;;
        *"Boolean Logic"*|*"Truth Table"*|*"Memory Management"*)
            target_dir="diramc/docs/specs/pdfs"
            echo -e "  ${GREEN}[SPEC]${NC} $pdf_name"
            ;;
        *"Design"*|*"Manifesto"*)
            target_dir="diramc/docs/references"
            echo -e "  ${BLUE}[REF]${NC} $pdf_name"
            ;;
        *"festival"*|*"analogy"*)
            target_dir="diramc/docs/archived/pdfs/design"
            echo -e "  ${YELLOW}[ARCH]${NC} $pdf_name"
            ;;
        *)
            target_dir="diramc/docs/archived/pdfs/legacy"
            echo -e "  ${YELLOW}[LEGACY]${NC} $pdf_name"
            ;;
    esac
    
    # Copy the PDF to its new location
    if [ -f "$pdf_path" ]; then
        cp "$pdf_path" "$target_dir/$pdf_name" 2>/dev/null
        echo "       → $target_dir/"
    fi
}

# Find and organize all PDFs
echo -e "\n${BLUE}Scanning for PDF documents...${NC}"
echo ""

# List of expected PDFs based on the tree structure
declare -a EXPECTED_PDFS=(
    "DIRAM Boolean Logic Truth Table - Memory Management Gates.pdf"
    "OBINexus DIRAM Technical Specification.pdf"
    "The DIRAM Design and Technology Manifesto.pdf"
    "diram_festival_analogy.pdf"
)

# Process each expected PDF
for pdf_name in "${EXPECTED_PDFS[@]}"; do
    # Search for the PDF in the current directory structure
    find . -name "$pdf_name" -type f 2>/dev/null | head -1 | while read -r pdf_path; do
        organize_pdf "$pdf_path"
    done
done

# Also find any other PDFs not in the expected list
echo -e "\n${BLUE}Searching for additional PDFs...${NC}"
find . -name "*.pdf" -type f 2>/dev/null | while read -r pdf_path; do
    pdf_name=$(basename "$pdf_path")
    # Check if it's not in our expected list
    if [[ ! " ${EXPECTED_PDFS[@]} " =~ " ${pdf_name} " ]]; then
        echo -e "  ${PURPLE}[FOUND]${NC} $pdf_name"
        organize_pdf "$pdf_path"
    fi
done

# Create an index of all PDFs
echo -e "\n${BLUE}Creating PDF index...${NC}"
cat > diramc/docs/PDF_INDEX.md << 'EOF'
# DIRAMC PDF Document Index

## Technical Specifications (`docs/specs/pdfs/`)
Primary technical documentation for DIRAMC system.

- **OBINexus DIRAM Technical Specification.pdf**
  - Core DIRAMC technical specifications
  - Memory management architecture
  - Integration protocols

- **DIRAM Boolean Logic Truth Table - Memory Management Gates.pdf**
  - Boolean logic implementation
  - Memory gate operations
  - Truth table specifications

## References (`docs/references/`)
Design philosophy and conceptual documents.

- **The DIRAM Design and Technology Manifesto.pdf**
  - Design principles
  - Technology philosophy
  - Implementation guidelines

## Archived Documents (`docs/archived/pdfs/`)

### Design Documents (`archived/pdfs/design/`)
- **diram_festival_analogy.pdf**
  - Conceptual analogies
  - Educational materials

### Legacy Documents (`archived/pdfs/legacy/`)
Older versions and deprecated documentation.

---

## Quick Access Paths

```bash
# View technical specs
ls diramc/docs/specs/pdfs/

# View reference materials
ls diramc/docs/references/

# View archived content
ls diramc/docs/archived/pdfs/
```

## Document Categories

| Category | Location | Description |
|----------|----------|-------------|
| Specifications | `docs/specs/pdfs/` | Current technical specifications |
| References | `docs/references/` | Design and philosophy documents |
| Archived Design | `docs/archived/pdfs/design/` | Old design documents |
| Legacy | `docs/archived/pdfs/legacy/` | Deprecated documentation |

EOF

echo -e "${GREEN}✓ PDF index created at diramc/docs/PDF_INDEX.md${NC}"

# Summary
echo ""
echo -e "${PURPLE}════════════════════════════════════════${NC}"
echo -e "${GREEN}PDF Organization Complete!${NC}"
echo -e "${PURPLE}════════════════════════════════════════${NC}"
echo ""
echo "Summary:"
echo "  • Technical specs → docs/specs/pdfs/"
echo "  • References → docs/references/"
echo "  • Archived → docs/archived/pdfs/"
echo "  • Index created → docs/PDF_INDEX.md"
echo ""
echo -e "${YELLOW}Next: Run the full refactoring script to complete reorganization${NC}"
