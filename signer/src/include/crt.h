#include <stdint.h>
#include <stdio.h>

/* Chinese remainder theorem: returns x, a solution to the
   pair of congruence equations x=a1 (mod m1) and x=a2 (mod m2) 
   Assumes m1 and m2 are pairwise coprime */
uint64_t crt64(uint64_t a1, uint64_t m1, uint64_t a2, uint64_t m2);