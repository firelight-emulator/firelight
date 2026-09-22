// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

//   FLMenuItem { iconName: "settings"; label: "Appearance" }
//   FLMenuItem { label: "Volume"; description: "How loud everything is"; controlItem: FLSlider {} }
ItemDelegate {
    id: root

    enum Surface {
        Page,
        InMenu,
        InPopup
    }

    property int surface: FLMenuItem.Surface.InMenu

    property string iconName: ""
    property string label: ""
    property string description: ""
    property string longDescription: ""

    property Item controlItem: null
    property bool controlBelow: false

    default property alias trailing: sideSlot.data
    property alias nestedContent: nestedColumn.data

    property bool isFirstInSection: false
    property bool isLastInSection: false
    property bool showDivider: false

    property bool showDescription: root.surface === FLMenuItem.Surface.Page

    property bool subItem: false
    readonly property int contentIndent: subItem ? 40 : AppStyle.spacingLg

    property bool shown: true

    property bool resettable: false
    signal reset

    readonly property bool _inGroup: root.surface === FLMenuItem.Surface.Page
    readonly property bool _drawTopRadius: isFirstInSection && _inGroup
    readonly property bool _drawBottomRadius: isLastInSection && _inGroup

    readonly property int _topRadius: _drawTopRadius ? AppStyle.radiusLg : (_inGroup ? 0 : AppStyle.radiusMd)
    readonly property int _bottomRadius: _drawBottomRadius ? AppStyle.radiusLg : (_inGroup ? 0 : AppStyle.radiusMd)

    visible: shown
    opacity: root.enabled ? 1 : 0.4
    focusPolicy: Qt.StrongFocus
    Layout.fillWidth: true

    FLFocus.showCursor: true
    FLFocus.focusSound: SoundEffects.menuNavigate
    FLFocus.topLeftRadius: root._topRadius
    FLFocus.topRightRadius: root._topRadius
    FLFocus.bottomLeftRadius: root._bottomRadius
    FLFocus.bottomRightRadius: root._bottomRadius

    FLFocus.actions: [
        FLAction {
            label: "OK"
            keys: [Qt.Key_Enter, Qt.Key_Return, Qt.Key_Space, Qt.Key_Select]
            onTriggered: {
                root.click();
            }
        }
    ]

    topPadding: AppStyle.spacingMd
    bottomPadding: _inGroup ? AppStyle.rowSpacerPage : AppStyle.spacingMd
    leftPadding: root.contentIndent
    rightPadding: AppStyle.spacingLg

    implicitHeight: Math.max(AppStyle.listRowHeight, contentColumn.implicitHeight + topPadding + bottomPadding)
    implicitWidth: contentColumn.implicitWidth + leftPadding + rightPadding

    property Item focusProxy: null
    readonly property bool cursorFocused: FocusCursor.isOn(root) || (focusProxy !== null && FocusCursor.isOn(focusProxy))

    highlighted: root.cursorFocused || root.pressed || rowHover.hovered

    readonly property var activationKeys: [Qt.Key_Select, Qt.Key_Return, Qt.Key_Enter, Qt.Key_Space]

    Binding {
        target: root.controlItem
        property: "parent"
        value: root.controlBelow ? belowSlot : sideSlot
        when: root.controlItem !== null
    }
    Binding {
        target: root.controlItem
        property: "width"
        value: root.controlBelow ? belowSlot.width : sideSlot.width
        when: root.controlItem !== null && root.controlBelow
    }

    // Handle space and select for buttons (Qt normally handles it)
    Connections {
        target: root.Keys

        function onPressed(event) {
            const action = root.FLFocus.getActionFor(event.key, event.modifiers);

            if (action !== null) {
                action.triggerForPress(event.isAutoRepeat);
                event.accepted = true;
                return;
            }

            // TODO
            // Only an action opts into repeating: a held key never presses the item itself
            if (event.isAutoRepeat) {
                return;
            }

            if (root.activationKeys.indexOf(event.key) !== -1) {
                root.click();
                event.accepted = true;
            }
        }
    }

    HoverHandler {
        id: rowHover
        enabled: root.enabled
        cursorShape: Qt.PointingHandCursor
    }

    background: Rectangle {
        topLeftRadius: root._topRadius
        topRightRadius: root._topRadius
        bottomLeftRadius: root._bottomRadius
        bottomRightRadius: root._bottomRadius

        color: root.pressed ? Theme.surfaceElevated : root.highlighted ? Theme.surfaceHover : "transparent"

        Behavior on color {
            ColorAnimation {
                duration: AppStyle.rowHighlightDuration
            }
        }
    }

    contentItem: ColumnLayout {
        id: contentColumn
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            spacing: AppStyle.spacingMd

            Icon {
                visible: root.iconName !== ""
                Layout.alignment: Qt.AlignVCenter
                name: root.iconName
                size: AppStyle.iconSizeMd
                color: root.highlighted ? Theme.accent : Theme.textPrimary
            }

            Text {
                id: labelText
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                text: root.label
                color: (root.checked && !root._inGroup) ? Theme.switch2Color : root.subItem ? Theme.textMuted : Theme.textPrimary
                font.pixelSize: AppStyle.rowLabelSize
                font.family: AppStyle.fontFamily
                font.weight: AppStyle.rowLabelWeight
                wrapMode: Text.WordWrap
                verticalAlignment: Text.AlignVCenter
            }

            Button {
                id: resetButton
                visible: root.resettable
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.preferredHeight: labelText.implicitHeight
                leftPadding: AppStyle.spacingSm
                rightPadding: AppStyle.spacingSm
                topPadding: 0
                bottomPadding: 0
                focusPolicy: Qt.NoFocus
                hoverEnabled: true
                onClicked: root.reset()

                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }

                background: Rectangle {
                    radius: AppStyle.radiusMd
                    color: resetButton.hovered ? Theme.surfaceHover : "transparent"
                }
                contentItem: Text {
                    text: qsTr("Reset")
                    color: Theme.textMuted
                    font.pixelSize: AppStyle.fontSizeSmall
                    font.family: AppStyle.fontFamily
                    font.weight: Font.Medium
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Item {
                id: sideSlot
                visible: !root.controlBelow
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.maximumWidth: Math.round(root.width * 0.6)
                implicitWidth: childrenRect.width
                implicitHeight: childrenRect.height
            }
        }

        Item {
            id: belowSlot
            visible: root.controlBelow
            Layout.fillWidth: true
            Layout.topMargin: root.controlBelow ? AppStyle.spacingSm : 0
            implicitHeight: (root.controlBelow && root.controlItem) ? root.controlItem.implicitHeight : 0
        }

        Text {
            id: descText
            Layout.fillWidth: true
            Layout.topMargin: visible ? AppStyle.spacingXs : 0
            visible: root.showDescription && root.description !== ""
            text: root.description
            color: Theme.textMuted
            font.pixelSize: AppStyle.fontSizeSmall
            font.family: AppStyle.fontFamily
            font.weight: Font.Medium
            lineHeight: 1.3
            wrapMode: Text.WordWrap
        }

        ColumnLayout {
            id: nestedColumn
            Layout.fillWidth: true
            Layout.leftMargin: AppStyle.spacingSm
            Layout.topMargin: children.length > 0 ? AppStyle.spacingSm : 0
            spacing: AppStyle.spacingSm
            visible: children.length > 0
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        visible: root.showDivider
        color: Theme.border
    }
}
