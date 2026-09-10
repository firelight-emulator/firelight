// TODO: NEEDS REVIEW
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

    property string headerText
    default property alias content: column.data

    property bool showCancel: false
    property string acceptText: qsTr("OK")
    property string rejectText: qsTr("Cancel")

    signal accepted
    signal rejected

    property bool _accepting: false

    onClosed: {
        if (control._accepting) {
            control._accepting = false;
            return;
        }

        control.rejected();
    }

    contentItem: FocusScope {
        // TODO
        // The popup item above this is a focus scope of its own and keeps what it is given, so the
        // surface has to claim it for anything inside to be reached
        focus: true
        implicitWidth: Math.max(column.implicitWidth, buttonRow.implicitWidth)
        implicitHeight: contentFlickable.implicitHeight + control.contentButtonSpacing + buttonRow.implicitHeight

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
                horizontalCenter: parent.horizontalCenter
            }

            FLButton {
                text: control.rejectText
                visible: control.showCancel
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: AppStyle.buttonStandardWidth

                focus: control.showCancel

                onClicked: control.close()
            }

            FLButton {
                text: control.acceptText
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: AppStyle.buttonStandardWidth
                focus: !control.showCancel
                onClicked: {
                    control._accepting = true;
                    control.accepted();
                    control.close();
                }
            }
        }
    }
}
