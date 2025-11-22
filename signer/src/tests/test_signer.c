#include <assert.h>
#include <stdio.h>
#include <time.h>
#include "../include/rsa.h"

// Small, easy-to-factor RSA parameters keep arithmetic fast and readable.
static const uint64_t P = 61ULL;
static const uint64_t Q = 53ULL;
static const uint64_t E = 17ULL;
static const uint64_t ATTACK_MESSAGE = 37ULL;

typedef struct {
    double avg_ms;          // average time per rsa64_sign_crt call
    int attack_succeeded;   // Bellcore-style fault led to factor recovery
    int fault_signature_valid; // rsa64_verify(m, s_fault) result
    uint64_t diff;          // |s_ok - s_fault|
    uint64_t recovered;     // gcd(diff, N)
    uint64_t s_ok;
    uint64_t s_fault;
} AttackResult;

// Aggregated telemetry for the noisy campaign, letting us compare fault mixes and recovery rates.
typedef struct {
    size_t trials;
    size_t count_modp;
    size_t count_modq;
    size_t count_off;
    size_t total_successes;
    size_t modp_successes;
    size_t verify_passes;
    uint64_t diff_sum_modp;
    uint64_t diff_sum_success;
    double avg_ms;          // average time per rsa64_sign_crt call inside noisy loop
} NoisyAttackStats;

static uint64_t gcd_local(uint64_t a, uint64_t b) {
    while (b) {
        uint64_t t = a % b;
        a = b;
        b = t;
    }
    return a;
}

static double benchmark_sign(const RsaCrtKey64* k, uint64_t m, FaultMode mode, size_t iterations) {
    clock_t start = clock();
    for (size_t i = 0; i < iterations; ++i) {
        rsa64_sign_crt(m, k, mode);
    }
    clock_t end = clock();
    return ((double)(end - start) * 1000.0 / CLOCKS_PER_SEC) / (double)iterations;
}

static AttackResult run_fault_attack(const RsaCrtKey64* k, const RsaDefenseConfig* cfg) {
    if (cfg) {
        rsa_set_defense_config(cfg);
    } else {
        rsa_set_defense_config(NULL);
    }

    // Keep blinding deterministic for repeatable measurements.
    rsa_seed_random(1);
    const size_t iterations = 2000;
    double avg_ms = benchmark_sign(k, ATTACK_MESSAGE, FAULT_MODP, iterations);

    rsa_seed_random(1);
    uint64_t s_ok = rsa64_sign_crt(ATTACK_MESSAGE, k, FAULT_OFF);

    rsa_seed_random(1);
    uint64_t s_fault = rsa64_sign_crt(ATTACK_MESSAGE, k, FAULT_MODP);

    uint64_t diff = (s_ok >= s_fault) ? (s_ok - s_fault) : (s_fault - s_ok);
    uint64_t recovered = gcd_local(diff, k->N);
    int success = (recovered != 1 && recovered != k->N);
    int valid = rsa64_verify(ATTACK_MESSAGE, s_fault, k);

    AttackResult result = {
        .avg_ms = avg_ms,
        .attack_succeeded = success,
        .fault_signature_valid = valid,
        .diff = diff,
        .recovered = recovered,
        .s_ok = s_ok,
        .s_fault = s_fault
    };
    return result;
}

static void report_result(const char* label, const AttackResult* r) {
    printf("[%s]\n", label);
    printf("  time per sign  : %.4f ms\n", r->avg_ms);
    printf("  s_ok           : %lu\n", r->s_ok);
    printf("  s_fault        : %lu\n", r->s_fault);
    printf("  |Δ|            : %lu\n", r->diff);
    printf("  gcd(N, |Δ|)    : %lu\n", r->recovered);
    printf("  attack outcome : %s\n", r->attack_succeeded ? "SUCCESS" : "FAIL");
    printf("  verifies?      : %s\n\n", r->fault_signature_valid ? "YES" : "NO");
}

