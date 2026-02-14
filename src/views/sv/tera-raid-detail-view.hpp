#pragma once

#include "../../sv/tera/raid-reader.hpp"
#include "../../utils/general.hpp"
#include "../../components/button.hpp"
#include <memory>
#include <string>
#include <tesla.hpp>

class TeraRaidDetailView : public tsl::Gui {
 public:
  TeraRaidDetailView(std::shared_ptr<RaidInfo> raid) : m_raid(raid) { }

  virtual tsl::elm::Element *createUI() override {
    auto& d = m_raid->details;
    std::string species = TeraText::getSpeciesName(d.species);
    std::string title = species + " " + TeraText::getGenderString(d.gender);

    if (d.shiny == TeraShiny::Star || d.shiny == TeraShiny::Square) {
      title = "Shiny " + title;
    }

    auto frame = new tsl::elm::OverlayFrame(title, "Tera Raid Details");
    auto list = new tsl::elm::List();

    // Basic info
    list->addItem(new tsl::elm::CategoryHeader("Info"));
    list->addItem(new tsl::elm::ListItem("Stars: " + TeraText::getStarsString(d.stars)));
    list->addItem(new tsl::elm::ListItem("Level: " + std::to_string(d.level)));
    list->addItem(new tsl::elm::ListItem("Tera Type: " + TeraText::getTypeName(d.teraType)));
    list->addItem(new tsl::elm::ListItem("Nature: " + TeraText::getNatureName(d.nature)));

    std::string shinyStr = "None";
    if (d.shiny == TeraShiny::Star) shinyStr = "Star";
    else if (d.shiny == TeraShiny::Square) shinyStr = "Square";
    list->addItem(new tsl::elm::ListItem("Shiny: " + shinyStr));

    // IVs
    list->addItem(new tsl::elm::CategoryHeader("IVs (HP/Atk/Def/SpA/SpD/Spe)"));
    list->addItem(new tsl::elm::ListItem(TeraText::formatIVs(d.ivs)));

    // Seed & IDs
    list->addItem(new tsl::elm::CategoryHeader("Seed"));
    list->addItem(new tsl::elm::ListItem(utils::num_to_hex(d.seed)));

    list->addItem(new tsl::elm::CategoryHeader("PID / EC"));
    list->addItem(new tsl::elm::ListItem(utils::num_to_hex(d.PID) + " / " + utils::num_to_hex(d.EC)));

    // Region
    std::string region;
    switch (m_raid->map) {
      case TeraRaidMapParent::Paldea:    region = "Paldea"; break;
      case TeraRaidMapParent::Kitakami:   region = "Kitakami"; break;
      case TeraRaidMapParent::Blueberry:  region = "Blueberry"; break;
    }
    list->addItem(new tsl::elm::CategoryHeader("Region"));
    list->addItem(new tsl::elm::ListItem(region + " #" + std::to_string(m_raid->slotIndex)));

    frame->setContent(list);
    return frame;
  }

 private:
  std::shared_ptr<RaidInfo> m_raid;
};
