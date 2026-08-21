import QtQuick
import QtQuick.Layouts

// Appears anywhere in the app when the host starts a game: guests ready up,
// the host watches the count and launches (or cancels)
Rectangle {
    id: root

    property bool selfReady: false

    visible: NetworkService.inLobby && NetworkService.sessionPhase === "starting"
    onVisibleChanged: {
        if (!visible) {
            selfReady = false;
        }
    }

    width: Math.min(420, parent.width - 32)
    height: content.implicitHeight + AppStyle.spacingLg
    radius: AppStyle.radiusMd
    color: Theme.surfaceElevated
    border.color: Theme.border
    border.width: 1

    ColumnLayout {
        id: content
        anchors.centerIn: parent
        width: parent.width - 24
        spacing: AppStyle.spacingSm

        RowLayout {
            Layout.fillWidth: true
            spacing: AppStyle.spacingMd

            Image {
                source: NetworkService.gameArtUrl
                visible: NetworkService.gameArtUrl.length > 0
                sourceSize.width: 40
                sourceSize.height: 40
                Layout.preferredWidth: 40
                Layout.preferredHeight: 40
            }
            ColumnLayout {
                spacing: 2
                Layout.fillWidth: true

                Text {
                    text: NetworkService.isHost ? "Ready check" : "The host wants to play"
                    color: Theme.textMuted
                    font.pixelSize: AppStyle.fontSizeSmall
                }
                Text {
                    text: NetworkService.gameName
                    color: Theme.textPrimary
                    font.pixelSize: AppStyle.fontSizeMedium
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }
            Text {
                text: NetworkService.readySlotCount + "/" + NetworkService.occupiedSlotCount + " ready"
                color: NetworkService.readySlotCount === NetworkService.occupiedSlotCount ? Theme.success : Theme.textMuted
                font.pixelSize: AppStyle.fontSizeSmall
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: AppStyle.spacingSm

            FLButton {
                variant: "primary"
                visible: !NetworkService.isHost
                text: root.selfReady ? "Ready!" : "I'm ready"
                enabled: !root.selfReady
                Layout.fillWidth: true
                onClicked: {
                    root.selfReady = true;
                    NetworkService.setReady(true);
                }
            }

            FLButton {
                variant: "primary"
                visible: NetworkService.isHost
                text: "Launch"
                Layout.fillWidth: true
                onClicked: root.hostLaunchRequested()
            }
            FLButton {
                variant: "subtle"
                visible: NetworkService.isHost
                text: "Cancel"
                onClicked: NetworkService.endSession()
            }
        }
    }

    signal hostLaunchRequested
}
