import QtQuick
import QtQml
import QtQuick.Controls
import QtQuick.Layouts 1.0
import Firelight 1.0

FLPopup {
    id: control

    parent: Overlay.overlay
    modal: true
    anchors.centerIn: parent
    minWidth: AppStyle.defaultDialogMinimumWidth
    padding: AppStyle.spacingXl
    closePolicy: Popup.CloseOnPressOutsideParent

    // How tall the content area may grow before it scrolls instead
    readonly property int contentMaxHeight: control.parent ? Math.round(control.parent.height * AppStyle.dialogContentMaxHeightFraction) : AppStyle.dialogContentMinHeight

    // Between the content area and the buttons
    readonly property int contentButtonSpacing: control.padding

    openSound: SoundEffects.showDialog

    enter: Transition {
        NumberAnimation {
            target: control
            property: "opacity"
            from: 0
            to: 1
            duration: AppStyle.durationBase
            easing.type: AppStyle.easingStandard
        }
    }

    exit: Transition {
        NumberAnimation {
            target: control
            property: "opacity"
            from: 1
            to: 0
            duration: AppStyle.durationBase
            easing.type: AppStyle.easingStandard
        }
    }

    Connections {
        target: control

        function onAboutToShow() {
            SoundEffects.showDialog.play();
        }
    }

    property string headerText
    default property alias content: column.data

    signal accepted
    signal rejected

    contentItem: FocusScope {
        implicitWidth: Math.max(column.implicitWidth, buttonRow.implicitWidth)
        implicitHeight: contentFlickable.implicitHeight + control.contentButtonSpacing + buttonRow.implicitHeight

        onActiveFocusChanged: {
            if (activeFocus) {
                buttonRow.forceActiveFocus();
            }
        }

        FLFocus.actions: [
            FLAction {
                keys: [Qt.Key_Back, Qt.Key_Escape]
                label: qsTr("Close")
                sound: SoundEffects.back
                onTriggered: control.close()
            }
        ]

        Flickable {
            id: contentFlickable
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                bottom: buttonRow.top
                bottomMargin: control.contentButtonSpacing
            }

            implicitHeight: Math.max(AppStyle.dialogContentMinHeight, Math.min(column.implicitHeight, control.contentMaxHeight))

            contentWidth: width
            contentHeight: Math.max(column.implicitHeight, height)
            flickableDirection: Flickable.VerticalFlick
            clip: true

            FLColumnLayout {
                id: column
                width: contentFlickable.width

                y: Math.max(0, Math.min(contentFlickable.height - implicitHeight, (contentFlickable.height + control.contentButtonSpacing - implicitHeight) / 2))
                spacing: AppStyle.spacingXs
            }
        }

        FLRowLayout {
            id: buttonRow
            spacing: AppStyle.spacingMd
            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }

            // FLButton {
            //     id: cancelButton
            //     text: qsTr("Cancel")
            //     Layout.fillWidth: true
            //     Layout.horizontalStretchFactor: 1
            //     focus: true
            //     onClicked: {
            //         control.rejected();
            //         control.close();
            //     }
            // }

            FLButton {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: AppStyle.buttonStandardWidth
                focus: true
                onClicked: {
                    control.accepted();
                    control.close();
                }
            }
        }
    }
}
