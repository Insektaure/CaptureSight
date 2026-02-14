#pragma once

#include "../../components/button.hpp"
#include "../../sv/tera/raid-reader.hpp"
#include "./tera-raid-detail-view.hpp"
#include <memory>
#include <string>
#include <vector>
#include <tesla.hpp>

class TeraRaidListView : public tsl::Gui {
 public:
  TeraRaidListView(TeraRaidMapParent region) : m_region(region) { }

  virtual tsl::elm::Element *createUI() override {
    std::string title;
    switch (m_region) {
      case TeraRaidMapParent::Paldea:    title = "Paldea Raids"; break;
      case TeraRaidMapParent::Kitakami:   title = "Kitakami Raids"; break;
      case TeraRaidMapParent::Blueberry:  title = "Blueberry Raids"; break;
    }

    auto frame = new tsl::elm::OverlayFrame(title, " ");
    auto list = new tsl::elm::List();

    TeraRaidReader reader;
    reader.init();

    if (!reader.readLive()) {
      list->addItem(new tsl::elm::CategoryHeader("No raids found"));
      frame->setContent(list);
      return frame;
    }

    bool found = false;
    for (auto& raid : reader.raids()) {
      if (raid.map != m_region)
        continue;

      found = true;
      auto& d = raid.details;

      std::string label;
      if (d.shiny != TeraShiny::No)
        label += "\u2605 ";
      label += TeraText::getSpeciesName(d.species);
      label += " (" + std::to_string(d.stars) + "\u2605)";

      auto raidPtr = std::make_shared<RaidInfo>(raid);
      auto item = new Button(label);
      item->onClick([raidPtr]() {
        tsl::changeTo<TeraRaidDetailView>(raidPtr);
      });
      list->addItem(item);
    }

    if (!found)
      list->addItem(new tsl::elm::CategoryHeader("No active raids"));

    frame->setContent(list);
    return frame;
  }

 private:
  TeraRaidMapParent m_region;
};

class PaldeaRaidListViewButton : public Button {
 public:
  PaldeaRaidListViewButton() : Button("Paldea") {
    this->onClick([]() { tsl::changeTo<TeraRaidListView>(TeraRaidMapParent::Paldea); });
  }
};

class KitakamiRaidListViewButton : public Button {
 public:
  KitakamiRaidListViewButton() : Button("Kitakami") {
    this->onClick([]() { tsl::changeTo<TeraRaidListView>(TeraRaidMapParent::Kitakami); });
  }
};

class BlueberryRaidListViewButton : public Button {
 public:
  BlueberryRaidListViewButton() : Button("Blueberry") {
    this->onClick([]() { tsl::changeTo<TeraRaidListView>(TeraRaidMapParent::Blueberry); });
  }
};
