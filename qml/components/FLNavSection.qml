import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0
import Firelight 1.0

FocusScope {
    id: root

    property string title: ""

    implicitHeight: (titleText.visible ? titleText.height : 0) + navItems.height

    default property list<FLNavItem> items

    Text {
        id: titleText
        text: root.title
        font.pixelSize: 15
        // leftPadding: 12
        font.weight: Font.Bold
        font.family: Constants.regularFontFamily
        color: "#727272"
        width: parent.width
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        visible: root.title !== ""
        height: visible ? 20 : 0
        padding: 0
    }

    ColumnLayout {
        id: navItems
        // spacing: 12
        spacing: 8
        anchors.topMargin: root.title === "" ? 0 : 6
        anchors.top: titleText.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        focus: true
        Repeater {
            model: root.items
            delegate: Pane {
                required property var modelData
                focus: true
                Layout.minimumHeight: 42
                Layout.maximumHeight: 42
                Layout.fillWidth: true

                padding: 0
                background: Item {}
                contentItem: modelData
            }
            // delegate: Row {
            //     Layout.maximumHeight: 26
            //     Layout.minimumHeight: 26
            //
            //     spacing: 20
            //
            //     FLIcon {
            //         width: 26
            //         height: 26
            //         size: 26
            //         icon: "book"
            //     }
            //
            //     // Image {
            //     //     width: 26
            //     //     height: 26
            //     //     source: "file:book.svg"
            //     //     sourceSize.width: 26
            //     //     sourceSize.height: 26
            //     // }
            //
            //     // Text {
            //     //     width: 26
            //     //     text: "\uf53e"
            //     //     height: 26
            //     //     horizontalAlignment: Text.AlignHCenter
            //     //     verticalAlignment: Text.AlignVCenter
            //     //     font.family: Constants.symbolFontFamily
            //     //     padding: 0
            //     //     // antialiasing: true
            //     //     // smooth: true //Add this for smoother edges
            //     //     font.pixelSize: 28
            //     //     font.weight: Font.Light
            //     //     font.variableAxes: { "FILL": 1.0, "opsz": 40}
            //     //     renderType: Text.QtRendering
            //     //     renderTypeQuality: Text.VeryHighRenderTypeQuality
            //     //     color: "#DDDDDD"
            //     // }
            //
            //     // Rectangle {
            //     //     color: "white"
            //     //     width: 26
            //     //     height: 26
            //     // }
            //
            //     Text {
            //         text: modelData
            //         font.pixelSize: 17
            //         font.weight: Font.DemiBold
            //         font.family: Constants.regularFontFamily
            //         color: "#DDDDDD"
            //         height: 26
            //         horizontalAlignment: Text.AlignLeft
            //         verticalAlignment: Text.AlignVCenter
            //     }
            // }
        }
    }



}
