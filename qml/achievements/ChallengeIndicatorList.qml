import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ListView {
    id: root

    model: ListModel {
    }

    spacing: 8

    layoutDirection: Qt.RightToLeft
    orientation: ListView.Horizontal

    implicitWidth: root.model.count * 32 + (root.model.count - 1) * root.spacing
    implicitHeight: 32

    Connections {
        target: achievement_manager

        function onShowChallengeIndicator(id, imageUrl, name, description) {
            root.model.append({
                "achievementId": id,
                "imageUrl": imageUrl,
                "title": name,
                "description": description
            })
        }

        function onHideChallengeIndicator(id) {
            for (let i = 0; i < root.model.count; i++) {
                if (root.model.get(i).achievementId === id) {
                    root.model.remove(i)
                    break
                }
            }
        }
    }

    delegate: Image {
        id: icon
        required property int achievementId
        required property string imageUrl
        required property string title
        required property string description

        width: 32
        height: 32
        source: imageUrl
        fillMode: Image.PreserveAspectFit

        HoverHandler {
            id: hovvv
        }

        ToolTip {
            visible: hovvv.hovered
            text: title + "\n" + description

            background: Rectangle {
                color: "#1a1b1e"
                radius: 4
            }

            contentItem: ColumnLayout {
                spacing: 4
                width: 300

                Text {
                    id: titleText
                    Layout.fillWidth: true
                    text: icon.title
                    font.pointSize: 11
                    font.family: Constants.regularFontFamily
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                    maximumLineCount: 1
                    color: "white"
                }

                Text {
                    id: descriptionText
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: icon.description
                    font.pointSize: 10
                    font.family: Constants.regularFontFamily
                    wrapMode: Text.WordWrap
                    verticalAlignment: Text.AlignVCenter
                    font.weight: Font.Normal
                    color: "#c1c1c1"
                }
            }

        }
    }
}