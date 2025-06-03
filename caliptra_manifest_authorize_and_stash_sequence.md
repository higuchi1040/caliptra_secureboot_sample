# Caliptra Manifest Authorize and Stash Sample - Sequence Diagram

This document describes the sequence of operations performed by the sample code `caliptra_manifest_authorize_and_stash_sample.c` for Caliptra 1.2 measured boot/secure boot flows.

---

## Sequence Diagram

```mermaid
sequenceDiagram
    participant SoC_App as SoC Application
    participant Caliptra

    SoC_App->>Caliptra: caliptra_init_fuses()
    SoC_App->>Caliptra: caliptra_bootfsm_go()
    SoC_App->>Caliptra: caliptra_set_wdt_timeout()
    SoC_App->>Caliptra: caliptra_configure_itrng_entropy()
    SoC_App->>SoC_App: Read fw.bin
    SoC_App->>Caliptra: caliptra_upload_fw(fw.bin)
    SoC_App->>Caliptra: caliptra_ready_for_runtime()

    SoC_App->>SoC_App: Read manifest.bin
    SoC_App->>Caliptra: caliptra_mailbox_execute(SET_AUTH_MANIFEST, manifest.bin)
    Caliptra-->>SoC_App: Manifest sent and accepted.

    SoC_App->>SoC_App: Read image.bin
    SoC_App->>Caliptra: caliptra_mailbox_execute(AUTHORIZE_AND_STASH, image.bin)
    Caliptra-->>SoC_App: Image authorized and measurement stashed.

    Note over SoC_App,Caliptra: (Optional) Certificate retrieval or verification
```

---

## Description

- The SoC application initializes Caliptra, loads the firmware, and waits for runtime readiness.
- The application reads the manifest and sends it to Caliptra using the SET_AUTH_MANIFEST command (via `caliptra_mailbox_execute`).
- After the manifest is accepted, the application reads the firmware image and sends it to Caliptra using the AUTHORIZE_AND_STASH command (via `caliptra_mailbox_execute`).
- Caliptra verifies the image according to the manifest and stashes the measurement.
- Optionally, the application can retrieve certificates or perform further verification as needed.

---

For details, see the sample code: `caliptra_manifest_authorize_and_stash_sample.c`.
