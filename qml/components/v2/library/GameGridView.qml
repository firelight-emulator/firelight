import QtQuick
import QtQuick.Controls
import QtQuick.Window
import Firelight 1.0

Item {
    id: gridRoot
    property alias model: root.model

    // A remote URL passes through; a local file path (user-imported art) becomes
    // a file:// URL so Image can load it.
    function iconSource(u) {
        if (!u)
            return ""
        if (u.indexOf("://") >= 0)
            return u
        return "file:///" + u.replace(/\\/g, "/")
    }

    GameArtPickerDialog {
        id: artPicker
    }

    GridView {
        id: root

        property real initialContentY: contentY

        property string sortRole: "displayName"
        property bool sortAscending: true

        width: Math.max(cellWidth, Math.floor(parent.width / cellWidth) * cellWidth)
        height: parent.height
        x: Math.round((parent.width - width) / 2)

        cellWidth: 140
        cellHeight: 140

        Component.onCompleted: {
            initialContentY = contentY
        }

        ScrollBar.vertical: ScrollBar {
            anchors.left: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 8
            // contentItem: Rectangle {
            //     color: "#ffffff"
            //     radius: 4
            //     opacity: 0.12
            // }

        }
         boundsBehavior: Flickable.StopAtBounds

        delegate: Item {
            id: gameDelegate
            required property var model
            width: root.cellWidth
            height: root.cellHeight

            Button {
            id: control
                anchors.fill: parent
                anchors.margins: 4
                padding: 0
                hoverEnabled: true

                TapHandler {
                    acceptedButtons: Qt.LeftButton
                    onDoubleTapped: EmulationService.loadEntry(gameDelegate.model.entryId)
                }

                Keys.onPressed: function (event) {
                    if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Select) {
                        EmulationService.loadEntry(gameDelegate.model.entryId)
                        event.accepted = true
                    }
                }

                ContextMenu.menu: RightClickMenu {
                    RightClickMenuItem {
                        text: "Change artwork"
                        onTriggered: artPicker.openFor(gameDelegate.model.contentHash, gameDelegate.model.displayName, gameDelegate.model.platformId)
                    }
                    RightClickMenuItem {
                        text: "View media"
                        onTriggered: Router.navigateTo("/gallery/games/" + gameDelegate.model.contentHash)
                    }
                }

                scale: hovered ? 1.05 : 1
                Behavior on scale {
                    NumberAnimation {
                        duration: 120
                        easing.type: Easing.InOutQuad
                    }
                }

                background: Rectangle {
                    color: control.down ? "#FFFFFF" : "transparent"
                    opacity: control.down ? 0.05 : 0
                    radius: 4
                }
                contentItem: Item {
                    Image {
                        anchors.fill: parent
                        sourceSize.width: 256
                        sourceSize.height: 256
                        source: gridRoot.iconSource(model.icon1x1SourceUrl)
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                        mipmap: true
                        visible: model.icon1x1SourceUrl !== undefined && model.icon1x1SourceUrl !== ""
                    }

                    Rectangle {
                        anchors.fill: parent
                        color: "grey"
                        visible: model.icon1x1SourceUrl === "" || model.icon1x1SourceUrl === undefined
                    }

                }
            }
        }


        // Column {
        //     width: root.cellWidth
        //     height: root.cellHeight
        //
        //     Rectangle {
        //         color: "red"
        //         width: parent.width - 20
        //         height: parent.height - 20
        //         anchors.horizontalCenter: parent.horizontalCenter
        //     }
        //     Text {
        //         text: model.displayName
        //         font.pixelSize: 14
        //         horizontalAlignment: Text.AlignHCenter
        //         verticalAlignment: Text.AlignVCenter
        //         width: parent.width - 20
        //         anchors.horizontalCenter: parent.horizontalCenter
        //         height: 20
        //     }
        // }


     }
 }