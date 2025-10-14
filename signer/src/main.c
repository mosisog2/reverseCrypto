#include <stdio.h>
#include <stdint.h>
#include "include/rsa.h"

int main(void) {
    printf("RSA CRT Signer Demo\n");
    printf("==================\n\n");
    
    // Test primes (31-bit for demo)
    uint64_t p = 2147483659ULL;
    uint64_t q = 2147483693ULL;
    uint64_t e = 65537ULL;
    
    // Initialize RSA key
    RSACrtKey64 key;
    if (rsa64_init(&key, p, q, e) != 0) {
        printf("Error: Failed to initialize RSA key\n");
        return 1;
    }
    
    printf("RSA Key initialized:\n");
    printf("  N = %llu\n", key.N);
    printf("  e = %llu\n", key.e);
    printf("  p = %llu\n", key.p);
    printf("  q = %llu\n", key.q);
    printf("\n");
    
    // Test message
    uint64_t message = 1234567;
    printf("Message to sign: %llu\n\n", message);
    
    // Normal signatures
    uint64_t sig_full = rsa64_sign_full(message, &key);
    uint64_t sig_crt = rsa64_sign_crt(message, &key, FAULT_OFF);
    
    printf("Signatures:\n");
    printf("  Full method: %llu\n", sig_full);
    printf("  CRT method:  %llu\n", sig_crt);
    printf("  Match: %s\n\n", (sig_full == sig_crt) ? "YES" : "NO");
    
    // Verify signatures
    int verify_full = rsa64_verify(message, sig_full, &key);
    int verify_crt = rsa64_verify(message, sig_crt, &key);
    
    printf("Verification:\n");
    printf("  Full signature: %s\n", verify_full ? "VALID" : "INVALID");
    printf("  CRT signature:  %s\n\n", verify_crt ? "VALID" : "INVALID");
    
    // Fault injection demo
    printf("Fault Injection Attack Demo:\n");
    uint64_t sig_fault_p = rsa64_sign_crt(message, &key, FAULT_MODP);
    uint64_t sig_fault_q = rsa64_sign_crt(message, &key, FAULT_MODQ);
    
    printf("  Normal signature: %llu\n", sig_crt);
    printf("  Faulty (mod p):   %llu\n", sig_fault_p);
    printf("  Faulty (mod q):   %llu\n", sig_fault_q);
    
    // Bellcore attack simulation
    printf("\nBellcore Attack Simulation:\n");
    uint64_t diff = (sig_crt >= sig_fault_p) ? (sig_crt - sig_fault_p) : (key.N - (sig_fault_p - sig_crt));
    
    // Simple GCD to find factor
    uint64_t a = key.N, b = diff;
    while (b) {
        uint64_t temp = a % b;
        a = b;
        b = temp;
    }
    uint64_t gcd_result = a;
    
    printf("  GCD(N, sig_diff) = %llu\n", gcd_result);
    if (gcd_result == key.p || gcd_result == key.q) {
        printf("  SUCCESS: Recovered prime factor!\n");
    } else {
        printf("  Failed to recover prime factor\n");
    }
    
    printf("\nDemo completed.\n");
    return 0;
}
