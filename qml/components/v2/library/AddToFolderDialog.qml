import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "folder_tree.js" as FolderTree

// TODO
// Adds one or more entries to a manual folder. Shows the manual-folder tree
// (expandable) with a search box and an inline create-new-folder row. Opened
// from the game context menu and the multi-select bulk bar; acts on `targetIds`
FirelightDialog {
    id: control
    headerText: "Add to folder"
    acceptText: "Done"

    property var targetIds: []

    function openFor(ids) {
        control.targetIds = ids;
        searchField.text = "";
        newFolderName.text = "";
        control.expanded = ({});
        control.rebuild();
        control.open();
    }

    property string searchText: searchField.text
    property var expanded: ({})
    property var treeRows: []

    function toggleExpand(folderId) {
        var e = control.expanded;
        e[folderId] = (e[folderId] === false);
        control.expanded = e;
        control.rebuild();
    }

    function addTo(folderId) {
        for (var i = 0; i < control.targetIds.length; i++) {
            LibraryEntryModel.addEntryToFolder(control.targetIds[i], folderId);
        }
        control.close();
    }

    function createAndAdd() {
        var name = newFolderName.text.trim();
        if (name === "") {
            return;
        }
        var id = LibraryFolderModel.createFolder(name, -1);
        if (id !== -1) {
            control.addTo(id);
        }
    }

    // Manual-folder tree (smart folders can't hold entries). The shared helper
    // drops smart folders and reparents survivors to the root so none are orphaned
    function rebuild() {
        var all = [];
        for (var i = 0; i < folderCollector.count; i++) {
            var o = folderCollector.objectAt(i);
            if (!o) {
                continue;
            }
            all.push({
                folderId: o.folderId,
                parentId: o.parentId,
                position: o.position,
                displayName: o.displayName,
                color: o.color,
                icon1x1SourceUrl: o.icon1x1SourceUrl,
                folderType: o.folderType
            });
        }
        control.treeRows = FolderTree.flatten(all, control.expanded, true);
    }

    // Search collapses the tree into a flat list of name matches
    readonly property var filteredRows: {
        var q = control.searchText.trim().toLowerCase();
        if (q === "") {
            return [];
        }
        var out = [];
        for (var i = 0; i < folderCollector.count; i++) {
            var o = folderCollector.objectAt(i);
            if (!o || o.folderType !== 0) {
                continue;
            }
            if (o.displayName.toLowerCase().indexOf(q) !== -1) {
                out.push({
                    folderId: o.folderId,
                    displayName: o.displayName,
                    color: o.color,
                    icon1x1SourceUrl: o.icon1x1SourceUrl,
                    depth: 0,
                    hasChildren: false,
                    expanded: false
                });
            }
        }
        out.sort(function (a, b) {
            return a.displayName.localeCompare(b.displayName);
        });
        return out;
    }

    // A remote/qrc url passes through; a bare local path becomes a file:// url
    function normalizeIcon(u) {
        if (u === "" || u.indexOf("qrc:") === 0 || u.indexOf("file:") === 0 || u.indexOf("http") === 0) {
            return u;
        }
        return "file:///" + u.replace(/\\/g, "/");
    }

    Instantiator {
        id: folderCollector
        model: LibraryFolderModel
        delegate: QtObject {
            required property int folderId
            required property int parentId
            required property int position
            required property string displayName
            required property string color
            required property string icon1x1SourceUrl
            required property int folderType
        }
        onObjectAdded: Qt.callLater(control.rebuild)
        onObjectRemoved: Qt.callLater(control.rebuild)
    }
    Connections {
        target: LibraryFolderModel
        function onDataChanged() {
            Qt.callLater(control.rebuild);
        }
        function onModelReset() {
            Qt.callLater(control.rebuild);
        }
        function onLayoutChanged() {
            Qt.callLater(control.rebuild);
        }
    }

    contentItem: ColumnLayout {
        spacing: AppStyle.spacingSm

        FLSearchField {
            id: searchField
            Layout.fillWidth: true
            Layout.preferredWidth: Math.round(380 * AppStyle.scale)
            placeholder: "Search folders"
        }

        Text {
            Layout.fillWidth: true
            visible: control.targetIds.length > 1
            text: "Adding " + control.targetIds.length + " games"
            color: Theme.textMuted
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeSmall
        }

        Text {
            Layout.fillWidth: true
            Layout.topMargin: AppStyle.spacingSm
            visible: folderList.count === 0
            text: control.searchText.trim() !== "" ? "No folders match." : "No folders yet — create one below."
            color: Theme.textMuted
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeMedium
            wrapMode: Text.WordWrap
        }

        ListView {
            id: folderList
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(contentHeight, Math.round(300 * AppStyle.scale))
            clip: true
            model: control.searchText.trim() !== "" ? control.filteredRows : control.treeRows
            spacing: 2
            boundsBehavior: Flickable.StopAtBounds

            delegate: ItemDelegate {
                id: folderRow
                required property var modelData
                width: ListView.view.width
                height: AppStyle.rowHeight
                padding: AppStyle.spacingSm

                background: Rectangle {
                    color: folderRow.hovered ? Theme.surfaceHover : "transparent"
                    radius: AppStyle.radiusSm
                }

                contentItem: RowLayout {
                    spacing: AppStyle.spacingSm

                    Item {
                        visible: folderRow.modelData.depth > 0
                        Layout.preferredWidth: folderRow.modelData.depth * Math.round(14 * AppStyle.scale)
                        Layout.fillHeight: true
                    }

                    FLIconButton {
                        visible: folderRow.modelData.hasChildren && control.searchText.trim() === ""
                        compact: true
                        iconName: "chevron-forward"
                        rotation: folderRow.modelData.expanded ? 90 : 0
                        Layout.alignment: Qt.AlignVCenter
                        onClicked: control.toggleExpand(folderRow.modelData.folderId)
                    }
                    Item {
                        visible: !(folderRow.modelData.hasChildren && control.searchText.trim() === "")
                        Layout.preferredWidth: Math.round(18 * AppStyle.scale)
                        Layout.fillHeight: true
                    }

                    Icon {
                        visible: folderRow.modelData.icon1x1SourceUrl === ""
                        name: "folder"
                        filled: true
                        size: AppStyle.iconSizeMd
                        color: folderRow.modelData.color !== "" ? folderRow.modelData.color : Theme.textPrimary
                        Layout.alignment: Qt.AlignVCenter
                    }
                    Image {
                        visible: folderRow.modelData.icon1x1SourceUrl !== ""
                        Layout.preferredWidth: AppStyle.iconSizeMd
                        Layout.preferredHeight: AppStyle.iconSizeMd
                        Layout.alignment: Qt.AlignVCenter
                        fillMode: Image.PreserveAspectFit
                        sourceSize: Qt.size(AppStyle.iconSizeMd * 2, AppStyle.iconSizeMd * 2)
                        source: control.normalizeIcon(folderRow.modelData.icon1x1SourceUrl)
                    }

                    Text {
                        Layout.fillWidth: true
                        text: folderRow.modelData.displayName !== "" ? folderRow.modelData.displayName : qsTr("Untitled folder")
                        color: Theme.textPrimary
                        font.family: AppStyle.fontFamily
                        font.pixelSize: AppStyle.fontSizeMedium
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                onClicked: control.addTo(folderRow.modelData.folderId)
            }
        }

        FLDivider {
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: AppStyle.spacingSm

            FLTextField {
                id: newFolderName
                Layout.fillWidth: true
                placeholderText: "New folder name"
                onAccepted: control.createAndAdd()
            }
            FLButton {
                variant: "default"
                text: "Create"
                enabled: newFolderName.text.trim() !== ""
                onClicked: control.createAndAdd()
            }
        }
    }
}
