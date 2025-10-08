CFLAGS=-O2 -Wall -Wextra
INCLUDE=-Icsrc/include

all: test

csrc/mod.o: csrc/mod.c csrc/include/mod.h
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

csrc/rsa.o: csrc/rsa.c csrc/include/rsa.h csrc/include/mod.h
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

tests/test_signer: tests/test_signer.c csrc/mod.o csrc/rsa.o
	$(CC) $(CFLAGS) $(INCLUDE) $^ -o $@

test: tests/test_signer
	./tests/test_signer

clean:
	rm -f csrc/*.o tests/test_signer
