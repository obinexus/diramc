// src/cli/main_enhanced.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <dlfcn.h>
#include <pthread.h>
#include "diram/core/diram.h"
#include "diram/core/hotwire/hotwire.h"
#include "diram/core/monitor/diram_state_monitor.h"

#define MAX_LIBRARIES 256
#define MAX_PATH_LENGTH 1024

typedef struct {
    char path[MAX_PATH_LENGTH];
    void* handle;
    char name[256];
    int is_static;  // 0 for .so, 1 for .a
} library_entry_t;

typedef struct {
    int trace_enabled;
    int detach_mode;
    char library_paths[MAX_LIBRARIES][MAX_PATH_LENGTH];
    int lib_path_count;
    library_entry_t loaded_libs[MAX_LIBRARIES];
    int loaded_count;
    pthread_mutex_t lib_mutex;
    char log_path[MAX_PATH_LENGTH];
} diram_cli_context_t;

static struct option long_options[] = {
    {"trace", no_argument, 0, 't'},
    {"trace-lib", required_argument, 0, 'T'},
    {"detach", no_argument, 0, 'd'},
    {"log-path", required_argument, 0, 'P'},
    {"help", no_argument, 0, 'h'},
    {"version", no_argument, 0, 'v'},
    {0, 0, 0, 0}
};

// Thread-safe library loading
int load_library_threadsafe(diram_cli_context_t* ctx, const char* libname, const char* libpath) {
    pthread_mutex_lock(&ctx->lib_mutex);
    
    if (ctx->loaded_count >= MAX_LIBRARIES) {
        fprintf(stderr, "Error: Maximum library limit reached\n");
        pthread_mutex_unlock(&ctx->lib_mutex);
        return -1;
    }
    
    library_entry_t* entry = &ctx->loaded_libs[ctx->loaded_count];
    char full_path[MAX_PATH_LENGTH * 2];
    
    // Construct full path
    if (libpath) {
        snprintf(full_path, sizeof(full_path), "%s/%s", libpath, libname);
    } else {
        strncpy(full_path, libname, sizeof(full_path) - 1);
    }
    
    // Determine if static or shared
    int is_static = (strstr(libname, ".a") != NULL);
    
    if (is_static) {
        // Static library handling - not recommended but supported
        fprintf(stderr, "Warning: Static library %s detected. Dynamic libraries (.so) recommended\n", libname);
        // For static libraries, we'd need to link at compile time
        // Here we just register it for metadata purposes
        strncpy(entry->path, full_path, MAX_PATH_LENGTH - 1);
        strncpy(entry->name, libname, 255);
        entry->is_static = 1;
        entry->handle = NULL;
    } else {
        // Dynamic library loading with RTLD_NOW for immediate symbol resolution
        void* handle = dlopen(full_path, RTLD_NOW | RTLD_GLOBAL);
        if (!handle) {
            fprintf(stderr, "Error loading library %s: %s\n", full_path, dlerror());
            pthread_mutex_unlock(&ctx->lib_mutex);
            return -1;
        }
        
        strncpy(entry->path, full_path, MAX_PATH_LENGTH - 1);
        strncpy(entry->name, libname, 255);
        entry->handle = handle;
        entry->is_static = 0;
        
        if (ctx->trace_enabled) {
            printf("[TRACE] Loaded library: %s (handle: %p)\n", full_path, handle);
        }
    }
    
    ctx->loaded_count++;
    pthread_mutex_unlock(&ctx->lib_mutex);
    return 0;
}