static void run_defense_campaign(const RsaCrtKey64* k) {
    RsaDefenseConfig cfg = {0};

    AttackResult baseline = run_fault_attack(k, &cfg);
    report_result("baseline", &baseline);

    cfg.enable_post_crt_verify = 1;
    AttackResult post_verify = run_fault_attack(k, &cfg);
    report_result("post-crt verify", &post_verify);
    cfg.enable_post_crt_verify = 0;

    cfg.enable_double_crt = 1;
    AttackResult double_crt = run_fault_attack(k, &cfg);
    report_result("double computation", &double_crt);
    cfg.enable_double_crt = 0;

    cfg.enable_exponent_blinding = 1;
    AttackResult exp_blind = run_fault_attack(k, &cfg);
    report_result("exponent blinding", &exp_blind);
    cfg.enable_exponent_blinding = 0;

    cfg.enable_message_blinding = 1;
    AttackResult msg_blind = run_fault_attack(k, &cfg);
    report_result("message blinding", &msg_blind);
    cfg.enable_message_blinding = 0;
}

static uint64_t local_rng_state = 0;

// Separate RNG keeps host determinism while still modeling jittery hardware choices.
static void seed_local_rng(uint64_t seed) {
    local_rng_state = seed ? seed : 0xA5A5A5A5A5A5A5A5ULL;
}

static uint64_t next_local_rand(void) {
    uint64_t x = local_rng_state;
    x ^= x << 7;
    x ^= x >> 9;
    x ^= x << 8;
    local_rng_state = x ? x : 0xD2B74407B1CE6E93ULL;
    return local_rng_state;
}

// Run a Bellcore-style campaign with probabilistic misfires, wrong-branch glitches, and bit flips.
static NoisyAttackStats run_noisy_attack(const RsaCrtKey64* k, const RsaDefenseConfig* cfg, int noise_percent, size_t trials) {
    NoisyAttackStats stats = {
        .trials = trials,
        .count_modp = 0,
        .count_modq = 0,
        .count_off = 0,
        .total_successes = 0,
        .modp_successes = 0,
        .verify_passes = 0,
        .diff_sum_modp = 0,
        .diff_sum_success = 0,
        .avg_ms = 0.0
    };

    if (cfg) {
        rsa_set_defense_config(cfg);
    } else {
        rsa_set_defense_config(NULL);
    }

    seed_local_rng(0x123456789ULL + (uint64_t)(noise_percent * 17));

    clock_t start = clock();

    for (size_t i = 0; i < trials; ++i) {
        FaultMode mode = FAULT_MODP;
        int inject_bitflip = 0;

        if (noise_percent > 0) {
            uint64_t noise_roll = next_local_rand() % 100ULL;
            if (noise_roll < (uint64_t)noise_percent) {
                uint64_t noise_case = next_local_rand() % 3ULL;
                if (noise_case == 0ULL) {
                    mode = FAULT_OFF;       // fault missed entirely
                } else if (noise_case == 1ULL) {
                    mode = FAULT_MODQ;      // wrong branch corrupted
                } else {
                    mode = FAULT_MODP;
                    inject_bitflip = 1;     // analog noise corrupts result bits
                }
            }
        }

        uint64_t base_seed = 0xBADCAFE0DEADBEEFULL ^ ((uint64_t)i * 0x9E3779B97F4A7C15ULL);

        rsa_seed_random(base_seed);
        uint64_t s_ok = rsa64_sign_crt(ATTACK_MESSAGE, k, FAULT_OFF);

        rsa_seed_random(base_seed);
        uint64_t s_fault = rsa64_sign_crt(ATTACK_MESSAGE, k, mode);

        if (inject_bitflip) {
            uint64_t bit = next_local_rand() % 16ULL; // flip in low bits to mimic noisy readout
            uint64_t mask = 1ULL << bit;
            s_fault ^= mask;
        }

        if (mode == FAULT_OFF) {
            stats.count_off++;
        } else if (mode == FAULT_MODQ) {
            stats.count_modq++;
        } else {
            stats.count_modp++;
        }

        uint64_t diff = (s_ok >= s_fault) ? (s_ok - s_fault) : (s_fault - s_ok);
        if (mode == FAULT_MODP) {
            stats.diff_sum_modp += diff;
        }

        uint64_t recovered = gcd_local(diff, k->N);
        int success = (recovered != 1 && recovered != k->N);
        if (success) {
            stats.total_successes++;
            if (mode == FAULT_MODP) {
                stats.modp_successes++;
                stats.diff_sum_success += diff;
            }
        }

        int verifies = rsa64_verify(ATTACK_MESSAGE, s_fault, k);
        if (verifies) {
            stats.verify_passes++;
        }
    }

    clock_t end = clock();
    double total_calls = (double)trials * 2.0; // each trial performs two CRT signatures
    stats.avg_ms = ((double)(end - start) * 1000.0 / CLOCKS_PER_SEC) / total_calls;

    rsa_set_defense_config(NULL);
    return stats;
}

