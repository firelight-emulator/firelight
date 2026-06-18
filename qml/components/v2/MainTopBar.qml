import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0
import Firelight 1.0

FocusScope {
    id: root
    signal closeClicked()
    signal maximizeClicked()
    signal minimizeClicked()

    // Drag-to-move sits behind all content; interactive elements above it capture their own events
    Item {
        anchors.fill: parent
        z: -1

        DragHandler {
            target: null
            grabPermissions: DragHandler.ApprovesTakeOverByAnything
            onActiveChanged: if (active) root.Window.window.startSystemMove()
            margin: 8
        }

        TapHandler {
            onDoubleTapped: root.maximizeClicked()
            margin: 8
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 16

        // Image {
        //     source: "qrc:/icons/arrow-back"
        //     width: 32
        //     height: 32
        //     fillMode: Image.PreserveAspectFit
        //     Layout.alignment: Qt.AlignVCenter
        //     opacity: 0.7
        // }

        // IconButton {
        //     icon.width: 24
        //     icon.height: 24
        //     icon.source: "qrc:/icons/home"
        //     Layout.alignment: Qt.AlignVCenter
        //     Layout.topMargin: 2
        //     Layout.leftMargin: 16
        //     // tooltipText: "Open menu"
        //     opacity: 0.7
        // }
        //
        // IconButton {
        //     icon.width: 24
        //     icon.height: 24
        //     icon.source: "qrc:/icons/book"
        //     Layout.alignment: Qt.AlignVCenter
        //     Layout.topMargin: 2
        //     Layout.leftMargin: 16
        //     // tooltipText: "Open menu"
        //     opacity: 0.7
        // }
        //
        // IconButton {
        //     icon.width: 24
        //     icon.height: 24
        //     icon.source: "qrc:/icons/shopping-bag"
        //     Layout.alignment: Qt.AlignVCenter
        //     Layout.topMargin: 2
        //     Layout.leftMargin: 16
        //     // tooltipText: "Open menu"
        //     opacity: 0.7
        // }
        //
        // IconButton {
        //     icon.width: 24
        //     icon.height: 24
        //     icon.source: "qrc:/icons/controller"
        //     Layout.alignment: Qt.AlignVCenter
        //     Layout.topMargin: 2
        //     Layout.leftMargin: 16
        //     // tooltipText: "Open menu"
        //     opacity: 0.7
        // }
        //
        // IconButton {
        //     icon.width: 24
        //     icon.height: 24
        //     icon.source: "qrc:/icons/photo-library"
        //     Layout.alignment: Qt.AlignVCenter
        //     Layout.topMargin: 2
        //     Layout.leftMargin: 16
        //     // tooltipText: "Open menu"
        //     opacity: 0.7
        // }

        RowLayout {
            id: leftContentArea
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.horizontalStretchFactor: 1

            spacing: 16

            IconButton {
                icon.width: 24
                icon.height: 24
                icon.source: "qrc:/icons/menu"
                Layout.alignment: Qt.AlignVCenter
                Layout.topMargin: 2
                Layout.leftMargin: 16
                // tooltipText: "Open menu"
                opacity: 0.7
            }

            Text {
                text: "Library"
                font.family: Constants.regularFontFamily
                Layout.fillHeight: true
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 14
                font.weight: Font.DemiBold
                color: "white"
                opacity: 0.9
            }

            Item {
                Layout.fillWidth: true
                implicitHeight: 1
            }
        }

        // Text {
        //     text: "Home"
        //     font.family: Constants.regularFontFamily
        //     Layout.fillHeight: true
        //     Layout.leftMargin: 32
        //     verticalAlignment: Text.AlignVCenter
        //     font.pointSize: 14
        //     font.weight: Font.Medium
        //     color: "white"
        //     opacity: 0.7
        // }
        //
        // Text {
        //     text: "Mod Shop"
        //     font.family: Constants.regularFontFamily
        //     Layout.fillHeight: true
        //     verticalAlignment: Text.AlignVCenter
        //     font.pointSize: 14
        //     font.weight: Font.Medium
        //     color: "white"
        //     opacity: 0.7
        // }
        //
        // Text {
        //     text: "Controllers"
        //     font.family: Constants.regularFontFamily
        //     Layout.fillHeight: true
        //     verticalAlignment: Text.AlignVCenter
        //     font.pointSize: 14
        //     font.weight: Font.Medium
        //     color: "white"
        //     opacity: 0.7
        // }
        //
        // Text {
        //     text: "Gallery"
        //     font.family: Constants.regularFontFamily
        //     Layout.fillHeight: true
        //     verticalAlignment: Text.AlignVCenter
        //     font.pointSize: 14
        //     font.weight: Font.Medium
        //     color: "white"
        //     opacity: 0.7
        // }
        //
        // Item {
        //     Layout.fillWidth: true
        //     implicitHeight: 1
        // }

        Rectangle {
            id: searchBar
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: 600
            Layout.minimumWidth: 200
            Layout.fillWidth: true
            color: Qt.lighter("#12131A", 1.4)
            radius: height / 2
        }

        // Item {
        //     Layout.horizontalStretchFactor: 1
        //     Layout.fillWidth: true
        //     implicitHeight: 1
        // }
        //
        // IconButton {
        //     icon.width: 28
        //     icon.height: 28
        //     icon.source: "qrc:/icons/home"
        //     Layout.alignment: Qt.AlignVCenter
        //     tooltipText: "Home"
        //     opacity: 0.7
        // }
        //
        // IconButton {
        //     icon.width: 28
        //     icon.height: 28
        //     icon.source: "qrc:/icons/browse"
        //     Layout.alignment: Qt.AlignVCenter
        //     tooltipText: "Library"
        //     opacity: 0.7
        // }
        //
        // IconButton {
        //     icon.width: 28
        //     icon.height: 28
        //     icon.source: "qrc:/icons/controller"
        //     Layout.alignment: Qt.AlignVCenter
        //     tooltipText: "Controllers"
        //     opacity: 0.7
        // }
        //
        // IconButton {
        //     icon.width: 28
        //     icon.height: 28
        //     icon.source: "qrc:/icons/bar-chart"
        //     Layout.alignment: Qt.AlignVCenter
        //     tooltipText: "Activity"
        //     opacity: 0.7
        // }
        //
        // IconButton {
        //     icon.width: 28
        //     icon.height: 28
        //     icon.source: "qrc:/icons/trophy"
        //     Layout.alignment: Qt.AlignVCenter
        //     tooltipText: "Achievements"
        //     opacity: 0.7
        // }
        //
        // IconButton {
        //     icon.width: 28
        //     icon.height: 28
        //     icon.source: "qrc:/icons/settings"
        //     Layout.alignment: Qt.AlignVCenter
        //     tooltipText: "Settings"
        //     opacity: 0.7
        // }

        // Text {
        //     text: "Home"
        //     font.family: Constants.regularFontFamily
        //     Layout.alignment: Qt.AlignVCenter
        //     font.pointSize: 16
        //     font.weight: Font.Medium
        //     color: "white"
        //     opacity: 0.7
        // }
        // Text {
        //     text: "Library"
        //     font.family: Constants.regularFontFamily
        //     Layout.alignment: Qt.AlignVCenter
        //     font.pointSize: 16
        //     font.weight: Font.Medium
        //     color: "white"
        //     // opacity: 0.7
        // }
        // Text {
        //     text: "Controllers"
        //     font.family: Constants.regularFontFamily
        //     Layout.alignment: Qt.AlignVCenter
        //     font.pointSize: 16
        //     font.weight: Font.Medium
        //     color: "white"
        //     opacity: 0.7
        // }
        // Text {
        //     text: "Mod Shop"
        //     font.family: Constants.regularFontFamily
        //     Layout.alignment: Qt.AlignVCenter
        //     font.pointSize: 16
        //     font.weight: Font.Medium
        //     color: "white"
        //     opacity: 0.7
        // }
        // Text {
        //     text: "Gallery"
        //     font.family: Constants.regularFontFamily
        //     Layout.alignment: Qt.AlignVCenter
        //     font.pointSize: 16
        //     font.weight: Font.Medium
        //     color: "white"
        //     opacity: 0.7
        // }

        // Image {
        //     source: "qrc:/icons/arrow-forward"
        //     width: 32
        //     height: 32
        //     fillMode: Image.PreserveAspectFit
        //     Layout.alignment: Qt.AlignVCenter
        //     opacity: 0.7
        // }

        // Item {
        //     Layout.horizontalStretchFactor: 1
        //     Layout.fillWidth: true
        //     implicitHeight: 1
        // }

        // IconButton {
        //     icon.width: 24
        //     icon.height: 24
        //     icon.source: "qrc:/icons/search"
        //     Layout.alignment: Qt.AlignVCenter
        //     Layout.leftMargin: 8
        //     // tooltipText: "Open menu"
        //     opacity: 0.7
        // }
        RowLayout {
            id: rightContentArea
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.horizontalStretchFactor: 1

            spacing: 24

            Item {
                Layout.fillWidth: true
                implicitHeight: 1
            }

            IconButton {
                icon.width: 24
                icon.height: 24
                icon.source: "qrc:/icons/settings"
                Layout.alignment: Qt.AlignVCenter
                Layout.topMargin: 2
                // tooltipText: "Open menu"
                opacity: 0.7
            }

            Rectangle {
                implicitHeight: 38
                implicitWidth: 38
                radius: height / 2
                color: Qt.lighter("#12131A", 1.4)
                Layout.alignment: Qt.AlignVCenter
            }

            RowLayout {
                id: windowButtonRow
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignVCenter
                Layout.leftMargin: -(parent.spacing / 2)
                spacing: 0

                Button {
                    id: minButton
                    Layout.fillHeight: true
                    Layout.preferredWidth: AppStyle.topBarUtilityButtonWidth
                    Layout.topMargin: -(AppStyle.topBarPadding)
                    Layout.bottomMargin: -(AppStyle.topBarPadding)

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
                    Layout.fillHeight: true
                    Layout.preferredWidth: AppStyle.topBarUtilityButtonWidth
                    Layout.topMargin: -(AppStyle.topBarPadding)
                    Layout.bottomMargin: -(AppStyle.topBarPadding)

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
                    Layout.fillHeight: true
                    Layout.preferredWidth: AppStyle.topBarUtilityButtonWidth
                    Layout.topMargin: -(AppStyle.topBarPadding)
                    Layout.bottomMargin: -(AppStyle.topBarPadding)
                    Layout.rightMargin: -(AppStyle.topBarPadding)

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
                            // ctx.fillRect(width * 0.25, height * 0.45, width * 0.5, height * 0.1);
                        }
                    }

                    onClicked: root.closeClicked()
                }
            }


        }




    }
}