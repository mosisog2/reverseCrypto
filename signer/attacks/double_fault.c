#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "../src/include/rsa.h"
#include "../src/include/mod.h"

static uint64_t gcd64_local(uint64_t a, uint64_t b) {
    while (b) {
        uint64_t t = a % b;
        a = b;
        b = t;
    }
    return a;
}

static uint64_t double_fault_crt(uint64_t m, const RsaCrtKey64* k) {
    uint64_t sp = powmod64(m % k->p, k->dp, k->p);
    uint64_t sq = powmod64(m % k->q, k->dq, k->q);

    sp = (sp + 1) % k->p;  // corrupt branch modulo p
    sq = (sq + 1) % k->q;  // corrupt branch modulo q

    uint64_t sq_mod_p = sq % k->p;
    uint64_t diff = (sp >= sq_mod_p) ? (sp - sq_mod_p) : (sp + k->p - sq_mod_p);
    uint64_t h = mul_mod64(k->qinv, diff, k->p);

    __uint128_t acc = (__uint128_t)sq + (__uint128_t)h * k->q;
    return (uint64_t)(acc % k->N);
}

int main(void) {
    const uint64_t p = 61ULL;
    const uint64_t q = 53ULL;
    const uint64_t e = 17ULL;
    const uint64_t message = 37ULL;

    RsaCrtKey64 key;
    int init = rsa64_init(&key, p, q, e);
    if (init != 0) {
        fprintf(stderr, "rsa64_init failed: %d\n", init);
        return 1;
    }

    uint64_t healthy = rsa64_sign_crt(message, &key, FAULT_OFF);
    uint64_t single_fault = rsa64_sign_crt(message, &key, FAULT_MODP);
    uint64_t double_fault = double_fault_crt(message, &key);

    uint64_t diff_single = (healthy >= single_fault) ? (healthy - single_fault) : (single_fault - healthy);
    uint64_t diff_double = (healthy >= double_fault) ? (healthy - double_fault) : (double_fault - healthy);

    uint64_t factor_single = gcd64_local(diff_single, key.N);
    uint64_t factor_double = gcd64_local(diff_double, key.N);

    printf("Double Fault Experiment\n");
    printf("=======================\n\n");
    printf("Message                 : %" PRIu64 "\n", message);
    printf("Modulus N               : %" PRIu64 "\n", key.N);
    printf("Healthy signature       : %" PRIu64 "\n", healthy);
    printf("Single-fault signature  : %" PRIu64 "\n", single_fault);
    printf("Double-fault signature  : %" PRIu64 "\n\n", double_fault);

    printf("Single fault |Δ|        : %" PRIu64 "\n", diff_single);
    printf("gcd(N, |Δ|)             : %" PRIu64 "\n", factor_single);
    printf("Attack success?         : %s\n\n", (factor_single != 1 && factor_single != key.N) ? "YES" : "NO");

    printf("Double fault |Δ|        : %" PRIu64 "\n", diff_double);
    printf("gcd(N, |Δ|)             : %" PRIu64 "\n", factor_double);
    printf("Attack success?         : %s\n", (factor_double != 1 && factor_double != key.N) ? "YES" : "NO");

    return 0;
}
