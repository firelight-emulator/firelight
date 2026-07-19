import QtQuick
import QtQuick.Controls

Item {
    id: gridRoot
    property alias model: grid.model

    CaptureViewer {
        id: viewer
    }

    GridView {
        id: grid
        anchors.fill: parent
        anchors.margins: AppStyle.spacingSm
        cellWidth: 184
        cellHeight: 132
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: FLScrollBar {
            width: 8
        }

        delegate: Item {
            id: cell
            required property var model
            width: grid.cellWidth
            height: grid.cellHeight

            Button {
                id: control
                anchors.fill: parent
                anchors.margins: AppStyle.spacingXs
                padding: 0
                hoverEnabled: true

                ContextMenu.menu: RightClickMenu {
                    RightClickMenuItem {
                        text: cell.model.favorite ? "Remove favorite" : "Favorite"
                        onTriggered: CaptureModel.toggleFavorite(cell.model.captureId)
                    }
                    RightClickMenuItem {
                        text: "Delete"
                        onTriggered: CaptureModel.removeCapture(cell.model.captureId)
                    }
                }

                background: Rectangle {
                    color: Theme.surfaceElevated
                    radius: AppStyle.radiusSm
                    border.width: 2
                    border.color: control.hovered ? "#4a90d9" : "transparent"
                }

                onClicked: viewer.openWith(cell.model)

                contentItem: Item {
                    Image {
                        anchors.fill: parent
                        anchors.margins: 3
                        source: cell.model.thumbnailUrl
                        sourceSize.width: 320
                        fillMode: Image.PreserveAspectFit
                        asynchronous: true
                        cache: true
                    }

                    // Clip play badge
                    Rectangle {
                        visible: cell.model.captureType === "clip"
                        anchors.centerIn: parent
                        width: 36
                        height: 36
                        radius: height / 2
                        color: "#aa000000"
                        Text {
                            anchors.centerIn: parent
                            text: "▶"
                            color: "white"
                            font.pixelSize: AppStyle.fontSizeMedium
                        }
                    }

                    // Favorite marker
                    Text {
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.margins: 6
                        visible: cell.model.favorite
                        text: "★"
                        color: "gold"
                        font.pixelSize: AppStyle.fontSizeMedium
                    }
                }
            }
        }
    }
}
