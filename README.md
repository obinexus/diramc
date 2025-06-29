# DIRAM (Directed Instruction RAM)

[![OBINexus](https://img.shields.io/badge/OBINexus-Aegis_Project-blue)](https://github.com/obinexus)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](https://github.com/obinexus/diram)

DIRAM is a hybrid architecture for executing genetically-inspired instruction sets using a memory model that fuses stack resolution with RAM persistence. Part of the OBINexus Aegis Project, DIRAM provides cryptographically-enforced memory allocation with zero-trust governance integration.

## Table of Contents

- [Features](#features)
- [Architecture](#architecture)
- [Installation](#installation)
- [Usage](#usage)
- [Configuration](#configuration)
- [CLI Reference](#cli-reference)
- [Memory Governance](#memory-governance)
- [Development](#development)
- [Contributing](#contributing)

## Features

- **Cryptographic Memory Tracing**: SHA-256 receipts for all allocations
- **Heap Constraint Enforcement**: Sinphasé governance with ε(x) ≤ 0.6 constraint
- **Zero-Trust Memory Boundaries**: Cryptographically enforced isolation
- **Detached Daemon Mode**: Background operation with comprehensive logging
- **Enhanced Error Indexing**: Telemetry-driven error tracking and recovery
- **Memory Space Isolation**: Named memory spaces with configurable limits
- **REPL Interface**: Interactive memory allocation and inspection

## Architecture

DIRAM implements a multi-layer memory management system:

```
┌─────────────────────────────────────┐
│         Application Layer           │
├─────────────────────────────────────┤
│    Enhanced Feature Allocation      │
│  (Error Indexing & Governance)      │
├─────────────────────────────────────┤
│      Core Traced Allocation         │
│    (SHA-256 Receipt Generation)     │
├─────────────────────────────────────┤
│      Heap Event Constraints         │
│        (ε(x) ≤ 0.6 Enforcement)     │
└─────────────────────────────────────┘
```

## Installation

### Prerequisites

- GCC or compatible C compiler
- POSIX-compliant system (Linux, macOS, BSD)
- GNU Make
- pthread support

### Building from Source

```bash
git clone https://github.com/obinexus/diram.git
cd diram
make clean
make
```

### Installation

```bash
# System-wide installation (requires privileges)
sudo make install

# Custom prefix installation
make install PREFIX=$HOME/.local
```

## Usage

### Basic Commands

```bash
# Initialize DIRAM with standard governance
diram --init

# Run with tracing enabled
diram --trace

# Start interactive REPL
diram --repl

# Run in detached daemon mode
diram --detach -c /path/to/config.drc
```

### Detached Mode Example

```bash
# Start DIRAM daemon with custom configuration
diram --detach --config production.drc --trace

# Logs will be written to:
# logs/diram.out.log - Standard output
# logs/diram.err.log - Error output
# logs/alloc_trace.log - Allocation traces (if enabled)
```

## Configuration

DIRAM uses a hierarchical configuration system with `.dramrc` files:

### Configuration File Format

```ini
# ~/.dramrc or project-local .dramrc
memory_limit=2048       # Memory limit in MB
memory_space=production # Named memory space
trace=true              # Enable allocation tracing
log_dir=logs           # Log directory path

# Heap constraint configuration
max_heap_events=3      # Maximum allocations per epoch

# Process isolation
detach_timeout=30      # Daemon timeout in seconds
pid_binding=strict     # Fork safety enforcement

# Memory protection
guard_pages=true       # Enable guard pages
canary_values=true     # Enable canary values
aslr_enabled=true      # Address space randomization

# Zero-trust configuration
zero_trust=true        # Enable zero-trust boundaries
memory_audit=true      # Enable audit trail

# Telemetry settings
telemetry_level=2      # 0=disabled, 1=system, 2=opcode-bound
```

### Configuration Hierarchy

1. System-wide: `/etc/diram/config.dram`
2. User home: `~/.dramrc`
3. Project local: `./.dramrc`
4. Command line: `-c <file>`
5. Environment: `DIRAM_CONFIG=<file>`

## CLI Reference

### Command Line Options

```
diram [OPTIONS] [COMMAND]

Options:
  -c, --config FILE      Load configuration from FILE (default: .dramrc)
  -d, --detach          Run in detached mode (daemon)
  -t, --trace           Enable memory allocation tracing
  -r, --repl            Start interactive REPL
  -m, --memory LIMIT    Set memory limit in MB
  -s, --space NAME      Set memory space name
  -v, --verbose         Enable verbose output
  -h, --help            Show this help
  -V, --version         Show version

Examples:
  diram --detach -c production.drc
  diram --repl --trace
  diram --memory 1024 --space userspace
```

### REPL Commands

When running in REPL mode (`diram --repl`):

```
Commands:
  alloc <size> <tag>    Allocate traced memory
  free <addr>           Free allocated memory  
  trace                 Show allocation trace
  config                Show current configuration
  exit/quit             Exit REPL
```

### Example REPL Session

```
$ diram --repl --trace
DIRAM REPL v1.0.0
Type 'help' for commands, 'exit' to quit

diram> alloc 1024 user_buffer
Allocated 1024 bytes at 0x7f8a2c001000 (SHA: 3d4f2c8a9b6e1f...)

diram> trace
Active allocations:
  0x7f8a2c001000: 1024 bytes, tag=user_buffer, SHA=3d4f2c8a9b6e1f...

diram> config
Current configuration:
  Memory limit: 2048 MB
  Memory space: default
  Trace enabled: yes
```

## Memory Governance

### Heap Event Constraints

DIRAM enforces the Sinphasé governance constraint ε(x) ≤ 0.6:

- Maximum 3 heap events per command epoch
- Automatic epoch detection and counter reset
- Constraint violations result in allocation deferral

### Zero-Trust Enforcement

Memory boundaries are cryptographically enforced:

```c
// Each allocation generates a cryptographic receipt
typedef struct {
    void* base_addr;
    size_t size;
    uint64_t timestamp;
    char sha256_receipt[65];
    uint8_t heap_events;
    pid_t binding_pid;
} diram_allocation_t;
```

### Error Index Categories

DIRAM tracks and categorizes errors for governance:

- `0x1001`: Heap constraint violation (ε(x) > 0.6)
- `0x1002`: Memory exhausted condition
- `0x1003`: PID mismatch (fork safety)
- `0x1004`: Zero-trust boundary breach
- `0x1005`: SHA-256 verification failure

## Development

### Project Structure

```
diram/
├── include/
│   └── diram/
│       └── core/
│           └── feature-alloc/
│               ├── alloc.h
│               └── feature_alloc.h
├── src/
│   ├── cli/
│   │   └── main.c
│   └── core/
│       └── feature-alloc/
│           ├── alloc.c
│           └── feature_alloc.c
├── tests/
├── examples/
├── Makefile
├── diram.drc
└── README.md
```

### Building Debug Version

```bash
make clean
make DEBUG=1
```

### Running Tests

```bash
make test
```

### Static Analysis

```bash
make analyze
```

## Integration with OBINexus Ecosystem

DIRAM integrates seamlessly with other OBINexus components:

- **RIFTlang**: Governance contract validation
- **Polybuild**: Build orchestration
- **Git-RAF**: Version control with governance
- **Gosilang**: Runtime execution environment

## Performance Characteristics

- **Allocation Overhead**: O(1) with SHA-256 computation
- **Memory Overhead**: ~128 bytes per allocation for metadata
- **Constraint Checking**: O(1) epoch-based validation
- **Trace Log Writing**: Asynchronous with line buffering

## Security Considerations

1. **Fork Safety**: PID binding prevents cross-process memory access
2. **Cryptographic Receipts**: SHA-256 ensures allocation integrity
3. **Guard Pages**: Optional boundary protection (performance impact)
4. **ASLR**: Address randomization when enabled

## Contributing

Contributions to DIRAM must follow the Aegis Project waterfall methodology:

1. **Research Phase**: Problem analysis and solution design
2. **Implementation Phase**: Code development with governance
3. **Validation Phase**: Testing and compliance verification
4. **Integration Phase**: Ecosystem compatibility testing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details.

## License

DIRAM is part of the OBINexus Aegis Project and is licensed under the MIT License. See [LICENSE](LICENSE) for details.

## Acknowledgments

- OBINexus Protocol Engineering Group
- Aegis Project Technical Specification contributors
- NASA-STD-8739.8 Software Safety Standards

## Status

Currently in **active development** as part of the Aegis Project Phase 2.

---

*Designed for safety-critical systems requiring cryptographic memory governance.*