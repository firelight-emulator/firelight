import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Appears anywhere in the app when the host starts a game: guests ready up,
// the host watches the count and launches (or cancels).
Rectangle {
    id: root

    property bool selfReady: false

    visible: NetworkService.inLobby && NetworkService.sessionPhase === "starting"
    onVisibleChanged: {
        if (!visible) {
            selfReady = false
        }
    }

    width: Math.min(420, parent.width - 32)
    height: content.implicitHeight + 24
    radius: 10
    color: "#2b3341"
    border.color: "#4a5568"
    border.width: 1

    ColumnLayout {
        id: content
        anchors.centerIn: parent
        width: parent.width - 24
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

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
                    color: "#aaaaaa"
                    font.pixelSize: 12
                }
                Text {
                    text: NetworkService.gameName
                    color: "white"
                    font.pixelSize: 16
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }
            Text {
                text: NetworkService.readySlotCount + "/" + NetworkService.occupiedSlotCount + " ready"
                color: NetworkService.readySlotCount === NetworkService.occupiedSlotCount
                       ? "#66cc66" : "#aaaaaa"
                font.pixelSize: 13
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                visible: !NetworkService.isHost
                text: root.selfReady ? "Ready!" : "I'm ready"
                enabled: !root.selfReady
                Layout.fillWidth: true
                onClicked: {
                    root.selfReady = true
                    NetworkService.setReady(true)
                }
            }

            Button {
                visible: NetworkService.isHost
                text: "Launch"
                Layout.fillWidth: true
                onClicked: root.hostLaunchRequested()
            }
            Button {
                visible: NetworkService.isHost
                text: "Cancel"
                onClicked: NetworkService.endSession()
            }
        }
    }

    signal hostLaunchRequested()
}
