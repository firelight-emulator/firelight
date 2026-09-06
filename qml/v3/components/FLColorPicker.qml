import QtQuick

Flow {
    id: root

    property string selectedColor: ""
    property var colors: [""].concat(Theme.folderColors)
    signal picked(string color)

    spacing: AppStyle.spacingSm

    Repeater {
        model: root.colors
        delegate: Rectangle {
            required property string modelData
            width: AppStyle.iconChipSize
            height: AppStyle.iconChipSize
            radius: width / 2
            color: modelData === "" ? "transparent" : modelData
            border.width: root.selectedColor === modelData ? 2 : 1
            border.color: root.selectedColor === modelData ? Theme.textPrimary : (modelData === "" ? Theme.borderStrong : Theme.border)

            // "None" swatch shows a diagonal slash
            Rectangle {
                visible: parent.modelData === ""
                width: parent.width - AppStyle.spacingSm
                height: 2
                radius: 1
                color: Theme.textMuted
                anchors.centerIn: parent
                rotation: 45
            }

            TapHandler {
                onTapped: root.picked(parent.modelData)
            }

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
        }
    }
}
