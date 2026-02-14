#pragma once
#include <cstdint>

// Xoroshiro128+ RNG - ported from PKHeX.Core
struct Xoroshiro128Plus {
    static constexpr uint64_t XOROSHIRO_CONST = 0x82A2B175229D6A5B;

    uint64_t s0;
    uint64_t s1;

    explicit Xoroshiro128Plus(uint64_t seed)
        : s0(seed), s1(XOROSHIRO_CONST) {}

    uint64_t next() {
        uint64_t _s0 = s0;
        uint64_t _s1 = s1;
        uint64_t result = _s0 + _s1;

        _s1 ^= _s0;
        s0 = rotl(_s0, 24) ^ _s1 ^ (_s1 << 16);
        s1 = rotl(_s1, 37);

        return result;
    }

    // Bitmask rejection sampling (matches PKHeX)
    uint64_t nextInt(uint64_t max) {
        uint64_t mask = getBitmask(max);
        uint64_t result;
        do {
            result = next() & mask;
        } while (result >= max);
        return result;
    }

    uint64_t nextInt() {
        return next();
    }

private:
    static uint64_t rotl(uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
    }

    static uint64_t getBitmask(uint64_t exclusiveMax) {
        --exclusiveMax;
        if (exclusiveMax == 0) return 0;
        int lz = __builtin_clzll(exclusiveMax);
        return (1ULL << (64 - lz)) - 1;
    }
};
