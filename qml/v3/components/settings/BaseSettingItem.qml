// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// One settings row inside a section card: the label with its description beneath
// it, the control to the right (or full-width below when `controlBelow`), and a
// full-bleed hairline between rows. A subtle highlight appears on hover or
// gamepad/keyboard focus. Default children land in an indented slot for nested
// sub-settings
FocusScope {
    id: root

    required property string label
    required property Item control
    property string description: ""

    property bool isFirstInSection: false
    property bool isLastInSection: false

    property bool inMenu: false

    // Place the control full-width below the label instead of to its right
    property bool controlBelow: false

    // Bind to whatever a dependent setting hangs off of
    property bool shown: true

    // The between-rows divider. SettingsSection turns it off for a group's last
    // row so the card doesn't end with a line
    property bool showDivider: !inMenu

    // Marks this row as a dependent sub-setting of the one above it: indented
    // with a quieter label, so two cues carry the relationship rather than one
    property bool subItem: false
    readonly property int contentIndent: subItem ? 40 : 16

    // Sub-settings rendered indented below the row
    default property alias nestedContent: nestedColumn.data

    // Shown when this setting has an override at the current tier; emits reset()
    // so the caller can clear it (falling back to the inherited value)
    property bool resettable: false
    signal reset

    property var onClicked: function () {}

    readonly property bool _drawTopRadius: isFirstInSection && !inMenu
    readonly property bool _drawBottomRadius: isLastInSection && !inMenu

    FLFocus.focusSound: SoundEffects.menuNavigate
    FLFocus.showCursor: true
    FLFocus.topRightRadius: _drawTopRadius ? AppStyle.radiusLg : 0
    FLFocus.topLeftRadius: _drawTopRadius ? AppStyle.radiusLg : 0
    FLFocus.bottomRightRadius: _drawBottomRadius ? AppStyle.radiusLg : 0
    FLFocus.bottomLeftRadius: _drawBottomRadius ? AppStyle.radiusLg : 0

    // TODO
    // The row is what takes focus, not the control inside it, so the arrows a control reads reach
    // it. A FocusScope refuses focus unless it says otherwise, and a row that refuses it is skipped
    // by the cursor entirely
    focusPolicy: Qt.StrongFocus

    // Rows always span their column; a row with no width renders nothing, and
    // callers shouldn't have to remember this at every use site
    Layout.fillWidth: true

    // Dependent rows swap in instantly. Animating the height cascades into the
    // card and everything below it, which reads worse than a clean swap
    visible: shown
    implicitHeight: content.implicitHeight

    // The single `control` lives either in the side slot or the below slot
    Binding {
        target: root.control
        property: "parent"
        value: root.controlBelow ? belowSlot : sideSlot
        when: root.control !== null
    }
    Binding {
        target: root.control
        property: "width"
        value: root.controlBelow ? belowSlot.width : sideSlot.width
        when: root.control !== null
    }

    HoverHandler {
        id: hover
        cursorShape: root.onClicked ? Qt.PointingHandCursor : Qt.ArrowCursor
    }
    // TODO
    // Enabled whether or not the row does anything when tapped, because a row the cursor can land
    // on has to be reachable by clicking it too — a slider takes its arrows from having focus, and
    // clicking it is how a mouse user gets there
    TapHandler {
        enabled: root.enabled
        onTapped: {
            root.forceActiveFocus(Qt.MouseFocusReason);

            if (root.onClicked) {
                root.onClicked();
            }
        }
    }

    // Full-bleed hover / focus highlight behind the row
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: content.top
        anchors.bottom: content.bottom

        topLeftRadius: root.isFirstInSection ? AppStyle.radiusLg : 0
        topRightRadius: root.isFirstInSection ? AppStyle.radiusLg : 0
        bottomLeftRadius: root.isLastInSection ? AppStyle.radiusLg : 0
        bottomRightRadius: root.isLastInSection ? AppStyle.radiusLg : 0

        color: hover.hovered || (!InputMethodManager.usingMouse && root.activeFocus) ? Theme.surfaceHover : "transparent"

        Behavior on color {
            ColorAnimation {
                duration: 100
                easing.type: Easing.InOutQuad
            }
        }
    }

    ColumnLayout {
        id: content
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: root.contentIndent
            Layout.rightMargin: AppStyle.spacingLg
            Layout.topMargin: AppStyle.spacingMd
            spacing: AppStyle.spacingMd

            Text {
                id: labelText
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                text: root.label
                color: root.subItem ? Theme.textMuted : Theme.textPrimary
                font.pixelSize: AppStyle.fontSizeSmall
                font.family: AppStyle.fontFamily
                font.weight: Font.Medium
                wrapMode: Text.WordWrap
            }

            Button {
                id: resetButton
                visible: root.resettable
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                // Sized to the label so appearing and disappearing can't change
                // the row's height — nothing should shift when a value changes
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
                // A control that asks for more than it can have — a long file
                // path, say — would otherwise squeeze the label to nothing and
                // draw straight over it. Cap it and let the control shrink
                Layout.maximumWidth: Math.round(root.width * 0.6)
                implicitWidth: (!root.controlBelow && root.control) ? root.control.implicitWidth : 0
                implicitHeight: (!root.controlBelow && root.control) ? root.control.implicitHeight : 0
            }
        }

        Item {
            id: belowSlot
            visible: root.controlBelow
            Layout.fillWidth: true
            Layout.leftMargin: root.contentIndent
            Layout.rightMargin: AppStyle.spacingLg
            Layout.topMargin: root.controlBelow ? 10 : 0
            implicitHeight: (root.controlBelow && root.control) ? root.control.implicitHeight : 0
        }

        // Full width, below the label AND the control — a wide control must never
        // squeeze the description into a narrow column
        Text {
            id: descText
            Layout.fillWidth: true
            Layout.leftMargin: root.contentIndent
            Layout.rightMargin: AppStyle.spacingLg
            Layout.topMargin: root.description !== "" ? 6 : 0
            visible: root.description !== ""
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
            Layout.leftMargin: root.contentIndent + 8
            Layout.rightMargin: AppStyle.spacingLg
            Layout.topMargin: children.length > 0 ? 10 : 0
            spacing: AppStyle.spacingSm
            visible: children.length > 0
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 13
        }

        Rectangle {
            id: divider
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            visible: root.showDivider
            color: Theme.border
        }
    }
}
