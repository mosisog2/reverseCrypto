#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "../src/include/rsa.h"

typedef struct {
    const char* name;
    uint64_t p;
    uint64_t q;
    uint64_t e;
} KeySpec;

static const KeySpec SPECS[] = {
    {"N25", 4099ULL, 6101ULL, 17ULL},
    {"N33", 65537ULL, 67537ULL, 17ULL},
    {"N41", 1048583ULL, 1050593ULL, 65537ULL},
    {"N49", 16777259ULL, 16779229ULL, 65537ULL},
    {"N57", 268435459ULL, 268437457ULL, 65537ULL},
    {"N63", 2147483659ULL, 2147485649ULL, 65537ULL},
};

static volatile uint64_t g_sink = 0;

static uint64_t gcd64(uint64_t a, uint64_t b) {
    while (b) {
        uint64_t t = a % b;
        a = b;
        b = t;
    }
    return a;
}

static unsigned bit_length(uint64_t x) {
    unsigned bits = 0;
    while (x) {
        ++bits;
        x >>= 1U;
    }
    return bits;
}

static uint64_t choose_message(const RsaCrtKey64* key) {
    for (uint64_t m = 3; m < key->N; ++m) {
        if (gcd64(m, key->N) == 1) {
            return m;
        }
    }
    return 3;
}

static size_t pick_iterations(unsigned bits) {
    if (bits <= 16) return 150000;
    if (bits <= 24) return 60000;
    if (bits <= 28) return 30000;
    return 12000;
}

static double bench_full(const RsaCrtKey64* key, uint64_t message, size_t iterations) {
    clock_t start = clock();
    uint64_t acc = 0;
    for (size_t i = 0; i < iterations; ++i) {
        acc ^= rsa64_sign_full(message, key);
    }
    clock_t end = clock();
    g_sink ^= acc;
    double elapsed_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    return elapsed_ms / (double)iterations;
}

static double bench_crt(const RsaCrtKey64* key, uint64_t message, FaultMode mode, size_t iterations, uint64_t seed) {
    rsa_seed_random(seed);
    clock_t start = clock();
    uint64_t acc = 0;
    for (size_t i = 0; i < iterations; ++i) {
        acc ^= rsa64_sign_crt(message, key, mode);
    }
    clock_t end = clock();
    g_sink ^= acc;
    double elapsed_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    return elapsed_ms / (double)iterations;
}

static void emit_row(const char* key_name, unsigned bits, const char* method, double time_ms,
                     double rel_full, double rel_crt) {
    printf("%s,%u,%s,%.6f,%.6f,%.6f\n", key_name, bits, method, time_ms, rel_full, rel_crt);
}

int main(void) {
    printf("key,bits,method,time_ms,rel_to_full,rel_to_crt\n");

    for (size_t idx = 0; idx < sizeof(SPECS) / sizeof(SPECS[0]); ++idx) {
        const KeySpec* spec = &SPECS[idx];
        RsaCrtKey64 key;
        if (rsa64_init(&key, spec->p, spec->q, spec->e) != 0) {
            fprintf(stderr, "Failed to initialize RSA key for %s\n", spec->name);
            return 1;
        }

        uint64_t message = choose_message(&key);
        unsigned bits = bit_length(key.N);
        size_t iterations = pick_iterations(bits);

        rsa_set_defense_config(NULL);
        uint64_t sig_full = rsa64_sign_full(message, &key);
        uint64_t sig_crt = rsa64_sign_crt(message, &key, FAULT_OFF);
        assert(sig_full == sig_crt);

        double full_ms = bench_full(&key, message, iterations);
        double crt_ms = bench_crt(&key, message, FAULT_OFF, iterations, 0xABCDEF1234567890ULL);

        emit_row(spec->name, bits, "full", full_ms, 1.0, full_ms / crt_ms);
        emit_row(spec->name, bits, "crt", crt_ms, crt_ms / full_ms, 1.0);

        RsaDefenseConfig cfg = {0};

        cfg.enable_post_crt_verify = 1;
        rsa_set_defense_config(&cfg);
        double post_ms = bench_crt(&key, message, FAULT_OFF, iterations, 0x1111111111111111ULL);
        emit_row(spec->name, bits, "crt+post_verify", post_ms, post_ms / full_ms, post_ms / crt_ms);
        cfg.enable_post_crt_verify = 0;

        cfg.enable_double_crt = 1;
        rsa_set_defense_config(&cfg);
        double double_ms = bench_crt(&key, message, FAULT_OFF, iterations, 0x2222222222222222ULL);
        emit_row(spec->name, bits, "crt+double", double_ms, double_ms / full_ms, double_ms / crt_ms);
        cfg.enable_double_crt = 0;

        cfg.enable_exponent_blinding = 1;
        rsa_set_defense_config(&cfg);
        double exp_ms = bench_crt(&key, message, FAULT_OFF, iterations, 0x3333333333333333ULL);
        emit_row(spec->name, bits, "crt+exp_blind", exp_ms, exp_ms / full_ms, exp_ms / crt_ms);
        cfg.enable_exponent_blinding = 0;

        cfg.enable_message_blinding = 1;
        rsa_set_defense_config(&cfg);
        double msg_ms = bench_crt(&key, message, FAULT_OFF, iterations, 0x4444444444444444ULL);
        emit_row(spec->name, bits, "crt+msg_blind", msg_ms, msg_ms / full_ms, msg_ms / crt_ms);
        cfg.enable_message_blinding = 0;

        rsa_set_defense_config(NULL);
    }

    return (int)(g_sink & 1U); // keep compiler from eliding loops
}
