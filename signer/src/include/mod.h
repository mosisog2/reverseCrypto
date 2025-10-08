#pragma once
#include <stdint.h>

/* Extended GCD: return g, set *x,*y with ax+by=g */
uint64_t egcd64(uint64_t a, uint64_t b, int64_t* x, int64_t* y);

/* Modular inverse a^{-1} mod n (assume gcd(a,n)=1), return 0 if none */
uint64_t modinv64(uint64_t a, uint64_t n);

/* Square-and-multiply: computes a^e mod n */
uint64_t powmod64(uint64_t a, uint64_t e, uint64_t n);