import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    color: Theme.surface
    radius: 8

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        Text {
            text: "Chat"
            color: Theme.textMuted
            font.pixelSize: 14
        }

        ListView {
            id: chatList
            clip: true
            spacing: 6
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: NetplayChatModel

            delegate: ColumnLayout {
                width: ListView.view.width
                spacing: 0

                Text {
                    text: model.senderName
                    color: model.isSelf ? "#5588dd" : "#cc8855"
                    font.pixelSize: 11
                    font.weight: Font.DemiBold
                }
                Text {
                    text: model.text
                    color: Theme.textPrimary
                    font.pixelSize: 13
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
            }

            onCountChanged: positionViewAtEnd()
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            TextField {
                id: chatField
                placeholderText: "Say something..."
                Layout.fillWidth: true
                onAccepted: {
                    NetworkService.sendChat(text)
                    text = ""
                }
            }
            Button {
                text: "Send"
                enabled: chatField.text.trim().length > 0
                onClicked: chatField.accepted()
            }
        }
    }
}
