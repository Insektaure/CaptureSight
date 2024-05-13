#pragma once

#include "../../components/button.hpp"
#include "../../utils/general.hpp"
#include "../pokemon-view.hpp"
#include <csight-core.h>
#include <dmntcht.h>
#include <memory>
#include <string>
#include <switch.h>
#include <tesla.hpp>


class SvWildTradeRaidView : public tsl::Gui {
 public:
  SvWildTradeRaidView() { }

  virtual tsl::elm::Element *createUI() override {
    auto frame = new tsl::elm::OverlayFrame("Wild/Trade/Raid Pokemon", " ");
    auto list = new tsl::elm::List();

    list->addItem(new tsl::elm::CategoryHeader("Pokemon"));
    list->addItem(new PokemonViewButton("Trade", csight::sv::read_trade_pokemon));

    frame->setContent(list);

    return frame;
  }
};

class SvWildTradeRaidViewButton : public Button {
 public:
  SvWildTradeRaidViewButton() : Button("Wild/Trade/Raid") { this->onClick(tsl::changeTo<SvWildTradeRaidView>); }
};
