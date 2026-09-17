import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    id: root

    default property alias content: content.data

    property string headerText: ""
    property real horizontalPadding: AppStyle.windowPadding

    readonly property Component headerLeading: Component {
        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: root.headerText
            color: Theme.textPrimary
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeLarge
            font.weight: Font.Medium
        }
    }

    FocusScope {
        id: content
        anchors.fill: parent
        anchors.leftMargin: root.horizontalPadding
        anchors.rightMargin: root.horizontalPadding
    }
}
