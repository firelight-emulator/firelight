import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0

Button {
    id: control
    // required property int index

    objectName: "LibraryNavigationMenuItem|" + displayText

    property string iconSource: ""
    property string iconName: ""
    required property string displayText
    property var numberOfItems

    // Optional folder accent color (empty = none), shown as a thin left bar
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

    // Scales with the UI (36 at 100%) so the label isn't cramped when enlarged
    implicitHeight: AppStyle.controlHeight
    width: ListView.view.width

    // onClicked: {
    //     ListView.view.currentIndex = index
    // }

    hoverEnabled: true
    checkable: true
    padding: AppStyle.spacingSm

    background: Item {
        property alias radius: backgroundRect.radius
        Rectangle {
            anchors.fill: parent

            color: control.hovered || control.checked || control.down ? "#FFFFFF" : "transparent"
            opacity: control.down ? 0.05 : control.hovered ? 0.08 : control.checked ? 0.12 : 0
            radius: AppStyle.radiusSm
        }

        Rectangle {
            id: backgroundRect
            color: "transparent"
            anchors.fill: parent
            border.color: control.containsDrag ? "#c36d00" : "transparent"
            border.width: 2
            opacity: 1
            radius: AppStyle.radiusSm
        }

        Rectangle {
            id: accentBar
            visible: control.accentColor !== ""
            color: control.accentColor === "" ? "transparent" : control.accentColor
            width: 3
            radius: 1.5
            anchors.left: parent.left
            anchors.leftMargin: 2
            anchors.verticalCenter: parent.verticalCenter
            height: parent.height - 12
        }
    }
    contentItem: RowLayout {
        // iconSource may be a Material icon (qrc:/icons/<name>), a console logo, or folder
        // art. Render known Material names as crisp font glyphs; everything else as a crisp
        // (GPU curve-rendered) vector image
        readonly property string resolvedIconName: control.iconSource.indexOf("qrc:/icons/") === 0 ? control.iconSource.substring(11) : ""
        readonly property bool iconIsGlyph: resolvedIconName !== "" && MaterialSymbols.glyph(resolvedIconName) !== ""

        // Nesting indent
        Item {
            visible: control.depth > 0
            Layout.preferredWidth: control.depth * Math.round(14 * AppStyle.scale)
            Layout.fillHeight: true
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
                        duration: 120
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

        Icon {
            Layout.preferredWidth: height
            Layout.fillHeight: true
            // visible: parent.iconIsGlyph
            name: control.iconName
            filled: false
            size: Math.round(20 * AppStyle.scale)
            color: control.iconColor
        }
        Text {
            Layout.leftMargin: AppStyle.spacingSm
            Layout.fillWidth: true
            color: Theme.textPrimary
            font.family: Constants.regularFontFamily
            font.pixelSize: AppStyle.fontSizeMedium
            font.weight: Font.DemiBold
            text: control.displayText
            elide: Text.ElideRight
            // Hidden when the row is narrow (collapsed rail) — scales so it holds
            // at any UI scale
            visible: control.width > Math.round(64 * AppStyle.scale)
        }
        Text {
            Layout.leftMargin: AppStyle.spacingSm
            color: Theme.textMuted
            font.family: Constants.regularFontFamily
            font.pixelSize: AppStyle.fontSizeSmall
            font.weight: Font.DemiBold
            text: control.numberOfItems ?? ""
            Layout.fillHeight: true
            verticalAlignment: Text.AlignVCenter
            visible: control.numberOfItems !== undefined && control.numberOfItems !== null && control.width > Math.round(64 * AppStyle.scale)
        }
    }
}
