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

    // Extract public key from certificate
    BIO *cert_bio = BIO_new_mem_buf(cert_resp.cert, cert_resp.cert_size);
    if (!cert_bio) {
        printf("Failed to create certificate BIO\n");
        return 1;
    }
    X509 *x509 = d2i_X509_bio(cert_bio, NULL);
    if (!x509) {
        printf("Failed to parse certificate\n");
        BIO_free(cert_bio);
        return 1;
    }
    EVP_PKEY *pkey = X509_get_pubkey(x509);
    if (!pkey) {
        printf("Failed to extract public key\n");
        X509_free(x509);
        BIO_free(cert_bio);
        return 1;
    }
    EC_KEY *ec_key = EVP_PKEY_get1_EC_KEY(pkey);
    if (!ec_key) {
        printf("Failed to extract EC public key\n");
        EVP_PKEY_free(pkey);
        X509_free(x509);
        BIO_free(cert_bio);
        return 1;
    }
    const EC_POINT *pub_point = EC_KEY_get0_public_key(ec_key);
    const EC_GROUP *group = EC_KEY_get0_group(ec_key);
    BIGNUM *x = BN_new();
    BIGNUM *y = BN_new();
    if (!EC_POINT_get_affine_coordinates_GFp(group, pub_point, x, y, NULL)) {
        printf("Failed to extract public key coordinates\n");
        BN_free(x); BN_free(y);
        EC_KEY_free(ec_key);
        EVP_PKEY_free(pkey);
        X509_free(x509);
        BIO_free(cert_bio);
        return 1;
    }
    struct caliptra_ecdsa_verify_req verify_req = {0};
    BN_bn2binpad(x, verify_req.pub_key_x, 48);
    BN_bn2binpad(y, verify_req.pub_key_y, 48);
    BN_free(x); BN_free(y);
    EC_KEY_free(ec_key);
    EVP_PKEY_free(pkey);
    X509_free(x509);
    BIO_free(cert_bio);
    // Signature cannot be directly obtained from the certificate, so set dummy values here
    memset(verify_req.signature_r, 0, sizeof(verify_req.signature_r));
    memset(verify_req.signature_s, 0, sizeof(verify_req.signature_s));
    // Message
    const char *message = "Test message";
    memcpy(verify_req.msg, message, strlen(message));
    verify_req.msg_len = strlen(message);

    int verify_status = caliptra_ecdsa384_verify(&verify_req, false);
    if (verify_status != 0) {
        printf("Signature verification failed: %d\n", verify_status);
        return verify_status;
    }
    printf("Signature verification succeeded\n");

    return 0;
}
