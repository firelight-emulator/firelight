import QtQuick
import QtQuick.Controls
import QtMultimedia
import QtQuick.Effects
import QtQml
import Qt.labs.qmlmodels 1.0
// import Qt5Compat.GraphicalEffects
import Firelight 1.0

FocusScope {
    id: root

    Timer {
        id: wheelTimer
        interval: 500
        repeat: false
    }

    WheelHandler {
        onWheel: function(event) {
            if (wheelTimer.running) {
                if (list.currentIndex === 0 && event.angleDelta.y >= 0) {
                    wheelTimer.restart()
                    return
                } else if (list.currentIndex === list.count - 1 && event.angleDelta.y < 0) {
                    wheelTimer.restart()
                    return
                }
            }
            if (event.angleDelta.y >= 0) {
                list.decrementCurrentIndex()
            } else {
                list.incrementCurrentIndex()
            }
            wheelTimer.restart()
            // list.incrementCurrentIndex()
            console.log("Wheel event: ", event.angleDelta.y)

        }
    }



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
        id: list
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

        delegate: chooser

        model: ListModel {
            ListElement {
                cover: "https://cdn2.steamgriddb.com/thumb/914dffd52e08c7be71f43865656f703e.jpg"
                name: "Paper Mario"
                platform: "Nintendo 64"
                type: "game"
            }
            ListElement {
                cover: "https://cdn2.steamgriddb.com/thumb/2c323abe873b4f9fa8a72f45785df5f0.jpg"
                name: "Golden Sun"
                platform: "Game Boy Advance"
                type: "game"
            }
            ListElement {
                cover: "https://cdn2.steamgriddb.com/thumb/cf68791aa992ebbe8c0ea1ac0d887e90.jpg"
                name: "Pokémon Red Version"
                platform: "Game Boy"
                type: "game"
            }
            ListElement {
                cover: "https://cdn2.steamgriddb.com/thumb/1e75612dbe09ed687c51498db305d2ed.jpg"
                name: "Mario Kart 64: Amped Up"
                platform: "Nintendo 64"
                type: "game"
            }
            ListElement {
                cover: "https://cdn2.steamgriddb.com/thumb/280bb2c98809304e66151313503f14fb.jpg"
                name: "Super Mario 64 DS"
                platform: "Nintendo DS"
                type: "game"
            }
            ListElement {
                cover: "https://cdn2.steamgriddb.com/thumb/914dffd52e08c7be71f43865656f703e.jpg"
                name: "Item 1"
                platform: "Nintendo 64"
                type: "game"
            }
            ListElement {
                cover: "https://cdn2.steamgriddb.com/thumb/914dffd52e08c7be71f43865656f703e.jpg"
                name: "Item 1"
                platform: "Nintendo 64"
                type: "game"
            }
            ListElement {
                cover: "https://cdn2.steamgriddb.com/thumb/914dffd52e08c7be71f43865656f703e.jpg"
                name: "Item 1"
                platform: "Nintendo 64"
                type: "game"
            }
            ListElement {
                cover: "https://cdn2.steamgriddb.com/thumb/914dffd52e08c7be71f43865656f703e.jpg"
                name: "Item 1"
                platform: "Nintendo 64"
                type: "game"
            }
            ListElement {
                cover: "https://cdn2.steamgriddb.com/thumb/914dffd52e08c7be71f43865656f703e.jpg"
                name: "Item 1"
                platform: "Nintendo 64"
                type: "game"
            }
            ListElement {
                type: "allgames"
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

        DelegateChooser {
            id: chooser

            role: "type"

            DelegateChoice {
                roleValue: "game"

                FocusScope {
                    id: gameDele

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
                        height: gameDele.ListView.isCurrentItem ? 270 : 180
                        source: model.cover
                        width: gameDele.ListView.isCurrentItem ? 270 : 180

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

                        HoverHandler {
                            cursorShape: Qt.PointingHandCursor
                        }
                    }
                    Text {
                        id: titleText

                        anchors.left: gameDele.right
                        anchors.leftMargin: 16
                        anchors.top: gameDele.top
                        color: "white"
                        font.pixelSize: 32
                        font.weight: Font.Bold
                        text: model.name
                        visible: gameDele.ListView.isCurrentItem
                    }
                    Text {
                        anchors.left: gameDele.right
                        anchors.leftMargin: 16
                        anchors.top: titleText.bottom
                        anchors.topMargin: 4
                        color: "white"
                        font.pixelSize: 20
                        // font.weight: Font.Bold
                        text: model.platform
                        visible: gameDele.ListView.isCurrentItem
                    }
                }
            }
            DelegateChoice {
                roleValue: "allgames"

                FocusScope {
                    id: allDele

                    required property var model

                    height: 270
                    width: ListView.isCurrentItem ? 270 : 180

                    Behavior on width {
                        NumberAnimation {
                            duration: 80
                        }
                    }

                    Button {
                        property bool showGlobalCursor: true
                        padding: 12
                        focus: true
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: padding
                        height: (allDele.ListView.isCurrentItem ? 270 : 180) - padding * 2
                        width: (allDele.ListView.isCurrentItem ? 270 : 180) - padding * 2

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

                        background: Rectangle {
                            color: "white"
                            opacity: 0.2
                            radius: parent.height / 2
                        }

                        // contentItem: Image {
                        //     source: "https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcRfeXY-nKQCcXveINmPT6HWZAdF2ohWc_wvog&s"
                        // }
                    }

                    Text {
                        anchors.left: allDele.right
                        anchors.leftMargin: 16
                        anchors.top: allDele.top
                        color: "white"
                        font.pixelSize: 32
                        font.weight: Font.Bold
                        text: "All games"
                        visible: allDele.ListView.isCurrentItem
                    }
                }
            }

        }
    }
}
