#include <stdio.h>
#include <dlfcn.h>

int main() {
    printf("Loading libpolycall...\n");
    void* lib = dlopen("lib/libpolycall.so", RTLD_LAZY);
    if (!lib) {
        printf("Error: %s\n", dlerror());
        return 1;
    }
    printf("LibPolyCall loaded successfully\n");
    
    // Try to call a function if it exists
    void* (*polycall_init)(void) = dlsym(lib, "polycall_init");
    if (polycall_init) {
        printf("Calling polycall_init...\n");
        polycall_init();
    }
    
    dlclose(lib);
    return 0;
}
