#pragma once

#include "generated.h"
#include "pkx.h"
#include "rng_tracker.h"
#include <memory>

namespace csight::sv {
  std::shared_ptr<Pk9> read_party_pokemon(u8 index) {
    return std::make_shared<Pk9>(sv_read_party_pokemon(index));
  }

  u64 read_top_level_raid_seed() {
    return sv_read_top_level_raid_seed();
  }

  std::shared_ptr<TrainerInfo> read_trainer_info() {
    TrainerInfo trainer_info;
    sv_read_trainer_info(&trainer_info);
    return std::make_shared<TrainerInfo>(trainer_info);
  }
};
