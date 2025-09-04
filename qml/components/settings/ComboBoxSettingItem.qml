import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

BaseSettingItem {
    property alias currentIndex: comboBox.currentIndex
    property alias currentValue: comboBox.currentValue
    property alias comboBoxModel: comboBox.model
    property alias popup: comboBox.popup
    property alias textRole: comboBox.textRole
    property alias valueRole: comboBox.valueRole

    control: MyComboBox {
          id: comboBox
          focusPolicy: Qt.NoFocus
          // Component.onCompleted: {
          //     baseItem.onActiveFocusChanged.connect(function() {
          //         if (comboBox.popup.visible && !comboBox.popup.activeFocus) {
          //                comboBox.popup.close()
          //         }
          //     })
          // }
      }
}
