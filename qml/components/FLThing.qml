import QtQuick
import QtQuick.Controls
import QtMultimedia
import QtQuick.Effects
// import Qt5Compat.GraphicalEffects
import Firelight 1.0

FocusScope {
    id: root

    MediaPlayer {
        id: sound

        source: "qrc:sfx/click"

        audioOutput: AudioOutput {
        }
    }
    PropertyAnimation {
        id: volumeFadeOut

        duration: 50  // Quick fade-out
        property: "volume"
        target: sound.audioOutput
        to: 0.0

        onStopped: {
            sound.stop();
            sound.audioOutput.volume = 0.8;  // Reset volume
            sound.playbackRate = (Math.random() * 0.25) + 1;
            sound.play();
        }  // Stop sound once faded out
    }

    // SoundEffect {
    //     id: sound
    //     source: "qrc:sfx/click"
    //
    // }
    ListView {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 32
        focus: true
        height: 300
        highlightMoveDuration: 80
        highlightMoveVelocity: -1
        highlightRangeMode: ListView.StrictlyEnforceRange
        keyNavigationEnabled: true
        keyNavigationWraps: true
        orientation: ListView.Horizontal
        preferredHighlightBegin: 140
        preferredHighlightEnd: 340
        spacing: 16
        width: parent.width

        delegate: FocusScope {
            id: dele

            required property var model

            height: 270
            width: ListView.isCurrentItem ? 270 : 180

            Behavior on width {
                NumberAnimation {
                    duration: 80
                }
            }

            Image {
                id: sourceItem

                property bool showGlobalCursor: true

                anchors.bottom: parent.bottom
                focus: true
                height: dele.ListView.isCurrentItem ? 270 : 180
                source: model.cover
                width: dele.ListView.isCurrentItem ? 270 : 180

                Behavior on height {
                    NumberAnimation {
                        duration: 80
                    }
                }
                // radius: 4

                Behavior on width {
                    NumberAnimation {
                        duration: 80
                    }
                }
            }
            Text {
                id: titleText

                anchors.left: dele.right
                anchors.leftMargin: 16
                anchors.top: dele.top
                color: "white"
                font.pixelSize: 32
                font.weight: Font.Bold
                text: model.name
                visible: dele.ListView.isCurrentItem
            }
            Text {
                anchors.left: dele.right
                anchors.leftMargin: 16
                anchors.top: titleText.bottom
                anchors.topMargin: 4
                color: "white"
                font.pixelSize: 20
                // font.weight: Font.Bold
                text: model.platform
                visible: dele.ListView.isCurrentItem
            }
        }
        model: ListModel {
            ListElement {
                cover: "https://cdn2.steamgriddb.com/thumb/914dffd52e08c7be71f43865656f703e.jpg"
                name: "Paper Mario"
                platform: "Nintendo 64"
            }
            ListElement {
                cover: "https://cdn2.steamgriddb.com/thumb/2c323abe873b4f9fa8a72f45785df5f0.jpg"
                name: "Golden Sun"
                platform: "Game Boy Advance"
            }
            ListElement {
                cover: "https://cdn2.steamgriddb.com/thumb/cf68791aa992ebbe8c0ea1ac0d887e90.jpg"
                name: "Pokémon Red Version"
                platform: "Game Boy"
            }
            ListElement {
                cover: "https://cdn2.steamgriddb.com/thumb/1e75612dbe09ed687c51498db305d2ed.jpg"
                name: "Mario Kart 64: Amped Up"
                platform: "Nintendo 64"
            }
            ListElement {
                cover: "https://cdn2.steamgriddb.com/thumb/280bb2c98809304e66151313503f14fb.jpg"
                name: "Super Mario 64 DS"
                platform: "Nintendo DS"
            }
            ListElement {
                cover: "https://cdn2.steamgriddb.com/thumb/914dffd52e08c7be71f43865656f703e.jpg"
                name: "Item 1"
                platform: "Nintendo 64"
            }
            ListElement {
                cover: "https://cdn2.steamgriddb.com/thumb/914dffd52e08c7be71f43865656f703e.jpg"
                name: "Item 1"
                platform: "Nintendo 64"
            }
            ListElement {
                cover: "https://cdn2.steamgriddb.com/thumb/914dffd52e08c7be71f43865656f703e.jpg"
                name: "Item 1"
                platform: "Nintendo 64"
            }
            ListElement {
                cover: "https://cdn2.steamgriddb.com/thumb/914dffd52e08c7be71f43865656f703e.jpg"
                name: "Item 1"
                platform: "Nintendo 64"
            }
            ListElement {
                cover: "https://cdn2.steamgriddb.com/thumb/914dffd52e08c7be71f43865656f703e.jpg"
                name: "Item 1"
                platform: "Nintendo 64"
            }
        }

        onCurrentIndexChanged: {
            // sound.stop()
            // sound.play()
            volumeFadeOut.start();
            // if (sound.playbackState === SoundEffect.PlayingState) {
            //     volumeFadeOut.start()
            // } else {
            //     sound.audioOutput.volume = 0.8  // Reset volume
            //     sound.play()
            // }
        }
    }
}
