import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts 1.0
import Firelight 1.0

Button {
    id: control

    property string avatarUrlSource: ""

    RightClickMenu {
        id: contextMenu
        x: -(contextMenu.width) + control.width
        y: control.height + 8

        RightClickMenuItem {
            text: qsTr("Achievements")
            iconSource: "qrc:/icons/trophy"
            minWidth: 260
            onTriggered: { /* ... */ }
        }
        RightClickMenuItem {
            text: qsTr("Activity")
            iconSource: "qrc:/icons/bar-chart"
            minWidth: 260
            onTriggered: { /* ... */ }
        }
    }

    TapHandler {
        id: avatarButtonTapHandler
        onTapped: {
            contextMenu.open()
        }
    }

    HoverHandler {
        id: avatarButtonHoverHandler
        cursorShape: Qt.PointingHandCursor
    }

    background: Item {
        Rectangle {
            id: avatarMask
            anchors.fill: parent
            radius: width / 2
            layer.enabled: true
            visible: false
        }

        Image {
            id: avatarButton
            anchors.fill: parent
            sourceSize.width: 64
            sourceSize.height: 64
            source: control.avatarUrlSource
            fillMode: Image.PreserveAspectCrop
            smooth: true

            layer.enabled: true
            layer.effect: MultiEffect {
                maskEnabled: true
                smooth: true
                maskSource: avatarMask
                maskThresholdMin: 0.5
                maskSpreadAtMin: 1.0
            }
        }
    }
}
