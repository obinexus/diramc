# DIRAM Makefile - Directed Instruction RAM Build System
# OBINexus Project Component - Aegis Waterfall Methodology
# Modular build: core → libdiram.{a,so} → cli → diram.exe

# Compiler and Base Flags
CC = gcc
AR = ar
CFLAGS_BASE = -Wall -Wextra -Werror -pedantic -std=c11
CFLAGS_BASE += -fPIC  # Position Independent Code for shared library
CFLAGS_BASE += -D_GNU_SOURCE  # For fork(), execvp()
CFLAGS_BASE += -pthread       # For pthread_mutex

# Include Paths
INCLUDES = -I./include

# Library Flags
LDFLAGS_CORE = -shared -pthread -lm
LDFLAGS_CLI = -L$(LIB_DIR) -ldiram -pthread -lm -Wl,-rpath,$(LIB_DIR)

# Build Modes
DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CFLAGS = $(CFLAGS_BASE) -g -O0 -DDEBUG
    BUILD_DIR = build/debug
else
    CFLAGS = $(CFLAGS_BASE) -O3 -DNDEBUG
    BUILD_DIR = build/release
endif

# Directories
SRC_DIR = src
INCLUDE_DIR = include
TEST_DIR = tests
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin
LIB_DIR = $(BUILD_DIR)/lib

# Core Library Sources
CORE_SRCS = $(SRC_DIR)/core/feature-alloc/alloc.c \
            $(SRC_DIR)/core/feature-alloc/feature_alloc.c

# CLI Sources
CLI_SRCS = $(SRC_DIR)/cli/main.c

# Test Sources
TEST_SRCS = $(TEST_DIR)/core/alloc/test_alloc.c

# Object Files
CORE_OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(CORE_SRCS))
CLI_OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(CLI_SRCS))
TEST_OBJS = $(patsubst $(TEST_DIR)/%.c,$(OBJ_DIR)/test_%.o,$(TEST_SRCS))

# Target Libraries and Executables
LIBDIRAM_STATIC = $(LIB_DIR)/libdiram.a
LIBDIRAM_SHARED = $(LIB_DIR)/libdiram.so
DIRAM_EXE = $(BIN_DIR)/diram.exe
TEST_EXE = $(BIN_DIR)/test_alloc

# Version Info
DIRAM_VERSION = 1.0.0
SONAME = libdiram.so.1

# Default Target
all: directories $(LIBDIRAM_STATIC) $(LIBDIRAM_SHARED) $(DIRAM_EXE)

# Create build directory structure
directories:
	@echo "[MKDIR] Creating build directories..."
	@mkdir -p $(OBJ_DIR)/core/feature-alloc
	@mkdir -p $(OBJ_DIR)/cli
	@mkdir -p $(OBJ_DIR)/test_core/alloc
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(LIB_DIR)
	@mkdir -p logs

# Pattern Rules for Object Files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/test_%.o: $(TEST_DIR)/%.c
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Core Library Targets
$(LIBDIRAM_STATIC): $(CORE_OBJS)
	@echo "[AR] Building static library: $@"
	@$(AR) rcs $@ $^
	@echo "[INFO] Static library built: $(notdir $@)"

$(LIBDIRAM_SHARED): $(CORE_OBJS)
	@echo "[LD] Building shared library: $@"
	@$(CC) $(LDFLAGS_CORE) -Wl,-soname,$(SONAME) -o $@ $^
	@cd $(LIB_DIR) && ln -sf $(notdir $@) $(SONAME)
	@cd $(LIB_DIR) && ln -sf $(SONAME) libdiram.so
	@echo "[INFO] Shared library built: $(notdir $@)"

# CLI Executable Target
$(DIRAM_EXE): $(CLI_OBJS) $(LIBDIRAM_STATIC)
	@echo "[LD] Linking executable: $@"
	@$(CC) $(CFLAGS) $(CLI_OBJS) $(LDFLAGS_CLI) -o $@
	@echo "[INFO] Executable built: $(notdir $@)"

# Test Target
test: directories $(TEST_EXE)
	@echo "[TEST] Running DIRAM feature-alloc tests..."
	@LD_LIBRARY_PATH=$(LIB_DIR) $(TEST_EXE)

$(TEST_EXE): $(TEST_OBJS) $(LIBDIRAM_STATIC)
	@echo "[LD] Linking test executable: $@"
	@$(CC) $(CFLAGS) $(TEST_OBJS) $(LDFLAGS_CLI) -o $@

