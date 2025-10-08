#include <stdint.h>
#include <stdio.h>
#include "./include/mod.h"

// Extended GCD: return g, set *x,*y with ax+by=g 
uint64_t egcd64(uint64_t a, uint64_t b, int64_t* x, int64_t* y) {
    if (b == 0) {
        *x = 1; *y = 0;
        return a;
    }
    int64_t x1, y1;
    uint64_t g = egcd64(b, a % b, &x1, &y1);
    *x = y1;
    *y = x1 - (a / b) * y1;
    return g;
}

// Modular inverse a^{-1} mod n (assume gcd(a,n)=1), return 0 if none 
uint64_t modinv64(uint64_t a, uint64_t n) {
    int64_t x, y;
    uint64_t g = egcd64(a, n, &x, &y);
    if (g != 1) return 0; // no inverse
    int64_t res = x % (int64_t)n;
    if (res < 0) res += n;
    return (uint64_t)res;
}

// Square-and-multiply: computes a^e mod n 
uint64_t powmod64(uint64_t a, uint64_t e, uint64_t n) {
    uint64_t result = 1 % n;
    a = a % n;
    while (e > 0) {
        if (e & 1) result = (result * a) % n;
        a = (a * a) % n;
        e >>= 1;
    }
    return result;
}