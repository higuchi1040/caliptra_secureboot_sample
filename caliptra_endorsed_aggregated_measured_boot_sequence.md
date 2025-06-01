# Sequence Diagram for caliptra_endorsed_aggregated_measured_boot.c

```mermaid
sequenceDiagram
    participant App as User Application (main)
    participant FileIO as File I/O
    participant Caliptra as Caliptra API
    participant OpenSSL as OpenSSL

    App->>FileIO: read_file_or_exit("rom.bin")
    App->>FileIO: read_file_or_exit("fw.bin")
    App->>Caliptra: caliptra_bootfsm_go()
    App->>Caliptra: caliptra_set_wdt_timeout()
    App->>Caliptra: caliptra_configure_itrng_entropy()
    App->>Caliptra: caliptra_init_fuses(&fuses)
    App->>Caliptra: caliptra_upload_fw(&fw, false)
    App->>Caliptra: caliptra_ready_for_runtime()
    %% Aggregated Measured Boot
    App->>Caliptra: caliptra_stash_measurement(&stash_req, &stash_resp, false)
    Caliptra-->>App: stash_resp
    App->>Caliptra: caliptra_quote_pcrs(&quote_req, &quote_resp, false)
    Caliptra-->>App: quote_resp (digest, signature_r, signature_s, ...)
    %% (Optional) Attestation certificate retrieval omitted
    App->>OpenSSL: (Verify ECDSA384 signature using digest, signature_r, signature_s, public key)
    OpenSSL-->>App: Verification result
```

---

This sequence diagram shows the flow of:
- ROM/FW image loading
- Caliptra initialization, FW loading, and runtime wait
- Stashing measurements (measured boot)
- Aggregating and quoting PCRs
- (Optional) Attestation signature verification using OpenSSL
