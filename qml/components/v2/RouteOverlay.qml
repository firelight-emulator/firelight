import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import Firelight 1.0

// Hosts routes that present as a popup over the current view (e.g. /settings).
// Driven entirely by Router: opens when the current route is an overlay, closes
// otherwise. Dismissing the popup (Escape / click-away) calls Router.back() so
// history and the URL stay in sync. The content stays loaded while closed, so
// reopening returns to exactly where it was left.
Popup {
    id: overlay

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: parent ? parent.width - 160 : 600
    height: parent ? parent.height - 120 : 400

    modal: true
    padding: 0
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    // Overlay route pattern -> screen Component.
    readonly property var routes: ({
        "/settings": settingsComponent
    })

    Component { id: settingsComponent; SettingsScreen {} }

    property string _loadedPattern: ""

    background: Rectangle {
        color: Theme.surfaceElevated
        radius: 8
        border.color: Theme.border
        border.width: 1

        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowColor: Theme.background
            shadowBlur: 1.0
            shadowVerticalOffset: 4.0
            shadowHorizontalOffset: 4.0
        }
    }

    Overlay.modal: Rectangle {
        color: "black"
        opacity: 0.7
        Behavior on opacity {
            NumberAnimation { duration: 160; easing.type: Easing.InOutQuad }
        }
    }

    contentItem: Loader {
        id: contentLoader
    }

    function _setActive(active) {
        if (contentLoader.item && contentLoader.item.routeActive !== undefined) {
            contentLoader.item.routeActive = active;
        }
    }

    function _sync() {
        var component = Router.overlay ? routes[Router.matchedPattern] : null;
        if (component) {
            if (_loadedPattern !== Router.matchedPattern) {
                contentLoader.sourceComponent = component;
                _loadedPattern = Router.matchedPattern;
            }
            if (!overlay.visible) {
                overlay.open();
            }
        } else if (overlay.visible) {
            overlay.close();
        }
    }

    onOpened: _setActive(true)

    Connections {
        target: Router
        function onNavigated() {
            overlay._sync();
        }
    }

    Component.onCompleted: _sync()

    onClosed: {
        _setActive(false);
        // Only a user dismissal reaches here with the route still an overlay;
        // navigating away closes us via _sync after the route already changed.
        if (Router.overlay) {
            Router.back();
        }
    }

    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 160; easing.type: Easing.InOutQuad }
        NumberAnimation { property: "scale"; from: 0.98; to: 1.0; duration: 160; easing.type: Easing.InOutQuad }
    }
    exit: Transition {
        NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 160; easing.type: Easing.InOutQuad }
        NumberAnimation { property: "scale"; from: 1.0; to: 0.98; duration: 160; easing.type: Easing.InOutQuad }
    }
}
