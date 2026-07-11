import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// Bespoke delegate for the "border-picker" widget: a horizontally scrolling
// strip of border/bezel previews (with a leading "None" tile) read live from
// the BorderStore model. Selecting a tile emits activated(borderId).
FocusScope {
    id: root

    property string label: ""
    property string description: ""
    property string value: ""
    property bool resettable: false

    signal reset()
    signal activated(string value)

    implicitHeight: col.implicitHeight

    // A reusable preview tile.
    component BorderTile: Rectangle {
        id: tile
        property string tileName: ""
        property string tileImage: ""
        property bool selected: false
        signal clicked()

        width: 190
        height: 150
        radius: 8
        color: ColorPalette.neutral800
        border.width: selected ? 3 : 1
        border.color: selected ? Theme.textPrimary : Theme.border

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 6

            // Image-less tiles: "None" shows an empty placeholder; other rendered
            // borders (e.g. "ambient") get a colourful hint instead.
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: tile.tileImage === ""
                radius: 4
                color: "transparent"
                border.width: 1
                border.color: Theme.border

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 1
                    radius: 3
                    visible: tile.tileName !== qsTr("None")
                    opacity: 0.6
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: "#3a5fa0" }
                        GradientStop { position: 0.5; color: "#9a3f7f" }
                        GradientStop { position: 1.0; color: "#3fa060" }
                    }
                }
                Text {
                    anchors.centerIn: parent
                    visible: tile.tileName === qsTr("None")
                    text: qsTr("Off")
                    color: Theme.textMuted
                    font.pixelSize: 14
                    font.family: Constants.regularFontFamily
                }
            }
            Image {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: tile.tileImage !== ""
                source: tile.tileImage
                fillMode: Image.PreserveAspectFit
                asynchronous: true
                cache: true
            }
            Text {
                Layout.fillWidth: true
                text: tile.tileName
                color: Theme.textPrimary
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 13
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
            }
        }

        HoverHandler { cursorShape: Qt.PointingHandCursor }
        TapHandler { onTapped: tile.clicked() }
    }

    ColumnLayout {
        id: col
        width: parent.width
        spacing: 4

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 8
            Layout.rightMargin: 8

            Text {
                Layout.fillWidth: true
                text: root.label
                color: Theme.textPrimary
                font.pixelSize: 16
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                verticalAlignment: Text.AlignVCenter
            }

            Button {
                visible: root.resettable
                padding: 6
                focusPolicy: Qt.NoFocus
                hoverEnabled: true
                onClicked: root.reset()
                HoverHandler { cursorShape: Qt.PointingHandCursor }
                background: Rectangle {
                    radius: 6
                    color: parent.hovered ? ColorPalette.neutral300 : "transparent"
                    opacity: parent.hovered ? 0.2 : 0
                }
                contentItem: Text {
                    text: qsTr("Reset")
                    color: Theme.textMuted
                    font.pixelSize: 14
                    font.family: Constants.regularFontFamily
                    font.weight: Font.Medium
                }
            }
        }

        Text {
            Layout.fillWidth: true
            Layout.leftMargin: 8
            Layout.rightMargin: 8
            visible: root.description !== ""
            text: root.description
            color: Theme.textMuted
            font.pixelSize: 15
            font.family: Constants.regularFontFamily
            font.weight: Font.Medium
            wrapMode: Text.WordWrap
        }

        Flickable {
            Layout.fillWidth: true
            Layout.preferredHeight: 160
            Layout.topMargin: 4
            Layout.bottomMargin: 8
            leftMargin: 8
            rightMargin: 8
            clip: true
            contentWidth: strip.width
            contentHeight: height
            flickableDirection: Flickable.HorizontalFlick
            ScrollBar.horizontal: ScrollBar {}

            Row {
                id: strip
                height: parent.height
                spacing: 10

                BorderTile {
                    anchors.verticalCenter: parent.verticalCenter
                    tileName: qsTr("None")
                    tileImage: ""
                    selected: root.value === ""
                    onClicked: root.activated("")
                }

                Repeater {
                    model: BorderStore
                    delegate: BorderTile {
                        required property string borderId
                        required property string name
                        required property string imageUrl
                        anchors.verticalCenter: parent.verticalCenter
                        tileName: name
                        tileImage: imageUrl
                        selected: root.value === borderId
                        onClicked: root.activated(borderId)
                    }
                }
            }
        }
    }
}
