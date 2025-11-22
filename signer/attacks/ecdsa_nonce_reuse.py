#!/usr/bin/env python3
"""ECDSA nonce reuse demonstration.

Reusing an ECDSA nonce (k) across two signatures leaks the private key.
This script works entirely with big integers using the secp256k1 domain
parameters for concreteness.
"""

from __future__ import annotations

import hashlib
from dataclasses import dataclass
from typing import Optional, Tuple

# secp256k1 domain parameters
P = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
A = 0
B = 7
Gx = 55066263022277343669578718895168534326250603453777594175500187360389116729240
Gy = 32670510020758816978083085130507043184471273380659243275938904335757337482424
N = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141


@dataclass(frozen=True)
class Point:
    x: int
    y: int


INFINITY: Optional[Point] = None


def inv_mod(k: int, modulus: int) -> int:
    """Multiplicative inverse using pow's modular inverse support."""
    if k % modulus == 0:
        raise ZeroDivisionError("inverse does not exist")
    return pow(k, -1, modulus)


def point_add(p: Optional[Point], q: Optional[Point]) -> Optional[Point]:
    if p is INFINITY:
        return q
    if q is INFINITY:
        return p
    if p.x == q.x and (p.y != q.y or p.y == 0):
        return INFINITY
    if p.x == q.x:
        m = (3 * p.x * p.x + A) * inv_mod(2 * p.y, P) % P
    else:
        m = (q.y - p.y) * inv_mod((q.x - p.x) % P, P) % P
    rx = (m * m - p.x - q.x) % P
    ry = (m * (p.x - rx) - p.y) % P
    return Point(rx, ry)


def scalar_mul(k: int, point: Optional[Point]) -> Optional[Point]:
    result = INFINITY
    addend = point
    while k:
        if k & 1:
            result = point_add(result, addend)
        addend = point_add(addend, addend)
        k >>= 1
    return result


def hash_message(msg: bytes) -> int:
    return int.from_bytes(hashlib.sha256(msg).digest(), "big") % N


def ecdsa_sign(secret: int, msg_hash: int, nonce: int) -> Tuple[int, int]:
    R = scalar_mul(nonce, Point(Gx, Gy))
    if R is INFINITY:
        raise ValueError("invalid nonce produced point at infinity")
    r = R.x % N
    if r == 0:
        raise ValueError("invalid nonce produced r = 0")
    kinv = inv_mod(nonce, N)
    s = (kinv * (msg_hash + r * secret)) % N
    if s == 0:
        raise ValueError("invalid nonce produced s = 0")
    return r, s


def recover_nonce(z1: int, z2: int, s1: int, s2: int) -> int:
    numerator = (z1 - z2) % N
    denominator = (s1 - s2) % N
    return (numerator * inv_mod(denominator, N)) % N


def recover_secret(k_reused: int, r: int, s: int, z: int) -> int:
    return ((s * k_reused - z) * inv_mod(r, N)) % N


def main() -> None:
    secret_key = 0x1DEADBEEFCAFEBABE1234567890ABCDEFFEDCBA9876543210ABCDEFFEDCBA9
    nonce = 0xBADF00D1234567890FEDCBA9876543210CAFEBABE112233445566778899AABB

    msg1 = b"transfer 10 BTC"
    msg2 = b"transfer 250 BTC"  # distinct message, same nonce reused
    z1 = hash_message(msg1)
    z2 = hash_message(msg2)

    r1, s1 = ecdsa_sign(secret_key, z1, nonce)
    r2, s2 = ecdsa_sign(secret_key, z2, nonce)

    # Adversary observes (r1, s1, msg1) and (r2, s2, msg2) with same r.
    assert r1 == r2

    k_rec = recover_nonce(z1, z2, s1, s2)
    secret_rec = recover_secret(k_rec, r1, s1, z1)

    print("ECDSA Nonce Reuse Demo")
    print("=======================\n")
    print(f"Original secret key : 0x{secret_key:064x}")
    print(f"Recovered secret key: 0x{secret_rec:064x}")
    print(f"Nonce reused        : 0x{nonce:064x}")
    print(f"Recovered nonce     : 0x{k_rec:064x}\n")

    print("Signature 1")
    print(f"  r = 0x{r1:064x}")
    print(f"  s = 0x{s1:064x}")
    print(f"  hash = 0x{z1:064x}\n")

    print("Signature 2")
    print(f"  r = 0x{r2:064x}")
    print(f"  s = 0x{s2:064x}")
    print(f"  hash = 0x{z2:064x}\n")

    success = secret_rec == secret_key and k_rec == nonce
    print(f"Attack success? {'YES' if success else 'NO'}")


if __name__ == "__main__":
    main()
