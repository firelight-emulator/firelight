import QtQuick
import QtQuick.Layouts

FLPanel {
    id: root

    padding: AppStyle.spacingMd

    ColumnLayout {
        anchors.fill: parent
        spacing: AppStyle.spacingSm

        Text {
            text: "Chat"
            color: Theme.textMuted
            font.pixelSize: AppStyle.fontSizeSmall
        }

        ListView {
            id: chatList
            clip: true
            spacing: AppStyle.spacingXs
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: NetplayChatModel

            delegate: ColumnLayout {
                width: ListView.view.width
                spacing: 0

                Text {
                    text: model.senderName
                    color: model.isSelf ? Theme.accent : Theme.warning
                    font.pixelSize: AppStyle.fontSizeSmall
                    font.weight: Font.DemiBold
                }
                Text {
                    text: model.text
                    color: Theme.textPrimary
                    font.pixelSize: AppStyle.fontSizeSmall
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
            }

            onCountChanged: positionViewAtEnd()
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: AppStyle.spacingXs

            FLTextField {
                id: chatField
                placeholderText: "Say something..."
                Layout.fillWidth: true
                onAccepted: {
                    NetworkService.sendChat(text);
                    text = "";
                }
            }
            FLButton {
                variant: "primary"
                text: "Send"
                enabled: chatField.text.trim().length > 0
                onClicked: chatField.accepted()
            }
        }
    }
}
