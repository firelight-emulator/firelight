import QtQuick
import QtQuick.Layouts
import Firelight 1.0

ColumnLayout {
    id: root

    required property var entryId

    spacing: 12

    LibraryEntry {
        id: libEntry
        entryId: root.entryId
    }

    GameActivity {
        id: gameActivity
        contentHash: libEntry.contentHash
    }

    FLGameActivityPage {
        Layout.fillWidth: true
        Layout.fillHeight: true
        activity: gameActivity
    }
}
