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
        exitAnimationWithoutBackToZero.start();
    }

    Keys.onEscapePressed: function (event) {
        // root.StackView.view.popCurrentItem(StackView.Immediate)
        exitAnimation.start();
    }

    Keys.onBackPressed: function (event) {
        exitAnimation.start();
    }

    Keys.onSpacePressed: function (event) {
        if (theList.currentIndex === 0) {
            exitAnimation.start();
        } else {
            rewindPointSelected(theList.currentIndex);
        }
    }

    Keys.onSelectPressed: function (event) {
        if (theList.currentIndex === 0) {
            exitAnimation.start();
        } else {
            rewindPointSelected(theList.currentIndex);
        }
    }

    StackView.onActivated: function () {
        // root.openSfx.stop()
        // root.openSfx.play()
        // console.log(root.openSfx.mediaStatus)
        sfx_player.play("rewindopen");
        enterAnimation.start();
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
                root.StackView.view.popCurrentItem(StackView.Immediate);
            }
        }
    }

    SequentialAnimation {
        id: exitAnimation
        NumberAnimation {
            target: theList
            property: "currentIndex"
            to: 0
            duration: Math.min(Math.max(theList.currentIndex * 40, 0), 300)
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
                root.StackView.view.popCurrentItem(StackView.Immediate);
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
        fillMode: Image.Stretch
    }

    Pane {
        id: barthing
        focus: true

        width: parent.width
        height: root.barHeight
        y: parent.height - root.barHeight
        background: Rectangle {
            color: Theme.surface
        }

        contentItem: RewindList {
            id: theList
            model: root.model
            focus: true
            aspectRatio: root.aspectRatio

            onItemSelected: function (index) {
                if (index === 0) {
                    exitAnimation.start();
                } else {
                    root.rewindPointSelected(index);
                }
            }
        }
    }
}
