#pragma once
#include "raid-detail.hpp"
#include "encounter.hpp"
#include "raid-calc.hpp"
#include "personal-info.hpp"
#include "text-data.hpp"
#include "../../utils/debug.hpp"
#include <dmntcht.h>
#include <vector>
#include <cstdint>
#include <cstring>

// Pointer chains from Tera-Finder (SV 3.0.1)
namespace TeraPointers {
    constexpr uint64_t KTeraRaidPaldea[] = {0x47350D8, 0x1C0, 0x88, 0x40};
    constexpr int      KTeraRaidPaldeaLen = 4;
    constexpr size_t   KTeraRaidPaldeaSize = 0xC98;

    constexpr uint64_t KTeraRaidDLC[] = {0x47350D8, 0x1C0, 0x88, 0xCD8};
    constexpr int      KTeraRaidDLCLen = 4;
    constexpr size_t   KTeraRaidDLCSize = 0x1910;

    constexpr uint64_t KMyStatus[] = {0x47350D8, 0xD8, 0x08, 0xB8, 0x0, 0x40};
    constexpr int      KMyStatusLen = 6;
    constexpr size_t   KMyStatusSize = 0x68;
}

struct RaidInfo {
    TeraDetails details;
    TeraRaidMapParent map;
    RaidContent content;
    int slotIndex;
};

class TeraRaidReader {
public:
    // Initialize tables from embedded data
    inline void init() {
        personal_.init();
        paldeaStandard_.loadFromEmbedded(enc_paldea_std_bin, enc_paldea_std_bin_end, personal_);
        paldeaBlack_.loadFromEmbedded(enc_paldea_blk_bin, enc_paldea_blk_bin_end, personal_);
        kitakamiStandard_.loadFromEmbedded(enc_kitakami_std_bin, enc_kitakami_std_bin_end, personal_);
        kitakamiBlack_.loadFromEmbedded(enc_kitakami_blk_bin, enc_kitakami_blk_bin_end, personal_);
        blueberryStandard_.loadFromEmbedded(enc_blueberry_std_bin, enc_blueberry_std_bin_end, personal_);
        blueberryBlack_.loadFromEmbedded(enc_blueberry_blk_bin, enc_blueberry_blk_bin_end, personal_);
    }

    // Read all raids from live game memory
    inline bool readLive() {
        raids_.clear();

        GameVersion version = (dbg::GetCheatProcessTitleId() == 0x0100A3D008C5C000ULL)
            ? GameVersion::Scarlet
            : GameVersion::Violet;

        // Read Paldea raid block
        std::vector<uint8_t> paldeaBuf(TeraPointers::KTeraRaidPaldeaSize);
        if (!readBlock(TeraPointers::KTeraRaidPaldea, TeraPointers::KTeraRaidPaldeaLen,
                       paldeaBuf.data(), paldeaBuf.size()))
            return false;

        // Read DLC raid block
        std::vector<uint8_t> dlcBuf(TeraPointers::KTeraRaidDLCSize);
        if (!readBlock(TeraPointers::KTeraRaidDLC, TeraPointers::KTeraRaidDLCLen,
                       dlcBuf.data(), dlcBuf.size()))
            return false;

        // Read trainer ID for shiny calculation
        std::vector<uint8_t> statusBuf(TeraPointers::KMyStatusSize);
        if (readBlock(TeraPointers::KMyStatus, TeraPointers::KMyStatusLen,
                      statusBuf.data(), statusBuf.size())) {
            if (statusBuf.size() >= 8) {
                const uint8_t* d = statusBuf.data() + 0x04;
                id32_ = d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
            }
        }

        // Assume post-game for live mode
        progress_ = GameProgress::Unlocked6Stars;

        // Parse raid slots
        std::vector<TeraRaidDetail> paldea, kitakami, blueberry;
        RaidBlockData::parsePaldea(paldeaBuf.data(), paldeaBuf.size(), paldea);
        RaidBlockData::parseDLC(dlcBuf.data(), dlcBuf.size(), kitakami, blueberry);

        processSlots(paldea, TeraRaidMapParent::Paldea, version, 0);
        processSlots(kitakami, TeraRaidMapParent::Kitakami, version, 72);
        processSlots(blueberry, TeraRaidMapParent::Blueberry, version, 172);

        return !raids_.empty();
    }

    const std::vector<RaidInfo>& raids() const { return raids_; }

private:
    PersonalTable personal_;
    EncounterTable paldeaStandard_;
    EncounterTable paldeaBlack_;
    EncounterTable kitakamiStandard_;
    EncounterTable kitakamiBlack_;
    EncounterTable blueberryStandard_;
    EncounterTable blueberryBlack_;

    std::vector<RaidInfo> raids_;
    GameProgress progress_ = GameProgress::Beginning;
    uint32_t id32_ = 0;

    // Resolve pointer chain and read memory block
    inline bool readBlock(const uint64_t* chain, int chainLen, uint8_t* buf, size_t size) {
        if (chainLen < 1) return false;

        uint64_t addr = g_metadata.main_nso_extents.base + chain[0];

        for (int i = 1; i < chainLen; i++) {
            uint64_t ptr = 0;
            Result rc = dmntchtReadCheatProcessMemory(addr, &ptr, sizeof(uint64_t));
            if (R_FAILED(rc) || ptr == 0) return false;
            addr = ptr + chain[i];
        }

        Result rc = dmntchtReadCheatProcessMemory(addr, buf, size);
        return R_SUCCEEDED(rc);
    }

    inline void processSlots(const std::vector<TeraRaidDetail>& slots,
                              TeraRaidMapParent map, GameVersion version,
                              int startIndex) {
        for (int i = 0; i < (int)slots.size(); i++) {
            auto& slot = slots[i];

            if (!slot.isEnabled || slot.areaID == 0)
                continue;

            RaidContent rc = slot.raidContent();
            if (rc == RaidContent::Event || rc == RaidContent::Event_Mighty)
                continue;

            const std::vector<EncounterTeraTF9>* table = nullptr;
            switch (map) {
                case TeraRaidMapParent::Paldea:
                    table = (rc == RaidContent::Black)
                        ? &paldeaBlack_.entries
                        : &paldeaStandard_.entries;
                    break;
                case TeraRaidMapParent::Kitakami:
                    table = (rc == RaidContent::Black)
                        ? &kitakamiBlack_.entries
                        : &kitakamiStandard_.entries;
                    break;
                case TeraRaidMapParent::Blueberry:
                    table = (rc == RaidContent::Black)
                        ? &blueberryBlack_.entries
                        : &blueberryStandard_.entries;
                    break;
            }

            if (!table || table->empty())
                continue;

            auto* enc = getEncounterFromSeed(slot.seed, *table, version, progress_, rc, map);
            if (!enc)
                continue;

            RaidInfo info;
            info.details = RaidCalc::generateData(slot.seed, *enc, id32_, personal_);
            info.map = map;
            info.content = rc;
            info.slotIndex = startIndex + i;

            raids_.push_back(info);
        }
    }
};
