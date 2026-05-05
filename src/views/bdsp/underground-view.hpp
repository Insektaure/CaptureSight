#pragma once

#include "../../components/button.hpp"
#include "../detachable-view.hpp"
#include "../pokemon-view.hpp"
#include <csight-core.h>
#include <memory>
#include <string>
#include <tesla.hpp>
#include <vector>

class UndergroundView : public DetachableView {
 public:
  UndergroundView() : DetachableView("Underground") { }

  virtual void setupList(tsl::elm::List *list) {
    // We always need at least one item in a list to prevent it from crashing
    list->addItem(new tsl::elm::CategoryHeader("Pokemon"));

    m_pkxs = csight::bdsp::read_underground_pokemon();

    for (size_t i = 0; i < kMaxUndergroundPokemon; i++) {
      auto item = new tsl::elm::ListItem("");
      item->setClickListener([this, i](u64 keys) {
        if ((keys & HidNpadButton_A) && i < m_pkxs.size() && m_pkxs[i] != nullptr) {
          auto pkx = m_pkxs[i];
          tsl::changeTo<PokemonView>([pkx]() -> std::shared_ptr<csight::Pkx> { return pkx; });
          return true;
        }
        return false;
      });
      m_list_items.push_back(item);
      list->addItem(item);
    }

    refreshLabels();
  }

  virtual void update() override {
    m_pkxs = csight::bdsp::read_underground_pokemon();
    refreshLabels();
  }

  virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState leftJoyStick,
                           HidAnalogStickState rightJoyStick) override {
    if (keysDown & HidNpadButton_Up) {
      utils::toggleAttached();
      return true;
    }

    // When attached, let the list receive input so the user can scroll
    // through the pokemon and open their details with A
    if (utils::getIsAttached()) {
      return false;
    }

    return true;
  }

 private:
  // PokeFinder caps Underground hideaway spawns at 10
  // (Core/Gen8/UndergroundArea.cpp: std::array<TypeSize, 10>)
  static constexpr size_t kMaxUndergroundPokemon = 10;
  std::vector<std::shared_ptr<csight::Pk8>> m_pkxs;
  std::vector<tsl::elm::ListItem *> m_list_items;

  void refreshLabels() {
    for (size_t i = 0; i < m_list_items.size(); i++) {
      if (i < m_pkxs.size() && m_pkxs[i] != nullptr) {
        auto pkx = m_pkxs[i];
        m_list_items[i]->setText(pkx->SpeciesString() + (pkx->IsShiny() ? " ★" : " "));
      } else {
        m_list_items[i]->setText("");
      }
    }
  }
};

class UndergroundViewButton : public Button {
 public:
  UndergroundViewButton() : Button("Underground") { this->onClick(tsl::changeTo<UndergroundView>); }
};
