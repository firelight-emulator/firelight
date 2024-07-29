import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    id: root

    Component {
        id: n64Settings
        Nintendo64Settings {
        }
    }

    Component {
        id: snesSettings
        SnesSettings {
        }
    }

    Component {
        id: nesSettings
        NesSettings {
        }
    }

    Component {
        id: gbaSettings
        GbaSettings {
        }
    }

    Component {
        id: gbcSettings
        GameBoyColorSettings {
        }
    }

    Component {
        id: gbSettings
        GameBoySettings {
        }
    }


    Flickable {
        anchors.fill: parent
        contentHeight: col.height
        ColumnLayout {
            spacing: 0
            id: col
            width: parent.width

            Text {
                Layout.fillWidth: true
                text: qsTr("Platform settings")
                font.pixelSize: 26
                font.family: Constants.regularFontFamily
                font.weight: Font.Bold
                Layout.bottomMargin: 8
                color: "white"
            }

            Text {
                Layout.fillWidth: true
                text: qsTr("Here's where you can change settings for each platform. You can change settings on a per-game basis by selecting the game in your library and going to 'Settings'.")
                font.pixelSize: 15
                font.family: Constants.regularFontFamily
                font.weight: Font.Normal
                wrapMode: Text.WordWrap
                color: ColorPalette.neutral300
                Layout.bottomMargin: 8
            }

            Rectangle {
                Layout.topMargin: 8
                Layout.bottomMargin: 8
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#333333"
            }

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 80

                onClicked: function () {
                    root.StackView.view.push(gbSettings)
                }
                hoverEnabled: true
                background: Rectangle {
                    color: parent.hovered ? "#333333" : "transparent"
                    radius: 8
                }
                contentItem: Row {
                    spacing: 8
                    Image {
                        source: "file:system/_img/gb.svg"
                        width: parent.height
                        height: parent.height
                        fillMode: Image.PreserveAspectFit
                        sourceSize.height: parent.height
                    }
                    Text {
                        text: qsTr("Game Boy")
                        font.pointSize: 14
                        height: parent.height
                        font.family: Constants.regularFontFamily
                        font.weight: Font.Normal
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        color: "white"
                    }

                }

            }
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 80

                onClicked: function () {
                    root.StackView.view.push(gbcSettings)
                }
                hoverEnabled: true
                background: Rectangle {
                    color: parent.hovered ? "#333333" : "transparent"
                    radius: 8
                }
                contentItem: Row {
                    spacing: 8
                    Image {
                        source: "file:system/_img/gbc.svg"
                        width: parent.height
                        height: parent.height
                        fillMode: Image.PreserveAspectFit
                        sourceSize.height: parent.height
                    }
                    Text {
                        text: qsTr("Game Boy Color")
                        font.pointSize: 14
                        height: parent.height
                        font.family: Constants.regularFontFamily
                        font.weight: Font.Normal
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        color: "white"
                    }

                }

            }
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 80

                onClicked: function () {
                    root.StackView.view.push(gbaSettings)
                }
                hoverEnabled: true
                background: Rectangle {
                    color: parent.hovered ? "#333333" : "transparent"
                    radius: 8
                }
                contentItem: Row {
                    spacing: 8
                    Image {
                        source: "file:system/_img/gba.svg"
                        width: parent.height
                        height: parent.height
                        fillMode: Image.PreserveAspectFit
                        sourceSize.height: parent.height
                    }
                    Text {
                        text: qsTr("Game Boy Advance")
                        font.pointSize: 14
                        height: parent.height
                        font.family: Constants.regularFontFamily
                        font.weight: Font.Normal
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        color: "white"
                    }

                }

            }
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 80

                onClicked: function () {
                    root.StackView.view.push(nesSettings)
                }
                hoverEnabled: true
                background: Rectangle {
                    color: parent.hovered ? "#333333" : "transparent"
                    radius: 8
                }
                contentItem: Row {
                    spacing: 8
                    Image {
                        source: "file:system/_img/nes.svg"
                        width: parent.height
                        height: parent.height
                        fillMode: Image.PreserveAspectFit
                        sourceSize.height: parent.height
                    }
                    Text {
                        text: qsTr("NES")
                        font.pointSize: 14
                        height: parent.height
                        font.family: Constants.regularFontFamily
                        font.weight: Font.Normal
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        color: "white"
                    }

                }

            }

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 80

                onClicked: function () {
                    root.StackView.view.push(snesSettings)
                }

                hoverEnabled: true
                background: Rectangle {
                    color: parent.hovered ? "#333333" : "transparent"
                    radius: 8
                }
                contentItem: Row {
                    spacing: 8
                    Image {
                        source: "file:system/_img/SNES.svg"
                        width: parent.height
                        height: parent.height
                        fillMode: Image.PreserveAspectFit
                        sourceSize.height: parent.height
                    }
                    Text {
                        text: qsTr("SNES")
                        height: parent.height
                        font.pointSize: 14
                        font.family: Constants.regularFontFamily
                        font.weight: Font.Normal
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        color: "white"
                    }

                }

            }

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 80

                onClicked: function () {
                    root.StackView.view.push(n64Settings)
                }
                hoverEnabled: true
                background: Rectangle {
                    color: parent.hovered ? "#333333" : "transparent"
                    radius: 8
                }
                contentItem: Row {
                    spacing: 8
                    Image {
                        source: "file:system/_img/N64.svg"
                        width: parent.height
                        height: parent.height
                        fillMode: Image.PreserveAspectFit
                        sourceSize.height: parent.height
                    }
                    Text {
                        text: qsTr("Nintendo 64")
                        height: parent.height
                        font.pointSize: 14
                        font.family: Constants.regularFontFamily
                        font.weight: Font.Normal
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        color: "white"
                    }

                }

            }

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 80

                onClicked: function () {
                    root.StackView.view.push(n64Settings)
                }
                hoverEnabled: true
                background: Rectangle {
                    color: parent.hovered ? "#333333" : "transparent"
                    radius: 8
                }
                contentItem: Row {
                    spacing: 8
                    Image {
                        source: "file:system/_img/DS.svg"
                        width: parent.height
                        height: parent.height
                        fillMode: Image.PreserveAspectFit
                        sourceSize.height: parent.height
                    }
                    Text {
                        text: qsTr("Nintendo DS")
                        height: parent.height
                        font.pointSize: 14
                        font.family: Constants.regularFontFamily
                        font.weight: Font.Normal
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        color: "white"
                    }

                }

            }

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 80

                onClicked: function () {
                    root.StackView.view.push(n64Settings)
                }
                hoverEnabled: true
                background: Rectangle {
                    color: parent.hovered ? "#333333" : "transparent"
                    radius: 8
                }
                contentItem: Row {
                    spacing: 8
                    Image {
                        source: "file:system/_img/genesis.svg"
                        width: parent.height
                        height: parent.height
                        fillMode: Image.PreserveAspectFit
                        sourceSize.height: parent.height
                    }
                    Text {
                        text: qsTr("Sega Genesis")
                        height: parent.height
                        font.pointSize: 14
                        font.family: Constants.regularFontFamily
                        font.weight: Font.Normal
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        color: "white"
                    }

                }

            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }
}