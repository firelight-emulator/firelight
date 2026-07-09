import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    id: root

    // One generic, catalog-driven page for every console; pushed with the
    // target platform's id + name.
    Component {
        id: platformPage
        PlatformEmulationSettingsPage {
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
                color: Theme.textPrimary
            }

            Text {
                Layout.fillWidth: true
                // text: qsTr("Here's where you can change settings for each platform. You can change settings on a per-game basis by selecting the game in your library and going to 'Settings'.")
                text: qsTr("Here's where you can change the default settings for each platform. Later you'll be able to change them on a per-game basis.")
                font.pixelSize: 15
                font.family: Constants.regularFontFamily
                font.weight: Font.Normal
                wrapMode: Text.WordWrap
                color: Theme.textMuted
                Layout.bottomMargin: 8
            }

            Rectangle {
                Layout.topMargin: 8
                Layout.bottomMargin: 8
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: Theme.border
            }

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 80

                onClicked: function () {
                    root.StackView.view.push(platformPage, {platformId: 1, platformName: "Game Boy"})
                }
                hoverEnabled: true
                background: Rectangle {
                    color: parent.hovered ? "#333333" : "transparent"
                    radius: 8
                }
                contentItem: Row {
                    spacing: 8
                    Image {
                        source: "qrc:images/platform-icons/gb"
                        width: parent.height
                        height: parent.height
                        fillMode: Image.PreserveAspectFit
                        sourceSize.height: 128
                    }
                    Text {
                        text: qsTr("Game Boy")
                        font.pointSize: 14
                        height: parent.height
                        font.family: Constants.regularFontFamily
                        font.weight: Font.Normal
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        color: Theme.textPrimary
                    }

                }

            }
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 80

                onClicked: function () {
                    root.StackView.view.push(platformPage, {platformId: 2, platformName: "Game Boy Color"})
                }
                hoverEnabled: true
                background: Rectangle {
                    color: parent.hovered ? "#333333" : "transparent"
                    radius: 8
                }
                contentItem: Row {
                    spacing: 8
                    Image {
                        source: "qrc:images/platform-icons/gbc"
                        width: parent.height
                        height: parent.height
                        fillMode: Image.PreserveAspectFit
                        sourceSize.height: 128
                    }
                    Text {
                        text: qsTr("Game Boy Color")
                        font.pointSize: 14
                        height: parent.height
                        font.family: Constants.regularFontFamily
                        font.weight: Font.Normal
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        color: Theme.textPrimary
                    }

                }

            }
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 80

                onClicked: function () {
                    root.StackView.view.push(platformPage, {platformId: 3, platformName: "Game Boy Advance"})
                }
                hoverEnabled: true
                background: Rectangle {
                    color: parent.hovered ? "#333333" : "transparent"
                    radius: 8
                }
                contentItem: Row {
                    spacing: 8
                    Image {
                        source: "qrc:images/platform-icons/gba"
                        width: parent.height
                        height: parent.height
                        fillMode: Image.PreserveAspectFit
                        sourceSize.height: 128
                    }
                    Text {
                        text: qsTr("Game Boy Advance")
                        font.pointSize: 14
                        height: parent.height
                        font.family: Constants.regularFontFamily
                        font.weight: Font.Normal
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        color: Theme.textPrimary
                    }

                }

            }
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 80

                onClicked: function () {
                    root.StackView.view.push(platformPage, {platformId: 5, platformName: "NES"})
                }
                hoverEnabled: true
                background: Rectangle {
                    color: parent.hovered ? "#333333" : "transparent"
                    radius: 8
                }
                contentItem: Row {
                    spacing: 8
                    Image {
                        source: "qrc:images/platform-icons/nes"
                        width: parent.height
                        height: parent.height
                        fillMode: Image.PreserveAspectFit
                        sourceSize.height: 128
                    }
                    Text {
                        text: qsTr("NES")
                        font.pointSize: 14
                        height: parent.height
                        font.family: Constants.regularFontFamily
                        font.weight: Font.Normal
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        color: Theme.textPrimary
                    }

                }

            }

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 80

                onClicked: function () {
                    root.StackView.view.push(platformPage, {platformId: 6, platformName: "SNES"})
                }

                hoverEnabled: true
                background: Rectangle {
                    color: parent.hovered ? "#333333" : "transparent"
                    radius: 8
                }
                contentItem: Row {
                    spacing: 8
                    Image {
                        source: "qrc:images/platform-icons/snes"
                        width: parent.height
                        height: parent.height
                        fillMode: Image.PreserveAspectFit
                        sourceSize.height: 128
                    }
                    Text {
                        text: qsTr("SNES")
                        height: parent.height
                        font.pointSize: 14
                        font.family: Constants.regularFontFamily
                        font.weight: Font.Normal
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        color: Theme.textPrimary
                    }

                }

            }

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 80

                onClicked: function () {
                    root.StackView.view.push(platformPage, {platformId: 7, platformName: "Nintendo 64"})
                }
                hoverEnabled: true
                background: Rectangle {
                    color: parent.hovered ? "#333333" : "transparent"
                    radius: 8
                }
                contentItem: Row {
                    spacing: 8
                    Image {
                        source: "qrc:images/platform-icons/n64"
                        width: parent.height
                        height: parent.height
                        fillMode: Image.PreserveAspectFit
                        sourceSize.height: 128
                    }
                    Text {
                        text: qsTr("Nintendo 64")
                        height: parent.height
                        font.pointSize: 14
                        font.family: Constants.regularFontFamily
                        font.weight: Font.Normal
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        color: Theme.textPrimary
                    }

                }

            }

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 80

                onClicked: function () {
                    root.StackView.view.push(platformPage, {platformId: 10, platformName: "Nintendo DS"})
                }
                hoverEnabled: true
                background: Rectangle {
                    color: parent.hovered ? "#333333" : "transparent"
                    radius: 8
                }
                contentItem: Row {
                    spacing: 8
                    Image {
                        source: "qrc:images/platform-icons/ds"
                        width: parent.height
                        height: parent.height
                        fillMode: Image.PreserveAspectFit
                        sourceSize.height: 128
                    }
                    Text {
                        text: qsTr("Nintendo DS")
                        height: parent.height
                        font.pointSize: 14
                        font.family: Constants.regularFontFamily
                        font.weight: Font.Normal
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        color: Theme.textPrimary
                    }

                }

            }

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 80

                onClicked: function () {
                    root.StackView.view.push(platformPage, {platformId: 12, platformName: "Master System"})
                }
                hoverEnabled: true
                background: Rectangle {
                    color: parent.hovered ? "#333333" : "transparent"
                    radius: 8
                }
                contentItem: Row {
                    spacing: 8
                    Image {
                        source: "qrc:images/platform-icons/master-system"
                        width: parent.height
                        height: parent.height
                        fillMode: Image.PreserveAspectFit
                        sourceSize.height: 128
                    }
                    Text {
                        text: qsTr("Master System")
                        height: parent.height
                        font.pointSize: 14
                        font.family: Constants.regularFontFamily
                        font.weight: Font.Normal
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        color: Theme.textPrimary
                    }

                }

            }

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 80

                onClicked: function () {
                    root.StackView.view.push(platformPage, {platformId: 13, platformName: "Genesis/Mega Drive"})
                }
                hoverEnabled: true
                background: Rectangle {
                    color: parent.hovered ? "#333333" : "transparent"
                    radius: 8
                }
                contentItem: Row {
                    spacing: 8
                    Image {
                        source: "qrc:images/platform-icons/genesis"
                        width: parent.height
                        height: parent.height
                        fillMode: Image.PreserveAspectFit
                        sourceSize.height: 128
                    }
                    Text {
                        text: qsTr("Genesis/Mega Drive")
                        height: parent.height
                        font.pointSize: 14
                        font.family: Constants.regularFontFamily
                        font.weight: Font.Normal
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        color: Theme.textPrimary
                    }

                }

            }

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 80

                onClicked: function () {
                    root.StackView.view.push(platformPage, {platformId: 14, platformName: "Game Gear"})
                }
                hoverEnabled: true
                background: Rectangle {
                    color: parent.hovered ? "#333333" : "transparent"
                    radius: 8
                }
                contentItem: Row {
                    spacing: 8
                    Image {
                        source: "qrc:images/platform-icons/gamegear"
                        width: parent.height
                        height: parent.height
                        fillMode: Image.PreserveAspectFit
                        sourceSize.height: 128
                    }
                    Text {
                        text: qsTr("Game Gear")
                        height: parent.height
                        font.pointSize: 14
                        font.family: Constants.regularFontFamily
                        font.weight: Font.Normal
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        color: Theme.textPrimary
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