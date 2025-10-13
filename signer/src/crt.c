#include "./include/mod.h"
#include "./include/crt.h"

/*
 * Chinese remainder theorem: returns x, a solution to the
 * pair of congruence equations x=a1 (mod m1) and x=a2 (mod m2) 
 * Assumes m1 and m2 are pairwise coprime
 */ 
uint64_t crt64(uint64_t a1, uint64_t m1, uint64_t a2, uint64_t m2){
    uint64_t k = (a1 - a2) % m1;
    uint64_t invm2 = modinv64(m2, m1);
    k = (k * invm2) % m1;
    uint64_t x = m2 * k + a2;
    x = x % (m1*m2);
    return x;
}