// Summarize noisy run in one block so defense comparisons stay readable in logs.
static void report_noisy_result(const char* label, const NoisyAttackStats* stats, int noise_percent) {
    double total_success_rate = stats->trials ? (100.0 * (double)stats->total_successes / (double)stats->trials) : 0.0;
    double modp_success_rate = stats->count_modp ? (100.0 * (double)stats->modp_successes / (double)stats->count_modp) : 0.0;
    double mean_diff_modp = stats->count_modp ? ((double)stats->diff_sum_modp / (double)stats->count_modp) : 0.0;
    double mean_diff_success = stats->modp_successes ? ((double)stats->diff_sum_success / (double)stats->modp_successes) : 0.0;

    printf("[%s | noise=%d%%]\n", label, noise_percent);
    printf("  trials           : %zu\n", stats->trials);
    printf("  mode mix         : modp=%zu, modq=%zu, off=%zu\n", stats->count_modp, stats->count_modq, stats->count_off);
    printf("  success rate     : %.2f%% (overall)\n", total_success_rate);
    if (stats->count_modp) {
        printf("  success rate modp: %.2f%% of %zu attempts\n", modp_success_rate, stats->count_modp);
        printf("  mean |Δ| (modp)  : %.2f\n", mean_diff_modp);
    }
    if (stats->modp_successes) {
        printf("  mean |Δ| success : %.2f\n", mean_diff_success);
    }
    printf("  verify passes    : %zu\n", stats->verify_passes);
    printf("  time per sign    : %.4f ms\n\n", stats->avg_ms);
}

// Reuse the deterministic defense sweep but under the requested noise budget.
static void run_noisy_defense_campaign(const RsaCrtKey64* k, int noise_percent, size_t trials) {
    printf("Running noisy fault campaign on message %lu (noise=%d%%, trials=%zu)...\n\n", ATTACK_MESSAGE, noise_percent, trials);

    RsaDefenseConfig cfg = {0};

    NoisyAttackStats baseline = run_noisy_attack(k, &cfg, noise_percent, trials);
    report_noisy_result("baseline", &baseline, noise_percent);

    cfg.enable_post_crt_verify = 1;
    NoisyAttackStats post_verify = run_noisy_attack(k, &cfg, noise_percent, trials);
    report_noisy_result("post-crt verify", &post_verify, noise_percent);
    cfg.enable_post_crt_verify = 0;

    cfg.enable_double_crt = 1;
    NoisyAttackStats double_crt = run_noisy_attack(k, &cfg, noise_percent, trials);
    report_noisy_result("double computation", &double_crt, noise_percent);
    cfg.enable_double_crt = 0;

    cfg.enable_exponent_blinding = 1;
    NoisyAttackStats exp_blind = run_noisy_attack(k, &cfg, noise_percent, trials);
    report_noisy_result("exponent blinding", &exp_blind, noise_percent);
    cfg.enable_exponent_blinding = 0;

    cfg.enable_message_blinding = 1;
    NoisyAttackStats msg_blind = run_noisy_attack(k, &cfg, noise_percent, trials);
    report_noisy_result("message blinding", &msg_blind, noise_percent);
    cfg.enable_message_blinding = 0;
}

int main(void) {
    RsaCrtKey64 k;
    int init_result = rsa64_init(&k, P, Q, E);
    printf("RSA init result: %d\n", init_result);
    assert(init_result == 0);

    // Quick sanity: CRT and full paths agree when no faults or defenses.
    rsa_set_defense_config(NULL);
    rsa_seed_random(1);
    uint64_t sanity_full = rsa64_sign_full(ATTACK_MESSAGE, &k);
    uint64_t sanity_crt  = rsa64_sign_crt(ATTACK_MESSAGE, &k, FAULT_OFF);
    assert(sanity_full == sanity_crt);
    assert(rsa64_verify(ATTACK_MESSAGE, sanity_crt, &k));

    printf("\nRunning fault-attack campaign on message %lu...\n\n", ATTACK_MESSAGE);
    run_defense_campaign(&k);

    puts("[ok] defense sweep complete");

    puts("\n---");
    run_noisy_defense_campaign(&k, 30, 5000);
    return 0;
}
