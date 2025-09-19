#!/bin/bash
# ============================================================================
# DIRAMC Structure Verification - Check What Needs Cleaning
# ============================================================================

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m'

echo -e "${PURPLE}============================================${NC}"
echo -e "${PURPLE}    DIRAMC Structure Verification          ${NC}"
echo -e "${PURPLE}============================================${NC}"
echo ""

cd ~/obinexus/workspace/diramc 2>/dev/null || cd . 

echo -e "${BLUE}Checking current structure issues...${NC}"
echo ""

# Check for files that should be moved
echo -e "${YELLOW}Files in root that need organizing:${NC}"
echo ""

# PDFs in root
echo -e "${RED}PDFs in root (should be in docs/):${NC}"
for pdf in *.pdf; do
    if [ -f "$pdf" ]; then
        echo "  ❌ $pdf"
    fi
done
[ ! -f *.pdf ] 2>/dev/null && echo "  ✓ No PDFs in root"

echo ""

# TXT files in root
echo -e "${RED}TXT files in root (should be archived):${NC}"
for txt in *.txt; do
    if [ -f "$txt" ]; then
        echo "  ❌ $txt"
    fi
done
[ ! -f *.txt ] 2>/dev/null && echo "  ✓ No TXT files in root"

echo ""

# Multiple Makefiles
echo -e "${RED}Multiple Makefiles (should be consolidated):${NC}"
for makefile in Makefile.*; do
    if [ -f "$makefile" ]; then
        echo "  ❌ $makefile"
    fi
done
[ ! -f Makefile.* ] 2>/dev/null && echo "  ✓ No multiple Makefiles"

echo ""

# Shell scripts in root
echo -e "${RED}Shell scripts in root (should be in scripts/):${NC}"
for script in *.sh; do
    if [ -f "$script" ]; then
        echo "  ❌ $script"
    fi
done
[ ! -f *.sh ] 2>/dev/null && echo "  ✓ No loose scripts in root"

echo ""

# Check for duplicate/weird directories
echo -e "${YELLOW}Directory issues:${NC}"
if [ -d "logs " ]; then
    echo "  ❌ Duplicate 'logs ' directory with trailing space"
else
    echo "  ✓ No duplicate logs directories"
fi

echo ""

# Check what's properly organized
echo -e "${GREEN}Already organized:${NC}"
[ -d "src/core" ] && echo "  ✓ src/core structure exists"
[ -d "docs/specs" ] && echo "  ✓ docs/specs exists"
[ -d "docs/archived" ] && echo "  ✓ docs/archived exists"
[ -d "include/diram" ] && echo "  ✓ include/diram exists"
[ -d "tests" ] && echo "  ✓ tests directory exists"
[ -d "examples" ] && echo "  ✓ examples directory exists"

echo ""
echo -e "${BLUE}Checking directory completeness...${NC}"
echo ""

# Check for missing directories
echo -e "${YELLOW}Missing directories that should exist:${NC}"
missing=0

check_dir() {
    if [ ! -d "$1" ]; then
        echo "  ❌ Missing: $1"
        missing=$((missing + 1))
    fi
}

check_dir "docs/api"
check_dir "docs/guides"
check_dir "docs/internals"
check_dir "config/development"
check_dir "config/production"
check_dir "config/testing"
check_dir "scripts/build"
check_dir "scripts/install"
check_dir "scripts/utilities"
check_dir "resources/schemas"
check_dir "resources/templates"

if [ $missing -eq 0 ]; then
    echo "  ✓ All directories present"
fi

echo ""
echo -e "${PURPLE}============================================${NC}"
echo -e "${BLUE}Recommendation:${NC}"
echo ""

# Count issues
issues=0
[ -f *.pdf ] 2>/dev/null && issues=$((issues + 1))
[ -f *.txt ] 2>/dev/null && issues=$((issues + 1))
[ -f Makefile.* ] 2>/dev/null && issues=$((issues + 1))
[ -f *.sh ] 2>/dev/null && issues=$((issues + 1))
[ -d "logs " ] && issues=$((issues + 1))
[ $missing -gt 0 ] && issues=$((issues + missing))

if [ $issues -gt 0 ]; then
    echo -e "${YELLOW}Found $issues issues that need fixing.${NC}"
    echo ""
    echo "To fix these issues, run:"
    echo -e "  ${GREEN}chmod +x diramc_cleanup.sh${NC}"
    echo -e "  ${GREEN}./diramc_cleanup.sh${NC}"
    echo ""
    echo "This will:"
    echo "  • Move PDFs to docs/archived/pdfs/"
    echo "  • Archive .txt files"
    echo "  • Consolidate Makefiles"
    echo "  • Organize scripts"
    echo "  • Create missing directories"
else
    echo -e "${GREEN}✓ Structure is properly organized!${NC}"
fi

echo ""
echo -e "${PURPLE}============================================${NC}"
