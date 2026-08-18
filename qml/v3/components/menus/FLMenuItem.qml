// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// The one row. Used wherever a row is needed — a settings page, a menu, a popup menu — with the surface
// deciding what shows rather than the type deciding it.
//
//   FLMenuItem { iconName: "settings"; label: "Appearance" }
//   FLMenuItem { label: "Volume"; description: "How loud everything is"; controlItem: FLSlider {} }
ItemDelegate {
    id: root

    // TODO
    // Where this row is being shown. Everything that differs between a settings page and a menu hangs
    // off this rather than off which type the row is
    enum Surface {
        Page,
        InMenu,
        InPopup
    }

    // TODO
    // A row is a menu row unless something says otherwise; a settings card tells its rows it is a page
    property int surface: FLMenuItem.Surface.InMenu

    property string iconName: ""
    property string label: ""
    property string description: ""

    // TODO
    // The control this row wraps, moved between the side and below slots. A property rather than a slot
    // child because `controlBelow` changes at runtime and a declared child cannot move
    property Item controlItem: null
    property bool controlBelow: false

    default property alias trailing: sideSlot.data
    property alias nestedContent: nestedColumn.data

    // TODO
    // Where the row sits in its group, which decides the corners it rounds and whether it carries a line
    property bool isFirstInSection: false
    property bool isLastInSection: false
    // TODO
    // Off unless a group turns it on: a line belongs to the list a row is in, not to the row
    property bool showDivider: false

    // TODO
    // Hidden by default outside a settings page, and settable so a menu row that needs the explanation
    // can ask for it
    property bool showDescription: root.surface === FLMenuItem.Surface.Page

    // Marks this row as a dependent sub-setting of the one above it: indented
    // with a quieter label, so two cues carry the relationship rather than one
    property bool subItem: false
    readonly property int contentIndent: subItem ? 40 : AppStyle.spacingLg

    // Bind to whatever a dependent setting hangs off of
    property bool shown: true

    // Shown when this setting has an override at the current tier; emits reset()
    // so the caller can clear it (falling back to the inherited value)
    property bool resettable: false
    signal reset

    readonly property bool _inGroup: root.surface === FLMenuItem.Surface.Page
    readonly property bool _drawTopRadius: isFirstInSection && _inGroup
    readonly property bool _drawBottomRadius: isLastInSection && _inGroup

    // TODO
    // One pair of corners for the highlight and the cursor ring both, so the ring cannot end up square
    // around a rounded highlight
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

    topPadding: AppStyle.spacingMd
    bottomPadding: _inGroup ? AppStyle.rowSpacerPage : AppStyle.spacingMd
    leftPadding: root.contentIndent
    rightPadding: AppStyle.spacingLg

    implicitHeight: Math.max(AppStyle.listRowHeight, contentColumn.implicitHeight + topPadding + bottomPadding)
    implicitWidth: contentColumn.implicitWidth + leftPadding + rightPadding

    // TODO
    // Focus as the controller cursor shows it, shared with every other control so a row never stays lit
    // while the ring is blinked out. `focusProxy` covers the rows that hand focus to their control
    property Item focusProxy: null
    readonly property bool cursorFocused: FocusCursor.isOn(root) || (focusProxy !== null && FocusCursor.isOn(focusProxy))

    highlighted: root.cursorFocused || root.pressed || rowHover.hovered

    // TODO
    // The keys that activate a row when it declared no action answering to them
    readonly property var activationKeys: [Qt.Key_Select, Qt.Key_Return, Qt.Key_Enter, Qt.Key_Space]

    // The single `controlItem` lives either in the side slot or the below slot
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

    // TODO
    // Space and Select never leave a row — ItemDelegate takes both for itself and
    // turns them into a click — so a declared action answering to either has to
    // run here rather than at the window. A Connections because a derived type
    // declaring Keys.onPressed would replace an instance handler
    Connections {
        target: root.Keys

        function onPressed(event) {
            if (event.isAutoRepeat) {
                return;
            }

            const action = root.FLFocus.getActionFor(event.key);

            if (action !== null) {
                if (action.sound) {
                    action.sound.play(false);
                }

                action.triggered();
                event.accepted = true;
                return;
            }

            // TODO
            // Activation happens on the press for every key. Left alone, the row
            // takes Space and Select for itself and commits them on the release
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

    // TODO
    // Hover and keyboard focus read the same; pressing drops a step down the
    // surface ladder so the row darkens under the finger
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
                // TODO
                // In a menu `checked` marks the chosen option, so the label carries it. On a page it
                // only means a toggle is on, which the toggle already shows
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

        // Full width, below the label AND the control — a wide control must never
        // squeeze the description into a narrow column
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

    // TODO
    // Full-bleed, so it reaches the card's edges rather than stopping at the row's padding
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        visible: root.showDivider
        color: Theme.border
    }
}
