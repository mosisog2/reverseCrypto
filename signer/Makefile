CFLAGS=-O2 -Wall -Wextra
INCLUDE=-Isrc/include

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

test: src/tests/test_signer
	./src/tests/test_signer

clean:
	rm -f src/*.o src/tests/test_signer main
