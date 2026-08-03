import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0

// Window chrome, not part of the interface the cursor navigates: every button
// here is mouse-only, so none of them take focus
FocusScope {
    id: root
    objectName: "TitleBarUtilityButtons"

    signal closeClicked
    signal maximizeClicked
    signal minimizeClicked

    implicitWidth: buttonRow.implicitWidth
    implicitHeight: buttonRow.implicitHeight

    RowLayout {
        id: buttonRow
        objectName: "TitleBarUtilityButtons|Row"
        anchors.fill: parent
        spacing: 0

        Button {
            id: minButton
            objectName: "TitleBarUtilityButtons|Minimize"
            Layout.fillHeight: true
            Layout.preferredWidth: AppStyle.titleBarUtilityButtonWidth
            Layout.topMargin: -(AppStyle.titleBarPadding)
            Layout.bottomMargin: -(AppStyle.titleBarPadding)

            focusPolicy: Qt.NoFocus

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
                    ctx.fillRect(0, height / 2, width, 1);
                    // ctx.fillRect(width * 0.25, height * 0.45, width * 0.5, height * 0.1);
                }
            }

            background: Rectangle {
                color: minButton.down ? Theme.titleBarBtnPressed : minButton.hovered ? Theme.titleBarBtnHovered : "transparent"
                opacity: minButton.down ? Theme.titleBarBtnPressedOpacity : minButton.hovered ? Theme.titleBarBtnHoveredOpacity : 0
            }
        }

        Button {
            id: maxButton
            objectName: "TitleBarUtilityButtons|Maximize"
            Layout.fillHeight: true
            Layout.preferredWidth: AppStyle.titleBarUtilityButtonWidth
            Layout.topMargin: -(AppStyle.titleBarPadding)
            Layout.bottomMargin: -(AppStyle.titleBarPadding)

            focusPolicy: Qt.NoFocus

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }

            onClicked: root.maximizeClicked()

            background: Rectangle {
                color: maxButton.down ? Theme.titleBarBtnPressed : maxButton.hovered ? Theme.titleBarBtnHovered : "transparent"
                opacity: maxButton.down ? Theme.titleBarBtnPressedOpacity : maxButton.hovered ? Theme.titleBarBtnHoveredOpacity : 0
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
            objectName: "TitleBarUtilityButtons|Close"
            Layout.fillHeight: true
            Layout.preferredWidth: AppStyle.titleBarUtilityButtonWidth
            Layout.topMargin: -(AppStyle.titleBarPadding)
            Layout.bottomMargin: -(AppStyle.titleBarPadding)
            Layout.rightMargin: -(AppStyle.titleBarPadding)

            focusPolicy: Qt.NoFocus

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }

            background: Rectangle {
                color: closeButton.down ? Theme.titleBarClosePressed : closeButton.hovered ? Theme.titleBarCloseHovered : "transparent"
                opacity: closeButton.down ? Theme.titleBarClosePressedOpacity : closeButton.hovered ? Theme.titleBarCloseHoveredOpacity : 0
            }

            Canvas {
                anchors.centerIn: parent
                width: AppStyle.titleBarUtilityButtonIconSize
                height: AppStyle.titleBarUtilityButtonIconSize
                onPaint: {
                    var ctx = getContext("2d");
                    ctx.clearRect(0, 0, width, height);
                    // Without beginPath the path accumulates across repaints
                    // (e.g. the first paint at 0-size before the size binding
                    // resolves), leaving stray segments in the X
                    ctx.beginPath();
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
