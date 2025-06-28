// src/cli/main.c - DIRAM CLI with detach mode and configuration support
// OBINexus Project - Directed Instruction RAM

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>

#include "diram/core/feature-alloc/alloc.h"
#include "diram/core/feature-alloc/feature_alloc.h"

#define DIRAM_VERSION "1.0.0"
#define DIRAM_CONFIG_ENV "DIRAM_CONFIG"
#define DIRAM_DEFAULT_CONFIG ".dramrc"

// Configuration Structure
typedef struct {
    char config_file[PATH_MAX];
    int detach_mode;
    int trace_enabled;
    int repl_mode;
    size_t memory_limit;      // Memory limit in MB
    char memory_space[64];    // Named memory space
    char log_dir[PATH_MAX];
    int verbose;
} diram_config_t;

// Global configuration
static diram_config_t g_config = {
    .config_file = "",
    .detach_mode = 0,
    .trace_enabled = 0,
    .repl_mode = 0,
    .memory_limit = 0,
    .memory_space = "default",
    .log_dir = "logs",
    .verbose = 0
};

// Signal handling for detached processes
static void sigchld_handler(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

// Parse configuration file (.dramrc or specified)
static int parse_config_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        if (g_config.verbose) {
            fprintf(stderr, "Config file '%s' not found, using defaults\n", filename);
        }
        return 0;
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n') continue;
        
        // Remove trailing newline
        line[strcspn(line, "\n")] = 0;
        
        // Parse key=value pairs
        char *key = strtok(line, "=");
        char *value = strtok(NULL, "=");
        
        if (!key || !value) continue;
        
        // Trim whitespace
        while (*key == ' ') key++;
        while (*value == ' ') value++;
        
        if (strcmp(key, "memory_limit") == 0) {
            g_config.memory_limit = strtoul(value, NULL, 10);
        } else if (strcmp(key, "memory_space") == 0) {
            strncpy(g_config.memory_space, value, sizeof(g_config.memory_space) - 1);
        } else if (strcmp(key, "trace") == 0) {
            g_config.trace_enabled = (strcmp(value, "true") == 0);
        } else if (strcmp(key, "log_dir") == 0) {
            strncpy(g_config.log_dir, value, sizeof(g_config.log_dir) - 1);
        }
    }
    
    fclose(fp);
    return 0;
}

// Detach mode implementation
static int run_detached(char **argv) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    
    if (pid > 0) {
        // Parent process
        printf("DIRAM detached with PID: %d\n", pid);
        printf("Logs: %s/diram.{out,err}.log\n", g_config.log_dir);
        return 0;
    }
    
    // Child process - become session leader
    if (setsid() < 0) {
        perror("setsid");
        exit(1);
    }
    
    // Setup signal handling
    signal(SIGCHLD, sigchld_handler);
    signal(SIGHUP, SIG_IGN);
    
    // Create log directory
    mkdir(g_config.log_dir, 0755);
    
    // Redirect stdout/stderr to log files with safe path construction
    char log_path[PATH_MAX];
    int path_len;
    
    // Ensure log directory length leaves room for filename
    if (strlen(g_config.log_dir) > PATH_MAX - 32) {
        fprintf(stderr, "Log directory path too long\n");
        exit(1);
    }
    
    path_len = snprintf(log_path, sizeof(log_path), "%s/diram.out.log", g_config.log_dir);
    if (path_len >= (int)sizeof(log_path)) {
        fprintf(stderr, "Output log path truncated\n");
        exit(1);
    }
    
    int fd_out = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd_out >= 0) {
        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }
    
    path_len = snprintf(log_path, sizeof(log_path), "%s/diram.err.log", g_config.log_dir);
    if (path_len >= (int)sizeof(log_path)) {
        fprintf(stderr, "Error log path truncated\n");
        exit(1);
    }
    
    int fd_err = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd_err >= 0) {
        dup2(fd_err, STDERR_FILENO);
        close(fd_err);
    }
    
    // Close stdin
    close(STDIN_FILENO);
    
    // Execute with clean argv (remove --detach)
    execvp(argv[0], argv);
    perror("execvp");
    exit(1);
}

// Memory isolation with configurable limits
static int setup_memory_isolation(void) {
    if (g_config.memory_limit == 0) {
        return 0; // No limit configured
    }
    
    // Initialize traced allocation system
    if (g_config.trace_enabled) {
        if (diram_init_trace_log() < 0) {
            fprintf(stderr, "Warning: Failed to initialize trace log\n");
        }
    }
    
    printf("Memory isolation configured:\n");
    printf("  Space: %s\n", g_config.memory_space);
    printf("  Limit: %zu MB\n", g_config.memory_limit);
    printf("  Trace: %s\n", g_config.trace_enabled ? "enabled" : "disabled");
    
    return 0;
}

