// diram_state_monitor.h - System state monitoring with graduated response

typedef enum {
    // NEGATIVE STATES: Errors/Failures
    STATE_PANIC_HIGH    = -12,  // Human intervention required NOW
    STATE_PANIC_MED     = -11,  // System critical, preparing crash dump
    STATE_PANIC_LOW     = -10,  // SOS initiated, whitebook generation
    
    STATE_CRITICAL_HIGH = -9,   // Critical failure imminent
    STATE_CRITICAL_MED  = -8,   // Critical systems degrading
    STATE_CRITICAL_LOW  = -7,   // Critical threshold reached
    
    STATE_DANGER_HIGH   = -6,   // Danger zone, auto-recovery failing
    STATE_DANGER_MED    = -5,   // Danger detected, attempting recovery
    STATE_DANGER_LOW    = -4,   // Danger threshold, monitoring closely
    
    STATE_ERROR_HIGH    = -3,   // HOTL auto-handling, high priority
    STATE_ERROR_MED     = -2,   // HOTL auto-handling, medium priority  
    STATE_ERROR_LOW     = -1,   // HOTL auto-handling, low priority
    
    STATE_NORMAL        = 0,    // No error, normal operation
    
    // POSITIVE STATES: Warnings/Success with issues
    STATE_WARN_LOW      = 1,    // HITL warning, low priority
    STATE_WARN_MED      = 2,    // HITL warning, medium priority
    STATE_WARN_HIGH     = 3,    // HITL warning, high priority
    
    STATE_CAUTION_LOW   = 4,    // Caution, performance degraded
    STATE_CAUTION_MED   = 5,    // Caution, resources constrained
    STATE_CAUTION_HIGH  = 6,    // Caution, approaching limits
    
    STATE_NOTICE_LOW    = 7,    // Notable event, monitoring
    STATE_NOTICE_MED    = 8,    // Notable pattern detected
    STATE_NOTICE_HIGH   = 9,    // Notable deviation from norm
    
    STATE_COMM_LOW      = 10,   // Communication issue, minor
    STATE_COMM_MED      = 11,   // Communication issue, moderate
    STATE_COMM_HIGH     = 12    // Communication issue, severe (like "negah")
} diram_state_t;

typedef struct {
    diram_state_t current_state;
    time_t timestamp;
    char* component;
    char* message;
    uint64_t error_code;
    
    // Lookahead prediction
    diram_state_t predicted_next_state;
    float confidence;
    
    // Recovery action
    void (*recovery_fn)(void*);
    void* recovery_context;
} state_monitor_t;
