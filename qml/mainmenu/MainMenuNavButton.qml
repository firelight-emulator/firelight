import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


TabButton {
    id: myButton

    required property string iconCode
    required property string toolTipText

    implicitHeight: width

    contentItem: Text {
        // text: "Settings"
        text: iconCode
        color: (myHover.hovered || myButton.checked) ? "white" : "#b3b3b3"
        font.pixelSize: 28
        // font.family: Constants.strongFontFamily
        font.family: Constants.symbolFontFamily
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }
    scale: checked ? 1.1 : 1.0
    Behavior on scale {
        NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }

    background: Rectangle {
        color: "white"
        opacity: myButton.checked ? 0.1 : 0.0
        radius: 4

        anchors.centerIn: parent

        height: myButton.height * (2 / 3)
        width: myButton.width * (2 / 3)

        Behavior on opacity {
            NumberAnimation {
                duration: 200
                easing.type: Easing.InOutQuad
            }
        }
    }

    HoverHandler {
        id: myHover
        cursorShape: Qt.PointingHandCursor
    }

    ToolTip.visible: myHover.hovered
    ToolTip.text: toolTipText
    ToolTip.delay: 400
    ToolTip.timeout: 5000

    // ToolTip {
    //     id: tool
    //     visible: homeHover.hovered
    //     delay: 400
    //     timeout: 5000
    //
    //     ParallelAnimation {
    //         id: fadeIn
    //         NumberAnimation {
    //             target: tool
    //             property: "opacity"
    //             to: 1
    //             duration: 200
    //             easing.type: Easing.InOutQuad
    //         }
    //         NumberAnimation {
    //             target: tool
    //             property: "y"
    //             from: 43
    //             to: 48
    //             duration: 200
    //             easing.type: Easing.InOutQuad
    //         }
    //     }
    //
    //     onVisibleChanged: function () {
    //         if (visible) {
    //             opacity = 0 // Start from fully transparent
    //             y = 43
    //             fadeIn.start()
    //         }
    //     }
    //
    //     height: 24
    //
    //     contentItem: Text {
    //         text: "Home"
    //         color: Constants.rightClickMenuItem_TextColor
    //         font.pointSize: 12
    //         font.family: Constants.regularFontFamily
    //         horizontalAlignment: Text.AlignHCenter
    //         verticalAlignment: Text.AlignVCenter
    //     }
    //
    //     background: Rectangle {
    //         color: Constants.rightClickMenu_BackgroundColor
    //         radius: Constants.rightClickMenu_BackgroundRadius
    //     }
    // }
}