#ifndef DIRAM_HELPERS_H
#define DIRAM_HELPERS_H

// OBINexus DIRAM Helper Functions
// Toolchain: riftlang.exe → .so.a → rift.exe → gosilang

#include "diram_phenomenological.h"

// Include all helper function declarations from diram_phenomenological.h
// This header serves as a convenience include for utils/*.c files

// Additional helper macros for OBINexus compliance
#define OBINEXUS_STREAM_A_MAGIC 0xDEADBEEF00000000ULL
#define OBINEXUS_STREAM_B_MAGIC 0x5645524900000000ULL
#define OBINEXUS_STREAM_C_MAGIC 0x474F564500000000ULL

// Verify OBINexus stream alignment
static inline int obinexus_verify_alignment(triple_stream_result_t* result) {
    return (result->stream_a & 0xFFFFFFFF00000000ULL) == OBINEXUS_STREAM_A_MAGIC &&
           (result->stream_b & 0xFFFFFFFF00000000ULL) == OBINEXUS_STREAM_B_MAGIC &&
           (result->stream_c & 0xFFFFFFFF00000000ULL) == OBINEXUS_STREAM_C_MAGIC;
}

#endif // DIRAM_HELPERS_H
