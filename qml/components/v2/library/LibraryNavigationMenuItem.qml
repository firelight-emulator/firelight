import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQml.Models
import QtQuick.Layouts 1.0
import QtNetwork
import QtMultimedia
import QtQuick.VectorImage
import QtQuick.Effects
import Firelight 1.0

Button {
    id: control
    // required property int index

    objectName: "LibraryNavigationMenuItem|" + displayText

    property string iconSource: ""
    property string iconName: ""
    required property string displayText
    property var numberOfItems

    // Optional folder accent color (empty = none), shown as a thin left bar.
    property string accentColor: ""

    property bool containsDrag: false
    property bool showGlobalCursor: true
    property real globalCursorSpacing: 2

    // Scales with the UI (36 at 100%) so the label isn't cramped when enlarged.
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
        // (GPU curve-rendered) vector image.
        readonly property string resolvedIconName: control.iconSource.indexOf("qrc:/icons/") === 0
            ? control.iconSource.substring(11) : ""
        readonly property bool iconIsGlyph: resolvedIconName !== "" && MaterialSymbols.glyph(resolvedIconName) !== ""

        Icon {
            Layout.preferredWidth: height
            Layout.fillHeight: true
            // visible: parent.iconIsGlyph
            name: control.iconName
            filled: false
            size: Math.round(20 * AppStyle.scale)
            color: Theme.textPrimary
        }
        // VectorImage {
        //     Layout.preferredWidth: height
        //     Layout.fillHeight: true
        //     visible: !parent.iconIsGlyph
        //     source: control.iconSource
        //     fillMode: Image.PreserveAspectFit
        //     smooth: true
        //     preferredRendererType: VectorImage.CurveRenderer
        // }
        // Fills the row and elides: a long platform name ("Genesis/Mega Drive")
        // shrinks with "…" instead of shoving the count off the clipped edge.
        Text {
            Layout.leftMargin: AppStyle.spacingSm
            Layout.fillWidth: true
            color: Theme.textPrimary
            font.family: Constants.regularFontFamily
            font.pixelSize: AppStyle.fontSizeMedium
            font.weight: Font.DemiBold
            text: control.displayText
            elide: Text.ElideRight
            visible: control.width > 64
        }
        Text {
            Layout.leftMargin: AppStyle.spacingSm
            color: Theme.textMuted
            font.family: Constants.regularFontFamily
            font.pixelSize: AppStyle.fontSizeSmall
            font.weight: Font.DemiBold
            text: control.numberOfItems
            Layout.fillHeight: true
            verticalAlignment: Text.AlignVCenter
            visible: control.numberOfItems !== undefined && control.numberOfItems !== null
        }
    }
}