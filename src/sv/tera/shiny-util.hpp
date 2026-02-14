#pragma once
#include <cstdint>

namespace ShinyUtil {

inline uint32_t getShinyXor(uint32_t component) {
    return (component ^ (component >> 16)) & 0xFFFF;
}

inline uint32_t getShinyXor(uint32_t pid, uint32_t id32) {
    return getShinyXor(pid ^ id32);
}

inline bool getIsShiny(uint32_t id32, uint32_t pid) {
    return getShinyXor(pid, id32) < 16;
}

inline uint32_t getShinyPID(uint16_t tid, uint16_t sid, uint32_t pid, uint32_t xorType) {
    uint32_t low = pid & 0xFFFF;
    return ((xorType ^ tid ^ sid ^ low) << 16) | low;
}

inline void forceShinyState(bool isShiny, uint32_t& pid, uint32_t id32, uint32_t xorType) {
    if (isShiny) {
        if (!getIsShiny(id32, pid))
            pid = getShinyPID((uint16_t)(id32 & 0xFFFF), (uint16_t)(id32 >> 16), pid, xorType);
    } else {
        if (getIsShiny(id32, pid))
            pid ^= 0x10000000;
    }
}

} // namespace ShinyUtil
