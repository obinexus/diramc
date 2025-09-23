// diram_state_response.c - Graduated response implementation

typedef struct {
    diram_state_t state;
    const char* severity;
    bool auto_recovery;
    bool human_required;
    const char* action;
} state_response_t;

state_response_t response_matrix[] = {
    // PANIC STATES (-12 to -10): Kill process, need human
    {STATE_PANIC_HIGH,    "PANIC",    false, true,  "KILL_PROCESS_DUMP_CORE"},
    {STATE_PANIC_MED,     "PANIC",    false, true,  "GENERATE_WHITEBOOK"},
    {STATE_PANIC_LOW,     "PANIC",    false, true,  "SEND_SOS_ALERT"},
    
    // CRITICAL STATES (-9 to -7): System critical
    {STATE_CRITICAL_HIGH, "CRITICAL", false, false, "ISOLATE_COMPONENT"},
    {STATE_CRITICAL_MED,  "CRITICAL", true,  false, "ATTEMPT_RECOVERY"},
    {STATE_CRITICAL_LOW,  "CRITICAL", true,  false, "MONITOR_CLOSELY"},
    
    // DANGER STATES (-6 to -4): Danger zone
    {STATE_DANGER_HIGH,   "DANGER",   true,  false, "AUTO_ROLLBACK"},
    {STATE_DANGER_MED,    "DANGER",   true,  false, "REDUCE_LOAD"},
    {STATE_DANGER_LOW,    "DANGER",   true,  false, "INCREASE_MONITORING"},
    
    // ERROR STATES (-3 to -1): HOTL auto-handling
    {STATE_ERROR_HIGH,    "ERROR",    true,  false, "HOTL_AUTO_FIX"},
    {STATE_ERROR_MED,     "ERROR",    true,  false, "HOTL_RETRY"},
    {STATE_ERROR_LOW,     "ERROR",    true,  false, "HOTL_LOG_CONTINUE"},
    
    // NORMAL STATE (0)
    {STATE_NORMAL,        "OK",       false, false, "CONTINUE_NORMAL"},
    
    // WARNING STATES (1 to 3): HITL warnings
    {STATE_WARN_LOW,      "WARNING",  false, false, "HITL_NOTIFY"},
    {STATE_WARN_MED,      "WARNING",  false, false, "HITL_REVIEW"},
    {STATE_WARN_HIGH,     "WARNING",  false, false, "HITL_DECISION"},
    
    // POSITIVE ANOMALIES (4 to 12)
    {STATE_COMM_HIGH,     "COMM_ERR", false, false, "CLARIFY_INTENT"}
};
