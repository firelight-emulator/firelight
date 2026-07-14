import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// The lobby hub: everyone returns here between games. Slots, game choice,
// members, and chat persist for the life of the lobby.
Item {
    id: root

    property int pickingSlot: -1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
                text: "Lobby"
                color: Theme.textPrimary
                font.pixelSize: AppStyle.fontSizeLarge
                font.weight: Font.DemiBold
            }
            Rectangle {
                color: Theme.surface
                radius: 6
                implicitWidth: codeRow.implicitWidth + 20
                implicitHeight: 32

                RowLayout {
                    id: codeRow
                    anchors.centerIn: parent
                    spacing: 8
                    Text {
                        text: NetworkService.joinCode
                        color: Theme.textPrimary
                        font.family: "Consolas"
                        font.pixelSize: AppStyle.fontSizeSmall
                    }
                    ToolButton {
                        text: "Copy"
                        onClicked: NetworkService.copyJoinCode()
                    }
                }
            }
            Item {
                Layout.fillWidth: true
            }
            TextField {
                id: lobbyNameField
                text: NetworkService.playerName
                maximumLength: 24
                implicitWidth: 140
                onEditingFinished: {
                    if (text.trim().length > 0) {
                        NetworkService.setPlayerName(text)
                    }
                }
            }
            Button {
                text: NetworkService.isHost ? "Close lobby" : "Leave lobby"
                onClicked: NetworkService.leaveLobby()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            // Left: game + slots + controls.
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 12

                Rectangle {
                    color: Theme.surface
                    radius: 8
                    Layout.fillWidth: true
                    implicitHeight: 96

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 12

                        Image {
                            source: NetworkService.gameArtUrl
                            visible: NetworkService.gameArtUrl.length > 0
                            fillMode: Image.PreserveAspectFit
                            Layout.preferredWidth: 72
                            Layout.preferredHeight: 72
                        }
                        ColumnLayout {
                            spacing: 4
                            Layout.fillWidth: true
                            Text {
                                text: NetworkService.hasGame
                                      ? NetworkService.gameName
                                      : "No game selected"
                                color: Theme.textPrimary
                                font.pixelSize: AppStyle.fontSizeMedium
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                            Text {
                                visible: !NetworkService.isHost && !NetworkService.hasGame
                                text: "Waiting for the host to pick a game"
                                color: Theme.textMuted
                                font.pixelSize: AppStyle.fontSizeSmall
                            }
                        }
                        Button {
                            visible: NetworkService.isHost
                            text: NetworkService.hasGame ? "Change game" : "Choose game"
                            onClicked: gamePicker.open()
                        }
                    }
                }

                Text {
                    text: "Players"
                    color: Theme.textMuted
                    font.pixelSize: AppStyle.fontSizeSmall
                }

                GridLayout {
                    columns: 4
                    rowSpacing: 8
                    columnSpacing: 8
                    Layout.fillWidth: true

                    Repeater {
                        model: NetplaySlotsModel

                        delegate: SlotCard {
                            Layout.fillWidth: true
                            implicitHeight: 72
                            onClicked: {
                                if (NetworkService.isHost) {
                                    root.pickingSlot = model.slotNumber - 1
                                    memberPicker.open()
                                }
                            }
                        }
                    }
                }

                Item {
                    Layout.fillHeight: true
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Button {
                        id: readyButton
                        checkable: true
                        text: checked ? "Ready!" : "Ready up"
                        onToggled: NetworkService.setReady(checked)
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    Button {
                        visible: NetworkService.isHost
                        enabled: NetworkService.hasGame
                        text: "Start game"
                        onClicked: NetworkService.startSession()
                    }
                }
            }

            // Right: chat.
            LobbyChatPanel {
                Layout.preferredWidth: 280
                Layout.fillHeight: true
            }
        }
    }

    // Host-only: pick which member sits in the clicked slot.
    Popup {
        id: memberPicker
        modal: true
        anchors.centerIn: parent
        width: 280
        padding: 12

        contentItem: ColumnLayout {
            spacing: 8

            Text {
                text: "Player " + (root.pickingSlot + 1)
                color: Theme.textPrimary
                font.pixelSize: AppStyle.fontSizeMedium
            }

            Repeater {
                model: memberPicker.visible ? NetworkService.lobbyMembers() : []

                delegate: Button {
                    required property var modelData
                    text: modelData.displayName
                    Layout.fillWidth: true
                    onClicked: {
                        NetworkService.assignSlot(root.pickingSlot,
                                                  modelData.memberId, 0)
                        memberPicker.close()
                    }
                }
            }

            Button {
                text: "Clear slot"
                Layout.fillWidth: true
                onClicked: {
                    NetworkService.clearSlot(root.pickingSlot)
                    memberPicker.close()
                }
            }
        }
    }

    NetplayGamePickerDialog {
        id: gamePicker
    }

    Connections {
        target: NetworkService
        function onLobbyStateChanged() {
            if (!NetworkService.inLobby) {
                readyButton.checked = false
            }
        }
        function onPhaseChanged() {
            if (NetworkService.sessionPhase === "idle") {
                readyButton.checked = false
            }
        }
    }
}
