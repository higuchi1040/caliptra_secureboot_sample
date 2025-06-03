# Sequence Diagram for certificate_and_signature_verification.c (Revised)

This document presents a sequence diagram based on the latest API design and implementation of `certificate_and_signature_verification.c`.

---

## Sequence Diagram

```mermaid
sequenceDiagram
    participant UserApp as User C Application
    participant libcaliptra as libcaliptra
    participant CaliptraHW as Caliptra Hardware (Stub)

    UserApp->>libcaliptra: caliptra_bootfsm_go()
    libcaliptra->>CaliptraHW: caliptra_write_u32 (stub)
    CaliptraHW-->>libcaliptra: 0 (success)

    UserApp->>libcaliptra: caliptra_set_wdt_timeout()
    libcaliptra->>CaliptraHW: caliptra_write_u32 (stub)
    CaliptraHW-->>libcaliptra: 0

    UserApp->>libcaliptra: caliptra_configure_itrng_entropy()
    libcaliptra->>CaliptraHW: caliptra_write_u32 (stub)
    CaliptraHW-->>libcaliptra: 0

    UserApp->>libcaliptra: caliptra_init_fuses()
    libcaliptra->>CaliptraHW: caliptra_write_u32 (stub)
    CaliptraHW-->>libcaliptra: 0

    UserApp->>libcaliptra: caliptra_upload_fw()
    libcaliptra->>CaliptraHW: caliptra_write_u32 (stub)
    CaliptraHW-->>libcaliptra: 0

    UserApp->>libcaliptra: caliptra_ready_for_runtime()
    libcaliptra->>CaliptraHW: caliptra_read_u32 (stub)
    CaliptraHW-->>libcaliptra: 0

    UserApp->>libcaliptra: caliptra_get_idev_cert()
    libcaliptra->>CaliptraHW: caliptra_write_u32/call mailbox (stub)
    CaliptraHW-->>libcaliptra: 0

    UserApp->>libcaliptra: caliptra_ecdsa384_verify()
    libcaliptra->>CaliptraHW: caliptra_write_u32/call mailbox (stub)
    CaliptraHW-->>libcaliptra: 0

    UserApp-->>UserApp: Display result
```

---

## Key Points
- Direct use of `caliptra_mailbox_req`/`resp` is removed; all calls use the dedicated libcaliptra API functions.
- The signature verification API (`caliptra_ecdsa384_verify`) is called with an empty request, as the struct does not contain a message field.
- All hardware access is routed through stubs (`caliptra_write_u32`/`caliptra_read_u32`).

---

This diagram reflects the latest revision of `certificate_and_signature_verification.c`.
