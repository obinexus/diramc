# DIRAM Makefile - Directed Instruction RAM Build System
# OBINexus Project Component - Aegis Waterfall Methodology
# Modular build: core → libdiram.{a,so} → cli → diram.exe → examples

# Compiler and Base Flags
CC = gcc
AR = ar
CFLAGS_BASE = -Wall -Wextra -Werror -pedantic -std=c11
CFLAGS_BASE += -fPIC  # Position Independent Code for shared library
CFLAGS_BASE += -D_GNU_SOURCE  # For fork(), execvp()
CFLAGS_BASE += -pthread       # For pthread_mutex

# Include Paths
INCLUDES = -I./include

# Library Flags - Correct usage: -ldiram (not -llibdiram)
LDFLAGS_CORE = -shared -pthread -lm
LDFLAGS_CLI = -L$(LIB_DIR) -ldiram -pthread -lm -Wl,-rpath,$(LIB_DIR)
LDFLAGS_EXAMPLES = $(LDFLAGS_CLI)

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
EXAMPLES_DIR = examples
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin
LIB_DIR = $(BUILD_DIR)/lib
CONFIG_DIR = config

# Core Library Sources
CORE_SRCS = $(SRC_DIR)/core/feature-alloc/alloc.c \
            $(SRC_DIR)/core/feature-alloc/feature_alloc.c \
            $(SRC_DIR)/core/config/config.c

# CLI Sources
CLI_SRCS = $(SRC_DIR)/cli/main.c 

# Example Sources
EXAMPLE_CACHE_SRCS = $(EXAMPLES_DIR)/diram/cache-resolution-lookahead/main.c

# Test Sources
TEST_SRCS = $(TEST_DIR)/core/alloc/test_alloc.c

# Object Files
CORE_OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(CORE_SRCS))
CLI_OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(CLI_SRCS))
EXAMPLE_CACHE_OBJS = $(patsubst $(EXAMPLES_DIR)/%.c,$(OBJ_DIR)/examples/%.o,$(EXAMPLE_CACHE_SRCS))
TEST_OBJS = $(patsubst $(TEST_DIR)/%.c,$(OBJ_DIR)/test_%.o,$(TEST_SRCS))

# Target Libraries and Executables
LIBDIRAM_STATIC = $(LIB_DIR)/libdiram.a
LIBDIRAM_SHARED = $(LIB_DIR)/libdiram.so
LIBDIRAM_SONAME = $(LIB_DIR)/libdiram.so.1
DIRAM_EXE = $(BIN_DIR)/diram
EXAMPLE_CACHE_EXE = $(BIN_DIR)/examples/cache-resolution-lookahead
TEST_EXE = $(BIN_DIR)/test_alloc

# Configuration Files
CONFIG_FILES = diram.drc

# Version Info
DIRAM_VERSION = 1.0.0
SONAME = libdiram.so.1

# Default Target
all: directories $(LIBDIRAM_STATIC) $(LIBDIRAM_SHARED) $(DIRAM_EXE) configs

# Create build directory structure
directories:
	@echo "[MKDIR] Creating build directories..."
	@mkdir -p $(OBJ_DIR)/core/feature-alloc
	@mkdir -p $(OBJ_DIR)/cli
	@mkdir -p $(OBJ_DIR)/examples/diram/cache-resolution-lookahead
	@mkdir -p $(OBJ_DIR)/test_core/alloc
	@mkdir -p $(BIN_DIR)/examples
	@mkdir -p $(LIB_DIR)
	@mkdir -p $(BUILD_DIR)/config
	@mkdir -p logs

# Pattern Rules for Object Files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/examples/%.o: $(EXAMPLES_DIR)/%.c
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
	@$(CC) $(LDFLAGS_CORE) -Wl,-soname,$(SONAME) -o $(LIBDIRAM_SONAME) $^
	@cd $(LIB_DIR) && ln -sf $(SONAME) libdiram.so
	@echo "[INFO] Shared library built: $(SONAME)"

