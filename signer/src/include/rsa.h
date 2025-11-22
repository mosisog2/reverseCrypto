#pragma once
#include <stdint.h>

typedef enum { FAULT_OFF=0, FAULT_MODP=1, FAULT_MODQ=2 } FaultMode;

typedef struct {
    uint64_t N;      // RSA modulus: N = p * q
    uint64_t e;      // Public exponent (typically 65537)
    uint64_t d;      // Private exponent: d = e^-1 mod φ(N)
    uint64_t p;      // First prime factor of N
    uint64_t q;      // Second prime factor of N
    uint64_t phi;    // Euler totient φ(N) = (p-1)(q-1)
    uint64_t dp;     // CRT parameter: dp = d mod (p-1)
    uint64_t dq;     // CRT parameter: dq = d mod (q-1)
    uint64_t qinv;   // CRT parameter: qinv = q^-1 mod p
} RsaCrtKey64;

typedef struct {
    int enable_post_crt_verify;    // Re-run verification before returning signature
    int enable_double_crt;         // Recompute CRT and compare outputs
    int enable_exponent_blinding;  // Randomize private exponent per operation
    int enable_message_blinding;   // Blind message before exponentiation
} RsaDefenseConfig;


// Key init: fill p,q,e,d and precompute dp=d mod (p-1), dq, qinv. 
int rsa64_init(RsaCrtKey64* k, uint64_t p, uint64_t q, uint64_t e);

// Full and CRT signatures: s = m^d mod N.  (m < N; no padding)
uint64_t rsa64_sign_full(uint64_t m, const RsaCrtKey64* k);
uint64_t rsa64_sign_crt (uint64_t m, const RsaCrtKey64* k, FaultMode mode);

// Verification helper
int rsa64_verify(uint64_t m, uint64_t s, const RsaCrtKey64* k);

// Defense configuration helpers
void rsa_set_defense_config(const RsaDefenseConfig* cfg);
void rsa_get_defense_config(RsaDefenseConfig* cfg_out);
void rsa_seed_random(uint64_t seed);