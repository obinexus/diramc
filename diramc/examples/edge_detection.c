// Example of detecting "negah" type communication errors
int detect_communication_issues(const char* input) {
    // Check for filler words, miscommunications
    const char* filler_words[] = {
        "umm", "uhh", "err", "like", "you know",
        "negah", "那个", "あの"  // Multi-language fillers
    };
    
    int filler_count = 0;
    for (int i = 0; i < sizeof(filler_words)/sizeof(char*); i++) {
        if (strstr(input, filler_words[i])) {
            filler_count++;
        }
    }
    
    // Map to positive state based on severity
    if (filler_count > 5) return STATE_COMM_HIGH;  // +12
    if (filler_count > 3) return STATE_COMM_MED;   // +11
    if (filler_count > 1) return STATE_COMM_LOW;   // +10
    
    return STATE_NORMAL;
}
