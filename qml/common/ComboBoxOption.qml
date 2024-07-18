import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


Option {
    id: root

    required property var model
    property int currentIndex: 0
    property string currentValue
    property string textRole: "text"
    property string valueRole: "value"
    // property alias model: control.model
    // property alias currentIndex: control.currentIndex
    // property alias textRole: control.textRole
    // property alias valueRole: control.valueRole

    control: MyComboBox {
        id: control
        model: root.model
        currentIndex: root.currentIndex
        onCurrentIndexChanged: function () {
            root.currentIndex = control.currentIndex
            root.currentValue = control.model[control.currentIndex][root.valueRole]
        }
        // onCurrentValueChanged: root.currentValue = control.currentValue
        textRole: root.textRole
        valueRole: root.valueRole

        Component.onCompleted: {
            if (root.currentValue) {
                control.currentIndex = find(root.currentValue)
            }
        }
    }
}


