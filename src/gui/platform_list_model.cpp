#include "platform_list_model.hpp"

#include <QJsonObject>
#include <firelight/libretro/retropad.hpp>

#include "../app/platform_metadata.hpp"

namespace firelight::gui {
PlatformListModel::PlatformListModel() {
  m_items.push_back(
      {PlatformMetadata::PLATFORM_ID_NES,
       QString::fromStdString(PlatformMetadata::getPlatformName(
           PlatformMetadata::PLATFORM_ID_NES)),
       "qrc:images/platform-icons/nes",
       "platform-nes",
       {{{"display_name", "A"},
         {"icon_url", "file:system/_img/nes/a-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::EastFace}},
        {{"display_name", "B"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::SouthFace}},
        {{"display_name", "D-Pad Up"},
         {"icon_url", "file:system/_img/nes/dpad-up.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadUp}},
        {{"display_name", "D-Pad Down"},
         {"icon_url", "file:system/_img/nes/dpad-down.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadDown}},
        {{"display_name", "D-Pad Left"},
         {"icon_url", "file:system/_img/nes/dpad-left.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadLeft}},
        {{"display_name", "D-Pad Right"},
         {"icon_url", "file:system/_img/nes/dpad-right.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadRight}},
        {{"display_name", "Start"},
         {"icon_url", "file:system/_img/nes/start.svg"},
         {"retropad_button", libretro::IRetroPad::Input::Start}},
        {{"display_name", "Select"},
         {"icon_url", "file:system/_img/nes/select.svg"},
         {"retropad_button", libretro::IRetroPad::Input::Select}}},
       {}});
  m_items.push_back(
      {PlatformMetadata::PLATFORM_ID_SNES,
       QString::fromStdString(PlatformMetadata::getPlatformName(
           PlatformMetadata::PLATFORM_ID_SNES)),
       "qrc:images/platform-icons/snes",
       "platform-snes",
       {{{"display_name", "A"},
         {"icon_url", "file:system/_img/nes/a-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::EastFace}},
        {{"display_name", "B"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::SouthFace}},
        {{"display_name", "X"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::NorthFace}},
        {{"display_name", "Y"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::WestFace}},
        {{"display_name", "L"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::LeftBumper}},
        {{"display_name", "R"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::RightBumper}},
        {{"display_name", "D-Pad Up"},
         {"icon_url", "file:system/_img/nes/dpad-up.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadUp}},
        {{"display_name", "D-Pad Down"},
         {"icon_url", "file:system/_img/nes/dpad-down.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadDown}},
        {{"display_name", "D-Pad Left"},
         {"icon_url", "file:system/_img/nes/dpad-left.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadLeft}},
        {{"display_name", "D-Pad Right"},
         {"icon_url", "file:system/_img/nes/dpad-right.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadRight}},
        {{"display_name", "Start"},
         {"icon_url", "file:system/_img/nes/start.svg"},
         {"retropad_button", libretro::IRetroPad::Input::Start}},
        {{"display_name", "Select"},
         {"icon_url", "file:system/_img/nes/select.svg"},
         {"retropad_button", libretro::IRetroPad::Input::Select}}},
       {}});
  m_items.push_back(
      {PlatformMetadata::PLATFORM_ID_GAMEBOY,
       QString::fromStdString(PlatformMetadata::getPlatformName(
           PlatformMetadata::PLATFORM_ID_GAMEBOY)),
       "qrc:images/platform-icons/gb",
       "platform-gb",
       {{{"display_name", "A"},
         {"icon_url", "file:system/_img/nes/a-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::EastFace}},
        {{"display_name", "B"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::SouthFace}},
        {{"display_name", "D-Pad Up"},
         {"icon_url", "file:system/_img/nes/dpad-up.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadUp}},
        {{"display_name", "D-Pad Down"},
         {"icon_url", "file:system/_img/nes/dpad-down.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadDown}},
        {{"display_name", "D-Pad Left"},
         {"icon_url", "file:system/_img/nes/dpad-left.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadLeft}},
        {{"display_name", "D-Pad Right"},
         {"icon_url", "file:system/_img/nes/dpad-right.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadRight}},
        {{"display_name", "Start"},
         {"icon_url", "file:system/_img/nes/start.svg"},
         {"retropad_button", libretro::IRetroPad::Input::Start}},
        {{"display_name", "Select"},
         {"icon_url", "file:system/_img/nes/select.svg"},
         {"retropad_button", libretro::IRetroPad::Input::Select}}},
       {}});
  m_items.push_back(
      {PlatformMetadata::PLATFORM_ID_GAMEBOY_COLOR,
       QString::fromStdString(PlatformMetadata::getPlatformName(
           PlatformMetadata::PLATFORM_ID_GAMEBOY_COLOR)),
       "qrc:images/platform-icons/gbc",
       "platform-gbc",
       {{{"display_name", "A"},
         {"icon_url", "file:system/_img/nes/a-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::EastFace}},
        {{"display_name", "B"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::SouthFace}},
        {{"display_name", "D-Pad Up"},
         {"icon_url", "file:system/_img/nes/dpad-up.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadUp}},
        {{"display_name", "D-Pad Down"},
         {"icon_url", "file:system/_img/nes/dpad-down.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadDown}},
        {{"display_name", "D-Pad Left"},
         {"icon_url", "file:system/_img/nes/dpad-left.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadLeft}},
        {{"display_name", "D-Pad Right"},
         {"icon_url", "file:system/_img/nes/dpad-right.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadRight}},
        {{"display_name", "Start"},
         {"icon_url", "file:system/_img/nes/start.svg"},
         {"retropad_button", libretro::IRetroPad::Input::Start}},
        {{"display_name", "Select"},
         {"icon_url", "file:system/_img/nes/select.svg"},
         {"retropad_button", libretro::IRetroPad::Input::Select}}},
       {}});
  m_items.push_back(
      {PlatformMetadata::PLATFORM_ID_GAMEBOY_ADVANCE,
       QString::fromStdString(PlatformMetadata::getPlatformName(
           PlatformMetadata::PLATFORM_ID_GAMEBOY_ADVANCE)),
       "qrc:images/platform-icons/gba",
       "platform-gba",
       {{{"display_name", "A"},
         {"icon_url", "file:system/_img/nes/a-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::EastFace}},
        {{"display_name", "B"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::SouthFace}},
        {{"display_name", "A (turbo)"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::NorthFace}},
        {{"display_name", "B (turbo)"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::WestFace}},
        {{"display_name", "L"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::LeftBumper}},
        {{"display_name", "R"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::RightBumper}},
        {{"display_name", "L (turbo)"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::LeftTrigger}},
        {{"display_name", "R (turbo)"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::RightTrigger}},
        {{"display_name", "Darken solar sensor"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::L3}},
        {{"display_name", "Brighten solar sensor"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::R3}},
        {{"display_name", "D-Pad Up"},
         {"icon_url", "file:system/_img/nes/dpad-up.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadUp}},
        {{"display_name", "D-Pad Down"},
         {"icon_url", "file:system/_img/nes/dpad-down.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadDown}},
        {{"display_name", "D-Pad Left"},
         {"icon_url", "file:system/_img/nes/dpad-left.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadLeft}},
        {{"display_name", "D-Pad Right"},
         {"icon_url", "file:system/_img/nes/dpad-right.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadRight}},
        {{"display_name", "Start"},
         {"icon_url", "file:system/_img/nes/start.svg"},
         {"retropad_button", libretro::IRetroPad::Input::Start}},
        {{"display_name", "Select"},
         {"icon_url", "file:system/_img/nes/select.svg"},
         {"retropad_button", libretro::IRetroPad::Input::Select}}},
       {}});
  m_items.push_back(
      {PlatformMetadata::PLATFORM_ID_N64,
       QString::fromStdString(PlatformMetadata::getPlatformName(
           PlatformMetadata::PLATFORM_ID_N64)),
       "qrc:images/platform-icons/n64",
       "platform-n64",
       {{{"display_name", "A"},
         {"icon_url", "file:system/_img/n64/a-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::SouthFace}},
        {{"display_name", "B"},
         {"icon_url", "file:system/_img/n64/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::WestFace}},
        {{"display_name", "C-Up"},
         {"icon_url", "file:system/_img/n64/c-up.svg"},
         {"retropad_button", libretro::IRetroPad::Input::RightStickUp}},
        {{"display_name", "C-Down"},
         {"icon_url", "file:system/_img/n64/c-down.svg"},
         {"retropad_button", libretro::IRetroPad::Input::RightStickDown}},
        {{"display_name", "C-Left"},
         {"icon_url", "file:system/_img/n64/c-left.svg"},
         {"retropad_button", libretro::IRetroPad::Input::RightStickLeft}},
        {{"display_name", "C-Right"},
         {"icon_url", "file:system/_img/n64/c-right.svg"},
         {"retropad_button", libretro::IRetroPad::Input::RightStickRight}},
        {{"display_name", "L"},
         {"icon_url", "file:system/_img/n64/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::LeftBumper}},
        {{"display_name", "R"},
         {"icon_url", "file:system/_img/n64/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::RightBumper}},
        {{"display_name", "Z"},
         {"icon_url", "file:system/_img/n64/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::LeftTrigger}},
        {{"display_name", "Start"},
         {"icon_url", "file:system/_img/n64/start.svg"},
         {"retropad_button", libretro::IRetroPad::Input::Start}},
        {{"display_name", "Analog Stick Up"},
         {"icon_url", "file:system/_img/n64/start.svg"},
         {"retropad_button", libretro::IRetroPad::Input::LeftStickUp}},
        {{"display_name", "Analog Stick Down"},
         {"icon_url", "file:system/_img/n64/start.svg"},
         {"retropad_button", libretro::IRetroPad::Input::LeftStickDown}},
        {{"display_name", "Analog Stick Left"},
         {"icon_url", "file:system/_img/n64/start.svg"},
         {"retropad_button", libretro::IRetroPad::Input::LeftStickLeft}},
        {{"display_name", "Analog Stick Right"},
         {"icon_url", "file:system/_img/n64/start.svg"},
         {"retropad_button", libretro::IRetroPad::Input::LeftStickRight}},
        {{"display_name", "D-Pad Up"},
         {"icon_url", "file:system/_img/n64/start.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadUp}},
        {{"display_name", "D-Pad Down"},
         {"icon_url", "file:system/_img/n64/start.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadDown}},
        {{"display_name", "D-Pad Left"},
         {"icon_url", "file:system/_img/n64/start.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadLeft}},
        {{"display_name", "D-Pad Right"},
         {"icon_url", "file:system/_img/n64/start.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadRight}}},
       {{{"display_name", "Control Stick"}, {"retropad_axis", 0}}}});

  m_items.push_back(
      {PlatformMetadata::PLATFORM_ID_NINTENDO_DS,
       QString::fromStdString(PlatformMetadata::getPlatformName(
           PlatformMetadata::PLATFORM_ID_NINTENDO_DS)),
       "qrc:images/platform-icons/ds",
       "platform-ds",
       {{{"display_name", "A"},
         {"icon_url", "file:system/_img/nes/a-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::EastFace}},
        {{"display_name", "B"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::SouthFace}},
        {{"display_name", "X"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::NorthFace}},
        {{"display_name", "Y"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::WestFace}},
        {{"display_name", "L"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::LeftBumper}},
        {{"display_name", "R"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::RightBumper}},
        {{"display_name", "D-Pad Up"},
         {"icon_url", "file:system/_img/nes/dpad-up.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadUp}},
        {{"display_name", "D-Pad Down"},
         {"icon_url", "file:system/_img/nes/dpad-down.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadDown}},
        {{"display_name", "D-Pad Left"},
         {"icon_url", "file:system/_img/nes/dpad-left.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadLeft}},
        {{"display_name", "D-Pad Right"},
         {"icon_url", "file:system/_img/nes/dpad-right.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadRight}},
        {{"display_name", "Start"},
         {"icon_url", "file:system/_img/nes/start.svg"},
         {"retropad_button", libretro::IRetroPad::Input::Start}},
        {{"display_name", "Select"},
         {"icon_url", "file:system/_img/nes/select.svg"},
         {"retropad_button", libretro::IRetroPad::Input::Select}}},
       {}});

  m_items.push_back(
      {PlatformMetadata::PLATFORM_ID_SEGA_MASTER_SYSTEM,
       QString::fromStdString(PlatformMetadata::getPlatformName(
           PlatformMetadata::PLATFORM_ID_SEGA_MASTER_SYSTEM)),
       "qrc:images/platform-icons/master-system",
       "platform-mastersystem",
       {{{"display_name", "1"},
         {"icon_url", "file:system/_img/nes/a-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::SouthFace}},
        {{"display_name", "2"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::EastFace}},
        {{"display_name", "Start"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::Start}},
        {{"display_name", "D-Pad Up"},
         {"icon_url", "file:system/_img/nes/dpad-up.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadUp}},
        {{"display_name", "D-Pad Down"},
         {"icon_url", "file:system/_img/nes/dpad-down.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadDown}},
        {{"display_name", "D-Pad Left"},
         {"icon_url", "file:system/_img/nes/dpad-left.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadLeft}},
        {{"display_name", "D-Pad Right"},
         {"icon_url", "file:system/_img/nes/dpad-right.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadRight}}},
       {}});

  m_items.push_back(
      {PlatformMetadata::PLATFORM_ID_SEGA_GENESIS,
       QString::fromStdString(PlatformMetadata::getPlatformName(
           PlatformMetadata::PLATFORM_ID_SEGA_GENESIS)),
       "qrc:images/platform-icons/genesis",
       "platform-genesis",
       {{{"display_name", "A"},
         {"icon_url", "file:system/_img/nes/a-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::WestFace}},
        {{"display_name", "B"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::SouthFace}},
        {{"display_name", "C"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::EastFace}},
        {{"display_name", "X"},
         {"icon_url", "file:system/_img/nes/a-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::LeftBumper}},
        {{"display_name", "Y"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::NorthFace}},
        {{"display_name", "Z"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::RightBumper}},
        {{"display_name", "Start"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::Start}},
        {{"display_name", "Mode"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::Select}},
        {{"display_name", "D-Pad Up"},
         {"icon_url", "file:system/_img/nes/dpad-up.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadUp}},
        {{"display_name", "D-Pad Down"},
         {"icon_url", "file:system/_img/nes/dpad-down.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadDown}},
        {{"display_name", "D-Pad Left"},
         {"icon_url", "file:system/_img/nes/dpad-left.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadLeft}},
        {{"display_name", "D-Pad Right"},
         {"icon_url", "file:system/_img/nes/dpad-right.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadRight}}},
       {}});

  m_items.push_back(
      {PlatformMetadata::PLATFORM_ID_SEGA_GAMEGEAR,
       QString::fromStdString(PlatformMetadata::getPlatformName(
           PlatformMetadata::PLATFORM_ID_SEGA_GAMEGEAR)),
       "qrc:images/platform-icons/gamegear",
       "platform-gamegear",
       {{{"display_name", "1"},
         {"icon_url", "file:system/_img/nes/a-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::SouthFace}},
        {{"display_name", "2"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::EastFace}},
        {{"display_name", "Start"},
         {"icon_url", "file:system/_img/nes/b-button.svg"},
         {"retropad_button", libretro::IRetroPad::Input::Start}},
        {{"display_name", "D-Pad Up"},
         {"icon_url", "file:system/_img/nes/dpad-up.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadUp}},
        {{"display_name", "D-Pad Down"},
         {"icon_url", "file:system/_img/nes/dpad-down.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadDown}},
        {{"display_name", "D-Pad Left"},
         {"icon_url", "file:system/_img/nes/dpad-left.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadLeft}},
        {{"display_name", "D-Pad Right"},
         {"icon_url", "file:system/_img/nes/dpad-right.svg"},
         {"retropad_button", libretro::IRetroPad::Input::DpadRight}}},
       {}});

  m_items.push_back({PlatformMetadata::PLATFORM_ID_TURBOGRAFX16,
                     QString::fromStdString(PlatformMetadata::getPlatformName(
                         PlatformMetadata::PLATFORM_ID_TURBOGRAFX16)),
                     "qrc:images/platform-icons/turbografx16",
                     "platform-turbografx16",
                     {}});

  m_items.push_back({PlatformMetadata::PLATFORM_ID_SUPERGRAFX,
                     QString::fromStdString(PlatformMetadata::getPlatformName(
                         PlatformMetadata::PLATFORM_ID_SUPERGRAFX)),
                     "qrc:images/platform-icons/supergrafx",
                     "platform-supergrafx",
                     {}});

  m_items.push_back({PlatformMetadata::PLATFORM_ID_WONDERSWAN,
                     QString::fromStdString(PlatformMetadata::getPlatformName(
                     PlatformMetadata::PLATFORM_ID_WONDERSWAN)),
                     "", // TODO: Get correct icons
                     "platform-wonderswan",
                     {{{"display_name", "A"},
                       {"icon_url", ""},
                       {"retropad_button", libretro::IRetroPad::Input::EastFace}},
                      {{"display_name", "B"},
                       {"icon_url", ""},
                       {"retropad_button", libretro::IRetroPad::Input::SouthFace}},
                      {{"display_name", "Start"},
                       {"icon_url", ""},
                       {"retropad_button", libretro::IRetroPad::Input::Start}},
                      {{"display_name", "X1 (Horizontal)"},
                       {"icon_url", ""},
                       {"retropad_button", libretro::IRetroPad::Input::DpadUp}},
                      {{"display_name", "X3 (Horizontal)"},
                       {"icon_url", ""},
                       {"retropad_button", libretro::IRetroPad::Input::DpadDown}},
                      {{"display_name", "X4 (Horizontal)"},
                       {"icon_url", ""},
                       {"retropad_button", libretro::IRetroPad::Input::DpadLeft}},
                      {{"display_name", "X2 (Horizontal)"},
                       {"icon_url", ""},
                       {"retropad_button", libretro::IRetroPad::Input::DpadRight}},
                      {{"display_name", "Y2"},
                       {"icon_url", ""},
                       {"retropad_button", libretro::IRetroPad::Input::DpadUp}},
                      {{"display_name", "Y4"},
                       {"icon_url", ""},
                       {"retropad_button", libretro::IRetroPad::Input::DpadDown}},
                      {{"display_name", "Y1"},
                       {"icon_url", ""},
                       {"retropad_button", libretro::IRetroPad::Input::DpadLeft}},
                      {{"display_name", "Y3"},
                       {"icon_url", ""},
                       {"retropad_button", libretro::IRetroPad::Input::DpadRight}},
                      {{"display_name", "X2 (Vertical)"},
                       {"icon_url", ""},
                       {"retropad_button", libretro::IRetroPad::Input::NorthFace}},
                      {{"display_name", "X4 (Vertical)"},
                       {"icon_url", ""},
                       {"retropad_button", libretro::IRetroPad::Input::SouthFace}},
                      {{"display_name", "X1 (Vertical)"},
                       {"icon_url", ""},
                       {"retropad_button", libretro::IRetroPad::Input::WestFace}},
                      {{"display_name", "X3 (Vertical)"},
                       {"icon_url", ""},
                       {"retropad_button", libretro::IRetroPad::Input::EastFace}},
                      {{"display_name", "Rotate Screen"},
                       {"icon_url", ""},
                       {"retropad_button", libretro::IRetroPad::Input::Select}}},
                      {}});
}

int PlatformListModel::rowCount(const QModelIndex &parent) const {
  return m_items.length();
}

QVariant PlatformListModel::data(const QModelIndex &index, int role) const {
  if (role < Qt::UserRole || index.row() >= m_items.size()) {
    return QVariant{};
  }

  auto item = m_items.at(index.row());

  switch (role) {
  case PlatformId:
    return item.platformId;
  case DisplayName:
    return item.displayName;
  case IconUrl:
    return item.iconUrl;
  case Buttons:
    return QVariant::fromValue(item.buttons);
  case Sticks:
    return QVariant::fromValue(item.sticks);
  default:
    return QVariant{};
  }
}

QHash<int, QByteArray> PlatformListModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[PlatformId] = "platform_id";
  roles[DisplayName] = "display_name";
  roles[IconUrl] = "icon_url";
  roles[Buttons] = "buttons";
  roles[Sticks] = "sticks";
  return roles;
}

Q_INVOKABLE QString
PlatformListModel::getPlatformIconName(int platformId) const {
  for (const auto &item : m_items) {
    if (item.platformId == platformId) {
      return item.iconName;
    }
  }

  return {};
}

Q_INVOKABLE QString
PlatformListModel::getPlatformDisplayName(int platformId) const {
  for (const auto &item : m_items) {
    if (item.platformId == platformId) {
      return item.displayName;
    }
  }

  return {};
}

} // namespace firelight::gui
