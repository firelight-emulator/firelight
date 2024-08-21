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
    property var barHeight: 150

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

    StackView.onActivated: function () {
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
        fillMode: Image.PreserveAspectFit
    }


    Pane {
        id: barthing


        width: parent.width
        height: root.barHeight
        y: parent.height - root.barHeight
        background: Rectangle {
            color: "grey"
        }

        contentItem: ListView {
            id: theList
            orientation: ListView.Horizontal
            layoutDirection: Qt.RightToLeft
            focus: true
            highlightMoveDuration: 80
            highlightMoveVelocity: -1
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
            //     source: "file:system/sfx/glass_001.wav"
            // }

            onCurrentIndexChanged: {
                // if (currentIndex == 0) {
                //     coverImage.source = "image://gameImages/0"
                // } else {
                //     coverImage.source = theList.itemAtIndex(currentIndex).model.image_url
                // }
                // if (coverImageBg.state === "showing") {
                //     sfx_player.play()
                // }
            }


            model: root.model
            delegate: Image {
                required property var model
                required property var index
                Rectangle {
                    visible: parent.ListView.isCurrentItem
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
                source: model.modelData.image_url
                anchors.verticalCenter: ListView.view.contentItem.verticalCenter
                height: ListView.isCurrentItem ? ListView.view.height + 20 : ListView.view.height
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
    }


}