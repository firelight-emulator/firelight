import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0
import QtQml.Models
import Firelight 1.0

Button {
    id: root
    required property var model
    required property var index

    implicitWidth: ListView.view.width

    signal startGameClicked(int entryId)

    Keys.onSpacePressed: {
        startGameClicked(root.model.entryId)
    }

    Keys.onSelectPressed: {
        startGameClicked(root.model.entryId)
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton
        onDoubleTapped: {
            startGameClicked(root.model.entryId)
        }

        onSingleTapped: function(event, button) {
            if (root.ListView.view.currentIndex !== root.index) {
                // theList.highlightRangeMode = ListView.NoHighlightRange
                root.ListView.view.currentIndex = root.index
            }
        }
    }

    HoverHandler {
        id: gameItemHover
        cursorShape: Qt.PointingHandCursor
    }

    height: 60
    padding: 6
    horizontalPadding: 12

    background: Rectangle {
        color: "white"
        radius: 2
        opacity: (root.activeFocus && root.ListView.isCurrentItem) ? 1 : gameItemHover.hovered ? 0.1 : 1
        visible: (root.activeFocus && root.ListView.isCurrentItem) || gameItemHover.hovered
    }

    contentItem: RowLayout {
        spacing: 16
        // FLIcon {
        //     icon: "star"
        //     color: "grey"
        //     Layout.preferredHeight: 32
        //     Layout.preferredWidth: 32
        //     Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        //     size: height
        // }
        // Image {
        //     source: tttt.index < theList.icons.length ? theList.icons[tttt.index] : "https://cdn2.steamgriddb.com/thumb/2c323abe873b4f9fa8a72f45785df5f0.jpg"
        //     Layout.preferredHeight: parent.height
        //     Layout.preferredWidth: parent.height
        //     Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        //     sourceSize.width: parent.height
        //     sourceSize.height: parent.height
        //     smooth: false
        //     fillMode: Image.PreserveAspectFit
        // }
        Rectangle {
            color: "#292929"
            visible: root.model.icon1x1SourceUrl === ""
            implicitWidth: 48
            implicitHeight: 48

            FLIcon {
                icon: root.model.platformIconName
                color: "#595959"
                anchors.centerIn: parent
                height: parent.height - 16
                width: parent.width - 16
                size: height
            }
        }
        Image {
            source: root.model.icon1x1SourceUrl
            visible: root.model.icon1x1SourceUrl !== ""
            asynchronous: true
            Layout.preferredHeight: 48
            Layout.preferredWidth: 48
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            sourceSize.width: 48
            sourceSize.height: 48
            smooth: true
            fillMode: Image.PreserveAspectFit
        }
        Text {
            text: root.model.displayName
            font.pixelSize: 17
            // font.weight: Font.DemiBold
            font.family: Constants.mainFontFamily
            color: (root.activeFocus && root.ListView.isCurrentItem) ? "black" : "white"
            Layout.fillHeight: true
            Layout.preferredWidth: 600
            elide: Text.ElideRight
            maximumLineCount: 1
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }
        Text {
            text: platform_model.getPlatformDisplayName(root.model.platformId)
            font.pixelSize: 17
            font.weight: Font.Medium
            font.family: Constants.mainFontFamily
            color: (root.activeFocus && root.ListView.isCurrentItem) ? "#2A2A2A" : "#d3d3d3"
            Layout.fillHeight: true
            Layout.preferredWidth: 300
            elide: Text.ElideRight
            maximumLineCount: 1
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }
        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }
}