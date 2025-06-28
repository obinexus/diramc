# DIRAM Makefile - Directed Instruction RAM Build System
# OBINexus Project Component

# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c11
CFLAGS += -I./include
CFLAGS += -D_GNU_SOURCE  # For fork(), execvp()
CFLAGS += -pthread       # For pthread_mutex
LDFLAGS = -lpthread -lm

# Build Modes
DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CFLAGS += -g -O0 -DDEBUG
    BUILD_DIR = build/debug
else
    CFLAGS += -O3 -DNDEBUG
    BUILD_DIR = build/release
endif

# Directories
SRC_DIR = src
INCLUDE_DIR = include
TEST_DIR = tests
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin
LIB_DIR = $(BUILD_DIR)/lib

# Source Files
ALLOC_SRCS = $(SRC_DIR)/core/alloc.c
CLI_SRCS = $(SRC_DIR)/cli/main.c
TEST_SRCS = $(TEST_DIR)/core/feature-alloc/test_alloc.c

# Object Files
ALLOC_OBJS = $(ALLOC_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
CLI_OBJS = $(CLI_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TEST_OBJS = $(TEST_SRCS:$(TEST_DIR)/%.c=$(OBJ_DIR)/test_%.o)

# Targets
LIBALLOC = $(LIB_DIR)/liballoc.a
DIRAM_BIN = $(BIN_DIR)/diram
TEST_BIN = $(BIN_DIR)/test_alloc

# Default Target
all: directories $(LIBALLOC) $(DIRAM_BIN)

# Create build directories
directories:
	@mkdir -p $(OBJ_DIR)/core
	@mkdir -p $(OBJ_DIR)/cli
	@mkdir -p $(OBJ_DIR)/test_core/feature-alloc
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(LIB_DIR)
	@mkdir -p logs

# Compilation Rules
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/test_%.o: $(TEST_DIR)/%.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Library Target
$(LIBALLOC): $(ALLOC_OBJS)
	@echo "AR $@"
	@ar rcs $@ $^

# Binary Targets
$(DIRAM_BIN): $(CLI_OBJS) $(LIBALLOC)
	@echo "LD $@"
	@$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

# Test Target
test: directories $(TEST_BIN)
	@echo "Running feature-alloc tests..."
	@$(TEST_BIN)

$(TEST_BIN): $(TEST_OBJS) $(LIBALLOC)
	@echo "LD $@"
	@$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

# Install Target
PREFIX ?= /usr/local
install: all
	@echo "Installing DIRAM to $(PREFIX)"
	@install -d $(PREFIX)/bin
	@install -d $(PREFIX)/lib
	@install -d $(PREFIX)/include/diram
	@install -m 755 $(DIRAM_BIN) $(PREFIX)/bin/
	@install -m 644 $(LIBALLOC) $(PREFIX)/lib/
	@cp -r $(INCLUDE_DIR)/diram/* $(PREFIX)/include/diram/

# Clean Targets
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf build/

distclean: clean
	@echo "Cleaning logs..."
	@rm -rf logs/

# Development Helpers
.PHONY: check-syntax
check-syntax:
	@echo "Checking syntax..."
	@$(CC) $(CFLAGS) -fsyntax-only $(shell find $(SRC_DIR) -name "*.c")

.PHONY: format
format:
	@echo "Formatting code..."
	@find . -name "*.c" -o -name "*.h" | xargs clang-format -i

# OBINexus Integration
.PHONY: obinexus-deploy
obinexus-deploy: all
	@echo "Deploying to OBINexus environment..."
	@cp $(DIRAM_BIN) $(OBINEXUS_DEPLOY_PATH)/bin/
	@cp $(LIBALLOC) $(OBINEXUS_DEPLOY_PATH)/lib/

.PHONY: all directories test install clean distclean check-syntax format obinexus-deploy