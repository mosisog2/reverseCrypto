#include <assert.h>
#include <stdio.h>
#include "../include/rsa.h"

// Pick two 31-bit primes for toy testing (or generate them).
static const uint64_t P = 11ULL; // example prime ≥ 2^31
static const uint64_t Q = 17ULL; // example prime ≥ 2^31
static const uint64_t E = 3ULL;

int main(void) {
    RsaCrtKey64 k;
    printf("Initializing RSA key...\n");
    int init_result = rsa64_init(&k, P, Q, E);
    printf("Init result: %d\n", init_result);
    printf("N=%lu, d=%lu, dp=%lu, dq=%lu, qinv=%lu\n", 
           k.N, k.d, k.dp, k.dq, k.qinv);
    assert(init_result == 0);

    // Test with a single message first
    uint64_t test_m = 42;
    uint64_t s1 = rsa64_sign_full(test_m, &k);
    uint64_t s2 = rsa64_sign_crt(test_m, &k, FAULT_OFF);
    printf("Message: %lu\n", test_m);
    printf("Full signature: %lu\n", s1);
    printf("CRT signature:  %lu\n", s2);
    printf("Match: %s\n", (s1 == s2) ? "YES" : "NO");
    
    // Also test verification of both
    int v1 = rsa64_verify(test_m, s1, &k);
    int v2 = rsa64_verify(test_m, s2, &k);
    printf("Full verify: %d, CRT verify: %d\n", v1, v2);
    
    if (s1 != s2) {
        printf("ERROR: FAULT_OFF signatures don't match!\n");
        return 1;
    }

    // 1) full == crt for many m
    for (uint64_t m=2; m<1000; ++m) {
        uint64_t s1 = rsa64_sign_full(m, &k);
        uint64_t s2 = rsa64_sign_crt (m, &k, FAULT_OFF);
        assert(s1 == s2);
        assert(rsa64_verify(m, s1, &k) == 1);
    }

    // 2) Bellcore: faulty pair factors N
    uint64_t m = 1234567;
    uint64_t s_ok  = rsa64_sign_crt(m, &k, FAULT_OFF);
    uint64_t s_bad = rsa64_sign_crt(m, &k, FAULT_MODP);
    uint64_t diff  = (s_ok >= s_bad) ? (s_ok - s_bad) : (k.N - (s_bad - s_ok));
    // naive gcd
    uint64_t a = k.N, b = diff; 
    while (b) { uint64_t t=a%b; a=b; b=t; }
    uint64_t g = a;
    assert(g == k.p || g == k.q);  // should recover a prime factor

    // 3) Defense sanity: recompute full-mod verify after CRT (you add this later)
    // If you implement a post-CRT full-mod check inside signer, the "faulty" output should never leave.

    puts("[ok] tests passed");
    return 0;
}
