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
            Layout.preferredWidth: AppStyle.titleBarUtilityButtonWidth
            Layout.topMargin: -(AppStyle.titleBarPadding)
            Layout.bottomMargin: -(AppStyle.titleBarPadding)

            focus: true

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }

            onClicked: root.minimizeClicked()

            Canvas {
                anchors.centerIn: parent
                width: AppStyle.titleBarUtilityButtonIconSize
                height: AppStyle.titleBarUtilityButtonIconSize
                onPaint: {
                    var ctx = getContext("2d");
                    ctx.clearRect(0, 0, width, height);
                    ctx.fillStyle = "white";
                    ctx.fillRect(0, height / 2, width, 1)
                    // ctx.fillRect(width * 0.25, height * 0.45, width * 0.5, height * 0.1);
                }
            }

            background: Rectangle {
                color: minButton.down ? AppStyle.titleBarMinimizeButtonColorPressed : minButton.hovered ? AppStyle.titleBarMinimizeButtonColorHovered : "transparent"
                opacity: minButton.down ? AppStyle.titleBarMinimizeButtonOpacityPressed : minButton.hovered ? AppStyle.titleBarMinimizeButtonOpacityHovered : 0
            }
        }

        Button {
            id: maxButton
            objectName: "Top Bar Maximize Button"
            Layout.fillHeight: true
            Layout.preferredWidth: AppStyle.titleBarUtilityButtonWidth
            Layout.topMargin: -(AppStyle.titleBarPadding)
            Layout.bottomMargin: -(AppStyle.titleBarPadding)

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }

            onClicked: root.maximizeClicked()

            background: Rectangle {
                color: maxButton.down ? AppStyle.titleBarMaximizeButtonColorPressed : maxButton.hovered ? AppStyle.titleBarMaximizeButtonColorHovered : "transparent"
                opacity: maxButton.down ? AppStyle.titleBarMaximizeButtonOpacityPressed : maxButton.hovered ? AppStyle.titleBarMaximizeButtonOpacityHovered : 0
            }

            Rectangle {
                anchors.centerIn: parent
                width: AppStyle.titleBarUtilityButtonIconSize
                height: AppStyle.titleBarUtilityButtonIconSize
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
            Layout.preferredWidth: AppStyle.titleBarUtilityButtonWidth
            Layout.topMargin: -(AppStyle.titleBarPadding)
            Layout.bottomMargin: -(AppStyle.titleBarPadding)
            Layout.rightMargin: -(AppStyle.titleBarPadding)

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }

            background: Rectangle {
                color: closeButton.down ? AppStyle.titleBarCloseButtonColorPressed : closeButton.hovered ? AppStyle.titleBarCloseButtonColorHovered : "transparent"
                opacity: closeButton.down ? AppStyle.titleBarCloseButtonOpacityPressed : closeButton.hovered ? AppStyle.titleBarCloseButtonOpacityHovered : 0
            }

            Canvas {
                anchors.centerIn: parent
                width: AppStyle.titleBarUtilityButtonIconSize
                height: AppStyle.titleBarUtilityButtonIconSize
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