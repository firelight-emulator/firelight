import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform

Button {
    id: root

    property string label
    property string description: ""
    property string value: ""
    property bool showGlobalCursor: true
    checkable: true
    horizontalPadding: 12
    verticalPadding: 12

    onValueChanged: function () {
        let folder = root.value
        if (FilesystemUtils.isFile(folder)) {
            folderDialog.currentFolder = folder
            return
        }

        folder = FilesystemUtils.prependFileURI(folder)
        folderDialog.currentFolder = folder
    }

    FolderDialog {
        id: folderDialog
        onAccepted: {
            root.value = folder
            // console.log("found it")
        }
    }

    onClicked: function () {
        folderDialog.open()
    }

    // implicitHeight: Math.max(72, theColumn.)
    hoverEnabled: true

    HoverHandler {
        cursorShape: Qt.PointingHandCursor
    }

    background: Rectangle {
        color: "white"
        radius: 2
        opacity: root.hovered ? 0.1 : 0
    }

    contentItem: RowLayout {
        Text {
            id: labelText
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            text: root.label
            color: Theme.textPrimary
            font.pixelSize: AppStyle.fontSizeMedium
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            verticalAlignment: Text.AlignVCenter
            font.family: Constants.regularFontFamily
            font.weight: Font.DemiBold
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Text {
            text: FilesystemUtils.isFile(root.value) ? FilesystemUtils.removeFileURI(root.value) : root.value
            font.pixelSize: AppStyle.fontSizeMedium
            Layout.alignment: Qt.AlignRight
            font.family: Constants.regularFontFamily
            // font.weight: Font.DemiBold
            wrapMode: Text.WordWrap
            Layout.leftMargin: 32
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            color: Theme.textMuted
        }

    }
}
