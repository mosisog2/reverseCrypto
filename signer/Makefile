CFLAGS=-O2 -Wall -Wextra
INCLUDE=-Isrc/include

ATTACK_BINS=attacks/double_fault attacks/timing_side_channel attacks/perf_benchmark

# Cross-compilation knobs for the RISC-V regression run under QEMU.
BUILD_DIR=build
RISCV_BUILD=$(BUILD_DIR)/riscv64
RISCV_CC ?= riscv64-linux-gnu-gcc
RISCV_QEMU ?= qemu-riscv64
RISCV_SYSROOT ?= /usr/riscv64-linux-gnu

all: test

src/mod.o: src/mod.c src/include/mod.h
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

src/rsa.o: src/rsa.c src/include/rsa.h src/include/mod.h
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

src/main.o: src/main.c src/include/rsa.h src/include/mod.h
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

src/tests/test_signer: src/tests/test_signer.c src/mod.o src/rsa.o
	$(CC) $(CFLAGS) $(INCLUDE) $^ -o $@

main: src/main.o src/mod.o src/rsa.o
	$(CC) $(CFLAGS) $^ -o $@

attacks/double_fault: attacks/double_fault.c src/mod.o src/rsa.o
	$(CC) $(CFLAGS) $(INCLUDE) $^ -o $@

attacks/timing_side_channel: attacks/timing_side_channel.c
	$(CC) $(CFLAGS) $(INCLUDE) $^ -o $@

attacks/perf_benchmark: attacks/perf_benchmark.c src/mod.o src/rsa.o
	$(CC) $(CFLAGS) $(INCLUDE) $^ -o $@

$(RISCV_BUILD)/test_signer: src/tests/test_signer.c src/rsa.c src/mod.c src/include/rsa.h src/include/mod.h
	mkdir -p $(dir $@)
	$(RISCV_CC) $(CFLAGS) $(INCLUDE) src/tests/test_signer.c src/rsa.c src/mod.c -o $@

riscv-test: $(RISCV_BUILD)/test_signer
	$(RISCV_QEMU) -L $(RISCV_SYSROOT) $<

attacks: $(ATTACK_BINS)
	@echo "Attack binaries ready: $(ATTACK_BINS)"

test: src/tests/test_signer
	./src/tests/test_signer

clean:
	rm -f src/*.o src/tests/test_signer main $(ATTACK_BINS)
	rm -rf $(BUILD_DIR)

.PHONY: all test clean attacks riscv-test
