import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    StackLayout {
        anchors.fill: parent
        currentIndex: {
            if (!NetworkService.inLobby) {
                return 0
            }
            if (!NetworkService.isHost && NetworkService.sessionPhase === "in-game") {
                return 2
            }
            return 1
        }

        // Entry view: sign in, then host or join by code.
        Item {
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 16
                width: Math.min(parent.width - 48, 420)

                Text {
                    text: "Play online"
                    color: Theme.textPrimary
                    font.pixelSize: 28
                    font.weight: Font.DemiBold
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: "Host a lobby and share your IP address, or enter a " +
                          "host's IP to join them. Friends can join and leave " +
                          "any time."
                    color: Theme.textMuted
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    Layout.fillWidth: true
                }

                // Signed out: one button, labeled from the provider.
                ColumnLayout {
                    visible: NetworkService.signInState !== "ready"
                    spacing: 8
                    Layout.fillWidth: true

                    Button {
                        text: NetworkService.signInState === "signing-in"
                              ? "Signing in..."
                              : "Sign in with " + NetworkService.providerName
                        enabled: NetworkService.signInState !== "signing-in"
                        Layout.alignment: Qt.AlignHCenter
                        onClicked: NetworkService.signIn()
                    }
                    Text {
                        visible: NetworkService.signInState === "failed"
                        text: "Sign-in didn't go through. Give it another try."
                        color: Theme.danger
                        font.pixelSize: 13
                        Layout.alignment: Qt.AlignHCenter
                    }
                }

                ColumnLayout {
                    visible: NetworkService.signInState === "ready"
                    spacing: 12
                    Layout.fillWidth: true

                    Text {
                        visible: NetworkService.providerName.length > 0
                        text: "Signed in as " + NetworkService.playerName
                        color: Theme.textMuted
                        font.pixelSize: 13
                        Layout.alignment: Qt.AlignHCenter
                    }

                    RowLayout {
                        visible: NetworkService.providerName.length === 0
                        spacing: 8
                        Layout.fillWidth: true

                        Text {
                            text: "Your name"
                            color: Theme.textMuted
                            font.pixelSize: 13
                        }
                        TextField {
                            id: playerNameField
                            text: NetworkService.playerName
                            maximumLength: 24
                            Layout.fillWidth: true
                            onEditingFinished: {
                                if (text.trim().length > 0) {
                                    NetworkService.setPlayerName(text)
                                }
                            }
                        }
                    }

                    Button {
                        text: "Host a lobby"
                        enabled: !NetworkService.busy
                        Layout.alignment: Qt.AlignHCenter
                        onClicked: NetworkService.hostLobby()
                    }

                    RowLayout {
                        spacing: 8
                        Layout.fillWidth: true

                        TextField {
                            id: joinCodeField
                            placeholderText: "Host IP (e.g. 192.168.1.50)"
                            Layout.fillWidth: true
                            onAccepted: joinButton.clicked()
                        }
                        Button {
                            id: joinButton
                            text: "Join"
                            enabled: joinCodeField.text.trim().length > 0
                                     && !NetworkService.busy
                            onClicked: NetworkService.joinLobby(joinCodeField.text)
                        }
                    }
                }

                Text {
                    id: errorLabel
                    text: ""
                    visible: text.length > 0
                    color: Theme.danger
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    Layout.fillWidth: true
                }
            }
        }

        NetplayLobbyPage {
        }

        GuestStreamPage {
        }
    }

    Connections {
        target: NetworkService
        function onErrorOccurred(message) {
            errorLabel.text = message
        }
        function onLobbyStateChanged() {
            if (NetworkService.inLobby) {
                errorLabel.text = ""
            }
        }
    }
}
