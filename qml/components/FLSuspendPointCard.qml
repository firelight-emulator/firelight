import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts 1.0
import Firelight 1.0

Button {
    id: root

    required property string imageUrl
    required property string dateTimeString
    required property int index

    signal deleteClicked()
    signal overwriteClicked()

    padding: 8

    background: Rectangle {
        color: "#222222"
        opacity: 0.9
        radius: 6
        border.color: "#737373"
    }

    contentItem: ColumnLayout {
        spacing: 8
        Rectangle {
            color: "black"
            Layout.fillWidth: true
            Layout.preferredHeight: width * 9 / 16

            Image {
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: root.imageUrl
            }
        }
        Text {
            leftPadding: 4
            rightPadding: 4
            text: "Slot " + (root.index + 1)
            font.pixelSize: 16
            font.family: Constants.regularFontFamily
            font.weight: Font.DemiBold
            color: "white"
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }
        Text {
            leftPadding: 4
            rightPadding: 4
            text: root.dateTimeString
            font.pixelSize: 16
            font.family: Constants.regularFontFamily
            font.weight: Font.Normal
            color: "#d1d1d1"
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }
        // RowLayout {
        //     Layout.fillWidth: true
        //
        //     Item {
        //         Layout.fillWidth: true
        //         Layout.fillHeight: true
        //     }
        //
        // }
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Button {
            id: loadButton
            Layout.fillWidth: true
            Layout.maximumHeight: 42
            Layout.minimumHeight: 42

            enabled: !achievement_manager.loggedIn || !achievement_manager.inHardcoreMode

            background: Rectangle {
                id: bg
                color: "white"
                opacity: parent.pressed ? 0.2 : theHoverHandler3.hovered ? 0.3 : 0.16
                radius: 4

                layer.enabled: true
                layer.effect: MultiEffect {
                    source: bg
                    anchors.fill: bg
                    autoPaddingEnabled: true
                    shadowBlur: 1.0
                    shadowColor: 'black'
                    shadowOpacity: 0.8
                    shadowEnabled: true
                    shadowVerticalOffset: 6
                }

                visible: parent.enabled
            }

            onClicked: function(event) {
                emulatorLoader.item.loadSuspendPoint(theThing.index)
            }

            HoverHandler {
                id: theHoverHandler3
                cursorShape: Qt.PointingHandCursor
            }
            contentItem: Text {
                color: parent.enabled ? "white" : "#727272"
                text: "Load Suspend Point"
                font.pixelSize: 15
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
        RowLayout {
            Layout.maximumHeight: 42
            Layout.minimumHeight: 42
            Layout.fillWidth: true
            Button {
                Layout.fillWidth: true
                Layout.fillHeight: true
                background: Rectangle {
                    color: "white"
                    opacity: parent.pressed ? 0.16 : 0.1
                    radius: 4
                    visible: theHoverHandler.hovered && parent.enabled
                }
                HoverHandler {
                    id: theHoverHandler
                    cursorShape: Qt.PointingHandCursor
                }
                contentItem: Text {
                    color: "white"
                    text: "Overwrite"
                    font.pixelSize: 15
                    font.family: Constants.regularFontFamily
                    font.weight: Font.Normal
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    root.overwriteClicked()
                }
            }

            Button {
                background: Rectangle {
                    color: "#9f1818"
                    opacity: parent.pressed ? 0.8 : 0.6
                    radius: 4
                    visible: theHoverHandler2.hovered && parent.enabled
                }
                HoverHandler {
                    id: theHoverHandler2
                    cursorShape: Qt.PointingHandCursor
                }
                padding: 4
                Layout.fillHeight: true
                Layout.preferredWidth: parent.height
                contentItem: FLIcon {
                    icon: "delete"
                    filled: false
                    size: height
                    color: parent.enabled ? "white" : "#727272"
                }

                onClicked: {
                    root.deleteClicked()
                }

            }
        }
    }
}
