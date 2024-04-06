import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models
import FirelightStyle 1.0

Pane {
    id: root

    background: Item {
    }

    FirelightDialog {
        id: contentPopup
        width: parent.width * 0.75
        height: parent.height * 0.9
        parent: Overlay.overlay
        x: (parent.width / 2) - (width / 2)
        y: (parent.height / 2) - (height / 2)
        contentItem: StoreContent {
            anchors.centerIn: parent
        }

        header: Item {
            height: 0
            width: 0
        }
        footer: Pane {
            background: Rectangle {
                color: "transparent"
                radius: 4
            }

            RowLayout {
                anchors.fill: parent
                spacing: 8

                Button {
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    text: "Add to Library"
                    onClicked: {
                        contentPopup.close()
                    }
                }

            }
        }
    }

    Pane {
        id: content
        anchors.fill: parent

        background: Item {
        }

        Button {
            id: backButton
            text: "Back"
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.topMargin: 8
            anchors.leftMargin: 8
            onClicked: {
                contentPopup.open()
            }
        }

        ListView {
            id: list
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 600
            clip: true
            spacing: 8
            model: ListModel {
                ListElement {
                    name: "Pokemon Radical Red"
                    source: "file:///Users/alexs/git/firelight/build/pkmnrr.png"
                    original: "Pokemon Fire Red"
                    platform: "Game Boy Advance"
                    creator: "soupercell"
                }
                ListElement {
                    name: "Paper Mario: Master Quest"
                    source: "file:///Users/alexs/git/firelight/build/pmmasterquest.jpg"
                    original: "Paper Mario"
                    platform: "Nintendo 64"
                    creator: "..."
                }
                ListElement {
                    name: "Pokemon Blaze Black"
                    source: "file:///Users/alexs/git/firelight/build/blazeblack.jpg"
                    original: "Pokemon Black"
                    platform: "Nintendo DS"
                    creator: "..."
                }
                ListElement {
                    name: "Pokemon Gaia Version"
                    source: "file:///Users/alexs/git/firelight/build/gaia.jpg"
                    original: "Pokemon Fire Red"
                    platform: "Game Boy Advance"
                    creator: "..."
                }
                ListElement {
                    name: "Ultimate Goomboss Challenge"
                    source: "file:///Users/alexs/git/firelight/build/ultimategoomboss.png"
                    original: "Paper Mario"
                    platform: "Nintendo 64"
                    creator: "Enneagon"
                }
                ListElement {
                    name: "Pokemon Unbound"
                    source: "file:///Users/alexs/git/firelight/build/unbound.png"
                    original: "Pokemon Fire Red"
                    platform: "Game Boy Advance"
                    creator: "..."
                }
                ListElement {
                    name: "Grand Poo World 2"
                    source: "file:///Users/alexs/git/firelight/build/gpw2.jpg"
                    original: "Super Mario World"
                    platform: "SNES"
                    creator: "BarbarousKing"
                }
            }

            delegate: Pane {
                height: 100
                width: parent.width
                padding: 4

                background: Rectangle {
                    color: "white"
                    opacity: hov.hovered ? 0.2 : 0.1
                    radius: 4

                    Behavior on opacity {
                        NumberAnimation {
                            duration: 100
                            easing.type: Easing.InOutQuad
                        }
                    }
                }

                HoverHandler {
                    id: hov
                    cursorShape: Qt.PointingHandCursor
                }

                RowLayout {
                    anchors.fill: parent
                    spacing: 8

                    Image {
                        Layout.fillHeight: true
                        Layout.preferredWidth: height * 2
                        source: model.source
                        fillMode: Image.Stretch
                    }

                    ColumnLayout {
                        Layout.topMargin: 4
                        Layout.bottomMargin: 4
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            Layout.fillWidth: true
                            text: model.name
                            font.family: Constants.semiboldFontFamily
                            font.pointSize: 16
                            verticalAlignment: Text.AlignVCenter
                            color: "white"
                        }

                        Text {
                            Layout.fillWidth: true
                            text: "by " + model.creator
                            font.family: Constants.regularFontFamily
                            font.pointSize: 10
                            verticalAlignment: Text.AlignVCenter
                            color: "#cfcfcf"
                        }
                        Item {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                        }

                        Text {
                            Layout.fillWidth: true
                            text: "Mod for " + model.original + " (" + model.platform + ")"
                            font.family: Constants.regularFontFamily
                            font.pointSize: 10
                            verticalAlignment: Text.AlignVCenter
                            color: "#cfcfcf"
                        }
                    }
                }
            }
        }

        // HackPage {
        //     anchors.left: list.right
        //     anchors.top: parent.top
        //     anchors.bottom: parent.bottom
        //     anchors.right: parent.right
        // }
    }
}