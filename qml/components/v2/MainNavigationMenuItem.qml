import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQml.Models
import QtQuick.Layouts 1.0
import QtNetwork
import QtMultimedia
import QtQuick.Effects
import Firelight 1.0

Button {
    id: control
    // required property int index

    objectName: "MainNavigationMenuItem|" + displayText

    required property string iconSource
    required property string displayText

    property bool showGlobalCursor: true
    property real globalCursorSpacing: 2

    implicitHeight: 36

    // onClicked: {
    //     ListView.view.currentIndex = index
    // }

    hoverEnabled: true
    checkable: true
    padding: 8

    background: Item {
        property alias radius: backgroundRect.radius
        Rectangle {
            anchors.fill: parent

            color: control.hovered || control.checked || control.down ? "#FFFFFF" : "transparent"
            opacity: control.down ? 0.05 : control.hovered ? 0.08 : control.checked ? 0.12 : 0
            radius: 4
        }

        Rectangle {
            id: backgroundRect
            color: "transparent"
            anchors.fill: parent
            opacity: 1
            radius: 4
        }
    }
    contentItem: RowLayout {
        Image {
            sourceSize.width: 20
            sourceSize.height: 20
            source: control.iconSource
            fillMode: Image.PreserveAspectFit
        }
        Text {
            Layout.leftMargin: 8
            color: Theme.textPrimary
            font.family: Constants.regularFontFamily
            font.pixelSize: AppStyle.fontSizeMedium
            font.weight: Font.DemiBold
            text: control.displayText
            visible: control.width > 64
        }
        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }
}