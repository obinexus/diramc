// diram_cli_monitor.c - CLI integration for state monitoring

int cmd_monitor(int argc, char** argv) {
    state_monitor_t* monitor = init_state_monitor();
    
    // Parse monitoring level
    int threshold = 0;
    if (argc > 0) {
        threshold = atoi(argv[0]);
    }
    
    printf("DIRAM State Monitor - Threshold: %d\n", threshold);
    printf("State Scale: -12 (PANIC) to +12 (COMM_ISSUE)\n");
    printf("Press Ctrl+C to stop monitoring\n\n");
    
    while (1) {
        // Check system state
        monitor->current_state = assess_system_state();
        
        // Predict next state using lookahead
        monitor->predicted_next_state = predict_next_state(monitor);
        monitor->confidence = calculate_confidence(monitor);
        
        // Display state with color coding
        const char* color = get_state_color(monitor->current_state);
        printf("\r[%s] State: %2d | Component: %-20s | Prediction: %2d (%.1f%%) ",
               get_timestamp(),
               monitor->current_state,
               monitor->component,
               monitor->predicted_next_state,
               monitor->confidence * 100);
        
        // Take action based on state
        if (monitor->current_state < threshold) {
            handle_state_transition(monitor);
        }
        
        // Emergency handling
        if (monitor->current_state <= STATE_PANIC_LOW) {
            printf("\n!!! PANIC STATE DETECTED !!!\n");
            generate_whitebook(monitor);
            
            if (monitor->current_state == STATE_PANIC_HIGH) {
                // Kill process and dump core
                abort();  // This will generate core dump
            }
        }
        
        usleep(100000);  // 100ms refresh
    }
    
    return 0;
}
