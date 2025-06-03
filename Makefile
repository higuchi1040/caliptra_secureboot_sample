Q=@

CC=gcc
AR=ar

SAMPLES = caliptra_endorsed_aggregated_measured_boot caliptra_manifest_authorize_and_stash_sample certificate_and_signature_verification

LIBCALIPTRA_ROOT = ../../../
LIBCALIPTRA_INC  = ../../inc
LIBCALIPTRA_LIB  = ../../

INCLUDES += -I$(LIBCALIPTRA_INC) -I./
CFLAGS += $(INCLUDES) -Wall
LDFLAGS += -L$(LIBCALIPTRA_LIB) -Wl,-rpath=$(LIBCALIPTRA_LIB) -lcaliptra -lssl -lcrypto

.PHONY: all clean $(SAMPLES)

all: $(SAMPLES)

caliptra_endorsed_aggregated_measured_boot: caliptra_endorsed_aggregated_measured_boot.o caliptra_if_host_stubs.o
	@echo [LINK] $@
	$(Q)$(CC) -o $@ $^ $(LDFLAGS)

caliptra_manifest_authorize_and_stash_sample: caliptra_manifest_authorize_and_stash_sample.o caliptra_if_host_stubs.o
	@echo [LINK] $@
	$(Q)$(CC) -o $@ $^ $(LDFLAGS)

certificate_and_signature_verification: certificate_and_signature_verification.o caliptra_if_host_stubs.o
	@echo [LINK] $@
	$(Q)$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	@echo [CC] $< -> $@
	$(Q)$(CC) $(CFLAGS) -g -c $< -o $@

clean:
	@echo [CLEAN] *.o $(SAMPLES)
	$(Q)rm -f *.o $(SAMPLES)
