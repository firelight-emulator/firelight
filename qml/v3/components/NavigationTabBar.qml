import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FocusScope {
    id: control

    required property list<string> tabs

    property int tabWidth: 160
    property int currentIndex: 0

    property var clickAction: null

    implicitHeight: contents.implicitHeight
    implicitWidth: leftIcon.width + leftIcon.anchors.rightMargin + contents.implicitWidth + rightIcon.anchors.leftMargin + rightIcon.width

    FLFocus.container: true

    FLInputGlyph {
        id: leftIcon
        input: Qt.Key_Minus
        anchors.right: contents.left
        anchors.rightMargin: AppStyle.spacingXl
        anchors.verticalCenter: parent.verticalCenter
        enabled: control.currentIndex > 0
        enabledOpacity: 0.8
    }

    FLRowLayout {
        id: contents
        spacing: AppStyle.spacingMd

        anchors.centerIn: parent

        FLFocus.container: false

        Repeater {
            id: tabRepeater
            model: control.tabs
            delegate: FLButton {
                required property var modelData
                required property var index

                text: modelData
                variant: "subtle"
                Layout.preferredWidth: control.tabWidth
                Layout.preferredHeight: AppStyle.tabBarHeight

                checkedColor: Theme.switch2Color
                checked: control.currentIndex === index

                FLFocus.focusSound: SoundEffects.tabBarNavigate
                FLFocus.mode: control.activeFocus || control.currentIndex === index ? FLFocus.Normal : FLFocus.Skip

                Rectangle {
                    anchors.left: parent.left
                    anchors.leftMargin: AppStyle.spacingMd
                    anchors.right: parent.right
                    anchors.rightMargin: AppStyle.spacingMd
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: AppStyle.spacingXs
                    height: 2
                    radius: 1
                    color: control.currentIndex === index ? Theme.switch2Color : "transparent"
                }

                onClicked: {
                    control.currentIndex = index

                    console.log("click action? ", control.clickAction)

                    if (control.clickAction) {
                        control.clickAction()
                    }
                }

                onActiveFocusChanged: {
                    if (activeFocus) {
                        control.currentIndex = index
                    }
                }
            }
        }
    }

    FLInputGlyph {
        id: rightIcon
        input: Qt.Key_Plus
        anchors.left: contents.right
        anchors.leftMargin: AppStyle.spacingXl
        anchors.verticalCenter: parent.verticalCenter
        enabled: control.currentIndex < control.tabs.length - 1
        enabledOpacity: 0.8
    }
}
