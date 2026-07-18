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
        font.pixelSize: AppStyle.fontSizeMedium
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
        }
    }
}
