// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Layouts 1.0

FocusScope {
    id: root
    signal closeClicked
    signal maximizeClicked
    signal minimizeClicked
    signal backRequested

    property Item page
    implicitHeight: 60

    function activateSearch() {
        searchBar.forceActiveFocus();
    }

    Item {
        objectName: "TitleBar|DragArea"
        anchors.fill: parent
        z: -1

        DragHandler {
            target: null
            grabPermissions: DragHandler.ApprovesTakeOverByAnything
            onActiveChanged: if (active)
                root.Window.window.startSystemMove()
            margin: 8
        }

        TapHandler {
            onDoubleTapped: root.maximizeClicked()
            margin: 8
        }
    }

    Item {
        id: contentRow
        anchors.fill: parent

        TitleBarBackButton {
            id: backButton
            x: AppStyle.spacingXl
            anchors.verticalCenter: parent.verticalCenter
            showButton: true
            onBackRequested: root.backRequested()
        }

        Loader {
            anchors.left: backButton.right
            anchors.leftMargin: AppStyle.spacingXl
            anchors.verticalCenter: parent.verticalCenter
            sourceComponent: root.page?.headerLeading ?? null
        }

        Loader {
            anchors.fill: parent
            sourceComponent: root.page?.headerCenter ?? null
        }

        Loader {
            anchors.right: windowButtons.left
            anchors.rightMargin: AppStyle.spacingLg
            anchors.verticalCenter: parent.verticalCenter
            sourceComponent: root.page?.headerTrailing ?? null
        }

        TitleBarUtilityButtons {
            id: windowButtons
            anchors.right: parent.right
            anchors.top: parent.top
            height: 32

            onMinimizeClicked: root.minimizeClicked()
            onMaximizeClicked: root.maximizeClicked()
            onCloseClicked: root.closeClicked()

            visible: InputMethodManager.usingMouse
        }
    }
}
