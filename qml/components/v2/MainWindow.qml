import QtQuick
import QtQuick.Controls
import QtQuick.Window

ApplicationWindow {
    id: root
    objectName: "MainWindow"

    property bool frameReady: false
    property real previousX: 0
    property real previousY: 0
    property real previousWidth: width
    property real previousHeight: height

    title: qsTr("Firelight")
    // Driven through visibility alone. Setting `visible` as well makes Qt warn
    // about conflicting properties, and the window stays hidden until the frame
    // has been positioned
    visibility: Window.Hidden

    width: WindowGeometry.mainWindowWidth
    height: WindowGeometry.mainWindowHeight

    minimumWidth: 800
    minimumHeight: 600

    Component.onCompleted: {
        WindowFrame.setWindow(root);
        WindowFrame.setNativePosition(WindowGeometry.mainWindowX, WindowGeometry.mainWindowY);
        // A --fullscreen/--windowed override applies to this launch only; -1
        // means none was given and the saved preference stands
        const override = StartupOptions.fullscreenOverride;
        const startFullscreen = override === 1 || (override === -1 && GeneralSettings.fullscreen);
        visibility = startFullscreen ? Window.FullScreen : Window.Windowed;
        frameReady = true;
    }

    function maximize() {
        if (visibility === Window.Maximized) {
            visibility = Window.Windowed;
            WindowFrame.setNativePosition(previousX, previousY);
            width = previousWidth;
            height = previousHeight;
        } else {
            previousX = WindowFrame.nativeX();
            previousY = WindowFrame.nativeY();
            previousWidth = width;
            previousHeight = height;
            visibility = Window.Maximized;
        }
    }

    onHeightChanged: {
        WindowGeometry.mainWindowHeight = height;
    }

    onWidthChanged: {
        WindowGeometry.mainWindowWidth = width;
    }
    onXChanged: {
        if (frameReady) {
            WindowGeometry.mainWindowX = WindowFrame.nativeX();
        }
    }
    onYChanged: {
        if (frameReady) {
            WindowGeometry.mainWindowY = WindowFrame.nativeY();
        }
    }

    FLFocusHighlight {
        target: root.activeFocusItem
        usingMouse: InputMethodManager.usingMouse
    }

    // Resize edges — z:100 so they sit above all content
    Item {
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
        width: 6
        z: 100
        HoverHandler {
            cursorShape: Qt.SizeHorCursor
        }
        DragHandler {
            target: null
            onActiveChanged: if (active)
                root.startSystemResize(Qt.LeftEdge)
        }
    }
    Item {
        anchors {
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
        width: 6
        z: 100
        HoverHandler {
            cursorShape: Qt.SizeHorCursor
        }
        DragHandler {
            target: null
            onActiveChanged: if (active)
                root.startSystemResize(Qt.RightEdge)
        }
    }
    Item {
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: 6
        z: 100
        HoverHandler {
            cursorShape: Qt.SizeVerCursor
        }
        DragHandler {
            target: null
            onActiveChanged: if (active)
                root.startSystemResize(Qt.BottomEdge)
        }
    }
    Item {
        anchors {
            left: parent.left
            bottom: parent.bottom
        }
        width: 12
        height: 12
        z: 100
        HoverHandler {
            cursorShape: Qt.SizeBDiagCursor
        }
        DragHandler {
            target: null
            onActiveChanged: if (active)
                root.startSystemResize(Qt.LeftEdge | Qt.BottomEdge)
        }
    }
    Item {
        anchors {
            right: parent.right
            bottom: parent.bottom
        }
        width: 12
        height: 12
        z: 100
        HoverHandler {
            cursorShape: Qt.SizeFDiagCursor
        }
        DragHandler {
            target: null
            onActiveChanged: if (active)
                root.startSystemResize(Qt.RightEdge | Qt.BottomEdge)
        }
    }
}
