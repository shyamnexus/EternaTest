# IR Control Test Sample
# Tests sysfs PWM interface for IR LED control

# Cross-compiler (set via environment or override)
CROSS_COMPILE ?= /opt/ivot/aarch64-ca53-linux-gnueabihf-8.4/bin/aarch64-ca53-linux-gnueabihf-

CC = $(CROSS_COMPILE)gcc
STRIP = $(CROSS_COMPILE)strip
CFLAGS = -Wall -O2

TARGETS = ir_test_sysfs

all: $(TARGETS)

ir_test_sysfs: ir_test_sysfs.c
	$(CC) $(CFLAGS) -o $@ $<
	$(STRIP) $@

install:
	@echo "ir_control_test install - standalone sample, nothing to install"

sample: all

sample_install: install

clean:
	rm -f $(TARGETS)

# Push to device via adb
push: ir_test_sysfs
	adb push ir_test_sysfs /tmp/
	adb shell chmod +x /tmp/ir_test_sysfs

# Quick test commands
test-led-on:
	adb shell /tmp/ir_test_sysfs led on 75

test-led-off:
	adb shell /tmp/ir_test_sysfs led off

test-all:
	adb shell /tmp/ir_test_sysfs test

.PHONY: all clean install sample sample_install push test-led-on test-led-off test-all

