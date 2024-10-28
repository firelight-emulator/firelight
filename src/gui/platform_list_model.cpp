#include "platform_list_model.hpp"

#include <QJsonObject>
#include <firelight/libretro/retropad.hpp>

#include "../app/input/input_mapping.hpp"

namespace firelight::gui {
    PlatformListModel::PlatformListModel() {
        // m_items.push_back({
        //     0,
        //     "NES",
        //         "qrc:images/platform-icons/nes.svg",
        //     {
        //         {
        //             {"display_name", "A"},
        //             {"icon_url", "file:system/_img/nes/a-button.svg"},
        //             {"mapping_id", input::InputMapping::Things::SOUTH_FACE}
        //         },
        //         {
        //             {"display_name", "B"},
        //             {"icon_url", "file:system/_img/nes/b-button.svg"},
        //             {"mapping_id", input::InputMapping::Things::EAST_FACE}
        //         },
        //         {
        //             {"display_name", "D-Pad Up"},
        //             {"icon_url", "file:system/_img/nes/dpad-up.svg"},
        //             {"mapping_id", input::InputMapping::Things::DPAD_UP}
        //         },
        //         {
        //             {"display_name", "D-Pad Down"},
        //             {"icon_url", "file:system/_img/nes/dpad-down.svg"},
        //             {"mapping_id", input::InputMapping::Things::DPAD_DOWN}
        //         },
        //         {
        //             {"display_name", "D-Pad Left"},
        //             {"icon_url", "file:system/_img/nes/dpad-left.svg"},
        //             {"mapping_id", input::InputMapping::Things::DPAD_LEFT}
        //         },
        //         {
        //             {"display_name", "D-Pad Right"},
        //             {"icon_url", "file:system/_img/nes/dpad-right.svg"},
        //             {"mapping_id", input::InputMapping::Things::DPAD_RIGHT}
        //         },
        //         {
        //             {"display_name", "Start"},
        //             {"icon_url", "file:system/_img/nes/start.svg"},
        //             {"mapping_id", input::InputMapping::Things::START}
        //         },
        //         {
        //             {"display_name", "Select"},
        //             {"icon_url", "file:system/_img/nes/select.svg"},
        //             {"mapping_id", input::InputMapping::Things::SELECT}
        //         }
        //     },
        //     {}
        // });
        // m_items.push_back({
        //     1,
        //     "SNES",
        //         "qrc:images/platform-icons/snes.svg",
        //     {
        //         {
        //             {"display_name", "A"},
        //             {"icon_url", "file:system/_img/nes/a-button.svg"},
        //             {"mapping_id", input::InputMapping::Things::EAST_FACE}
        //         },
        //         {
        //             {"display_name", "B"},
        //             {"icon_url", "file:system/_img/nes/b-button.svg"},
        //             {"mapping_id", input::InputMapping::Things::SOUTH_FACE}
        //         },
        //         {
        //             {"display_name", "X"},
        //             {"icon_url", "file:system/_img/nes/b-button.svg"},
        //             {"mapping_id", input::InputMapping::Things::NORTH_FACE}
        //         },
        //         {
        //             {"display_name", "Y"},
        //             {"icon_url", "file:system/_img/nes/b-button.svg"},
        //             {"mapping_id", input::InputMapping::Things::WEST_FACE}
        //         },
        //         {
        //             {"display_name", "L"},
        //             {"icon_url", "file:system/_img/nes/b-button.svg"},
        //             {"mapping_id", input::InputMapping::Things::L1}
        //         },
        //         {
        //             {"display_name", "R"},
        //             {"icon_url", "file:system/_img/nes/b-button.svg"},
        //             {"mapping_id", input::InputMapping::Things::R1}
        //         },
        //         {
        //             {"display_name", "D-Pad Up"},
        //             {"icon_url", "file:system/_img/nes/dpad-up.svg"},
        //             {"mapping_id", input::InputMapping::Things::DPAD_UP}
        //         },
        //         {
        //             {"display_name", "D-Pad Down"},
        //             {"icon_url", "file:system/_img/nes/dpad-down.svg"},
        //             {"mapping_id", input::InputMapping::Things::DPAD_DOWN}
        //         },
        //         {
        //             {"display_name", "D-Pad Left"},
        //             {"icon_url", "file:system/_img/nes/dpad-left.svg"},
        //             {"mapping_id", input::InputMapping::Things::DPAD_LEFT}
        //         },
        //         {
        //             {"display_name", "D-Pad Right"},
        //             {"icon_url", "file:system/_img/nes/dpad-right.svg"},
        //             {"mapping_id", input::InputMapping::Things::DPAD_RIGHT}
        //         },
        //         {
        //             {"display_name", "Start"},
        //             {"icon_url", "file:system/_img/nes/start.svg"},
        //             {"mapping_id", input::InputMapping::Things::START}
        //         },
        //         {
        //             {"display_name", "Select"},
        //             {"icon_url", "file:system/_img/nes/select.svg"},
        //             {"mapping_id", input::InputMapping::Things::SELECT}
        //         }
        //     },
        //     {}
        // });
        // m_items.push_back({
        //     2,
        //     "Game Boy",
        //         "qrc:images/platform-icons/gb.svg",
        //     {
        //         {
        //             {"display_name", "A"},
        //             {"icon_url", "file:system/_img/nes/a-button.svg"},
        //             {"mapping_id", input::InputMapping::Things::SOUTH_FACE}
        //         },
        //         {
        //             {"display_name", "B"},
        //             {"icon_url", "file:system/_img/nes/b-button.svg"},
        //             {"mapping_id", input::InputMapping::Things::EAST_FACE}
        //         },
        //         {
        //             {"display_name", "D-Pad Up"},
        //             {"icon_url", "file:system/_img/nes/dpad-up.svg"},
        //             {"mapping_id", input::InputMapping::Things::DPAD_UP}
        //         },
        //         {
        //             {"display_name", "D-Pad Down"},
        //             {"icon_url", "file:system/_img/nes/dpad-down.svg"},
        //             {"mapping_id", input::InputMapping::Things::DPAD_DOWN}
        //         },
        //         {
        //             {"display_name", "D-Pad Left"},
        //             {"icon_url", "file:system/_img/nes/dpad-left.svg"},
        //             {"mapping_id", input::InputMapping::Things::DPAD_LEFT}
        //         },
        //         {
        //             {"display_name", "D-Pad Right"},
        //             {"icon_url", "file:system/_img/nes/dpad-right.svg"},
        //             {"mapping_id", input::InputMapping::Things::DPAD_RIGHT}
        //         },
        //         {
        //             {"display_name", "Start"},
        //             {"icon_url", "file:system/_img/nes/start.svg"},
        //             {"mapping_id", input::InputMapping::Things::START}
        //         },
        //         {
        //             {"display_name", "Select"},
        //             {"icon_url", "file:system/_img/nes/select.svg"},
        //             {"mapping_id", input::InputMapping::Things::SELECT}
        //         }
        //     },
        //     {}
        // });
        // m_items.push_back({
        //     3,
        //     "Game Boy Color",
        //         "qrc:images/platform-icons/gbc.svg",
        //     {
        //         {
        //             {"display_name", "A"},
        //             {"icon_url", "file:system/_img/nes/a-button.svg"},
        //             {"mapping_id", input::InputMapping::Things::SOUTH_FACE}
        //         },
        //         {
        //             {"display_name", "B"},
        //             {"icon_url", "file:system/_img/nes/b-button.svg"},
        //             {"mapping_id", input::InputMapping::Things::EAST_FACE}
        //         },
        //         {
        //             {"display_name", "D-Pad Up"},
        //             {"icon_url", "file:system/_img/nes/dpad-up.svg"},
        //             {"mapping_id", input::InputMapping::Things::DPAD_UP}
        //         },
        //         {
        //             {"display_name", "D-Pad Down"},
        //             {"icon_url", "file:system/_img/nes/dpad-down.svg"},
        //             {"mapping_id", input::InputMapping::Things::DPAD_DOWN}
        //         },
        //         {
        //             {"display_name", "D-Pad Left"},
        //             {"icon_url", "file:system/_img/nes/dpad-left.svg"},
        //             {"mapping_id", input::InputMapping::Things::DPAD_LEFT}
        //         },
        //         {
        //             {"display_name", "D-Pad Right"},
        //             {"icon_url", "file:system/_img/nes/dpad-right.svg"},
        //             {"mapping_id", input::InputMapping::Things::DPAD_RIGHT}
        //         },
        //         {
        //             {"display_name", "Start"},
        //             {"icon_url", "file:system/_img/nes/start.svg"},
        //             {"mapping_id", input::InputMapping::Things::START}
        //         },
        //         {
        //             {"display_name", "Select"},
        //             {"icon_url", "file:system/_img/nes/select.svg"},
        //             {"mapping_id", input::InputMapping::Things::SELECT}
        //         }
        //     },
        //     {}
        // });
        // m_items.push_back({
        //     4,
        //     "Game Boy Advance",
        //         "qrc:images/platform-icons/gba.svg",
        //     {
        //         {
        //             {"display_name", "A"},
        //             {"icon_url", "file:system/_img/nes/a-button.svg"},
        //             {"mapping_id", input::InputMapping::Things::SOUTH_FACE}
        //         },
        //         {
        //             {"display_name", "B"},
        //             {"icon_url", "file:system/_img/nes/b-button.svg"},
        //             {"mapping_id", input::InputMapping::Things::EAST_FACE}
        //         },
        //         {
        //             {"display_name", "L"},
        //             {"icon_url", "file:system/_img/nes/b-button.svg"},
        //             {"mapping_id", input::InputMapping::Things::L1}
        //         },
        //         {
        //             {"display_name", "R"},
        //             {"icon_url", "file:system/_img/nes/b-button.svg"},
        //             {"mapping_id", input::InputMapping::Things::R1}
        //         },
        //         {
        //             {"display_name", "D-Pad Up"},
        //             {"icon_url", "file:system/_img/nes/dpad-up.svg"},
        //             {"mapping_id", input::InputMapping::Things::DPAD_UP}
        //         },
        //         {
        //             {"display_name", "D-Pad Down"},
        //             {"icon_url", "file:system/_img/nes/dpad-down.svg"},
        //             {"mapping_id", input::InputMapping::Things::DPAD_DOWN}
        //         },
        //         {
        //             {"display_name", "D-Pad Left"},
        //             {"icon_url", "file:system/_img/nes/dpad-left.svg"},
        //             {"mapping_id", input::InputMapping::Things::DPAD_LEFT}
        //         },
        //         {
        //             {"display_name", "D-Pad Right"},
        //             {"icon_url", "file:system/_img/nes/dpad-right.svg"},
        //             {"mapping_id", input::InputMapping::Things::DPAD_RIGHT}
        //         },
        //         {
        //             {"display_name", "Start"},
        //             {"icon_url", "file:system/_img/nes/start.svg"},
        //             {"mapping_id", input::InputMapping::Things::START}
        //         },
        //         {
        //             {"display_name", "Select"},
        //             {"icon_url", "file:system/_img/nes/select.svg"},
        //             {"mapping_id", input::InputMapping::Things::SELECT}
        //         }
        //     },
        //     {}
        // });
        // m_items.push_back({
        //         5,
        //         "Nintendo 64",
        //         "qrc:images/platform-icons/n64.svg",
        //         {
        //             {
        //                 {"display_name", "A"},
        //                 {"icon_url", "file:system/_img/n64/a-button.svg"},
        //                 {"mapping_id", input::InputMapping::Things::SOUTH_FACE}
        //             },
        //             {
        //                 {"display_name", "B"},
        //                 {"icon_url", "file:system/_img/n64/b-button.svg"},
        //                 {"mapping_id", input::InputMapping::Things::WEST_FACE}
        //             },
        //             {
        //                 {"display_name", "C-Up"},
        //                 {"icon_url", "file:system/_img/n64/c-up.svg"},
        //                 {"mapping_id", input::InputMapping::Things::RIGHT_STICK_UP}
        //             },
        //             {
        //                 {"display_name", "C-Down"},
        //                 {"icon_url", "file:system/_img/n64/c-down.svg"},
        //                 {"mapping_id", input::InputMapping::Things::RIGHT_STICK_DOWN}
        //             },
        //             {
        //                 {"display_name", "C-Left"},
        //                 {"icon_url", "file:system/_img/n64/c-left.svg"},
        //                 {"mapping_id", input::InputMapping::Things::RIGHT_STICK_LEFT}
        //             },
        //             {
        //                 {"display_name", "C-Right"},
        //                 {"icon_url", "file:system/_img/n64/c-right.svg"},
        //                 {"mapping_id", input::InputMapping::Things::RIGHT_STICK_RIGHT}
        //             },
        //             {
        //                 {"display_name", "L"},
        //                 {"icon_url", "file:system/_img/n64/b-button.svg"},
        //                 {"mapping_id", input::InputMapping::Things::L1}
        //             },
        //             {
        //                 {"display_name", "R"},
        //                 {"icon_url", "file:system/_img/n64/b-button.svg"},
        //                 {"mapping_id", input::InputMapping::Things::R1}
        //             },
        //             {
        //                 {"display_name", "Z"},
        //                 {"icon_url", "file:system/_img/n64/b-button.svg"},
        //                 {"mapping_id", input::InputMapping::Things::L2}
        //             },
        //             {
        //                 {"display_name", "Start"},
        //                 {"icon_url", "file:system/_img/n64/start.svg"},
        //                 {"mapping_id", input::InputMapping::Things::START}
        //             }
        //         },
        //         {
        //             {
        //                 {"display_name", "Control Stick"},
        //                 {"retropad_axis", libretro::IRetroPad::Stick::Left}
        //             }
        //         }
        //     }
        //
        // );
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
