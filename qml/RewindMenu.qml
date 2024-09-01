import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts 1.0
import QtMultimedia
import QtQuick.Effects
import Firelight 1.0

FocusScope {
    id: root

    required property var model
    required property SoundEffect scrollSfx
    required property SoundEffect openSfx

    property var barHeight: 220

    Rectangle {
        color: "black"
        anchors.fill: parent
    }

    signal rewindPointSelected(var index)

    function close() {
        exitAnimationWithoutBackToZero.start()
    }

    Keys.onEscapePressed: function (event) {
        // root.StackView.view.popCurrentItem(StackView.Immediate)
        exitAnimation.start()
    }

    Keys.onSpacePressed: function (event) {
        if (theList.currentIndex === 0) {
            exitAnimation.start()
        } else {
            rewindPointSelected(theList.currentIndex)
        }
    }

    Keys.onSelectPressed: function (event) {
        if (theList.currentIndex === 0) {
            exitAnimation.start()
        } else {
            rewindPointSelected(theList.currentIndex)
        }
    }

    StackView.onActivated: function () {
        // root.openSfx.stop()
        // root.openSfx.play()
        // console.log(root.openSfx.mediaStatus)
        sfx_player.play("rewindopen")
        enterAnimation.start()
    }

    ParallelAnimation {
        id: enterAnimation
        NumberAnimation {
            target: barthing
            property: "y"
            from: root.height
            to: root.height - root.barHeight
            duration: 220
            easing.type: Easing.InOutQuad
        }
        NumberAnimation {
            target: coverImage
            property: "height"
            from: root.height
            to: root.height - root.barHeight
            duration: 220
            easing.type: Easing.InOutQuad
        }
    }

    SequentialAnimation {
        id: exitAnimationWithoutBackToZero
        PauseAnimation {
            duration: 100
        }
        ParallelAnimation {
            NumberAnimation {
                target: barthing
                property: "y"
                from: root.height - root.barHeight
                to: root.height
                duration: 220
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: coverImage
                property: "height"
                from: root.height - root.barHeight
                to: root.height
                duration: 220
                easing.type: Easing.InOutQuad
            }
        }
        ScriptAction {
            script: {
                console.log("Popping rewind menu")
                root.StackView.view.popCurrentItem(StackView.Immediate)
            }
        }
    }

    SequentialAnimation {
        id: exitAnimation
        NumberAnimation {
            target: theList
            property: "currentIndex"
            to: 0
            duration: (theList.currentIndex * 40)
            easing.type: Easing.OutQuad
        }
        PauseAnimation {
            duration: 100
        }
        ParallelAnimation {
            NumberAnimation {
                target: barthing
                property: "y"
                from: root.height - root.barHeight
                to: root.height
                duration: 220
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: coverImage
                property: "height"
                from: root.height - root.barHeight
                to: root.height
                duration: 220
                easing.type: Easing.InOutQuad
            }
        }
        ScriptAction {
            script: {
                root.StackView.view.popCurrentItem(StackView.Immediate)
            }
        }
    }

    Image {
        id: coverImage
        anchors.horizontalCenter: parent.horizontalCenter
        // height: parent.height
        width: parent.width
        height: parent.height - root.barHeight
        mipmap: true
        smooth: false
        cache: false
        source: theList.count > 0 ? theList.itemAtIndex(theList.currentIndex).model.modelData.image_url : ""
        // onSourceChanged: function () {
        // }
        fillMode: Image.PreserveAspectFit
    }


    Pane {
        id: barthing
        focus: true


        width: parent.width
        height: root.barHeight
        y: parent.height - root.barHeight
        background: Rectangle {
            color: ColorPalette.neutral1000
        }

        contentItem: ListView {
            id: theList
            orientation: ListView.Horizontal
            layoutDirection: Qt.RightToLeft
            highlightMoveDuration: 80
            highlightMoveVelocity: -1
            focus: true
            keyNavigationEnabled: true
            highlightRangeMode: ListView.StrictlyEnforceRange
            preferredHighlightBegin: width / 2 - 75
            preferredHighlightEnd: width / 2 + 75
            currentIndex: 0
            spacing: 8
            highlight: Item {
            }

            // SoundEffect {
            //     id: scrollSound
            //     loops: 0
            //     source: "file:system/sfx/cursor5.wav"
            // }

            onCurrentIndexChanged: {
                // scrollSound.play()
                // if (currentIndex == 0) {
                //     coverImage.source = "image://gameImages/0"
                // } else {
                //     coverImage.source = theList.itemAtIndex(currentIndex).model.image_url
                // }
                // if (coverImageBg.state === "showing") {
                // }

                // console.log(root.scrollSfx.mediaStatus)
                // root.scrollSfx.stop()
                // root.scrollSfx.play()
                sfx_player.play("rewindscroll")
            }


            model: root.model
            delegate: Item {
                id: dele
                required property var model
                required property var index
                height: dele.ListView.view.height
                width: imageThing.width
                focus: true

                Item {
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.left: parent.left
                    anchors.bottom: timeLabel.top

                    Image {
                        id: imageThing
                        Rectangle {
                            visible: dele.ListView.isCurrentItem
                            z: -1
                            width: parent.width + 8
                            height: parent.height + 8
                            x: -4
                            y: -4
                            color: "white"
                        }

                        cache: false
                        TapHandler {
                            onTapped: {
                                theList.currentIndex = index
                            }
                        }


                        mipmap: true
                        smooth: false
                        source: dele.model.modelData.image_url
                        height: parent.height
                        // height: dele.ListView.isCurrentItem ? dele.height - 20 : dele.height - 40
                        // height: dele.height - 20
                        Behavior on height {
                            NumberAnimation {
                                duration: 100
                                easing.type: Easing.InOutQuad
                            }
                        }
                        Behavior on width {
                            NumberAnimation {
                                duration: 100
                                easing.type: Easing.InOutQuad
                            }
                        }
                        fillMode: Image.PreserveAspectFit
                    }
                }


                Text {
                    id: timeLabel
                    anchors.left: parent.left
                    // anchors.bottom: timeAgoLabel.top
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    height: 24
                    text: dele.model.modelData.time
                    font.pixelSize: 14
                    font.family: Constants.regularFontFamily
                    font.weight: Font.DemiBold
                    wrapMode: Text.WordWrap
                    padding: 12
                    color: ColorPalette.neutral200
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }


                // Text {
                //     id: timeAgoLabel
                //     anchors.left: parent.left
                //     anchors.bottom: parent.bottom
                //     anchors.right: parent.right
                //     height: 24
                //     text: dele.model.modelData.ago
                //     font.pixelSize: 15
                //     font.family: Constants.regularFontFamily
                //     font.weight: Font.DemiBold
                //     wrapMode: Text.WordWrap
                //     color: ColorPalette.neutral200
                //     horizontalAlignment: Text.AlignHCenter
                //     verticalAlignment: Text.AlignVCenter
                // }
            }
        }
    }


}