# Library Info Target
.PHONY: libinfo
libinfo: $(LIBDIRAM_STATIC) $(LIBDIRAM_SHARED)
	@echo "=== DIRAM Library Information ==="
	@echo "Static Library: $(LIBDIRAM_STATIC)"
	@size $(LIBDIRAM_STATIC)
	@echo ""
	@echo "Shared Library: $(LIBDIRAM_SHARED)"
	@ls -la $(LIB_DIR)/libdiram*
	@echo ""
	@echo "Symbols in static library:"
	@nm -C $(LIBDIRAM_STATIC) | grep " T " | head -10
	@echo "..."

# Install Target
PREFIX ?= /usr/local
install: all
	@echo "[INSTALL] Installing DIRAM to $(PREFIX)"
	@install -d $(PREFIX)/bin
	@install -d $(PREFIX)/lib
	@install -d $(PREFIX)/include/diram/core/feature-alloc
	@install -d $(PREFIX)/include/diram/cli
	@install -m 755 $(DIRAM_EXE) $(PREFIX)/bin/diram
	@install -m 644 $(LIBDIRAM_STATIC) $(PREFIX)/lib/
	@install -m 755 $(LIBDIRAM_SHARED) $(PREFIX)/lib/
	@ldconfig $(PREFIX)/lib 2>/dev/null || true
	@cp -r $(INCLUDE_DIR)/diram/* $(PREFIX)/include/diram/

# Development Targets
.PHONY: debug
debug:
	@$(MAKE) DEBUG=1 all

.PHONY: release
release:
	@$(MAKE) DEBUG=0 all

# Clean Targets
clean:
	@echo "[CLEAN] Removing build artifacts..."
	@rm -rf $(BUILD_DIR)

distclean: clean
	@echo "[CLEAN] Removing logs and all generated files..."
	@rm -rf build/ logs/

# Static Analysis
.PHONY: analyze
analyze:
	@echo "[ANALYZE] Running static analysis..."
	@cppcheck --enable=all --inconclusive --std=c11 \
	          --suppress=missingIncludeSystem \
	          -I$(INCLUDE_DIR) $(SRC_DIR)

# Format Code
.PHONY: format
format:
	@echo "[FORMAT] Formatting source code..."
	@find $(SRC_DIR) $(INCLUDE_DIR) -name "*.c" -o -name "*.h" | \
	 xargs clang-format -i -style="{BasedOnStyle: LLVM, IndentWidth: 4}"

# Generate Documentation
.PHONY: docs
docs:
	@echo "[DOCS] Generating documentation..."
	@doxygen Doxyfile 2>/dev/null || echo "Doxygen not found"

# OBINexus Integration
OBINEXUS_DEPLOY_PATH ?= /opt/obinexus
.PHONY: obinexus-deploy
obinexus-deploy: all
	@echo "[DEPLOY] Deploying to OBINexus environment..."
	@install -d $(OBINEXUS_DEPLOY_PATH)/bin
	@install -d $(OBINEXUS_DEPLOY_PATH)/lib
	@install -m 755 $(DIRAM_EXE) $(OBINEXUS_DEPLOY_PATH)/bin/
	@install -m 644 $(LIBDIRAM_STATIC) $(OBINEXUS_DEPLOY_PATH)/lib/
	@install -m 755 $(LIBDIRAM_SHARED) $(OBINEXUS_DEPLOY_PATH)/lib/
	@echo "[DEPLOY] OBINexus deployment complete"

# Help Target
.PHONY: help
help:
	@echo "DIRAM Build System - OBINexus Project"
	@echo "====================================="
	@echo "Targets:"
	@echo "  all          - Build libraries and executable (default)"
	@echo "  debug        - Build with debug symbols"
	@echo "  release      - Build optimized release"
	@echo "  test         - Build and run tests"
	@echo "  libinfo      - Display library information"
	@echo "  install      - Install to system (PREFIX=$(PREFIX))"
	@echo "  clean        - Remove build artifacts"
	@echo "  distclean    - Remove all generated files"
	@echo "  analyze      - Run static code analysis"
	@echo "  format       - Format source code"
	@echo "  docs         - Generate documentation"
	@echo "  obinexus-deploy - Deploy to OBINexus environment"
	@echo ""
	@echo "Variables:"
	@echo "  DEBUG=1      - Enable debug build"
	@echo "  PREFIX=path  - Installation prefix (default: /usr/local)"

.PHONY: all directories test libinfo install debug release clean distclean analyze format docs obinexus-deploy help