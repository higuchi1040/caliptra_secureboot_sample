# Caliptra Secure Boot Sample

This directory contains sample C code for Caliptra secure boot and cryptographic API usage. Each sample can be built and run individually.

## Build Instructions

To build all samples:

```
make
```

To build a specific sample (for example, certificate_and_signature_verification):

```
make certificate_and_signature_verification
```

Each sample will produce a separate executable in this directory.

## Run

To run a sample, execute the corresponding binary. For example:

```
./caliptra_endorsed_aggregated_measured_boot
./caliptra_manifest_authorize_and_stash_sample
./certificate_and_signature_verification
```

> Note: `certificate_and_signature_verification` requires `fw.bin` (and optionally `rom.bin`) in the same directory.

---

All code and comments are in English. Host stubs are used for hardware-dependent functions. See each sample's corresponding `*_sequence.md` for a sequence diagram and detailed flow.

These are sample codes for caliptra.
Just coding by Github copilot.