// Hook function for tracing library calls
void* hook_library_function(diram_cli_context_t* ctx, const char* libname, const char* funcname) {
    pthread_mutex_lock(&ctx->lib_mutex);
    
    for (int i = 0; i < ctx->loaded_count; i++) {
        if (strcmp(ctx->loaded_libs[i].name, libname) == 0) {
            if (ctx->loaded_libs[i].is_static) {
                fprintf(stderr, "Cannot dynamically hook static library %s\n", libname);
                pthread_mutex_unlock(&ctx->lib_mutex);
                return NULL;
            }
            
            void* symbol = dlsym(ctx->loaded_libs[i].handle, funcname);
            if (!symbol) {
                fprintf(stderr, "Symbol %s not found in %s: %s\n", 
                        funcname, libname, dlerror());
                pthread_mutex_unlock(&ctx->lib_mutex);
                return NULL;
            }
            
            if (ctx->trace_enabled) {
                printf("[TRACE] Hooked %s::%s at %p\n", libname, funcname, symbol);
            }
            
            pthread_mutex_unlock(&ctx->lib_mutex);
            return symbol;
        }
    }
    
    pthread_mutex_unlock(&ctx->lib_mutex);
    return NULL;
}

// Thread worker for concurrent library operations
void* library_worker(void* arg) {
    diram_cli_context_t* ctx = (diram_cli_context_t*)arg;
    
    // Example: Monitor loaded libraries for changes
    while (1) {
        pthread_mutex_lock(&ctx->lib_mutex);
        
        if (ctx->trace_enabled) {
            for (int i = 0; i < ctx->loaded_count; i++) {
                if (!ctx->loaded_libs[i].is_static && ctx->loaded_libs[i].handle) {
                    // Check library health
                    printf("[MONITOR] Library %s is active\n", ctx->loaded_libs[i].name);
                }
            }
        }
        
        pthread_mutex_unlock(&ctx->lib_mutex);
        sleep(5); // Check every 5 seconds
    }
    
    return NULL;
}

void print_usage(const char* progname) {
    printf("DIRAM CLI - Directed Instruction RAM with Dynamic Library Support\n\n");
    printf("Usage: %s [OPTIONS] [SCRIPT]\n\n", progname);
    printf("Options:\n");
    printf("  -t, --trace              Enable tracing mode\n");
    printf("  -T, --trace-lib PATH     Trace specific library\n");
    printf("  -L PATH                  Add library search path\n");
    printf("  -l LIBNAME              Load library (e.g., -l custom.so)\n");
    printf("  -d, --detach            Run in detached/daemon mode\n");
    printf("  -P, --log-path PATH     Set log file path\n");
    printf("  -h, --help              Show this help\n");
    printf("  -v, --version           Show version\n\n");
    printf("Examples:\n");
    printf("  %s --trace -L /usr/local/lib -l libsensor.so script.dr\n", progname);
    printf("  %s --trace-lib /opt/drone/libnavigation.so --detach\n", progname);
    printf("\nNote: .so (shared objects) recommended. .a (static) supported but requires recompilation.\n");
}

