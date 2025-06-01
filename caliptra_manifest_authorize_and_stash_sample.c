#include "caliptra_api.h"
#include "caliptra_image.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>
#include <openssl/bn.h>

// File read utility
static struct caliptra_buffer read_file_or_exit(const char* path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        printf("Cannot find file %s \n", path);
        exit(-1);
    }
    struct caliptra_buffer buffer = {0};
    fseek(fp, 0L, SEEK_END);
    buffer.len = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    buffer.data = malloc(buffer.len);
    if (!buffer.data) {
        printf("Cannot allocate memory for buffer->data \n");
        exit(-1);
    }
    size_t bytes_read = fread((char *)buffer.data, 1, buffer.len, fp);
    if (bytes_read != buffer.len) {
        printf("Bytes read (%zu) does not match file size (%zu)\n", bytes_read, buffer.len);
        free((void*)buffer.data);
        exit(-1);
    }
    fclose(fp);
    return buffer;
}

int main() {
    // SoC boot code: Initialize Caliptra using only libcaliptra APIs
    struct caliptra_fuses fuses = {0};
    // Set fuse values as needed
    if (caliptra_init_fuses(&fuses) != 0) {
        printf("Failed to initialize Caliptra fuses\n");
        return 1;
    }

    // Start Caliptra BootFSM
    if (caliptra_bootfsm_go() != 0) {
        printf("Failed to start Caliptra BootFSM\n");
        return 1;
    }
    caliptra_set_wdt_timeout(0xA0000000);
    caliptra_configure_itrng_entropy(0x1, 0xFFFF, 0xFFFF);

    // Load firmware image
    struct caliptra_buffer fw = read_file_or_exit("fw.bin");
    if (caliptra_upload_fw(&fw, false) != 0) {
        printf("Failed to load FW image\n");
        free((void*)fw.data);
        return 1;
    }
    free((void*)fw.data);

    if (caliptra_ready_for_runtime() != 0) {
        printf("Failed to wait for Caliptra runtime\n");
        return 1;
    }

    // --- Send SET_AUTH_MANIFEST command ---
    struct caliptra_buffer manifest = read_file_or_exit("manifest.bin");
    struct caliptra_buffer manifest_resp = {0};
    int status = caliptra_mailbox_execute(0x46534D41, &manifest, &manifest_resp, false); // 'AMSF'
    free((void*)manifest.data);
    if (status != 0) {
        printf("SET_AUTH_MANIFEST command failed: %d\n", status);
        return status;
    }
    printf("Manifest sent and accepted.\n");

    // --- Send AUTHORIZE_AND_STASH command ---
    struct caliptra_buffer image = read_file_or_exit("image.bin");
    struct caliptra_buffer image_resp = {0};
    status = caliptra_mailbox_execute(0x48535441, &image, &image_resp, false); // 'ATSH'
    free((void*)image.data);
    if (status != 0) {
        printf("AUTHORIZE_AND_STASH command failed: %d\n", status);
        return status;
    }
    printf("Image authorized and measurement stashed.\n");

    // Add certificate retrieval or verification as needed

    return 0;
}
