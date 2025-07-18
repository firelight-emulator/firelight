import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

FirelightDialog {
    id: root

    property var imageSourceUrl
    property var platformName
    property variant buttons: []

    property int currentIndex: 0
    property bool canAcceptAxisInput: true

    showButtons: false
    closePolicy: Popup.NoAutoClose

    onAboutToShow: {
        controller_manager.blockGamepadInput = true
        currentIndex = 0
        frameAnimation.reset()
    }

    onOpened: function () {
        timer.start()
    }

    onClosed: function () {
        timer.stop()
        controller_manager.blockGamepadInput = false
        dialog.buttons = []
    }

    Timer {
        id: axisDebounceTimer
        interval: 300
        running: false
        repeat: false
    }

    Connections {
        target: controller_manager

        function onRetropadInputStateChanged(player, input, activated) {
            if (!dialog.visible) {
                return
            }
            if (activated && !axisDebounceTimer.running) {
                axisDebounceTimer.restart()
                inputMapping.addMapping(dialog.buttons[dialog.currentIndex].retropad_button, input)
                if (dialog.buttons.length > dialog.currentIndex + 1) {
                    dialog.currentIndex++
                    timer.stop()
                    frameAnimation.reset()
                    timer.restart()

                } else {
                    dialog.accept()
                    // dialog.close()
                    // saveMapping()
                }
            }
        }
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
            text: dialog.buttons.length > 0 && dialog.currentIndex < dialog.buttons.length ? dialog.buttons[dialog.currentIndex].display_name : "Nothing"
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