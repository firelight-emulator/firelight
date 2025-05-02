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
    required property var aspectRatio

    property var barHeight: 300

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

    Keys.onBackPressed: function(event) {
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
        width: height * root.aspectRatio
        height: parent.height - root.barHeight
        mipmap: true
        smooth: false
        cache: false
        source: theList.count > 0 ? theList.itemAtIndex(theList.currentIndex).model.modelData.image_url : ""
        // onSourceChanged: function () {
        // }
        fillMode: Image.Stretch
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

        contentItem: RewindList {
            id: theList
            model: root.model
            aspectRatio: root.aspectRatio

            onItemSelected: function (index) {
                if (index === 0) {
                    exitAnimation.start()
                } else {
                    root.rewindPointSelected(index)
                }
            }
        }

        // contentItem: ListView {
        //     id: theList
        //     orientation: ListView.Horizontal
        //     layoutDirection: Qt.RightToLeft
        //     highlightMoveDuration: 80
        //     highlightMoveVelocity: -1
        //     focus: true
        //     keyNavigationEnabled: true
        //     highlightRangeMode: ListView.StrictlyEnforceRange
        //     preferredHighlightBegin: width / 2 - (theList.height * root.aspectRatio / 2)
        //     preferredHighlightEnd: width / 2 + (theList.height * root.aspectRatio / 2)
        //     currentIndex: 0
        //     spacing: 8
        //     highlight: Item {
        //     }
        //
        //     model: root.model
        //     delegate: Button {
        //         id: dele
        //         required property var model
        //         required property var index
        //         height: ListView.view.height
        //         width: height * root.aspectRatio
        //         focus: true
        //
        //         onClicked: {
        //             if (dele.ListView.isCurrentItem) {
        //                 theList.highlightFollowsCurrentItem = false
        //                 root.rewindPointSelected(dele.index)
        //             } else {
        //                 theList.currentIndex = index
        //             }
        //         }
        //
        //         background: Rectangle {
        //             color: "transparent"
        //             radius: 2
        //         }
        //
        //         property bool showGlobalCursor: true
        //
        //         contentItem: Image {
        //             id: imageThing
        //
        //             cache: false
        //             mipmap: true
        //             smooth: false
        //             source: dele.model.modelData.image_url
        //             fillMode: Image.Stretch
        //
        //             Behavior on height {
        //                 NumberAnimation {
        //                     duration: 100
        //                     easing.type: Easing.InOutQuad
        //                 }
        //             }
        //             Behavior on width {
        //                 NumberAnimation {
        //                     duration: 100
        //                     easing.type: Easing.InOutQuad
        //                 }
        //             }
        //         }
        //     }
        //     // delegate: Item {
        //     //     id: dele
        //     //     required property var model
        //     //     required property var index
        //     //     height: dele.ListView.view.height
        //     //     width: imageThing.width
        //     //     focus: true
        //     //
        //     //     property bool showGlobalCursor: true
        //     //
        //     //     Item {
        //     //         anchors.top: parent.top
        //     //         anchors.right: parent.right
        //     //         anchors.left: parent.left
        //     //         anchors.bottom: timeLabel.top
        //     //
        //     //         Image {
        //     //             id: imageThing
        //     //             // Rectangle {
        //     //             //     visible: dele.ListView.isCurrentItem
        //     //             //     z: -10
        //     //             //     width: parent.width + 8
        //     //             //     height: parent.height + 8
        //     //             //     x: -4
        //     //             //     y: -4
        //     //             //     color: "white"
        //     //             // }
        //     //
        //     //             cache: false
        //     //             TapHandler {
        //     //                 onTapped: {
        //     //                     theList.currentIndex = index
        //     //                 }
        //     //             }
        //     //
        //     //
        //     //             mipmap: true
        //     //             smooth: false
        //     //             source: dele.model.modelData.image_url
        //     //             height: parent.height
        //     //             // height: dele.ListView.isCurrentItem ? dele.height - 20 : dele.height - 40
        //     //             // height: dele.height - 20
        //     //             Behavior on height {
        //     //                 NumberAnimation {
        //     //                     duration: 100
        //     //                     easing.type: Easing.InOutQuad
        //     //                 }
        //     //             }
        //     //             Behavior on width {
        //     //                 NumberAnimation {
        //     //                     duration: 100
        //     //                     easing.type: Easing.InOutQuad
        //     //                 }
        //     //             }
        //     //             fillMode: Image.PreserveAspectFit
        //     //         }
        //     //     }
        //     //
        //     //
        //     //     Text {
        //     //         id: timeLabel
        //     //         anchors.left: parent.left
        //     //         // anchors.bottom: timeAgoLabel.top
        //     //         anchors.bottom: parent.bottom
        //     //         anchors.right: parent.right
        //     //         height: 24
        //     //         text: dele.model.modelData.time
        //     //         font.pixelSize: 14
        //     //         font.family: Constants.regularFontFamily
        //     //         font.weight: Font.DemiBold
        //     //         wrapMode: Text.WordWrap
        //     //         padding: 12
        //     //         color: ColorPalette.neutral200
        //     //         horizontalAlignment: Text.AlignHCenter
        //     //         verticalAlignment: Text.AlignVCenter
        //     //     }
        //     // }
        // }
    }


}