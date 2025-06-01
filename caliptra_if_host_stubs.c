// caliptra_if_host_stubs.c
// Host-side stubs for Caliptra hardware access functions
#include <stdint.h>
#include <stdio.h>
#include "caliptra_if.h"

int caliptra_write_u32(uint32_t address, uint32_t data) {
    // Stub: just print and return success
    printf("[STUB] caliptra_write_u32(0x%08x, 0x%08x)\n", address, data);
    return 0;
}

int caliptra_read_u32(uint32_t address, uint32_t *data) {
    // Stub: just print and return dummy data
    printf("[STUB] caliptra_read_u32(0x%08x)\n", address);
    if (data) *data = 0xDEADBEEF;
    return 0;
}

void caliptra_wait(void) {
    // Stub: just print
    printf("[STUB] caliptra_wait()\n");
}
