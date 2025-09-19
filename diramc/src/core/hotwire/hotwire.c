#include "diram/core/hotwire/hotwire.h"
#include <stdarg.h>
#include <string.h>

// Emit assembly directive with variable arguments
void diram_hotwire_emit_asm_directive(diram_hotwire_context_t* context, 
                                      const char* format, ...) {
    if (!context || !context->output_file) return;
    
    va_list args;
    va_start(args, format);
    vfprintf(context->output_file, format, args);
    va_end(args);
    fprintf(context->output_file, "\n");
}

// Emit assembly instruction
void diram_hotwire_emit_asm_instruction(diram_hotwire_context_t* context,
                                        diram_asm_opcode_t opcode,
                                        const char* operand1,
                                        const char* operand2) {
    if (!context || !context->output_file) return;
    
    const char* mnemonic = NULL;
    switch (opcode) {
        case ASM_MOV:  mnemonic = "mov"; break;
        case ASM_PUSH: mnemonic = "push"; break;
        case ASM_POP:  mnemonic = "pop"; break;
        case ASM_CALL: mnemonic = "call"; break;
        case ASM_RET:  mnemonic = "ret"; break;
        case ASM_JMP:  mnemonic = "jmp"; break;
        case ASM_JZ:   mnemonic = "jz"; break;
        case ASM_JNZ:  mnemonic = "jnz"; break;
        case ASM_LEA:  mnemonic = "lea"; break;
        case ASM_STORE: mnemonic = "mov"; break;
        case ASM_LOAD:  mnemonic = "mov"; break;
        default: mnemonic = "nop"; break;
    }
    
    fprintf(context->output_file, "\t%s", mnemonic);
    if (operand1) {
        fprintf(context->output_file, " %s", operand1);
        if (operand2) {
            fprintf(context->output_file, ", %s", operand2);
        }
    }
    fprintf(context->output_file, "\n");
}

// Emit assembly label
void diram_hotwire_emit_asm_label(diram_hotwire_context_t* context,
                                  const char* label) {
    if (!context || !context->output_file) return;
    fprintf(context->output_file, "%s:\n", label);
}

// Register feature
void diram_hotwire_register_feature(diram_hotwire_context_t* context,
                                    const char* name, bool enabled) {
    if (!context || !name) return;
    
    for (int i = 0; i < 32; i++) {
        if (context->feature_names[i][0] == '\0') {
            strncpy(context->feature_names[i], name, 63);
            context->features[i] = enabled;
            break;
        }
    }
}

// Check feature
bool diram_hotwire_check_feature(diram_hotwire_context_t* context,
                                 const char* name) {
    if (!context || !name) return false;
    
    for (int i = 0; i < 32; i++) {
        if (strcmp(context->feature_names[i], name) == 0) {
            return context->features[i];
        }
    }
    return false;
}
