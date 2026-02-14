#pragma once

#include "../trainer-view.hpp"
#include "./wild-trade-raid-view.hpp"
#include "./tera-raid-list-view.hpp"
#include "../party-list-view.hpp"
#include "../rng-view.hpp"
#include <csight-core.h>
#include <cstring>
#include <iomanip>
#include <memory>
#include <string>
#include <switch.h>
#include <tesla.hpp>

class MainSvView : public tsl::Gui {
 public:
  MainSvView() { }

  virtual tsl::elm::Element *createUI() override {
    auto frame = new tsl::elm::OverlayFrame("CaptureSight", " ");
    auto list = new tsl::elm::List();

    list->addItem(new tsl::elm::CategoryHeader("Pokemon"));
    list->addItem(new SvWildTradeRaidViewButton());
    list->addItem(new PartyListViewButton(csight::sv::read_party_pokemon));

    list->addItem(new tsl::elm::CategoryHeader("Raids"));
    list->addItem(new PaldeaRaidListViewButton());
    list->addItem(new KitakamiRaidListViewButton());
    list->addItem(new BlueberryRaidListViewButton());

    list->addItem(new tsl::elm::CategoryHeader("Trainer Info"));
    auto trainer_info = csight::sv::read_trainer_info();
    list->addItem(new TrainerViewButton(trainer_info));

    frame->setContent(list);

    return frame;
  }

};
