#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static uint64_t xorshift64(uint64_t* state) {
    uint64_t x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x ? x : 0x9E3779B97F4A7C15ULL;
    return *state;
}

static uint64_t powmod_branchy(uint64_t a, uint64_t e, uint64_t n) {
    uint64_t result = 1 % n;
    a = a % n;
    while (e > 0) {
        if (e & 1ULL) {
            __uint128_t prod = (__uint128_t)result * a;
            result = (uint64_t)(prod % n);
        }
        __uint128_t square = (__uint128_t)a * a;
        a = (uint64_t)(square % n);
        e >>= 1;
    }
    return result;
}

static uint64_t powmod_constant_time(uint64_t a, uint64_t e, uint64_t n) {
    uint64_t result = 1 % n;
    uint64_t base = a % n;
    for (int bit = 63; bit >= 0; --bit) {
        __uint128_t sq = (__uint128_t)result * result;
        uint64_t squared = (uint64_t)(sq % n);
        __uint128_t mul = (__uint128_t)squared * base;
        uint64_t fused = (uint64_t)(mul % n);
        uint64_t mask = 0ULL - ((e >> bit) & 1ULL);
        result = (squared & ~mask) | (fused & mask);
    }
    return result;
}

static double time_function(uint64_t (*fn)(uint64_t, uint64_t, uint64_t), uint64_t mod,
                            uint64_t* bases, uint64_t* exps, size_t count) {
    clock_t start = clock();
    uint64_t acc = 0;
    for (size_t i = 0; i < count; ++i) {
        acc ^= fn(bases[i], exps[i], mod);
    }
    clock_t end = clock();
    (void)acc; // keep compiler from optimizing the loop completely away
    double elapsed_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    return elapsed_ms / (double)count;
}

int main(void) {
    const uint64_t modulus = 0x1FFFFFFFFULL; // ~33-bit prime-ish modulus
    const size_t samples = 50000;

    uint64_t* bases = (uint64_t*)malloc(sizeof(uint64_t) * samples);
    uint64_t* exps  = (uint64_t*)malloc(sizeof(uint64_t) * samples);
    if (!bases || !exps) {
        fprintf(stderr, "allocation failure\n");
        free(bases);
        free(exps);
        return 1;
    }

    uint64_t seed = 0xC0FFEE1234567890ULL;
    for (size_t i = 0; i < samples; ++i) {
        bases[i] = xorshift64(&seed) % modulus;
        exps[i] = xorshift64(&seed);
    }

    // Sanity: both implementations must agree for all samples.
    for (size_t i = 0; i < samples; ++i) {
        uint64_t lhs = powmod_branchy(bases[i], exps[i], modulus);
        uint64_t rhs = powmod_constant_time(bases[i], exps[i], modulus);
        if (lhs != rhs) {
            fprintf(stderr, "mismatch at %zu: %llu vs %llu\n", i,
                    (unsigned long long)lhs, (unsigned long long)rhs);
            free(bases);
            free(exps);
            return 1;
        }
    }

    double branchy_ms = time_function(powmod_branchy, modulus, bases, exps, samples);
    double constant_ms = time_function(powmod_constant_time, modulus, bases, exps, samples);

    printf("Timing Side-Channel Experiment\n");
    printf("================================\n\n");
    printf("Samples evaluated       : %zu\n", samples);
    printf("Branchy avg time        : %.6f ms per call\n", branchy_ms);
    printf("Constant-time avg time  : %.6f ms per call\n", constant_ms);
    printf("Overhead multiplier     : %.2fx\n", constant_ms / branchy_ms);

    free(bases);
    free(exps);
    return 0;
}
