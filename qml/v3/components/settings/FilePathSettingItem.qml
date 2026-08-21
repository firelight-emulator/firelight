import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs

// File / folder path setting. `value` is a filesystem path; `directoryMode`
// picks a folder instead of a file; `extensions` (no dot) filters files. Emits
// `chosen` when a path is picked or cleared
FLMenuItem {
    id: root

    controlBelow: true

    property string value: ""
    property bool directoryMode: false
    property var extensions: []
    signal chosen(string path)

    function urlToPath(u) {
        // file:///C:/x -> C:/x (Windows); file:///home/x -> /home/x (Unix)
        var s = u.toString().replace(/^file:\/\//, "");
        if (/^\/[A-Za-z]:/.test(s)) {
            s = s.substring(1);
        }
        return decodeURIComponent(s);
    }

    function nameFilters() {
        if (!extensions || extensions.length === 0) {
            return ["All files (*)"];
        }
        var globs = [];
        for (var i = 0; i < extensions.length; i++) {
            globs.push("*." + extensions[i]);
        }
        return ["Supported files (" + globs.join(" ") + ")", "All files (*)"];
    }

    controlItem: RowLayout {
        spacing: AppStyle.spacingSm

        Text {
            Layout.fillWidth: true
            text: root.value === "" ? qsTr("Not set") : root.value
            color: root.value === "" ? Theme.textMuted : Theme.textPrimary
            elide: Text.ElideMiddle
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeSmall
            verticalAlignment: Text.AlignVCenter
        }

        FLButton {
            compact: true
            text: qsTr("Browse…")
            enabled: root.enabled
            focusPolicy: Qt.NoFocus
            onClicked: root.directoryMode ? folderDialog.open() : fileDialog.open()
        }

        FLButton {
            compact: true
            variant: "subtle"
            text: qsTr("Clear")
            visible: root.value !== ""
            enabled: root.enabled
            focusPolicy: Qt.NoFocus
            onClicked: root.chosen("")
        }
    }

    FileDialog {
        id: fileDialog
        nameFilters: root.nameFilters()
        onAccepted: root.chosen(root.urlToPath(selectedFile))
    }

    FolderDialog {
        id: folderDialog
        onAccepted: root.chosen(root.urlToPath(selectedFolder))
    }
}
