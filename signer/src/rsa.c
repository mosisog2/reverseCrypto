#include <stdint.h>
#include <stdio.h>
#include "./include/rsa.h"
#include "./include/mod.h"

// Key init: fill p,q,e,d and precompute dp=d mod (p-1), dq, qinv.
int rsa64_init(RsaCrtKey64* k, uint64_t p, uint64_t q, uint64_t e) {
    // Store basic parameters
    k->p = p;
    k->q = q;
    k->e = e;
    k->N = p * q;
    
    // Calculate Euler's totient: o(N) = (p-1)(q-1)
    uint64_t phi_n = (p - 1) * (q - 1);
    
    // Calculate private exponent: d = e^-1 mod o(N)
    k->d = modinv64(e, phi_n);
    if (k->d == 0) return -1; // e and o(N) are not coprime
    
    // Precompute CRT parameters
    k->dp = k->d % (p - 1);    // dp = d mod (p-1)
    k->dq = k->d % (q - 1);    // dq = d mod (q-1)
    k->qinv = modinv64(q, p); // qinv = q^-1 mod p
    if (k->qinv == 0) return -1; // q and p are not coprime
    
    return 0; 
}

// Full and CRT signatures: s = m^d mod N.  (m < N; no padding)
uint64_t rsa64_sign_full(uint64_t m, const RsaCrtKey64* k) {
    return powmod64(m, k->d, k->N);
}

// Faulty CRT signature: s = m^d mod N, but with a fault injected
uint64_t rsa64_sign_crt(uint64_t m, const RsaCrtKey64* k, FaultMode mode) {
    // Exponentiate modulo each prime using reduced base
    uint64_t s_p = powmod64(m % k->p, k->dp, k->p);
    uint64_t s_q = powmod64(m % k->q, k->dq, k->q);

    // Optional fault injection
    if (mode == FAULT_MODP)      s_p = (s_p + 1) % k->p;
    else if (mode == FAULT_MODQ) s_q = (s_q + 1) % k->q;

    // t = (s_p - s_q) mod p  (branchless style)
    uint64_t t = (s_p >= s_q) ? (s_p - s_q) : (s_p + k->p - s_q);

    // h = qinv * t mod p  (use 128-bit mul)
    uint64_t h = mul_mod64(k->qinv, t, k->p);

    // s = s_q + q*h  (128-bit to avoid overflow), then reduce mod N
    __uint128_t S = (__uint128_t)k->q * h + s_q;
    uint64_t s = (uint64_t)(S % k->N);  // final reduction

    return s;
}

// Verification helper
int rsa64_verify(uint64_t m, uint64_t s, const RsaCrtKey64* k) {
    return (powmod64(s, k->e, k->N) == m);
}