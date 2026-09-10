// TODO: NEEDS REVIEW
pragma Singleton
import QtQuick

// TODO
// Raises the on-screen keyboard from anywhere. The overlay itself is mounted once by Main4 and
// registered here, since a singleton has no place in the scene graph of its own
//
//   FLKeyboard.open({ text: entry.name, maxLength: 64 }, function (result) { ... })
//
// The callback is handed the entered string, or null if the user backed out
QtObject {
    id: root

    // TODO
    // A Popup is not an Item, so this cannot be typed as one: the assignment would silently fail
    // and every request would be dropped
    property QtObject overlay: null

    property var _callback: null

    readonly property bool available: root.overlay !== null

    function open(options, callback) {
        if (root.overlay === null) {
            console.warn("FLKeyboard.open called before the overlay was mounted");
            return false;
        }

        root._callback = callback !== undefined ? callback : null;
        root.overlay.reset(options);
        root.overlay.open();

        return true;
    }

    function _finish(result) {
        const callback = root._callback;
        root._callback = null;

        if (callback) {
            callback(result);
        }
    }

    property list<QtObject> wiring: [
        Connections {
            target: root.overlay

            function onAccepted(text) {
                root._finish(text);
            }

            function onRejected() {
                root._finish(null);
            }
        }
    ]
}
