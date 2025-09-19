#ifndef DIRAM_CONFIG_H
#define DIRAM_CONFIG_H

#include <stddef.h>

typedef struct {
    char config_file[256];
    int detach_mode;
    int trace_enabled;
    int repl_mode;
    size_t memory_limit;
    char memory_space[64];
    char log_dir[256];
    int verbose;
} diram_config_t;

#endif
