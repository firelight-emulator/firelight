import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// Generic per-console settings page: shows the friendly emulation settings for a
// platform at the Platform tier (defaults for all of that console's games).
// Driven entirely by `platformId`, so every console gets a real page.
FocusScope {
    id: root

    property int platformId: -1
    property string platformName: ""

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            spacing: 8

            Button {
                id: backButton
                Layout.fillHeight: true
                Layout.preferredWidth: parent.height
                hoverEnabled: true

                background: Rectangle {
                    color: backButton.hovered ? "#404143" : "transparent"
                    radius: height / 2
                }
                contentItem: Text {
                    text: ""
                    font.family: Constants.symbolFontFamily
                    leftPadding: 8
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 24
                    color: ColorPalette.neutral400
                }

                onClicked: root.StackView.view.pop()
            }

            Text {
                Layout.fillWidth: true
                Layout.fillHeight: true
                text: qsTr("Default %1 settings").arg(root.platformName)
                font.pixelSize: 26
                font.family: Constants.regularFontFamily
                font.weight: Font.Bold
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                color: ColorPalette.neutral100
            }
        }

        Rectangle {
            Layout.topMargin: 8
            Layout.bottomMargin: 8
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#333333"
        }

        EmulationSettingsSurface {
            Layout.fillWidth: true
            Layout.fillHeight: true
            level: 1 // Platform tier
            platformId: root.platformId
        }
    }
}
