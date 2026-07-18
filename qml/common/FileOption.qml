import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

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
        let file = root.value;
        if (FilesystemUtils.isFile(file)) {
            fileDialog.selectedFile = file;
            return;
        }

        file = FilesystemUtils.prependFileURI(file);
        fileDialog.selectedFile = file;
    }

    FileDialog {
        id: fileDialog
        nameFilters: ["Image files (*.png *.jpg *.jpeg *.gif)"]
        onAccepted: {
            root.value = fileDialog.selectedFile;
            // console.log("found it")
        }
    }

    onClicked: function () {
        fileDialog.open();
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
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            font.family: Constants.regularFontFamily
            font.weight: Font.DemiBold
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Text {
            text: FilesystemUtils.isFile(root.value) ? FilesystemUtils.removeFileURI(root.value) : root.value
            maximumLineCount: 1
            elide: Text.ElideRight
            font.pixelSize: AppStyle.fontSizeMedium
            Layout.alignment: Qt.AlignRight
            Layout.maximumWidth: parent.width / 2
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