# CLI Executable Target - Links against libdiram correctly
$(DIRAM_EXE): $(CLI_OBJS) $(LIBDIRAM_STATIC)
	@echo "[LD] Linking executable: $@"
	@$(CC) $(CFLAGS) $(CLI_OBJS) $(LDFLAGS_CLI) -o $@
	@echo "[INFO] Executable built: $(notdir $@)"

# Example Targets
examples: directories $(LIBDIRAM_STATIC) $(EXAMPLE_CACHE_EXE)

$(EXAMPLE_CACHE_EXE): $(EXAMPLE_CACHE_OBJS) $(LIBDIRAM_STATIC)
	@echo "[LD] Linking example: $@"
	@$(CC) $(CFLAGS) $(EXAMPLE_CACHE_OBJS) $(LDFLAGS_EXAMPLES) -o $@
	@echo "[INFO] Example built: $(notdir $@)"

# Configuration Files
configs: directories
	@echo "[CONFIG] Installing configuration files..."
	@cp $(CONFIG_FILES) $(BUILD_DIR)/config/
	@echo "[INFO] Configuration files installed to $(BUILD_DIR)/config/"

# Test Target
test: directories $(TEST_EXE)
	@echo "[TEST] Running DIRAM feature-alloc tests..."
	@LD_LIBRARY_PATH=$(LIB_DIR) $(TEST_EXE)

$(TEST_EXE): $(TEST_OBJS) $(LIBDIRAM_STATIC)
	@echo "[LD] Linking test executable: $@"
	@$(CC) $(CFLAGS) $(TEST_OBJS) $(LDFLAGS_CLI) -o $@

# Run Examples
run-example-cache: $(EXAMPLE_CACHE_EXE)
	@echo "[RUN] Executing cache resolution lookahead example..."
	@cd $(BUILD_DIR) && LD_LIBRARY_PATH=$(LIB_DIR) $(EXAMPLE_CACHE_EXE) -c config/diram.drc

# Library Info Target
.PHONY: libinfo
libinfo: $(LIBDIRAM_STATIC) $(LIBDIRAM_SHARED)
	@echo "=== DIRAM Library Information ==="
	@echo "Static Library: $(LIBDIRAM_STATIC)"
	@size $(LIBDIRAM_STATIC) 2>/dev/null || echo "Size information not available"
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
	@install -d $(PREFIX)/etc/diram
	@install -m 755 $(DIRAM_EXE) $(PREFIX)/bin/diram
	@install -m 644 $(LIBDIRAM_STATIC) $(PREFIX)/lib/
	@install -m 755 $(LIBDIRAM_SONAME) $(PREFIX)/lib/
	@cd $(PREFIX)/lib && ln -sf $(SONAME) libdiram.so
	@install -m 644 diram.drc $(PREFIX)/etc/diram/
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
	@install -d $(OBINEXUS_DEPLOY_PATH)/etc/diram
	@install -m 755 $(DIRAM_EXE) $(OBINEXUS_DEPLOY_PATH)/bin/
	@install -m 644 $(LIBDIRAM_STATIC) $(OBINEXUS_DEPLOY_PATH)/lib/
	@install -m 755 $(LIBDIRAM_SONAME) $(OBINEXUS_DEPLOY_PATH)/lib/
	@cd $(OBINEXUS_DEPLOY_PATH)/lib && ln -sf $(SONAME) libdiram.so
	@install -m 644 diram.drc $(OBINEXUS_DEPLOY_PATH)/etc/diram/
	@echo "[DEPLOY] OBINexus deployment complete"

# Help Target
.PHONY: help
help:
	@echo "DIRAM Build System - OBINexus Project"
	@echo "====================================="
	@echo "Targets:"
	@echo "  all          - Build libraries and executable (default)"
	@echo "  examples     - Build example programs"
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
	@echo "  run-example-cache - Run cache resolution lookahead example"
	@echo "  obinexus-deploy - Deploy to OBINexus environment"
	@echo ""
	@echo "Variables:"
	@echo "  DEBUG=1      - Enable debug build"
	@echo "  PREFIX=path  - Installation prefix (default: /usr/local)"

.PHONY: all directories test libinfo install debug release clean distclean analyze format docs obinexus-deploy help examples run-example-cache configs