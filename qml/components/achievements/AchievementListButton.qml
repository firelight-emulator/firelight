import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts 1.0
import Firelight 1.0

Button {
    id: control
    required property var name
    required property var description
    required property var iconUrl
    required property var points
    required property var type
    required property var earned
    required property var earnedDate
    required property var achievementId

    width: ListView.view.width
    property bool showGlobalCursor: true
    focus: true

    hoverEnabled: true

    ContextMenu.menu: RightClickMenu {
           id: rightClickMenu

           RightClickMenuItem {
               text: "Open at RetroAchievements.org"
               externalLink: true
               onTriggered: {
                   Qt.openUrlExternally("https://retroachievements.org/achievement/" + control.achievementId)
               }
           }
       }

    background: Rectangle {
        color: "transparent"
        border.color: control.earned ? "#FFD700" : "transparent"
        opacity: 0.6
        radius: 6

        Rectangle {
            anchors.fill: parent
            color: control.earned ? "#FFD700" : "white"
            opacity: control.hovered ? 0.20 : 0.15
            radius: 6
        }
    }
    contentItem: RowLayout {
        spacing: 10
        Item {
            implicitWidth: 64
            implicitHeight: 64
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
            Image {
                id: achievementIcon
                anchors.fill: parent
                source: control.iconUrl
                fillMode: Image.PreserveAspectFit
                smooth: true
                visible: control.iconUrl !== ""

                layer.enabled: !control.earned
                layer.effect: MultiEffect {
                    source: achievementIcon
                    saturation: -1
                }
            }
            Rectangle {
                anchors.fill: parent
                color: "black"
                visible: control.iconUrl === ""
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: control.name
                    color: "white"
                    font.family: Constants.mainFontFamily
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    lineHeight: 1.2
                    wrapMode: Text.WordWrap
                }
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
                Pane {
                    visible: control.type !== ""
                    horizontalPadding: 10
                    verticalPadding: 4
                    background: Rectangle {
                        color: "transparent"
                        radius: height / 2
                        border.color: "#c3c3c3"
                    }

                    contentItem: Text {
                        text: control.type
                        color: "#c3c3c3"
                        font.family: Constants.mainFontFamily
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                }
            }

            Text {
                Layout.fillWidth: true
                Layout.fillHeight: true
                text: control.description
                color: "#c3c3c3"
                font.family: Constants.mainFontFamily
                font.weight: Font.Normal
                font.pixelSize: 15
                lineHeight: 1.2
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignTop
            }
        }

        Rectangle {
            color: "#c3c3c3"
            Layout.fillHeight: true
            implicitWidth: 1
        }
        ColumnLayout {
            Layout.preferredWidth: parent.width > 800 ? 200 : 120
            Layout.maximumWidth: parent.width > 800 ? 200 : 120
            Layout.minimumWidth: parent.width > 800 ? 200 : 120
            Layout.fillHeight: true
            spacing: 8

            Text {
                text: control.points + " pt" + (control.points !== 1 ? "s" : "")
                color: "white"
                font.family: Constants.mainFontFamily
                font.pixelSize: 16
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignBottom
                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            Text {
                text: control.earned ? control.earnedDate : "Not earned"
                color: "#c3c3c3"
                font.family: Constants.mainFontFamily
                font.pixelSize: 15
                lineHeight: 1.2
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignTop
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
        }
    }
}