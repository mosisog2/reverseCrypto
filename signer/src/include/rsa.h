#pragma once
#include <stdint.h>

typedef enum { FAULT_OFF=0, FAULT_MODP=1, FAULT_MODQ=2 } FaultMode;

typedef struct {
    uint64_t N;     // RSA modulus: N = p * q
    uint64_t e;     // Public exponent (typically 65537)
    uint64_t d;     // Private exponent: d = e^-1 mod φ(N)
    uint64_t p;     // First prime factor of N
    uint64_t q;     // Second prime factor of N
    uint64_t dp;    // CRT parameter: dp = d mod (p-1)
    uint64_t dq;    // CRT parameter: dq = d mod (q-1)
    uint64_t qinv;  // CRT parameter: qinv = q^-1 mod p
} RsaCrtKey64;


// Key init: fill p,q,e,d and precompute dp=d mod (p-1), dq, qinv. 
int rsa64_init(RsaCrtKey64* k, uint64_t p, uint64_t q, uint64_t e);

// Full and CRT signatures: s = m^d mod N.  (m < N; no padding)
uint64_t rsa64_sign_full(uint64_t m, const RsaCrtKey64* k);
uint64_t rsa64_sign_crt (uint64_t m, const RsaCrtKey64* k, FaultMode mode);

// Verification helper
int rsa64_verify(uint64_t m, uint64_t s, const RsaCrtKey64* k);