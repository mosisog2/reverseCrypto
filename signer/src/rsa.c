#include <stdint.h>
#include <stdio.h>
#include "./include/rsa.h"
#include "./include/mod.h"

static RsaDefenseConfig g_defense = {0};
static uint64_t g_rand_state = 0x9E3779B97F4A7C15ULL;

static uint64_t next_rand64(void) {
    uint64_t x = g_rand_state;
    x ^= x << 7;
    x ^= x >> 9;
    x ^= x << 8;
    g_rand_state = x ? x : 0xA0761D6478BD642FULL; // fallback to non-zero seed
    return g_rand_state;
}

static uint64_t gcd64(uint64_t a, uint64_t b) {
    while (b) {
        uint64_t t = a % b;
        a = b;
        b = t;
    }
    return a;
}

static uint64_t crt_core(uint64_t m, const RsaCrtKey64* k, uint64_t dp, uint64_t dq, FaultMode mode) {
    uint64_t s_p = powmod64(m % k->p, dp, k->p);
    uint64_t s_q = powmod64(m % k->q, dq, k->q);

    if (mode == FAULT_MODP) {
        s_p = (s_p + 1) % k->p;
    } else if (mode == FAULT_MODQ) {
        s_q = (s_q + 1) % k->q;
    }

    uint64_t sq_mod_p = s_q % k->p;
    uint64_t diff = (s_p >= sq_mod_p) ? (s_p - sq_mod_p) : (s_p + k->p - sq_mod_p);
    uint64_t h = mul_mod64(k->qinv, diff, k->p);

    __uint128_t result = (__uint128_t)s_q + (__uint128_t)h * k->q;
    return (uint64_t)(result % k->N);
}

void rsa_seed_random(uint64_t seed) {
    if (seed == 0) {
        g_rand_state = 0x9E3779B97F4A7C15ULL;
    } else {
        g_rand_state = seed;
    }
}

void rsa_set_defense_config(const RsaDefenseConfig* cfg) {
    if (cfg) {
        g_defense = *cfg;
    } else {
        g_defense.enable_post_crt_verify = 0;
        g_defense.enable_double_crt = 0;
        g_defense.enable_exponent_blinding = 0;
        g_defense.enable_message_blinding = 0;
    }
}

void rsa_get_defense_config(RsaDefenseConfig* cfg_out) {
    if (cfg_out) {
        *cfg_out = g_defense;
    }
}

// Key init: fill p,q,e,d and precompute dp=d mod (p-1), dq, qinv.
int rsa64_init(RsaCrtKey64* k, uint64_t p, uint64_t q, uint64_t e) {
    // Store basic parameters
    k->p = p;
    k->q = q;
    k->e = e;
    k->N = p * q;
    
    // Calculate Euler's totient: o(N) = (p-1)(q-1)
    uint64_t phi_n = (p - 1) * (q - 1);
    k->phi = phi_n;
    
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
    return powmod64(m % k->N, k->d, k->N);
}

// Faulty CRT signature: s = m^d mod N, but with a fault injected
uint64_t rsa64_sign_crt(uint64_t m, const RsaCrtKey64* k, FaultMode mode) {
    uint64_t base_m = m % k->N;

    // Edge case: if m is not coprime to N, fall back to full method
    if (gcd64(base_m, k->N) != 1) {
        return powmod64(base_m, k->d, k->N);
    }

    uint64_t work_m = base_m;
    uint64_t unblind = 1;

    if (g_defense.enable_message_blinding) {
        uint64_t r;
        do {
            r = next_rand64() % k->N;
            if (r < 2) r += 2; // avoid trivial values
        } while (gcd64(r, k->N) != 1);

        uint64_t r_to_e = powmod64(r % k->N, k->e, k->N);
        work_m = mul_mod64(work_m, r_to_e, k->N);
        unblind = modinv64(r % k->N, k->N);
    }

    uint64_t dp_used = k->dp;
    uint64_t dq_used = k->dq;
    if (g_defense.enable_exponent_blinding) {
        uint64_t blind_factor = (next_rand64() & 0xFFFFULL) + 1ULL; // small random multiplier
        __uint128_t d_ext = (__uint128_t)k->d + (__uint128_t)blind_factor * k->phi;
        dp_used = (uint64_t)(d_ext % (k->p - 1));
        dq_used = (uint64_t)(d_ext % (k->q - 1));
    }

    uint64_t signature = crt_core(work_m, k, dp_used, dq_used, mode);

    if (g_defense.enable_double_crt) {
        uint64_t check = crt_core(work_m, k, dp_used, dq_used, FAULT_OFF);
        if (check != signature) {
            signature = check;
        }
    }

    if (g_defense.enable_message_blinding) {
        signature = mul_mod64(signature, unblind, k->N);
    }

    if (g_defense.enable_post_crt_verify) {
        if (powmod64(signature, k->e, k->N) != base_m) {
            signature = powmod64(base_m, k->d, k->N);
        }
    }

    return signature;
}

// Verification helper
int rsa64_verify(uint64_t m, uint64_t s, const RsaCrtKey64* k) {
    return (powmod64(s % k->N, k->e, k->N) == (m % k->N));
}