#include "platform_list_model.hpp"

#include <QJsonObject>
#include <firelight/libretro/retropad.hpp>

#include "../app/platform_metadata.hpp"

namespace firelight::gui {
    PlatformListModel::PlatformListModel() {
        m_items.push_back({
            PlatformMetadata::PLATFORM_ID_NES,
            "NES",
            "qrc:images/platform-icons/nes",
            {
                {
                    {"display_name", "A"},
                    {"icon_url", "file:system/_img/nes/a-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::EastFace}
                },
                {
                    {"display_name", "B"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::SouthFace}
                },
                {
                    {"display_name", "D-Pad Up"},
                    {"icon_url", "file:system/_img/nes/dpad-up.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadUp}
                },
                {
                    {"display_name", "D-Pad Down"},
                    {"icon_url", "file:system/_img/nes/dpad-down.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadDown}
                },
                {
                    {"display_name", "D-Pad Left"},
                    {"icon_url", "file:system/_img/nes/dpad-left.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadLeft}
                },
                {
                    {"display_name", "D-Pad Right"},
                    {"icon_url", "file:system/_img/nes/dpad-right.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadRight}
                },
                {
                    {"display_name", "Start"},
                    {"icon_url", "file:system/_img/nes/start.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::Start}
                },
                {
                    {"display_name", "Select"},
                    {"icon_url", "file:system/_img/nes/select.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::Select}
                }
            },
            {}
        });
        m_items.push_back({
            PlatformMetadata::PLATFORM_ID_SNES,
            "SNES",
            "qrc:images/platform-icons/snes",
            {
                {
                    {"display_name", "A"},
                    {"icon_url", "file:system/_img/nes/a-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::EastFace}
                },
                {
                    {"display_name", "B"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::SouthFace}
                },
                {
                    {"display_name", "X"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::NorthFace}
                },
                {
                    {"display_name", "Y"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::WestFace}
                },
                {
                    {"display_name", "L"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::LeftBumper}
                },
                {
                    {"display_name", "R"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::RightBumper}
                },
                {
                    {"display_name", "D-Pad Up"},
                    {"icon_url", "file:system/_img/nes/dpad-up.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadUp}
                },
                {
                    {"display_name", "D-Pad Down"},
                    {"icon_url", "file:system/_img/nes/dpad-down.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadDown}
                },
                {
                    {"display_name", "D-Pad Left"},
                    {"icon_url", "file:system/_img/nes/dpad-left.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadLeft}
                },
                {
                    {"display_name", "D-Pad Right"},
                    {"icon_url", "file:system/_img/nes/dpad-right.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadRight}
                },
                {
                    {"display_name", "Start"},
                    {"icon_url", "file:system/_img/nes/start.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::Start}
                },
                {
                    {"display_name", "Select"},
                    {"icon_url", "file:system/_img/nes/select.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::Select}
                }
            },
            {}
        });
        m_items.push_back({
            PlatformMetadata::PLATFORM_ID_GAMEBOY,
            "Game Boy",
            "qrc:images/platform-icons/gb",
            {
                {
                    {"display_name", "A"},
                    {"icon_url", "file:system/_img/nes/a-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::EastFace}
                },
                {
                    {"display_name", "B"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::SouthFace}
                },
                {
                    {"display_name", "D-Pad Up"},
                    {"icon_url", "file:system/_img/nes/dpad-up.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadUp}
                },
                {
                    {"display_name", "D-Pad Down"},
                    {"icon_url", "file:system/_img/nes/dpad-down.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadDown}
                },
                {
                    {"display_name", "D-Pad Left"},
                    {"icon_url", "file:system/_img/nes/dpad-left.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadLeft}
                },
                {
                    {"display_name", "D-Pad Right"},
                    {"icon_url", "file:system/_img/nes/dpad-right.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadRight}
                },
                {
                    {"display_name", "Start"},
                    {"icon_url", "file:system/_img/nes/start.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::Start}
                },
                {
                    {"display_name", "Select"},
                    {"icon_url", "file:system/_img/nes/select.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::Select}
                }
            },
            {}
        });
        m_items.push_back({
            PlatformMetadata::PLATFORM_ID_GAMEBOY_COLOR,
            "Game Boy Color",
            "qrc:images/platform-icons/gbc",
            {
                {
                    {"display_name", "A"},
                    {"icon_url", "file:system/_img/nes/a-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::EastFace}
                },
                {
                    {"display_name", "B"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::SouthFace}
                },
                {
                    {"display_name", "D-Pad Up"},
                    {"icon_url", "file:system/_img/nes/dpad-up.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadUp}
                },
                {
                    {"display_name", "D-Pad Down"},
                    {"icon_url", "file:system/_img/nes/dpad-down.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadDown}
                },
                {
                    {"display_name", "D-Pad Left"},
                    {"icon_url", "file:system/_img/nes/dpad-left.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadLeft}
                },
                {
                    {"display_name", "D-Pad Right"},
                    {"icon_url", "file:system/_img/nes/dpad-right.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadRight}
                },
                {
                    {"display_name", "Start"},
                    {"icon_url", "file:system/_img/nes/start.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::Start}
                },
                {
                    {"display_name", "Select"},
                    {"icon_url", "file:system/_img/nes/select.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::Select}
                }
            },
            {}
        });
        m_items.push_back({
            PlatformMetadata::PLATFORM_ID_GAMEBOY_ADVANCE,
            "Game Boy Advance",
            "qrc:images/platform-icons/gba",
            {
                {
                    {"display_name", "A"},
                    {"icon_url", "file:system/_img/nes/a-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::EastFace}
                },
                {
                    {"display_name", "B"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::SouthFace}
                },
                {
                    {"display_name", "L"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::LeftBumper}
                },
                {
                    {"display_name", "R"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::RightBumper}
                },
                {
                    {"display_name", "D-Pad Up"},
                    {"icon_url", "file:system/_img/nes/dpad-up.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadUp}
                },
                {
                    {"display_name", "D-Pad Down"},
                    {"icon_url", "file:system/_img/nes/dpad-down.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadDown}
                },
                {
                    {"display_name", "D-Pad Left"},
                    {"icon_url", "file:system/_img/nes/dpad-left.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadLeft}
                },
                {
                    {"display_name", "D-Pad Right"},
                    {"icon_url", "file:system/_img/nes/dpad-right.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadRight}
                },
                {
                    {"display_name", "Start"},
                    {"icon_url", "file:system/_img/nes/start.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::Start}
                },
                {
                    {"display_name", "Select"},
                    {"icon_url", "file:system/_img/nes/select.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::Select}
                }
            },
            {}
        });
        m_items.push_back({
                PlatformMetadata::PLATFORM_ID_N64,
                "Nintendo 64",
                "qrc:images/platform-icons/n64",
                {
                    {
                        {"display_name", "A"},
                        {"icon_url", "file:system/_img/n64/a-button.svg"},
                        {"retropad_button", libretro::IRetroPad::Input::SouthFace}
                    },
                    {
                        {"display_name", "B"},
                        {"icon_url", "file:system/_img/n64/b-button.svg"},
                        {"retropad_button", libretro::IRetroPad::Input::WestFace}
                    },
                    {
                        {"display_name", "C-Up"},
                        {"icon_url", "file:system/_img/n64/c-up.svg"},
                        {"retropad_button", libretro::IRetroPad::Input::RightStickUp}
                    },
                    {
                        {"display_name", "C-Down"},
                        {"icon_url", "file:system/_img/n64/c-down.svg"},
                        {"retropad_button", libretro::IRetroPad::Input::RightStickDown}
                    },
                    {
                        {"display_name", "C-Left"},
                        {"icon_url", "file:system/_img/n64/c-left.svg"},
                        {"retropad_button", libretro::IRetroPad::Input::RightStickLeft}
                    },
                    {
                        {"display_name", "C-Right"},
                        {"icon_url", "file:system/_img/n64/c-right.svg"},
                        {"retropad_button", libretro::IRetroPad::Input::RightStickRight}
                    },
                    {
                        {"display_name", "L"},
                        {"icon_url", "file:system/_img/n64/b-button.svg"},
                        {"retropad_button", libretro::IRetroPad::Input::LeftBumper}
                    },
                    {
                        {"display_name", "R"},
                        {"icon_url", "file:system/_img/n64/b-button.svg"},
                        {"retropad_button", libretro::IRetroPad::Input::RightBumper}
                    },
                    {
                        {"display_name", "Z"},
                        {"icon_url", "file:system/_img/n64/b-button.svg"},
                        {"retropad_button", libretro::IRetroPad::Input::LeftTrigger}
                    },
                    {
                        {"display_name", "Start"},
                        {"icon_url", "file:system/_img/n64/start.svg"},
                        {"retropad_button", libretro::IRetroPad::Input::Start}
                    },
                    {
                        {"display_name", "Analog Stick Up"},
                        {"icon_url", "file:system/_img/n64/start.svg"},
                        {"retropad_button", libretro::IRetroPad::Input::LeftStickUp}
                    },
                    {
                        {"display_name", "Analog Stick Down"},
                        {"icon_url", "file:system/_img/n64/start.svg"},
                        {"retropad_button", libretro::IRetroPad::Input::LeftStickDown}
                    },
                    {
                        {"display_name", "Analog Stick Left"},
                        {"icon_url", "file:system/_img/n64/start.svg"},
                        {"retropad_button", libretro::IRetroPad::Input::LeftStickLeft}
                    },
                    {
                        {"display_name", "Analog Stick Right"},
                        {"icon_url", "file:system/_img/n64/start.svg"},
                        {"retropad_button", libretro::IRetroPad::Input::LeftStickRight}
                    }
                },
                {
                    {
                        {"display_name", "Control Stick"},
                        {"retropad_axis", 0}
                    }
                }
            }
        );

        m_items.push_back({
            PlatformMetadata::PLATFORM_ID_NINTENDO_DS,
            "Nintendo DS",
            "qrc:images/platform-icons/ds",
            {
                {
                    {"display_name", "A"},
                    {"icon_url", "file:system/_img/nes/a-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::EastFace}
                },
                {
                    {"display_name", "B"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::SouthFace}
                },
                {
                    {"display_name", "X"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::NorthFace}
                },
                {
                    {"display_name", "Y"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::WestFace}
                },
                {
                    {"display_name", "L"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::LeftBumper}
                },
                {
                    {"display_name", "R"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::RightBumper}
                },
                {
                    {"display_name", "D-Pad Up"},
                    {"icon_url", "file:system/_img/nes/dpad-up.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadUp}
                },
                {
                    {"display_name", "D-Pad Down"},
                    {"icon_url", "file:system/_img/nes/dpad-down.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadDown}
                },
                {
                    {"display_name", "D-Pad Left"},
                    {"icon_url", "file:system/_img/nes/dpad-left.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadLeft}
                },
                {
                    {"display_name", "D-Pad Right"},
                    {"icon_url", "file:system/_img/nes/dpad-right.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadRight}
                },
                {
                    {"display_name", "Start"},
                    {"icon_url", "file:system/_img/nes/start.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::Start}
                },
                {
                    {"display_name", "Select"},
                    {"icon_url", "file:system/_img/nes/select.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::Select}
                }
            },
            {}
        });

        m_items.push_back({
            PlatformMetadata::PLATFORM_ID_SEGA_MASTER_SYSTEM,
            "Master System",
            "qrc:images/platform-icons/master-system",
            {
                {
                    {"display_name", "1"},
                    {"icon_url", "file:system/_img/nes/a-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::SouthFace}
                },
                {
                    {"display_name", "2"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::EastFace}
                },
                {
                    {"display_name", "Start"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::Start}
                },
                {
                    {"display_name", "D-Pad Up"},
                    {"icon_url", "file:system/_img/nes/dpad-up.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadUp}
                },
                {
                    {"display_name", "D-Pad Down"},
                    {"icon_url", "file:system/_img/nes/dpad-down.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadDown}
                },
                {
                    {"display_name", "D-Pad Left"},
                    {"icon_url", "file:system/_img/nes/dpad-left.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadLeft}
                },
                {
                    {"display_name", "D-Pad Right"},
                    {"icon_url", "file:system/_img/nes/dpad-right.svg"},
                    {"retropad_button", libretro::IRetroPad::Input::DpadRight}
                }
            },
            {}
        });

        m_items.push_back({
            PlatformMetadata::PLATFORM_ID_SEGA_GENESIS,
            "Genesis",
            "qrc:images/platform-icons/genesis",
            {
                {
                      {"display_name", "A"},
                      {"icon_url", "file:system/_img/nes/a-button.svg"},
                      {"retropad_button", libretro::IRetroPad::Input::WestFace}
                  },
                  {
                      {"display_name", "B"},
                      {"icon_url", "file:system/_img/nes/b-button.svg"},
                      {"retropad_button", libretro::IRetroPad::Input::SouthFace}
                  },
                  {
                      {"display_name", "C"},
                      {"icon_url", "file:system/_img/nes/b-button.svg"},
                      {"retropad_button", libretro::IRetroPad::Input::EastFace}
                  },
                {
                      {"display_name", "X"},
                      {"icon_url", "file:system/_img/nes/a-button.svg"},
                      {"retropad_button", libretro::IRetroPad::Input::LeftBumper}
                  },
                  {
                      {"display_name", "Y"},
                      {"icon_url", "file:system/_img/nes/b-button.svg"},
                      {"retropad_button", libretro::IRetroPad::Input::NorthFace}
                  },
                  {
                      {"display_name", "Z"},
                      {"icon_url", "file:system/_img/nes/b-button.svg"},
                      {"retropad_button", libretro::IRetroPad::Input::RightBumper}
                  },
                  {
                      {"display_name", "Start"},
                      {"icon_url", "file:system/_img/nes/b-button.svg"},
                      {"retropad_button", libretro::IRetroPad::Input::Start}
                  },
                  {
                      {"display_name", "Mode"},
                      {"icon_url", "file:system/_img/nes/b-button.svg"},
                      {"retropad_button", libretro::IRetroPad::Input::Select}
                  },
                  {
                      {"display_name", "D-Pad Up"},
                      {"icon_url", "file:system/_img/nes/dpad-up.svg"},
                      {"retropad_button", libretro::IRetroPad::Input::DpadUp}
                  },
                  {
                      {"display_name", "D-Pad Down"},
                      {"icon_url", "file:system/_img/nes/dpad-down.svg"},
                      {"retropad_button", libretro::IRetroPad::Input::DpadDown}
                  },
                  {
                      {"display_name", "D-Pad Left"},
                      {"icon_url", "file:system/_img/nes/dpad-left.svg"},
                      {"retropad_button", libretro::IRetroPad::Input::DpadLeft}
                  },
                  {
                      {"display_name", "D-Pad Right"},
                      {"icon_url", "file:system/_img/nes/dpad-right.svg"},
                      {"retropad_button", libretro::IRetroPad::Input::DpadRight}
                  }
            },
            {}
        });

        m_items.push_back({
            PlatformMetadata::PLATFORM_ID_SEGA_GAMEGEAR,
            "Game Gear",
            "qrc:images/platform-icons/gamegear",
            {
              {
                      {"display_name", "1"},
                      {"icon_url", "file:system/_img/nes/a-button.svg"},
                      {"retropad_button", libretro::IRetroPad::Input::SouthFace}
                  },
                  {
                      {"display_name", "2"},
                      {"icon_url", "file:system/_img/nes/b-button.svg"},
                      {"retropad_button", libretro::IRetroPad::Input::EastFace}
                  },
                  {
                      {"display_name", "Start"},
                      {"icon_url", "file:system/_img/nes/b-button.svg"},
                      {"retropad_button", libretro::IRetroPad::Input::Start}
                  },
                  {
                      {"display_name", "D-Pad Up"},
                      {"icon_url", "file:system/_img/nes/dpad-up.svg"},
                      {"retropad_button", libretro::IRetroPad::Input::DpadUp}
                  },
                  {
                      {"display_name", "D-Pad Down"},
                      {"icon_url", "file:system/_img/nes/dpad-down.svg"},
                      {"retropad_button", libretro::IRetroPad::Input::DpadDown}
                  },
                  {
                      {"display_name", "D-Pad Left"},
                      {"icon_url", "file:system/_img/nes/dpad-left.svg"},
                      {"retropad_button", libretro::IRetroPad::Input::DpadLeft}
                  },
                  {
                      {"display_name", "D-Pad Right"},
                      {"icon_url", "file:system/_img/nes/dpad-right.svg"},
                      {"retropad_button", libretro::IRetroPad::Input::DpadRight}
                  }
            },
            {}
        });
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
} // firelight::gui
