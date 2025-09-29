# DIRAM Enhanced Makefile with Dynamic Library Support
CC = gcc
CFLAGS = -Wall -Wextra -g -fPIC -pthread -I./include
LDFLAGS = -ldl -lpthread -Wl,-rpath,'$$ORIGIN/../lib'
AR = ar
ARFLAGS = rcs

# Directories
SRC_DIR = src
BUILD_DIR = build
LIB_DIR = lib
BIN_DIR = bin
INCLUDE_DIR = include

# Source files
CLI_SRCS = $(SRC_DIR)/cli/main.c
CORE_SRCS = $(wildcard $(SRC_DIR)/core/*.c) \
            $(wildcard $(SRC_DIR)/core/feature-alloc/*.c) \
            $(wildcard $(SRC_DIR)/core/hotwire/*.c) \
            $(wildcard $(SRC_DIR)/core/monitor/*.c) \
            $(wildcard $(SRC_DIR)/core/parser/*.c) \
            $(wildcard $(SRC_DIR)/core/assembly/*.c) \
            $(wildcard $(SRC_DIR)/core/config/*.c)

# Object files
CLI_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(CLI_SRCS))
CORE_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(CORE_SRCS))

# Targets
TARGET = $(BIN_DIR)/diram
SHARED_LIB = $(LIB_DIR)/libdiram.so
STATIC_LIB = $(LIB_DIR)/libdiram.a

# Example libraries for testing
EXAMPLE_LIBS = $(LIB_DIR)/libsensor.so $(LIB_DIR)/libnavigation.so

.PHONY: all clean install test libs example-libs

all: directories $(TARGET) $(SHARED_LIB) $(STATIC_LIB) example-libs

directories:
	@mkdir -p $(BUILD_DIR)/cli
	@mkdir -p $(BUILD_DIR)/core/feature-alloc
	@mkdir -p $(BUILD_DIR)/core/hotwire
	@mkdir -p $(BUILD_DIR)/core/monitor
	@mkdir -p $(BUILD_DIR)/core/parser
	@mkdir -p $(BUILD_DIR)/core/assembly
	@mkdir -p $(BUILD_DIR)/core/config
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(LIB_DIR)

# Build main executable with dynamic linking support
$(TARGET): $(CLI_OBJS) $(SHARED_LIB)
	@echo "[LD] Building executable with dynamic library support..."
	$(CC) $(CLI_OBJS) -o $@ -L$(LIB_DIR) -ldiram $(LDFLAGS)
	@echo "[✓] Built: $@"

# Build shared library (.so)
$(SHARED_LIB): $(CORE_OBJS)
	@echo "[SO] Building shared library..."
	$(CC) -shared -o $@ $(CORE_OBJS) $(LDFLAGS)
	@echo "[✓] Built: $@"

# Build static library (.a) - not recommended but supported
$(STATIC_LIB): $(CORE_OBJS)
	@echo "[AR] Building static library (not recommended)..."
	$(AR) $(ARFLAGS) $@ $(CORE_OBJS)
	@echo "[✓] Built: $@"

# Build object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "[CC] $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Build example libraries for testing
example-libs: $(EXAMPLE_LIBS)

$(LIB_DIR)/libsensor.so: examples/libsensor.c
	@mkdir -p examples
	@echo "// Example sensor library" > examples/libsensor.c
	@echo "#include <stdio.h>" >> examples/libsensor.c
	@echo "void* read_gps() { printf(\"GPS: 40.7128N, 74.0060W\\n\"); return (void*)0x1; }" >> examples/libsensor.c
	@echo "int get_altitude() { return 1000; }" >> examples/libsensor.c
	$(CC) -shared -fPIC -o $@ examples/libsensor.c
	@echo "[✓] Built example: $@"

$(LIB_DIR)/libnavigation.so: examples/libnavigation.c
	@mkdir -p examples
	@echo "// Example navigation library" > examples/libnavigation.c
	@echo "#include <stdio.h>" >> examples/libnavigation.c
	@echo "void* calculate_route() { printf(\"Route calculated\\n\"); return (void*)0x2; }" >> examples/libnavigation.c
	@echo "int get_distance() { return 500; }" >> examples/libnavigation.c
	$(CC) -shared -fPIC -o $@ examples/libnavigation.c
	@echo "[✓] Built example: $@"

# Test commands
test: all
	@echo "[TEST] Testing CLI with library loading..."
	./$(TARGET) --trace -L $(LIB_DIR) -l libsensor.so -l libnavigation.so
	@echo "[TEST] Testing trace mode..."
	./$(TARGET) --trace-lib $(LIB_DIR)/libsensor.so

# Run with specific configuration
run-drone: all
	@echo "[RUN] Running drone monitor with libraries..."
	./$(TARGET) --trace \
		--trace-lib $(LIB_DIR)/libnavigation.so \
		--trace-lib $(LIB_DIR)/libsensor.so \
		--detach \
		--log-path logs/drone.log \
		config/drone_monitor.dr

# Install to system
install: all
	@echo "[INSTALL] Installing DIRAM..."
	sudo cp $(TARGET) /usr/local/bin/
	sudo cp $(SHARED_LIB) /usr/local/lib/
	sudo cp -r $(INCLUDE_DIR)/* /usr/local/include/
	sudo ldconfig
	@echo "[✓] Installation complete"

# Clean build artifacts
clean:
	@echo "[CLEAN] Removing build artifacts..."
	rm -rf $(BUILD_DIR)
	rm -rf $(BIN_DIR)
	rm -rf $(LIB_DIR)
	rm -rf examples
	@echo "[✓] Clean complete"

# Help
help:
	@echo "DIRAM Build System"
	@echo "=================="
	@echo "Targets:"
	@echo "  all          - Build everything (executable + libraries)"
	@echo "  clean        - Remove all build artifacts"
	@echo "  install      - Install to system directories"
	@echo "  test         - Run basic tests"
	@echo "  run-drone    - Run drone monitor example"
	@echo "  example-libs - Build example libraries for testing"
	@echo ""
	@echo "Usage:"
	@echo "  make all"
	@echo "  ./bin/diram --trace -L ./lib -l libsensor.so"
	@echo ""
	@echo "Library Support:"
	@echo "  .so files (shared objects) - RECOMMENDED"
	@echo "  .a files (static archives) - Supported but not recommended"
