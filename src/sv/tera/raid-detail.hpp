#pragma once
#include <cstdint>
#include <vector>

enum class TeraRaidContentType : uint32_t {
    Base05 = 0,
    Black6 = 1,
    Distribution = 2,
    Might7 = 3,
};

enum class TeraRaidMapParent : uint8_t {
    Paldea = 0,
    Kitakami = 1,
    Blueberry = 2,
};

enum class GameProgress : uint8_t {
    Beginning = 0,
    UnlockedTeraRaids = 1,
    Unlocked3Stars = 2,
    Unlocked4Stars = 3,
    Unlocked5Stars = 4,
    Unlocked6Stars = 5,
};

enum class RaidContent : uint8_t {
    Standard = 0,
    Black = 1,
    Event = 2,
    Event_Mighty = 3,
};

// 0x20 bytes per entry in raid spawn list
struct TeraRaidDetail {
    static constexpr int SIZE = 0x20;

    bool isEnabled;
    uint32_t areaID;
    uint32_t lotteryGroup;
    uint32_t spawnPointID;
    uint32_t seed;
    TeraRaidContentType content;
    bool isClaimedLP;

    static inline TeraRaidDetail readFrom(const uint8_t* data) {
        TeraRaidDetail d;
        auto r32 = [](const uint8_t* p) -> uint32_t {
            return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
        };
        d.isEnabled      = r32(data + 0x00) != 0;
        d.areaID         = r32(data + 0x04);
        d.lotteryGroup   = r32(data + 0x08);
        d.spawnPointID   = r32(data + 0x0C);
        d.seed           = r32(data + 0x10);
        d.content        = (TeraRaidContentType)r32(data + 0x18);
        d.isClaimedLP    = r32(data + 0x1C) != 0;
        return d;
    }

    RaidContent raidContent() const {
        switch (content) {
            case TeraRaidContentType::Base05:        return RaidContent::Standard;
            case TeraRaidContentType::Black6:        return RaidContent::Black;
            case TeraRaidContentType::Distribution:  return RaidContent::Event;
            case TeraRaidContentType::Might7:        return RaidContent::Event_Mighty;
        }
        return RaidContent::Standard;
    }
};

struct RaidBlockData {
    std::vector<TeraRaidDetail> paldea;
    std::vector<TeraRaidDetail> kitakami;
    std::vector<TeraRaidDetail> blueberry;

    // Paldea block: 0x10 header + up to 72 raids
    static inline void parsePaldea(const uint8_t* data, size_t len, std::vector<TeraRaidDetail>& out) {
        if (len < 0x10) return;
        const uint8_t* base = data + 0x10;
        size_t dataLen = len - 0x10;
        int count = (int)(dataLen / TeraRaidDetail::SIZE);
        if (count > 72) count = 72;
        for (int i = 0; i < count; i++)
            out.push_back(TeraRaidDetail::readFrom(base + i * TeraRaidDetail::SIZE));
    }

    // DLC block: Kitakami [0, 0xC80) + Blueberry [0xC80, 0x1900)
    static inline void parseDLC(const uint8_t* data, size_t len,
                         std::vector<TeraRaidDetail>& kitakami,
                         std::vector<TeraRaidDetail>& blueberry) {
        constexpr int regionSize = 0xC80;
        if (len >= (size_t)regionSize) {
            int kitaCount = regionSize / TeraRaidDetail::SIZE;
            if (kitaCount > 100) kitaCount = 100;
            for (int i = 0; i < kitaCount; i++)
                kitakami.push_back(TeraRaidDetail::readFrom(data + i * TeraRaidDetail::SIZE));
        }
        if (len >= 0x1900) {
            const uint8_t* blueBase = data + regionSize;
            int blueCount = regionSize / TeraRaidDetail::SIZE;
            if (blueCount > 80) blueCount = 80;
            for (int i = 0; i < blueCount; i++)
                blueberry.push_back(TeraRaidDetail::readFrom(blueBase + i * TeraRaidDetail::SIZE));
        }
    }
};
