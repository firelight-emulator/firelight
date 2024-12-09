//
// Created by alexs on 6/20/2024.
//

#include "PlatformInputListModel.hpp"

PlatformInputListModel::PlatformInputListModel(QObject *parent) {
    m_items.push_back({
        firelight::libretro::IRetroPad::Input::SouthFace, "file:system/_img/n64/a-button.svg", "A"
    });
    m_items.push_back(
        {firelight::libretro::IRetroPad::Input::EastFace, "file:system/_img/n64/b-button.svg", "B"});
    // m_items.push_back(
    //     {firelight::libretro::IRetroPad::Button::WestFace, "file:system/_img/n64/xbutton.svg", "X"});
    // m_items.push_back({
    //     firelight::libretro::IRetroPad::Button::NorthFace, "file:system/_img/n64/ybutton.svg", "Y"
    // });

    m_items.push_back({
        firelight::libretro::IRetroPad::Input::DpadLeft, "file:system/_img/n64/c-up.svg", "C-Up"
    });
    m_items.push_back({
        firelight::libretro::IRetroPad::Input::DpadUp, "file:system/_img/n64/c-down.svg", "C-Down"
    });
    m_items.push_back({
        firelight::libretro::IRetroPad::Input::DpadDown, "file:system/_img/n64/c-left.svg", "C-Left"
    });
    m_items.push_back({
        firelight::libretro::IRetroPad::Input::DpadRight, "file:system/_img/n64/c-right.svg", "C-Right"
    });
    m_items.push_back({
        firelight::libretro::IRetroPad::Input::Start, "file:system/_img/n64/start.svg", "Start"
    });
    // m_items.push_back({
    //     firelight::libretro::IRetroPad::Button::Select, "file:system/_img/xboxseries/abutton.svg", "Select"
    // });
    // m_items.push_back({
    //     firelight::libretro::IRetroPad::Button::LeftBumper, "file:system/_img/xboxseries/abutton.svg", "L"
    // });
    // m_items.push_back({
    //     firelight::libretro::IRetroPad::Button::RightBumper, "file:system/_img/xboxseries/abutton.svg", "R"
    // });
}

int PlatformInputListModel::rowCount(const QModelIndex &parent) const {
    return m_items.length();
}

QVariant PlatformInputListModel::data(const QModelIndex &index, int role) const {
    if (role < Qt::UserRole || index.row() >= m_items.size()) {
        return QVariant{};
    }

    auto item = m_items.at(index.row());

    switch (role) {
        case ButtonId:
            return item.button;
        case IconUrl:
            return item.iconUrl;
        case DisplayName:
            return item.displayName;
        default:
            return QVariant{};
    }
}

QHash<int, QByteArray> PlatformInputListModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[ButtonId] = "button_id";
    roles[IconUrl] = "icon_url";
    roles[DisplayName] = "display_name";
    return roles;
}