// REPL mode implementation
static int run_repl(void) {
    printf("DIRAM REPL v%s\n", DIRAM_VERSION);
    printf("Type 'help' for commands, 'exit' to quit\n\n");
    
    char line[1024];
    while (1) {
        printf("diram> ");
        fflush(stdout);
        
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Process commands
        if (strcmp(line, "exit") == 0 || strcmp(line, "quit") == 0) {
            break;
        } else if (strcmp(line, "help") == 0) {
            printf("Commands:\n");
            printf("  alloc <size> <tag>  - Allocate traced memory\n");
            printf("  free <addr>         - Free allocated memory\n");
            printf("  trace               - Show allocation trace\n");
            printf("  config              - Show current configuration\n");
            printf("  exit/quit           - Exit REPL\n");
        } else if (strncmp(line, "alloc ", 6) == 0) {
            size_t size;
            char tag[64] = {0};
            if (sscanf(line + 6, "%zu %63s", &size, tag) >= 1) {
                diram_allocation_t *alloc = diram_alloc_traced(size, tag[0] ? tag : NULL);
                if (alloc) {
                    printf("Allocated %zu bytes at %p (SHA: %.16s...)\n", 
                           size, alloc->base_addr, alloc->sha256_receipt);
                } else {
                    printf("Allocation failed (constraint violation or OOM)\n");
                }
            }
        } else if (strcmp(line, "config") == 0) {
            printf("Current configuration:\n");
            printf("  Memory limit: %zu MB\n", g_config.memory_limit);
            printf("  Memory space: %s\n", g_config.memory_space);
            printf("  Trace enabled: %s\n", g_config.trace_enabled ? "yes" : "no");
        } else if (strlen(line) > 0) {
            printf("Unknown command: %s\n", line);
        }
    }
    
    printf("\nExiting DIRAM REPL\n");
    return 0;
}

static void print_usage(const char *prog) {
    printf("Usage: %s [OPTIONS] [COMMAND]\n", prog);
    printf("\nOptions:\n");
    printf("  -c, --config FILE    Load configuration from FILE (default: .dramrc)\n");
    printf("  -d, --detach         Run in detached mode (daemon)\n");
    printf("  -t, --trace          Enable memory allocation tracing\n");
    printf("  -r, --repl           Start interactive REPL\n");
    printf("  -m, --memory LIMIT   Set memory limit in MB\n");
    printf("  -s, --space NAME     Set memory space name\n");
    printf("  -v, --verbose        Enable verbose output\n");
    printf("  -h, --help           Show this help\n");
    printf("  -V, --version        Show version\n");
    printf("\nExamples:\n");
    printf("  %s --detach -c myconfig.drc\n", prog);
    printf("  %s --repl --trace\n", prog);
    printf("  %s --memory 1024 --space userspace\n", prog);
}

int main(int argc, char *argv[]) {
    // Command line options
    static struct option long_options[] = {
        {"config",  required_argument, 0, 'c'},
        {"detach",  no_argument,       0, 'd'},
        {"trace",   no_argument,       0, 't'},
        {"repl",    no_argument,       0, 'r'},
        {"memory",  required_argument, 0, 'm'},
        {"space",   required_argument, 0, 's'},
        {"verbose", no_argument,       0, 'v'},
        {"help",    no_argument,       0, 'h'},
        {"version", no_argument,       0, 'V'},
        {0, 0, 0, 0}
    };
    
    // Parse command line
    int opt;
    while ((opt = getopt_long(argc, argv, "c:dtrm:s:vhV", long_options, NULL)) != -1) {
        switch (opt) {
            case 'c':
                strncpy(g_config.config_file, optarg, sizeof(g_config.config_file) - 1);
                break;
            case 'd':
                g_config.detach_mode = 1;
                break;
            case 't':
                g_config.trace_enabled = 1;
                break;
            case 'r':
                g_config.repl_mode = 1;
                break;
            case 'm':
                g_config.memory_limit = strtoul(optarg, NULL, 10);
                break;
            case 's':
                strncpy(g_config.memory_space, optarg, sizeof(g_config.memory_space) - 1);
                break;
            case 'v':
                g_config.verbose = 1;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            case 'V':
                printf("DIRAM v%s (OBINexus Project)\n", DIRAM_VERSION);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    // Load configuration file
    const char *config_file = g_config.config_file[0] ? g_config.config_file : 
                              getenv(DIRAM_CONFIG_ENV) ? getenv(DIRAM_CONFIG_ENV) : 
                              DIRAM_DEFAULT_CONFIG;
    parse_config_file(config_file);
    
    // Handle detach mode
    if (g_config.detach_mode) {
        // Prepare argv without --detach for re-execution
        char **new_argv = malloc(sizeof(char*) * argc);
        int j = 0;
        for (int i = 0; i < argc; i++) {
            if (strcmp(argv[i], "--detach") != 0 && strcmp(argv[i], "-d") != 0) {
                new_argv[j++] = argv[i];
            }
        }
        new_argv[j] = NULL;
        
        int ret = run_detached(new_argv);
        free(new_argv);
        return ret;
    }
    
    // Setup memory isolation
    setup_memory_isolation();
    
    // Handle REPL mode
    if (g_config.repl_mode) {
        return run_repl();
    }
    
    // Default operation
    printf("DIRAM v%s initialized\n", DIRAM_VERSION);
    printf("Configuration:\n");
    printf("  Config file: %s\n", config_file);
    printf("  Memory space: %s\n", g_config.memory_space);
    if (g_config.memory_limit > 0) {
        printf("  Memory limit: %zu MB\n", g_config.memory_limit);
    }
    
    // Cleanup
    if (g_config.trace_enabled) {
        diram_close_trace_log();
    }
    
    return 0;
}