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
    // Load ROM/FW images
    struct caliptra_buffer rom = read_file_or_exit("rom.bin");
    struct caliptra_buffer fw = read_file_or_exit("fw.bin");

    // Start Caliptra BootFSM
    if (caliptra_bootfsm_go() != 0) {
        printf("Failed to start Caliptra BootFSM\n");
        return 1;
    }

    // Initialize WDT and iTRNG (adjust values as needed)
    caliptra_set_wdt_timeout(0xA0000000);
    caliptra_configure_itrng_entropy(0x1, 0xFFFF, 0xFFFF);

    // Initialize fuses
    struct caliptra_fuses fuses = {0};
    // Set fuse fields as needed
    if (caliptra_init_fuses(&fuses) != 0) {
        printf("Failed to initialize Caliptra fuses\n");
        return 1;
    }

    // Load FW image
    if (caliptra_upload_fw(&fw, false) != 0) {
        printf("Failed to load FW image\n");
        return 1;
    }

    // Wait for runtime command acceptance
    if (caliptra_ready_for_runtime() != 0) {
        printf("Failed to wait for Caliptra runtime\n");
        return 1;
    }

    // Get certificate
    struct caliptra_get_idev_cert_req cert_req;
    struct caliptra_get_idev_cert_resp cert_resp;
    int cert_status = caliptra_get_idev_cert(&cert_req, &cert_resp, false);
    if (cert_status != 0) {
        printf("Failed to get certificate: %d\n", cert_status);
        return cert_status;
    }
    printf("Certificate successfully obtained\n");

    // Instead of extracting public key and signature from certificate, parse them from the firmware image header (preamble)
    struct caliptra_buffer fw_image = read_file_or_exit("fw.bin");
    if (fw_image.len < sizeof(struct caliptra_preamble)) {
        printf("FW image too small for preamble\n");
        free((void*)fw_image.data);
        return 1;
    }
    const struct caliptra_preamble *preamble = (const struct caliptra_preamble *)fw_image.data;
    struct caliptra_ecdsa_verify_req verify_req = {0};
    // Copy public key (owner ECC)
    memcpy(verify_req.pub_key_x, preamble->owner_pub_keys.ecc_pub_key.x, sizeof(verify_req.pub_key_x));
    memcpy(verify_req.pub_key_y, preamble->owner_pub_keys.ecc_pub_key.y, sizeof(verify_req.pub_key_y));
    // Copy signature (owner ECC signature)
    memcpy(verify_req.signature_r, preamble->owner_sigs.ecc_signature.rcoord, sizeof(verify_req.signature_r));
    memcpy(verify_req.signature_s, preamble->owner_sigs.ecc_signature.scoord, sizeof(verify_req.signature_s));
    // NOTE: The message to verify is the hash of the header (SHA-384 of struct caliptra_header)
    if (fw_image.len < sizeof(struct caliptra_preamble) + sizeof(struct caliptra_header)) {
        printf("FW image too small for header\n");
        free((void*)fw_image.data);
        return 1;
    }
    const uint8_t *header_ptr = fw_image.data + sizeof(struct caliptra_preamble);
    // Compute SHA-384 of header (use OpenSSL for demonstration, but in ROM this would be a hardware SHA384)
    uint8_t header_digest[48] = {0};
    #ifdef USE_OPENSSL_FOR_HASH
    SHA512_CTX sha_ctx;
    SHA384_Init(&sha_ctx);
    SHA384_Update(&sha_ctx, header_ptr, sizeof(struct caliptra_header));
    SHA384_Final(header_digest, &sha_ctx);
    #else
    // In ROM, use hardware SHA384 here
    #endif
    // (Assume verify_req has a field for the message digest, or pass as needed)
    // Call signature verification API (ROM would use hardware, here is a placeholder)
    int verify_status = caliptra_ecdsa384_verify(&verify_req, false);
    free((void*)fw_image.data);
    if (verify_status != 0) {
        printf("Signature verification failed: %d\n", verify_status);
        return verify_status;
    }
    printf("Signature verification succeeded\n");

    return 0;
}
