import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Firelight 1.0

// Standalone controller-profile manager: create, clone, rename, delete, and
// import/export profiles. Self-contained; bind nothing — it loads all profiles
// from the input service
FocusScope {
    id: root

    property int pendingId: -1
    // "create" | "rename" | "clone"
    property string nameMode: "create"

    ProfileListModel {
        id: profiles
    }

    function openNameDialog(mode, id, suggested) {
        root.nameMode = mode;
        root.pendingId = id;
        nameField.text = suggested;
        nameDialog.open();
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            Text {
                text: "Controller Profiles"
                color: Theme.textPrimary
                font.family: AppStyle.fontFamily
                font.weight: Font.DemiBold
                font.pixelSize: AppStyle.fontSizeLarge
                Layout.fillWidth: true
            }
            FLButton {
                text: "Import…"
                onClicked: importDialog.open()
            }
            FLButton {
                variant: "primary"
                text: "New Profile"
                onClicked: root.openNameDialog("create", -1, "New Profile")
            }
        }

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 4
            model: profiles

            delegate: Rectangle {
                required property int profileId
                required property string name
                required property bool builtin

                width: ListView.view.width
                height: AppStyle.rowHeight + AppStyle.spacingMd
                radius: AppStyle.radiusSm
                color: Theme.surface

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 10
                    spacing: 8

                    Text {
                        text: name
                        color: Theme.textPrimary
                        font.family: AppStyle.fontFamily
                        font.pixelSize: AppStyle.fontSizeMedium
                        Layout.fillWidth: true
                    }

                    Text {
                        visible: builtin
                        text: "Built-in"
                        color: Theme.textMuted
                        font.family: AppStyle.fontFamily
                        font.pixelSize: AppStyle.fontSizeSmall
                    }

                    FLButton {
                        compact: true
                        text: "Clone"
                        onClicked: root.openNameDialog("clone", profileId, name + " copy")
                    }
                    FLButton {
                        compact: true
                        text: "Rename"
                        enabled: !builtin
                        onClicked: root.openNameDialog("rename", profileId, name)
                    }
                    FLButton {
                        compact: true
                        text: "Export"
                        onClicked: {
                            root.pendingId = profileId;
                            exportDialog.currentFile = "file:///" + name + ".json";
                            exportDialog.open();
                        }
                    }
                    FLButton {
                        compact: true
                        variant: "danger"
                        text: "Delete"
                        enabled: !builtin
                        onClicked: {
                            root.pendingId = profileId;
                            deleteDialog.open();
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: nameDialog
        anchors.centerIn: parent
        modal: true
        title: root.nameMode === "rename" ? "Rename Profile" : root.nameMode === "clone" ? "Clone Profile" : "New Profile"
        standardButtons: Dialog.Ok | Dialog.Cancel

        FLTextField {
            id: nameField
            width: 320
            placeholderText: "Profile name"
            onAccepted: nameDialog.accept()
        }

        onAccepted: {
            const text = nameField.text.trim();
            if (text.length === 0) {
                return;
            }
            if (root.nameMode === "create") {
                profiles.createProfile(text);
            } else if (root.nameMode === "clone") {
                profiles.cloneProfile(root.pendingId, text);
            } else if (root.nameMode === "rename") {
                profiles.renameProfile(root.pendingId, text);
            }
        }
    }

    Dialog {
        id: deleteDialog
        anchors.centerIn: parent
        modal: true
        title: "Delete Profile"
        standardButtons: Dialog.Yes | Dialog.No

        Text {
            text: "Delete this profile? This cannot be undone."
            color: Theme.textPrimary
            font.family: AppStyle.fontFamily
        }

        onAccepted: profiles.deleteProfile(root.pendingId)
    }

    FileDialog {
        id: exportDialog
        title: "Export Profile"
        fileMode: FileDialog.SaveFile
        nameFilters: ["JSON files (*.json)"]
        onAccepted: profiles.exportProfile(root.pendingId, selectedFile)
    }

    FileDialog {
        id: importDialog
        title: "Import Profile"
        fileMode: FileDialog.OpenFile
        nameFilters: ["JSON files (*.json)"]
        onAccepted: profiles.importProfile(selectedFile)
    }
}
