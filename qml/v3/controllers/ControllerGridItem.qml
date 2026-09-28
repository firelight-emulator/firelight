import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import Firelight 1.0

FLButtonBase {
    id: control
    required property string imageUrl
    required property bool connected
    required property int index

    required property bool isLifted

    variant: "default"
    rounded: false

    actionLabel: control.isLifted ? qsTr("Drop") : qsTr("Configure")
    actionSound: control.isLifted ? SoundEffects.back : SoundEffects.openPopup

    layer.enabled: control.isLifted
    layer.effect: MultiEffect {
        shadowEnabled: true
        shadowColor: Theme.shadow
        shadowBlur: AppStyle.elevationBlur
        shadowVerticalOffset: AppStyle.elevationOffset
        shadowHorizontalOffset: AppStyle.elevationOffset
    }

    transform: Translate {
        y: control.isLifted ? -AppStyle.reorderingLiftHeight : 0

        Behavior on y {
            NumberAnimation {
                duration: AppStyle.durationFast
                easing.type: Easing.Linear
            }
        }
    }

    signal reorderingRequested()
    signal configureRequested()

    ContextMenu.menu: ControllerContextMenu {
        enabled: !control.isLifted

        // TODO
        // The tile the menu is for takes focus before the menu opens
        onAboutToShow: control.forceActiveFocus()

        onReorderingRequested: {
            control.reorderingRequested();
        }

        onConfigureRequested: {
            control.configureRequested();
        }
    }

    FLFocus.actions: [
        FLAction {
            keys: [Qt.Key_Menu]
            label: qsTr("Options")
            sound: SoundEffects.openPopup
            enabled: control.connected && !control.isLifted
            hidden: !control.connected
            onTriggered: control.ContextMenu.menu.popupFor(control, control.width + AppStyle.spacingSm, 0)
        },
        FLAction {
            keys: [Qt.Key_Return, Qt.Key_Enter, Qt.Key_Space, Qt.Key_Select]
            modifiers: Qt.ControlModifier
            label: qsTr("Options")
            sound: SoundEffects.openPopup
            enabled: control.connected && !control.isLifted
            hidden: !control.connected
            onTriggered: control.ContextMenu.menu.popupFor(control, control.width + AppStyle.spacingSm, 0)
        }
    ]

    contentItem: Item {
        anchors.fill: parent
        anchors.margins: AppStyle.spacingMd
        Image {
            id: controllerIcon
            source: control.imageUrl
            anchors.fill: parent
            visible: control.connected
            fillMode: Image.PreserveAspectFit
            sourceSize.width: {
                const n = Math.max(control.width, control.height);
                if (n <= 0) {
                    return 1;
                }

                return Math.pow(2, Math.round(Math.log2(n)));
            }
        }

        Text {
            anchors.centerIn: parent
            text: control.index + 1
            font.pixelSize: AppStyle.fontSizeMedium
            font.family: AppStyle.fontFamily
            color: "white"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            visible: !control.connected
        }
    }
}