int main(int argc, char** argv) {
    diram_cli_context_t ctx = {0};
    pthread_mutex_init(&ctx.lib_mutex, NULL);
    strcpy(ctx.log_path, "./diram.log");
    
    int opt;
    int option_index = 0;
    
    // Parse both short and long options
    while ((opt = getopt_long(argc, argv, "tT:L:l:dP:hv", long_options, &option_index)) != -1) {
        switch (opt) {
            case 't':
                ctx.trace_enabled = 1;
                printf("[CONFIG] Trace mode enabled\n");
                break;
                
            case 'T': {
                // Trace specific library
                ctx.trace_enabled = 1;
                if (load_library_threadsafe(&ctx, optarg, NULL) == 0) {
                    printf("[TRACE] Monitoring library: %s\n", optarg);
                }
                break;
            }
            
            case 'L':
                // Add library path
                if (ctx.lib_path_count < MAX_LIBRARIES) {
                    strncpy(ctx.library_paths[ctx.lib_path_count], optarg, MAX_PATH_LENGTH - 1);
                    ctx.lib_path_count++;
                    printf("[CONFIG] Added library path: %s\n", optarg);
                }
                break;
                
            case 'l': {
                // Load library
                const char* search_path = ctx.lib_path_count > 0 ? 
                    ctx.library_paths[ctx.lib_path_count - 1] : NULL;
                    
                if (load_library_threadsafe(&ctx, optarg, search_path) == 0) {
                    printf("[LOAD] Successfully loaded library: %s\n", optarg);
                }
                break;
            }
            
            case 'd':
                ctx.detach_mode = 1;
                printf("[CONFIG] Detached mode enabled\n");
                break;
                
            case 'P':
                strncpy(ctx.log_path, optarg, MAX_PATH_LENGTH - 1);
                printf("[CONFIG] Log path: %s\n", ctx.log_path);
                break;
                
            case 'h':
                print_usage(argv[0]);
                return 0;
                
            case 'v':
                printf("DIRAM CLI v1.0.0 (OBINexus Project)\n");
                return 0;
                
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    // Start monitoring thread if in detach mode
    pthread_t monitor_thread;
    if (ctx.detach_mode) {
        if (pthread_create(&monitor_thread, NULL, library_worker, &ctx) != 0) {
            fprintf(stderr, "Failed to create monitor thread\n");
            return 1;
        }
        printf("[DAEMON] Started background monitoring thread\n");
    }
    
    // Process script file if provided
    if (optind < argc) {
        const char* script_file = argv[optind];
        printf("[EXEC] Processing script: %s\n", script_file);
        
        // Here you would integrate with your existing DIRAM parser
        // For now, we'll demonstrate library interaction
        
        // Example: If drone_monitor.dr references libnavigation.so
        if (strstr(script_file, "drone_monitor.dr")) {
            void* nav_func = hook_library_function(&ctx, "libnavigation.so", "calculate_route");
            if (nav_func && ctx.trace_enabled) {
                printf("[TRACE] Ready to monitor navigation calculations\n");
            }
        }
    }
    
    // Interactive mode or daemon mode
    if (ctx.detach_mode) {
        printf("[DAEMON] Running in background. PID: %d\n", getpid());
        printf("[DAEMON] Logs: %s\n", ctx.log_path);
        
        // Keep running
        while (1) {
            sleep(1);
        }
    } else if (optind >= argc) {
        // Interactive REPL mode
        printf("DIRAM Interactive Mode (type 'help' for commands, 'exit' to quit)\n");
        
        char buffer[1024];
        while (1) {
            printf("diram> ");
            if (!fgets(buffer, sizeof(buffer), stdin)) break;
            
            buffer[strcspn(buffer, "\n")] = 0; // Remove newline
            
            if (strcmp(buffer, "exit") == 0) break;
            if (strcmp(buffer, "help") == 0) {
                printf("Commands:\n");
                printf("  libs     - List loaded libraries\n");
                printf("  load LIB - Load a library\n");
                printf("  hook LIB FUNC - Hook a function\n");
                printf("  trace on/off - Toggle tracing\n");
                printf("  exit     - Exit\n");
                continue;
            }
            
            if (strcmp(buffer, "libs") == 0) {
                pthread_mutex_lock(&ctx.lib_mutex);
                printf("Loaded libraries (%d):\n", ctx.loaded_count);
                for (int i = 0; i < ctx.loaded_count; i++) {
                    printf("  [%d] %s (%s)\n", i, ctx.loaded_libs[i].name,
                           ctx.loaded_libs[i].is_static ? "static" : "dynamic");
                }
                pthread_mutex_unlock(&ctx.lib_mutex);
                continue;
            }
            
            // Process other commands...
        }
    }
    
    // Cleanup
    pthread_mutex_lock(&ctx.lib_mutex);
    for (int i = 0; i < ctx.loaded_count; i++) {
        if (!ctx.loaded_libs[i].is_static && ctx.loaded_libs[i].handle) {
            dlclose(ctx.loaded_libs[i].handle);
        }
    }
    pthread_mutex_unlock(&ctx.lib_mutex);
    
    pthread_mutex_destroy(&ctx.lib_mutex);
    
    return 0;
}
