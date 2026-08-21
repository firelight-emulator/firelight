import QtQuick
import QtQml

// TODO
// A read-only live view of one library entry, looked up by id. `entry` is the
// list model's own row object — so it carries every role (including the joined
// play stats and achievement counts), stays live as the row changes, and writes
// through it (entry.<role> = value) persist via the model's setData. `entry` is
// null until a matching row exists
Item {
    id: root

    property int entryId: -1

    // The live row object, or null. Re-resolves when the id changes or the row
    // appears/disappears; field reads (entry.favorite, ...) stay reactive because
    // the row object itself emits role-change notifications
    readonly property var entry: {
        const _dep = root.entryId + mirror.count;
        return mirror.count > 0 && mirror.objectAt(0) ? mirror.objectAt(0).model : null;
    }

    SortFilterProxyModel {
        id: proxy
        model: LibraryEntryModel
        filters: ValueFilter {
            roleName: "entryId"
            value: root.entryId
            enabled: root.entryId !== -1
        }
    }
    Instantiator {
        id: mirror
        model: proxy
        delegate: QtObject {
            required property var model
        }
    }
}
