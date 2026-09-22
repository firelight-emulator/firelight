// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Firelight 1.0

FLTwoColumnPage {
    id: root
    objectName: "CreateCollectionFlow|" + root.kind

    property string kind: "manual"
    readonly property bool isSmart: root.kind === "smart"

    headerText: root.isSmart ? qsTr("Create Smart Collection") : qsTr("Create Collection")
    menuOnRight: true
    activateOnFocus: false
    contentMaxWidth: root.currentKey === "details" ? AppStyle.readableContentWidth : Infinity

    property bool routeActive: true

    property string discardMessage: qsTr("Leave without creating this collection?")
    property string discardConfirmText: qsTr("Leave")
    property string discardCancelText: qsTr("Keep editing")

    property string draftName: ""
    property string draftDescription: ""
    property string draftImage: ""
    property var draftEntryIds: []
    property string draftCriteriaJson: ""

    readonly property bool nameTaken: LibraryFolderModel.hasFolderNamed(root.draftName)
    readonly property bool canSave: root.draftName.trim() !== "" && !root.nameTaken

    signal loaded

    model: [
        {
            "type": "page",
            "key": "details",
            "label": qsTr("Details"),
            "page": detailsPage
        },
        {
            "type": "page",
            "key": "contents",
            "label": root.isSmart ? qsTr("Criteria") : qsTr("Games"),
            "page": contentsPage
        }
    ]

    function clearDraft() {
        root.draftName = "";
        root.draftDescription = "";
        root.draftImage = "";
        root.draftEntryIds = [];
        root.draftCriteriaJson = "";
    }

    function isDirty() {
        return root.draftName !== "" || root.draftDescription !== "" || root.draftImage !== "" || root.draftEntryIds.length > 0 || root.draftCriteriaJson !== "";
    }

    function save() {
        const folderId = LibraryFolderModel.createCollection({
            "displayName": root.draftName.trim(),
            "description": root.draftDescription,
            "icon1x1SourceUrl": root.draftImage,
            "folderType": root.isSmart ? 1 : 0,
            "filterJson": root.isSmart ? root.draftCriteriaJson : "",
            "parentId": -1
        });

        if (folderId < 0) {
            saveFailedDialog.open();
            return;
        }

        if (!root.isSmart) {
            LibraryEntryModel.addEntriesToFolder(folderId, root.draftEntryIds);
        }

        root.clearDraft();

        if (!Router.back()) {
            Router.replace("/library/collections");
        }
    }

    function mayLeave() {
        if (!root.isDirty()) {
            return true;
        }

        confirmDialog.open();
        return false;
    }

    function enter() {
        root.clearDraft();
        root.currentKey = "details";

        Router.setLeaveGuard(root.mayLeave);

        root.loaded();
        root.focusContent();
    }

    Component.onCompleted: root.enter()

    onRouteActiveChanged: {
        if (root.routeActive) {
            root.enter();
            return;
        }

        Router.clearLeaveGuard();
    }

    footer: FLButton {
        id: saveButton
        Layout.alignment: Qt.AlignHCenter
        Layout.margins: AppStyle.spacingXl
        Layout.preferredWidth: root.menuWidth * 0.8
        text: qsTr("Save")
        onClicked: {
            if (!canInteract) {
                return;
            }

            root.save();
        }
        canInteract: root.canSave
    }

    FLDialog {
        id: confirmDialog

        showCancel: true
        acceptText: root.discardConfirmText
        rejectText: root.discardCancelText

        onAccepted: Router.resumePending()
        onRejected: Router.cancelPending()

        Text {
            Layout.fillWidth: true
            text: root.discardMessage
            color: Theme.textMuted
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeMedium
        }
    }

    FLDialog {
        id: saveFailedDialog

        Text {
            Layout.fillWidth: true
            text: qsTr("Couldn't create the collection")
            color: Theme.textPrimary
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeMedium
        }
    }

    component FieldLabel: Text {
        Layout.fillWidth: true
        color: Theme.textMuted
        font.family: AppStyle.fontFamily
        font.pixelSize: AppStyle.fontSizeSmall
    }

    Component {
        id: detailsPage

        FocusScope {
            id: details
            objectName: "CreateCollectionDetails|" + root.kind

            implicitHeight: detailsColumn.implicitHeight + AppStyle.spacingXl

            function seed() {
                nameField.text = root.draftName;
                descriptionField.text = root.draftDescription;
            }

            Component.onCompleted: details.seed()

            Connections {
                target: root

                function onLoaded() {
                    details.seed();
                }
            }

            FileDialog {
                id: imageDialog
                title: qsTr("Choose collection image")
                nameFilters: [qsTr("Images (*.png *.jpg *.jpeg *.svg *.webp)")]
                onAccepted: root.draftImage = imageDialog.selectedFile.toString()
            }

            FLColumnLayout {
                id: detailsColumn
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: AppStyle.spacingSm

                FLRowLayout {
                    Layout.fillWidth: true
                    Layout.bottomMargin: AppStyle.spacingLg
                    spacing: AppStyle.spacingLg

                    Rectangle {
                        Layout.preferredWidth: AppStyle.artIconSizeLg
                        Layout.preferredHeight: AppStyle.artIconSizeLg
                        radius: AppStyle.radiusMd
                        color: Theme.surfaceElevated

                        Icon {
                            anchors.centerIn: parent
                            visible: preview.status !== Image.Ready
                            name: "photo-library"
                            size: AppStyle.iconSizeLg
                            color: Theme.textMuted
                        }

                        Image {
                            id: preview
                            anchors.fill: parent
                            anchors.margins: AppStyle.spacingSm
                            visible: preview.status === Image.Ready
                            source: root.draftImage
                            sourceSize: Qt.size(width * 2, height * 2)
                            fillMode: Image.PreserveAspectFit
                        }
                    }

                    FLColumnLayout {
                        Layout.fillWidth: false
                        Layout.alignment: Qt.AlignVCenter
                        spacing: AppStyle.spacingSm

                        FLButton {
                            objectName: "CreateCollectionFlow|chooseImage"
                            text: qsTr("Choose image…")
                            onClicked: imageDialog.open()
                        }

                        FLButton {
                            id: removeImageButton
                            objectName: "CreateCollectionFlow|removeImage"
                            text: qsTr("Remove")
                            canInteract: root.draftImage !== ""
                            onClicked: {
                                if (!removeImageButton.canInteract) {
                                    return;
                                }

                                root.draftImage = "";
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }
                }

                FieldLabel {
                    text: qsTr("Name")
                }

                FLKeyboardField {
                    id: nameField
                    Layout.fillWidth: true
                    focus: true
                    placeholderText: qsTr("Collection name")
                    onTextChanged: root.draftName = nameField.text
                }

                FieldLabel {
                    visible: root.nameTaken
                    text: qsTr("A collection with this name already exists")
                }

                FieldLabel {
                    Layout.topMargin: AppStyle.spacingSm
                    text: qsTr("Description")
                }

                FLKeyboardField {
                    id: descriptionField
                    Layout.fillWidth: true
                    placeholderText: qsTr("Optional")
                    onTextChanged: root.draftDescription = descriptionField.text
                }
            }
        }
    }

    Component {
        id: contentsPage

        CollectionGamePicker {
            id: picker
            selecting: !root.isSmart

            Component.onCompleted: picker.load(root.draftEntryIds, root.draftCriteriaJson)

            onPicksChanged: ids => root.draftEntryIds = ids
            onCriteriaChanged: json => root.draftCriteriaJson = json

            Connections {
                target: root

                function onLoaded() {
                    picker.load(root.draftEntryIds, root.draftCriteriaJson);
                }
            }
        }
    }
}
