#include "input_mapping_item.hpp"

namespace firelight::input {
    InputMappingItem::InputMappingItem(QObject *parent) {
        refreshMapping();
    }

    QVariantMap InputMappingItem::getInputMappings() const {
        return m_inputMappings;
    }

    int InputMappingItem::getProfileId() const {
        return m_profileId;
    }

    int InputMappingItem::getPlatformId() const {
        return m_platformId;
    }

    void InputMappingItem::setProfileId(const int profileId) {
        if (profileId == m_profileId) {
            return;
        }

        m_profileId = profileId;
        emit profileIdChanged();
        refreshMapping();
    }

    void InputMappingItem::setPlatformId(const int platformId) {
        if (platformId == m_platformId) {
            return;
        }

        m_platformId = platformId;
        emit platformIdChanged();
        refreshMapping();
    }

    void InputMappingItem::addMapping(int input, int mappedInput) {
        m_inputMapping->addMapping(static_cast<libretro::IRetroPad::Input>(input),
                                   static_cast<libretro::IRetroPad::Input>(mappedInput));
        m_inputMapping->sync();
        m_inputMappings.insert(QString::number(input), mappedInput);
        emit inputMappingsChanged();
    }

    void InputMappingItem::removeMapping(int input) {
        m_inputMapping->removeMapping(static_cast<libretro::IRetroPad::Input>(input));
        m_inputMapping->sync();
        m_inputMappings.remove(QString::number(input));
        emit inputMappingsChanged();
    }

    void InputMappingItem::refreshMapping() {
        m_inputMappings.clear();
        m_inputMapping = getControllerRepository()->getInputMapping(m_profileId, m_platformId);
        for (const auto &[input, mappedInput]: m_inputMapping->getMappings()) {
            m_inputMappings.insert(QString::number(input), mappedInput);
        }
        emit inputMappingsChanged();
    }
} // firelight::input
