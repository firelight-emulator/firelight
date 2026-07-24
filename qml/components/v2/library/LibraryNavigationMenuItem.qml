import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0

Button {
    id: control

    objectName: "LibraryNavigationMenuItem|" + displayText

    property string iconSource: ""
    property string iconName: ""

    // TODO
    // Platform rows render their logo via FLPlatformIcon (>= 0 = a platform row)
    property int platformId: -1

    // TODO
    // Custom folder art (empty = use the glyph/logo). Takes priority in the chip
    property string customIconUrl: ""
    required property string displayText

    // TODO
    // Row count badge; negative hides it
    property int numberOfItems: -1

    // TODO
    // Folder color (empty = none). Tints the icon chip and the selection pill
    property string accentColor: ""

    property bool containsDrag: false
    property bool showGlobalCursor: true
    property real globalCursorSpacing: 2

    // Tree rendering (folders only): indent by depth and show a disclosure
    // chevron for rows that have children. Off by default so flat sections
    // (platforms, settings nav) are untouched
    property bool treeItem: false
    property int depth: 0
    property bool hasChildren: false
    property bool expanded: false
    property color iconColor: Theme.textPrimary
    signal toggleExpanded

    // TODO
    // The color the chip/pill tint derive from, and whether one was set
    readonly property bool hasAccent: accentColor !== ""
    readonly property color tintBase: hasAccent ? Qt.color(accentColor) : Theme.textMuted

    // Scales with the UI (36 at 100%) so the label isn't cramped when enlarged
    implicitHeight: AppStyle.controlHeight
    width: ListView.view.width

    hoverEnabled: true
    checkable: true
    padding: AppStyle.spacingSm

    background: Item {
        // TODO
        // Reported to the global focus ring so it rounds to match the pill
        readonly property int radius: AppStyle.radiusMd

        // TODO
        // Selection/hover pill, inset so it floats off the sidebar edges. Tints
        // with the folder color when selected, else a neutral wash
        Rectangle {
            anchors.fill: parent
            anchors.leftMargin: AppStyle.spacingXs
            anchors.rightMargin: AppStyle.spacingXs
            radius: AppStyle.radiusMd
            color: {
                if (control.checked) {
                    return control.hasAccent ? Qt.rgba(control.tintBase.r, control.tintBase.g, control.tintBase.b, 0.16) : Theme.surfaceElevated;
                }
                if (control.down) {
                    return Theme.surfaceHover;
                }
                if (control.hovered) {
                    return Theme.surfaceHover;
                }
                return "transparent";
            }
        }

        Rectangle {
            anchors.fill: parent
            anchors.leftMargin: AppStyle.spacingXs
            anchors.rightMargin: AppStyle.spacingXs
            color: "transparent"
            border.color: control.containsDrag ? Theme.focusRing : "transparent"
            border.width: 2
            radius: AppStyle.radiusMd
        }
    }

    contentItem: RowLayout {
        id: contentRow
        spacing: 0

        // qrc:/icons/<name> may be a Material glyph (browse, folder) or a console
        // logo vector image; only the former resolves in MaterialSymbols
        readonly property string resolvedIconName: control.iconSource.indexOf("qrc:/icons/") === 0 ? control.iconSource.substring(11) : ""
        readonly property bool iconIsGlyph: resolvedIconName !== "" && MaterialSymbols.glyph(resolvedIconName) !== ""

        // TODO
        // The glyph to draw, if any (explicit iconName or a glyph-backed source)
        readonly property string glyphName: control.iconName !== "" ? control.iconName : (iconIsGlyph ? resolvedIconName : "")

        // TODO
        // Otherwise a vector image: custom folder art first, then a console logo
        readonly property string imageUrl: control.customIconUrl !== "" ? control.customIconUrl : (glyphName === "" ? control.iconSource : "")

        // Nesting indent (with a quiet tree guide on the right edge)
        Item {
            visible: control.depth > 0
            Layout.preferredWidth: control.depth * Math.round(14 * AppStyle.scale)
            Layout.fillHeight: true

            Rectangle {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                width: 1
                height: parent.height - AppStyle.spacingSm
                color: Theme.border
            }
        }

        // Disclosure chevron (a fixed column in tree sections so leaf and
        // parent rows keep their icons aligned)
        Item {
            visible: control.treeItem
            Layout.preferredWidth: control.treeItem ? Math.round(18 * AppStyle.scale) : 0
            Layout.fillHeight: true
            Icon {
                anchors.centerIn: parent
                visible: control.hasChildren
                name: "chevron-forward"
                size: AppStyle.iconSizeSm
                color: Theme.textMuted
                rotation: control.expanded ? 90 : 0
                Behavior on rotation {
                    NumberAnimation {
                        duration: AppStyle.durationFast
                    }
                }
            }
            TapHandler {
                enabled: control.hasChildren
                onTapped: control.toggleExpanded()
            }
            HoverHandler {
                enabled: control.hasChildren
                cursorShape: Qt.PointingHandCursor
            }
        }

        // TODO
        // Colored icon chip: the row's glanceable cue
        Rectangle {
            Layout.preferredWidth: AppStyle.iconChipSize
            Layout.preferredHeight: AppStyle.iconChipSize
            Layout.alignment: Qt.AlignVCenter
            radius: AppStyle.radiusSm
            color: Qt.rgba(control.tintBase.r, control.tintBase.g, control.tintBase.b, control.hasAccent ? 0.18 : 0.1)

            FLPlatformIcon {
                anchors.centerIn: parent
                visible: control.platformId >= 0
                platformId: control.platformId
                width: AppStyle.iconSizeSm
                height: AppStyle.iconSizeSm
            }

            Icon {
                anchors.centerIn: parent
                visible: control.platformId < 0 && contentRow.imageUrl === ""
                name: contentRow.glyphName
                filled: false
                size: AppStyle.iconSizeSm
                color: control.iconColor
            }

            Image {
                anchors.centerIn: parent
                visible: control.platformId < 0 && contentRow.imageUrl !== ""
                width: AppStyle.iconSizeSm
                height: AppStyle.iconSizeSm
                fillMode: Image.PreserveAspectFit
                sourceSize: Qt.size(width * 2, height * 2)
                source: contentRow.imageUrl
            }
        }

        Text {
            Layout.leftMargin: AppStyle.spacingSm
            Layout.fillWidth: true
            color: Theme.textPrimary
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeMedium
            font.weight: Font.DemiBold
            text: control.displayText
            elide: Text.ElideRight
            // Hidden when the row is narrow (collapsed rail) — scales so it holds
            // at any UI scale
            visible: control.width > AppStyle.sidebarLabelThreshold
        }
        Text {
            Layout.leftMargin: AppStyle.spacingSm
            Layout.rightMargin: AppStyle.spacingXs
            color: Theme.textMuted
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeSmall
            font.weight: Font.DemiBold
            text: control.numberOfItems >= 0 ? control.numberOfItems : ""
            Layout.fillHeight: true
            verticalAlignment: Text.AlignVCenter
            visible: control.numberOfItems >= 0 && control.width > AppStyle.sidebarLabelThreshold
        }
    }
}
