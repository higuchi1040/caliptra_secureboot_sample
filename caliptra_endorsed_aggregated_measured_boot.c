// caliptra_endorsed_aggregated_measured_boot.c
// Sample: Caliptra-Endorsed Aggregated Measured Boot
// This code demonstrates a typical flow for measured boot aggregation and attestation using Caliptra APIs.
// Assumes libcaliptra and OpenSSL are available and properly set up.

#include "caliptra_api.h"
#include "caliptra_image.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/bn.h>

// Utility to read a file into a caliptra_buffer
static struct caliptra_buffer read_file_or_exit(const char* path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        printf("Cannot find file %s\n", path);
        exit(-1);
    }
    struct caliptra_buffer buffer = {0};
    fseek(fp, 0L, SEEK_END);
    buffer.len = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    buffer.data = malloc(buffer.len);
    if (!buffer.data) {
        printf("Cannot allocate memory for buffer->data\n");
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
    // SoC起動コード: libcaliptraのAPIでCaliptraを初期化
    struct caliptra_fuses fuses = {0};
    // 必要に応じてfuse値をセット
    if (caliptra_init_fuses(&fuses) != 0) {
        printf("Failed to initialize Caliptra fuses\n");
        return 1;
    }

    // Caliptra BootFSM開始
    if (caliptra_bootfsm_go() != 0) {
        printf("Failed to start Caliptra BootFSM\n");
        return 1;
    }
    caliptra_set_wdt_timeout(0xA0000000);
    caliptra_configure_itrng_entropy(0x1, 0xFFFF, 0xFFFF);

    // ファームウェアイメージのロード
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

    // --- Aggregated Measured Boot ---
    // 1. Stash measurements (simulate boot stage measurements)
    struct caliptra_stash_measurement_req stash_req = {0};
    struct caliptra_stash_measurement_resp stash_resp = {0};
    // Fill stash_req.measurement, stash_req.metadata, etc. as needed
    if (caliptra_stash_measurement(&stash_req, &stash_resp, false) != 0) {
        printf("Failed to stash measurement\n");
        return 1;
    }
    printf("Measurement stashed\n");

    // 2. Quote PCRs (aggregate all measurements)
    struct caliptra_quote_pcrs_req quote_req = {0};
    struct caliptra_quote_pcrs_resp quote_resp = {0};
    // Optionally set quote_req.nonce
    if (caliptra_quote_pcrs(&quote_req, &quote_resp, false) != 0) {
        printf("Failed to quote PCRs\n");
        return 1;
    }
    printf("PCRs quoted. Digest: ");
    for (int i = 0; i < 48; ++i) printf("%02x", quote_resp.digest[i]);
    printf("\n");

    // 3. Verify the aggregated signature (省略)
    printf("Aggregated measured boot attestation complete.\n");
    return 0;
}
