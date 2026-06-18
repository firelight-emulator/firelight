import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts 1.0
import Firelight 1.0

FocusScope {
    id: root
    objectName: "Top Bar Utility Buttons"

    signal closeClicked()
    signal maximizeClicked()
    signal minimizeClicked()

    implicitWidth: buttonRow.implicitWidth
    implicitHeight: buttonRow.implicitHeight

    RowLayout {
        id: buttonRow
        objectName: "Top Bar Utility Buttons RowLayout"
        anchors.fill: parent
        spacing: 0

        Button {
            id: minButton
            objectName: "Top Bar Minimize Button"
            Layout.fillHeight: true
            Layout.preferredWidth: AppStyle.topBarUtilityButtonWidth
            Layout.topMargin: -(AppStyle.topBarPadding)
            Layout.bottomMargin: -(AppStyle.topBarPadding)

            focus: true

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }

            onClicked: root.minimizeClicked()

            Canvas {
                anchors.centerIn: parent
                width: AppStyle.topBarUtilityButtonIconSize
                height: AppStyle.topBarUtilityButtonIconSize
                onPaint: {
                    var ctx = getContext("2d");
                    ctx.clearRect(0, 0, width, height);
                    ctx.fillStyle = "white";
                    ctx.fillRect(0, height / 2, width, 1)
                    // ctx.fillRect(width * 0.25, height * 0.45, width * 0.5, height * 0.1);
                }
            }

            background: Rectangle {
                color: minButton.down ? AppStyle.topBarMinimizeButtonColorPressed : minButton.hovered ? AppStyle.topBarMinimizeButtonColorHovered : "transparent"
                opacity: minButton.down ? AppStyle.topBarMinimizeButtonOpacityPressed : minButton.hovered ? AppStyle.topBarMinimizeButtonOpacityHovered : 0
            }
        }

        Button {
            id: maxButton
            objectName: "Top Bar Maximize Button"
            Layout.fillHeight: true
            Layout.preferredWidth: AppStyle.topBarUtilityButtonWidth
            Layout.topMargin: -(AppStyle.topBarPadding)
            Layout.bottomMargin: -(AppStyle.topBarPadding)

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }

            onClicked: root.maximizeClicked()

            background: Rectangle {
                color: maxButton.down ? AppStyle.topBarMaximizeButtonColorPressed : maxButton.hovered ? AppStyle.topBarMaximizeButtonColorHovered : "transparent"
                opacity: maxButton.down ? AppStyle.topBarMaximizeButtonOpacityPressed : maxButton.hovered ? AppStyle.topBarMaximizeButtonOpacityHovered : 0
            }

            Rectangle {
                anchors.centerIn: parent
                width: AppStyle.topBarUtilityButtonIconSize
                height: AppStyle.topBarUtilityButtonIconSize
                color: "transparent"
                border.color: "white"
                border.width: 1
                radius: 1
            }
        }

        Button {
            id: closeButton
            objectName: "Top Bar Close Button"
            Layout.fillHeight: true
            Layout.preferredWidth: AppStyle.topBarUtilityButtonWidth
            Layout.topMargin: -(AppStyle.topBarPadding)
            Layout.bottomMargin: -(AppStyle.topBarPadding)
            Layout.rightMargin: -(AppStyle.topBarPadding)

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }

            background: Rectangle {
                color: closeButton.down ? AppStyle.topBarCloseButtonColorPressed : closeButton.hovered ? AppStyle.topBarCloseButtonColorHovered : "transparent"
                opacity: closeButton.down ? AppStyle.topBarCloseButtonOpacityPressed : closeButton.hovered ? AppStyle.topBarCloseButtonOpacityHovered : 0
            }

            Canvas {
                anchors.centerIn: parent
                width: AppStyle.topBarUtilityButtonIconSize
                height: AppStyle.topBarUtilityButtonIconSize
                onPaint: {
                    var ctx = getContext("2d");
                    ctx.clearRect(0, 0, width, height);
                    ctx.lineWidth = 1;
                    ctx.strokeStyle = "white";
                    ctx.moveTo(0, 0);
                    ctx.lineTo(width, height);
                    ctx.moveTo(width, 0);
                    ctx.lineTo(0, height);
                    ctx.stroke();
                }
            }

            onClicked: root.closeClicked()
        }
    }
}