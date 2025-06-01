Q=@

CC=gcc
AR=ar

TARGET=caliptra_endorsed_aggregated_measured_boot
SOURCE = caliptra_endorsed_aggregated_measured_boot.c caliptra_manifest_authorize_and_stash_sample.c certificate_and_signature_verification.c caliptra_if_host_stubs.c

LIBCALIPTRA_ROOT = ../../../
LIBCALIPTRA_INC  = ../../inc
LIBCALIPTRA_LIB  = ../../

OBJS := $(patsubst %.c,%.o, $(filter %.c,$(SOURCE)))

# INCLUDES
INCLUDES += -I$(LIBCALIPTRA_INC) -I./

CFLAGS += $(INCLUDES) -Wall
LDFLAGS += -L$(LIBCALIPTRA_LIB) -Wl,-rpath=$(LIBCALIPTRA_LIB) -lcaliptra -lssl -lcrypto

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo [LINK] $@
	$(Q)$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	@echo [CC] $< -> $@
	$(Q)$(CC) $(CFLAGS) -g -c $< -o $@

clean:
	@echo [CLEAN] $(OBJS) $(TARGET)
	$(Q)rm -f $(OBJS) $(TARGET)
