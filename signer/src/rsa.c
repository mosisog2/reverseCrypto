#include <stdint.h>
#include <stdio.h>
#include "./include/rsa.h"

// Key init: fill p,q,e,d and precompute dp=d mod (p-1), dq, qinv.
int rsa64_init(RsaCrtKey64* k, uint64_t p, uint64_t q uint64_t e) {

}

// Full and CRT signatures: s = m^d mod N.  (m < N; no padding)
uint64_t rsa64_sign_full(uint64_t m, const RsaCrtKey64* k) {
    return powmod64(m, k->d, k->N);
}

// Faulty CRT signature: s = m^d mod N, but with a fault injected
uint64_t rsa54_sign_crt(uint64_t m, const RsaCrtKey64* k, FaultMode mode) {
    
}