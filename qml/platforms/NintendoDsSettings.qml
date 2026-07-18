import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    id: root
    Flickable {
        anchors.fill: parent
        contentHeight: column.height
        ColumnLayout {
            id: column
            width: parent.width

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                spacing: 8

                Button {
                    id: backButton
                    background: Rectangle {
                        color: enabled ? (backButton.hovered ? "#404143" : "transparent") : "transparent"
                        radius: height / 2

                    }

                    Layout.fillHeight: true
                    Layout.preferredWidth: parent.height

                    hoverEnabled: true

                    contentItem: Icon {
                        name: "arrow_back_ios"
                        leftPadding: 8
                        size: 24
                        color: Theme.textMuted
                    }

                    checkable: false

                    onClicked: {
                        root.StackView.view.pop()
                    }
                }

                Text {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: qsTr("Default Nintendo DS settings")
                    font.pixelSize: AppStyle.fontSizeLarge
                    font.family: Constants.regularFontFamily
                    font.weight: Font.Bold
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    color: Theme.textPrimary
                }
                Layout.bottomMargin: 8
            }

            Rectangle {
                Layout.topMargin: 8
                Layout.bottomMargin: 8
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: Theme.border
            }

            Text {
                Layout.fillWidth: true
                text: "There's nothing here yet."
                color: Theme.textPrimary
                font.pixelSize: AppStyle.fontSizeMedium
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }
}
