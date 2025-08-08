import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

FirelightDialog {
    id: root

    required property GamepadStatus gamepad

    property var imageSourceUrl
    property var platformName
    property variant buttons: []

    property int currentIndex: 0
    property bool canAcceptAxisInput: true

    property int numButtons: 0

    showButtons: false
    closePolicy: Popup.NoAutoClose

    signal mappingAdded(var original, var mapped)

    onAboutToShow: {
        currentIndex = 0
        numButtons = buttons.length
        frameAnimation.reset()
    }

    onOpened: function () {
        timer.start()
        console.log(dialog.buttons.length)
        console.log(dialog.currentIndex)
    }

    onClosed: function () {
        timer.stop()
        dialog.buttons = []
    }

    Connections {
        target: gamepad
        enabled: root.visible
        function onInputChanged(input, activated) {
            if (activated && root.canAcceptAxisInput) {
                axisDebounceTimer.restart()
                root.mappingAdded(root.buttons[root.currentIndex].retropad_button, input)
                if (root.buttons.length > root.currentIndex + 1) {
                    root.currentIndex++
                    timer.stop()
                    frameAnimation.reset()
                    timer.restart()
                } else {
                    root.accept()
                }
            }
        }
    }

    // GamepadStatus {
    //     id: thing
    //     playerNumber: gamepad.playerNumber
    //     onInputChanged: function(input, activated) {
    //         if  (!root.visible) {
    //             return
    //         }
    //         if (activated && root.canAcceptAxisInput) {
    //             axisDebounceTimer.restart()
    //             root.mappingAdded(root.buttons[root.currentIndex].retropad_button, input)
    //             if (root.buttons.length > root.currentIndex + 1) {
    //                 root.currentIndex++
    //                 timer.stop()
    //                 frameAnimation.reset()
    //                 timer.restart()
    //             } else {
    //                 root.accept()
    //             }
    //         }
    //     }
    // }

    Timer {
        id: axisDebounceTimer
        interval: 300
        running: false
        repeat: false
    }

    contentItem: ColumnLayout {
        spacing: 12

        Keys.onPressed: function (event) {
            event.accept = false
        }

        Keys.onReleased: function (event) {
            event.accept = false
        }

        Image {
            // Layout.preferredHeight: 300
            Layout.maximumWidth: 300
            Layout.maximumHeight: 300
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            source: root.imageSourceUrl
            sourceSize.height: 512
            fillMode: Image.PreserveAspectFit
        }
        Text {
            text: "Press a button on your controller to assign it to this " + platformList.currentItem.model.display_name + " input:"
            wrapMode: Text.WordWrap
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: parent.width * 5 / 6

            color: "white"
            font.family: Constants.regularFontFamily
            font.weight: Font.Light
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 18
        }

        Text {
            text: root.numButtons > 0 && dialog.currentIndex < root.numButtons ? dialog.buttons[dialog.currentIndex].display_name : "Nothing"
            wrapMode: Text.WordWrap
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            color: ColorPalette.neutral200
            font.pixelSize: 20
            font.weight: Font.Bold
            font.family: Constants.regularFontFamily
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        Timer {
            id: timer
            interval: 4000
            running: false
            repeat: false
            onTriggered: {
                dialog.reject()
                // dialog.close()
                // saveMapping()
            }
        }

        FrameAnimation {
            id: frameAnimation
            running: timer.running
        }

        Rectangle {
            id: theBar
            property var widthThing: parent.width * ((timer.interval - frameAnimation.elapsedTime * 1000) / timer.interval)
            Layout.preferredWidth: widthThing
            Layout.topMargin: 8
            Layout.preferredHeight: 10
            color: "green"
        }


    }

}