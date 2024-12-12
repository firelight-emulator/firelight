#include "input_mapping_item.hpp"

namespace firelight::input {
    QVariantMap InputMappingItem::getInputMappings() const {
        return m_inputMappings;
    }

    void InputMappingItem::setInputMappings(const QVariantMap &inputMappings) {
    }

    void InputMappingItem::setButtonMapping(int retropadInput, int sdlButton) {
        m_inputMappings.insert(QString::number(retropadInput), sdlButton);
        emit inputMappingsChanged();
    }
} // firelight::input
