import QtQuick
import QtQuick.Controls
import QtMultimedia
import QtQuick.Effects
import QtQuick.Layouts
import QtQml
import Qt.labs.qmlmodels 1.0
import QtQuick.Shapes 1.8
// import Qt5Compat.GraphicalEffects
import Firelight 1.0

FocusScope {
    id: root

    // required property Component libraryPage

    Timer {
        id: wheelTimer
        interval: 500
        repeat: false
    }

    Component {
        id: gameDetailsPanel
        FLGameDetailsPanel {

        }
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
        }
    }

    FLSoundEffect {
        id: sfx
        source: "qrc:sfx/click"
    }

    ListView {
        id: list
        anchors.top: parent.top
        anchors.topMargin: 60
        focus: true
        height: 300
        highlightMoveDuration: 80
        highlightMoveVelocity: -1
        highlightRangeMode: ListView.StrictlyEnforceRange
        keyNavigationEnabled: true
        keyNavigationWraps: true
        orientation: ListView.Horizontal
        preferredHighlightBegin: 100
        preferredHighlightEnd: 300
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
            sfx.play();
            // if (sound.playbackState === SoundEffect.PlayingState) {
            //     volumeFadeOut.start()
            // } else {
            //     sound.audioOutput.volume = 0.8  // Reset volume
            //     sound.play()
            // }
        }

        SequentialAnimation {
            id: listAnimation

            property Item target

            PropertyAction {
                target: listAnimation.target
                property: "opacity"
                value: 0.24
            }

            PropertyAction {
                target: listAnimation.target
                property: "visible"
                value: true
            }

            ParallelAnimation {
                ScriptAction {
                    script: {
                        sfx_player.play("startgame")
                    }
                }
                NumberAnimation {
                    target: listAnimation.target
                    property: "scale"
                    from: 1.0
                    to: 1.2
                    duration: 800
                    easing.type: Easing.OutQuad
                }
                SequentialAnimation {
                    PauseAnimation {
                        duration: 760
                    }
                    NumberAnimation {
                        target: listAnimation.target
                        property: "opacity"
                        to: 0
                        duration: 300
                        easing.type: Easing.InQuad
                    }
                }
            }

            PropertyAction {
                target: listAnimation.target
                property: "visible"
                value: false
            }

            NumberAnimation {
                target: overlay
                property: "opacity"
                from: 0
                to: 1
                duration: 350
                easing.type: Easing.InQuad
            }

            PauseAnimation {
                duration: 1000
            }

            NumberAnimation {
                target: overlay
                property: "opacity"
                from: 1
                to: 0
                duration: 350
                easing.type: Easing.InQuad
            }
        }

        DelegateChooser {
            id: chooser

            role: "type"

            DelegateChoice {
                roleValue: "game"

                FocusScope {
                    id: gameDele

                    required property var model
                    required property var index

                    height: 270
                    width: ListView.isCurrentItem ? 270 : 180

                    Behavior on width {
                        NumberAnimation {
                            duration: 80
                        }
                    }

                    Button {
                        property bool showGlobalCursor: true
                        anchors.top: parent.top
                        padding: 0
                        spacing: 0
                        horizontalPadding: 0
                        focus: true
                        height: gameDele.ListView.isCurrentItem ? 270 : 180
                        width: gameDele.ListView.isCurrentItem ? 270 : 180

                        property var inputHints: [
                            {
                                input: Qt.Key_Select,
                                label: "Play"
                            },
                            {
                                input: Qt.Key_Menu,
                                label: "Details"
                            }
                        ]

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

                        Keys.onMenuPressed: function (event) {
                            root.StackView.view.push(gameDetailsPanel, {}, StackView.PushTransition)
                        }

                        onClicked: {
                            if (!gameDele.ListView.isCurrentItem) {
                                gameDele.ListView.view.currentIndex = gameDele.index
                                return
                            } else {
                                listAnimation.target = imageClone
                                listAnimation.start()
                            }
                        }

                        HoverHandler {
                            cursorShape: Qt.PointingHandCursor
                        }

                        background: Item {}

                        contentItem: Item {

                            // Image {
                            //     id: imageClone
                            //     visible: false
                            //     anchors.fill: parent
                            //     source: model.cover
                            // }

                            // Rectangle {
                            //     id: maskShape
                            //     width: 256
                            //     height: 256
                            //     radius: 16
                            //     color: "black" // Color determines opacity for the mask
                            //     visible: false // Don't need to see the mask itself
                            //
                            //     // *** Crucial Part for MultiEffect ***
                            //     layer.enabled: true // Rasterize the rectangle into a texture
                            //     layer.smooth: true  // Improve edge quality
                            //     layer.samples: 16
                            //     // Optional: layer.samples helps with anti-aliasing, especially for curves
                            //     // layer.samples: 4
                            // }

                            Image {
                                id: sourceImage
                                anchors.fill: parent
                                source: model.cover
                                // layer.enabled: true
                                // layer.effect: MultiEffect {
                                //     id: rounded
                                //     source: sourceImage
                                //     height: 256
                                //     width: 256
                                //     maskEnabled: true
                                //     maskSource: maskShape
                                //     // maskSpreadAtMax: 0.02 // Adjust for smoother fade
                                //     // maskSpreadAtMin: 0.02 // Adjust for smoother fade
                                //     // maskThresholdMax: 1.0 // Usually leave at 1.0
                                //     // maskThresholdMin: 0 // Usually leave at 0.0
                                // }
                            }

                            // MultiEffect {
                            //     id: imageClone
                            //     visible: false
                            //     source: sourceImage
                            //     anchors.fill: parent
                            //     maskEnabled: true
                            //
                            //     // maskInverted: true
                            //     // maskSource: ShaderEffectSource {
                            //     //     anchors.fill: parent
                            //     //     sourceItem: Rectangle {
                            //     //         anchors.fill: parent
                            //     //         radius: height / 4
                            //     //         color: "#000000ff"
                            //     //     }
                            //     // }
                            //     // maskSource: Image {
                            //     //     source: "file:rectangle17.png"
                            //     //     anchors.fill: parent
                            //     // }
                            //     maskSpreadAtMax: 1.0 // Adjust for smoother fade
                            //     maskSpreadAtMin: 1.0 // Adjust for smoother fade
                            //     maskThresholdMax: 1.0 // Usually leave at 1.0
                            //     maskThresholdMin: 0.4 // Usually leave at 0.0
                            //     // blurEnabled: true
                            //     // blurMax: 64
                            //     // blur: 1.0
                            // }
                        }
                    }

                    Text {
                        id: titleText

                        anchors.left: gameDele.right
                        anchors.leftMargin: 16
                        anchors.bottom: platformText.top
                        anchors.bottomMargin: 4
                        color: "white"
                        font.pixelSize: 32
                        font.weight: Font.Bold
                        text: model.name
                        visible: gameDele.ListView.isCurrentItem
                    }
                    Text {
                        id: platformText
                        anchors.left: gameDele.right
                        anchors.leftMargin: 16
                        anchors.bottom: gameDele.bottom
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

                        property var inputHints: [
                            {
                                input: Qt.Key_Select,
                                label: "Select"
                            }
                        ]

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

                        onClicked: {
                            root.StackView.view.pushItem(libraryPage, {}, StackView.PushTransition)
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

    Rectangle {
        id: overlay
        anchors.fill: parent
        color: "black"
        opacity: 0
    }
}
