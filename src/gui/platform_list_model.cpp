#include "platform_list_model.hpp"

#include <QJsonObject>
#include <firelight/libretro/retropad.hpp>

namespace firelight::gui {
    PlatformListModel::PlatformListModel() {
        m_items.push_back({
            0,
            "NES",
            "file:system/_img/nes.svg",
            {
                {
                    {"display_name", "A"},
                    {"icon_url", "file:system/_img/nes/a-button.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "B"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "D-Pad Up"},
                    {"icon_url", "file:system/_img/nes/dpad-up.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "D-Pad Down"},
                    {"icon_url", "file:system/_img/nes/dpad-down.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "D-Pad Left"},
                    {"icon_url", "file:system/_img/nes/dpad-left.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "D-Pad Right"},
                    {"icon_url", "file:system/_img/nes/dpad-right.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "Start"},
                    {"icon_url", "file:system/_img/nes/start.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "Select"},
                    {"icon_url", "file:system/_img/nes/select.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                }
            },
            {}
        });
        m_items.push_back({
            1,
            "SNES",
            "file:system/_img/SNES.svg",
            {
                {
                    {"display_name", "A"},
                    {"icon_url", "file:system/_img/nes/a-button.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "B"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "X"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "Y"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "L"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "R"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "D-Pad Up"},
                    {"icon_url", "file:system/_img/nes/dpad-up.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "D-Pad Down"},
                    {"icon_url", "file:system/_img/nes/dpad-down.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "D-Pad Left"},
                    {"icon_url", "file:system/_img/nes/dpad-left.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "D-Pad Right"},
                    {"icon_url", "file:system/_img/nes/dpad-right.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "Start"},
                    {"icon_url", "file:system/_img/nes/start.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "Select"},
                    {"icon_url", "file:system/_img/nes/select.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                }
            },
            {}
        });
        m_items.push_back({
            2,
            "Game Boy",
            "file:system/_img/gb.svg",
            {
                {
                    {"display_name", "A"},
                    {"icon_url", "file:system/_img/nes/a-button.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "B"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "D-Pad Up"},
                    {"icon_url", "file:system/_img/nes/dpad-up.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "D-Pad Down"},
                    {"icon_url", "file:system/_img/nes/dpad-down.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "D-Pad Left"},
                    {"icon_url", "file:system/_img/nes/dpad-left.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "D-Pad Right"},
                    {"icon_url", "file:system/_img/nes/dpad-right.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "Start"},
                    {"icon_url", "file:system/_img/nes/start.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "Select"},
                    {"icon_url", "file:system/_img/nes/select.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                }
            },
            {}
        });
        m_items.push_back({
            3,
            "Game Boy Color",
            "file:system/_img/gbc.svg",
            {
                {
                    {"display_name", "A"},
                    {"icon_url", "file:system/_img/nes/a-button.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "B"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "D-Pad Up"},
                    {"icon_url", "file:system/_img/nes/dpad-up.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "D-Pad Down"},
                    {"icon_url", "file:system/_img/nes/dpad-down.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "D-Pad Left"},
                    {"icon_url", "file:system/_img/nes/dpad-left.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "D-Pad Right"},
                    {"icon_url", "file:system/_img/nes/dpad-right.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "Start"},
                    {"icon_url", "file:system/_img/nes/start.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "Select"},
                    {"icon_url", "file:system/_img/nes/select.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                }
            },
            {}
        });
        m_items.push_back({
            4,
            "Game Boy Advance",
            "file:system/_img/gba.svg",
            {
                {
                    {"display_name", "A"},
                    {"icon_url", "file:system/_img/nes/a-button.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "B"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "L"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "R"},
                    {"icon_url", "file:system/_img/nes/b-button.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "D-Pad Up"},
                    {"icon_url", "file:system/_img/nes/dpad-up.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "D-Pad Down"},
                    {"icon_url", "file:system/_img/nes/dpad-down.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "D-Pad Left"},
                    {"icon_url", "file:system/_img/nes/dpad-left.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "D-Pad Right"},
                    {"icon_url", "file:system/_img/nes/dpad-right.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "Start"},
                    {"icon_url", "file:system/_img/nes/start.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                },
                {
                    {"display_name", "Select"},
                    {"icon_url", "file:system/_img/nes/select.svg"},
                    {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                }
            },
            {}
        });
        m_items.push_back({
                5,
                "Nintendo 64",
                "file:system/_img/N64.svg",
                {
                    {
                        {"display_name", "A"},
                        {"icon_url", "file:system/_img/n64/a-button.svg"},
                        {"retropad_button_id", libretro::IRetroPad::Button::SouthFace}
                    },
                    {
                        {"display_name", "B"},
                        {"icon_url", "file:system/_img/n64/b-button.svg"},
                        {"retropad_button_id", libretro::IRetroPad::Button::WestFace}
                    },
                    {
                        {"display_name", "C-Up"},
                        {"icon_url", "file:system/_img/n64/c-up.svg"},
                        {"retropad_button_id", libretro::IRetroPad::Button::WestFace}
                    },
                    {
                        {"display_name", "C-Down"},
                        {"icon_url", "file:system/_img/n64/c-down.svg"},
                        {"retropad_button_id", libretro::IRetroPad::Button::WestFace}
                    },
                    {
                        {"display_name", "C-Left"},
                        {"icon_url", "file:system/_img/n64/c-left.svg"},
                        {"retropad_button_id", libretro::IRetroPad::Button::WestFace}
                    },
                    {
                        {"display_name", "C-Right"},
                        {"icon_url", "file:system/_img/n64/c-right.svg"},
                        {"retropad_button_id", libretro::IRetroPad::Button::WestFace}
                    },
                    {
                        {"display_name", "L"},
                        {"icon_url", "file:system/_img/n64/b-button.svg"},
                        {"retropad_button_id", libretro::IRetroPad::Button::WestFace}
                    },
                    {
                        {"display_name", "R"},
                        {"icon_url", "file:system/_img/n64/b-button.svg"},
                        {"retropad_button_id", libretro::IRetroPad::Button::WestFace}
                    },
                    {
                        {"display_name", "Z"},
                        {"icon_url", "file:system/_img/n64/b-button.svg"},
                        {"retropad_button_id", libretro::IRetroPad::Button::WestFace}
                    },
                    {
                        {"display_name", "Start"},
                        {"icon_url", "file:system/_img/n64/start.svg"},
                        {"retropad_button_id", libretro::IRetroPad::Button::WestFace}
                    }
                },
                {
                    {
                        {"display_name", "Control Stick"},
                        {"retropad_axis", libretro::IRetroPad::Stick::Left}
                    }
                }
            }

        );
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
