#ifndef DIRAM_HOTWIRE_H
#define DIRAM_HOTWIRE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// Forward declarations
typedef struct diram_hotwire_context diram_hotwire_context_t;
typedef struct diram_ast_node diram_ast_node_t;
typedef struct diram_ast_visitor diram_ast_visitor_t;

// ASM instruction types
typedef enum {
    ASM_MOV,
    ASM_MV = ASM_MOV,
    ASM_PUSH,
    ASM_POP,
    ASM_CALL,
    ASM_RET,
    ASM_JMP,
    ASM_JZ,
    ASM_JNZ,
    ASM_LEA,
    ASM_STORE,
    ASM_LOAD
} diram_asm_opcode_t;

// Hotwire configuration
typedef struct {
    bool enable_optimization;
    bool generate_debug_info;
    int optimization_level;
    struct {
        unsigned int memory_pages;
    } wasm_config;
} diram_hotwire_config_t;

// Hotwire context
struct diram_hotwire_context {
    FILE* output_file;
    diram_hotwire_config_t config;
    void* user_data;
    bool features[32];
    char feature_names[32][64];
};

// AST Node types
typedef enum {
    AST_NODE_ROOT,
    AST_NODE_ALLOCATION,
    AST_NODE_OPCODE,
    AST_NODE_OPERAND,
    AST_NODE_CONSTRAINT,
    AST_NODE_POLICY,
    AST_NODE_FEATURE_TOGGLE,
    AST_NODE_MEMORY_REGION,
    AST_NODE_BUILD_TARGET
} diram_ast_node_type_t;

// AST Node structure
struct diram_ast_node {
    diram_ast_node_type_t type;
    union {
        struct {
            size_t size;
            const char* tag;
            uint64_t address;
            char sha256_receipt[65];
        } allocation;
        struct {
            const char* name;
            uint8_t value;
        } opcode;
        struct {
            const char* name;
            float epsilon;
            uint32_t max_events;
        } constraint;
        struct {
            const char* name;
            const char* type;
            char** rules;
            size_t rule_count;
            bool enforced;
        } policy;
        struct {
            const char* name;
            bool enabled;
        } feature;
        struct {
            const char* name;
            uint64_t base_addr;
            size_t size;
            uint8_t protection_flags;
        } memory_region;
    } data;
    struct diram_ast_node* children[10];
    size_t child_count;
};

// Visitor pattern for AST traversal
struct diram_ast_visitor {
    void* (*visit_root)(diram_ast_visitor_t* self, diram_ast_node_t* node);
    void* (*visit_allocation)(diram_ast_visitor_t* self, diram_ast_node_t* node);
    void* (*visit_opcode)(diram_ast_visitor_t* self, diram_ast_node_t* node);
    void* (*visit_operand)(diram_ast_visitor_t* self, diram_ast_node_t* node);
    void* (*visit_constraint)(diram_ast_visitor_t* self, diram_ast_node_t* node);
    void* (*visit_policy)(diram_ast_visitor_t* self, diram_ast_node_t* node);
    void* (*visit_feature_toggle)(diram_ast_visitor_t* self, diram_ast_node_t* node);
    void* (*visit_memory_region)(diram_ast_visitor_t* self, diram_ast_node_t* node);
    void* (*visit_build_target)(diram_ast_visitor_t* self, diram_ast_node_t* node);
};

// Function declarations - Fixed to use variable arguments
void diram_hotwire_emit_asm_directive(diram_hotwire_context_t* context, 
                                      const char* format, ...);
void diram_hotwire_emit_asm_instruction(diram_hotwire_context_t* context,
                                        diram_asm_opcode_t opcode,
                                        const char* operand1,
                                        const char* operand2);
void diram_hotwire_emit_asm_label(diram_hotwire_context_t* context,
                                  const char* label);
void diram_hotwire_register_feature(diram_hotwire_context_t* context,
                                    const char* name, bool enabled);
bool diram_hotwire_check_feature(diram_hotwire_context_t* context,
                                 const char* name);
diram_ast_visitor_t* diram_hotwire_create_asm_visitor(diram_hotwire_context_t* context);
diram_ast_visitor_t* diram_hotwire_create_wasm_visitor(diram_hotwire_context_t* context);

#endif /* DIRAM_HOTWIRE_H */
