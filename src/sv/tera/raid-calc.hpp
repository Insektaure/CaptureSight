#pragma once
#include "encounter.hpp"
#include "xoroshiro.hpp"
#include "shiny-util.hpp"
#include "personal-info.hpp"
#include <cstdint>

enum class TeraShiny : int {
    Any = 0,
    No = 1,
    Yes = 2,
    Star = 3,
    Square = 4,
};

enum class Gender : uint8_t {
    Male = 0,
    Female = 1,
    Genderless = 2,
};

struct TeraDetails {
    uint32_t seed;
    TeraShiny shiny;
    uint8_t stars;
    uint16_t species;
    uint8_t form;
    uint8_t level;
    uint8_t teraType;
    uint32_t EC;
    uint32_t PID;
    int ivs[6]; // HP, ATK, DEF, SPA, SPD, SPE
    int ability;
    int abilityNumber;
    uint8_t nature;
    Gender gender;
    uint16_t moves[4];
};

namespace RaidCalc {

inline uint8_t getTeraType(uint64_t seed, GemType gem, uint16_t species, uint8_t form, const PersonalTable& pt) {
    uint8_t specifiedType;
    if (gemTypeIsSpecified(gem, specifiedType))
        return specifiedType;

    auto rand = Xoroshiro128Plus(seed);
    if (gem == GemType::Random)
        return (uint8_t)rand.nextInt(18);

    auto pivot = rand.nextInt(2);
    auto pi = pt.getFormEntry(species, form);
    return pivot == 0 ? pi.type1() : pi.type2();
}

inline Gender getGender(uint8_t ratio, uint64_t rand100) {
    switch (ratio) {
        case 0x1F: return rand100 < 12 ? Gender::Female : Gender::Male;
        case 0x3F: return rand100 < 25 ? Gender::Female : Gender::Male;
        case 0x7F: return rand100 < 50 ? Gender::Female : Gender::Male;
        case 0xBF: return rand100 < 75 ? Gender::Female : Gender::Male;
        case 0xE1: return rand100 < 89 ? Gender::Female : Gender::Male;
        default:   return rand100 < 50 ? Gender::Female : Gender::Male;
    }
}

inline TeraDetails generateData(uint32_t seed, const EncounterTeraTF9& encounter, uint32_t id32, const PersonalTable& pt) {
    TeraDetails result{};
    result.seed = seed;
    result.stars = encounter.stars;
    result.species = encounter.species;
    result.form = encounter.form;
    result.level = encounter.level;
    result.moves[0] = encounter.moves[0];
    result.moves[1] = encounter.moves[1];
    result.moves[2] = encounter.moves[2];
    result.moves[3] = encounter.moves[3];

    // 1. Tera Type
    result.teraType = getTeraType(seed, encounter.teraType, encounter.species, encounter.form, pt);

    // 2. RNG sequence
    auto rand = Xoroshiro128Plus(seed);

    // EC
    result.EC = (uint32_t)rand.nextInt((uint64_t)UINT32_MAX);

    // PID + Shiny logic
    uint32_t fakeTID = (uint32_t)rand.nextInt();
    uint32_t pid = (uint32_t)rand.nextInt();

    if (encounter.shiny == ShinyType::Random) {
        auto xor_val = ShinyUtil::getShinyXor(pid, fakeTID);
        if (xor_val < 16) {
            if (xor_val != 0) xor_val = 1;
            ShinyUtil::forceShinyState(true, pid, id32, xor_val);
            result.shiny = (xor_val == 0) ? TeraShiny::Square : TeraShiny::Star;
        } else {
            ShinyUtil::forceShinyState(false, pid, id32, xor_val);
            result.shiny = TeraShiny::No;
        }
    } else if (encounter.shiny == ShinyType::Always) {
        uint16_t tid16 = (uint16_t)fakeTID;
        uint16_t sid16 = (uint16_t)(fakeTID >> 16);
        auto xor_val = ShinyUtil::getShinyXor(pid, fakeTID);
        if (xor_val > 16)
            pid = ShinyUtil::getShinyPID(tid16, sid16, pid, 0);
        if (!ShinyUtil::getIsShiny(id32, pid)) {
            xor_val = ShinyUtil::getShinyXor(pid, fakeTID);
            pid = ShinyUtil::getShinyPID(
                (uint16_t)(id32 & 0xFFFF),
                (uint16_t)(id32 >> 16),
                pid, xor_val == 0 ? 0u : 1u);
        }
        xor_val = ShinyUtil::getShinyXor(pid, fakeTID);
        result.shiny = (xor_val == 0) ? TeraShiny::Square : TeraShiny::Star;
    } else { // Never
        if (ShinyUtil::getIsShiny(fakeTID, pid))
            pid ^= 0x10000000;
        if (ShinyUtil::getIsShiny(id32, pid))
            pid ^= 0x10000000;
        result.shiny = TeraShiny::No;
    }
    result.PID = pid;

    // 3. IVs
    constexpr int UNSET = -1;
    constexpr int MAX = 31;
    for (int i = 0; i < 6; i++)
        result.ivs[i] = UNSET;

    for (int i = 0; i < encounter.flawlessIVCount; i++) {
        int index;
        do { index = (int)rand.nextInt(6); }
        while (result.ivs[index] != UNSET);
        result.ivs[index] = MAX;
    }

    for (int i = 0; i < 6; i++) {
        if (result.ivs[i] == UNSET)
            result.ivs[i] = (int)rand.nextInt(MAX + 1);
    }

    // 4. Ability
    int abilNum;
    switch (encounter.ability) {
        case AbilityPermission::Any12H:
            abilNum = (int)rand.nextInt(3) << 1;
            break;
        case AbilityPermission::Any12:
            abilNum = (int)rand.nextInt(2) << 1;
            break;
        default:
            abilNum = (int)encounter.ability;
            break;
    }
    {
        auto pi = pt.getFormEntry(encounter.species, encounter.form);
        int idx = abilNum >> 1;
        if ((uint32_t)idx < (uint32_t)pi.abilityCount())
            result.ability = pi.getAbilityAtIndex(idx);
        else
            result.ability = idx;
    }
    result.abilityNumber = (abilNum == 0) ? 1 : abilNum;

    // 5. Gender
    uint8_t genderRatio = encounter.genderRatio;
    if (genderRatio == PersonalInfo9SV::RatioMagicGenderless) {
        result.gender = Gender::Genderless;
    } else if (genderRatio == PersonalInfo9SV::RatioMagicFemale) {
        result.gender = Gender::Female;
    } else if (genderRatio == PersonalInfo9SV::RatioMagicMale) {
        result.gender = Gender::Male;
    } else {
        result.gender = getGender(genderRatio, rand.nextInt(100));
    }

    // 6. Nature (Toxtricity=849 has special tables)
    if (encounter.species == 849) {
        if (encounter.form == 0) {
            constexpr uint8_t amped[] = {0,3,4,6,7,8,9,11,13,14,16,19,22,24};
            result.nature = amped[rand.nextInt(14)];
        } else {
            constexpr uint8_t lowKey[] = {1,2,5,10,12,15,16,17,18,20,21,23};
            result.nature = lowKey[rand.nextInt(12)];
        }
    } else {
        result.nature = (uint8_t)rand.nextInt(25);
    }

    return result;
}

} // namespace RaidCalc
