import QtQuick

FLMenuItem {
    property alias currentIndex: comboBox.currentIndex
    property alias currentValue: comboBox.currentValue
    property alias comboBoxModel: comboBox.model
    property alias popup: comboBox.popup
    property alias textRole: comboBox.textRole
    property alias valueRole: comboBox.valueRole

    controlItem: MyComboBox {
        id: comboBox
        focusPolicy: Qt.NoFocus
    }
}
