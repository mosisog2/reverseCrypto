#include <assert.h>
#include <stdio.h>
#include "./src/include/rsa.h"

// Pick two 31-bit primes for toy testing (or generate them).
static const uint64_t P = 2147483659ULL; // example prime ≥ 2^31
static const uint64_t Q = 2147483693ULL; // example prime ≥ 2^31
static const uint64_t E = 65537ULL;

int main(void) {
    RsaCrtKey64 k; 
    assert(rsa64_init(&k, P, Q, E) == 1);

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
