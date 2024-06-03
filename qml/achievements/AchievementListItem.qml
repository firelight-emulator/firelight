import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: row
    required property var index
    required property var model

    padding: 8

    background: Item {
        Rectangle {
            anchors.fill: parent
            color: paneHover.hovered ? "#323232" : "#292929"
            radius: 8
            // border.color: model.earned ? "#cfa912" : "transparent"
            // border.width: 2
        }
        Rectangle {
            anchors.fill: parent
            visible: row.model.earned
            radius: 4
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop {
                    position: 0
                    color: "transparent"
                }
                GradientStop {
                    position: 1
                    color: "#6c5312"
                    // color: "#7e7d7d"
                }

            }
        }

    }

    HoverHandler {
        id: paneHover
    }

    TapHandler {
        acceptedButtons: Qt.RightButton
        onTapped: {
            rightClickMenu.popup()
        }
    }

    RightClickMenu {
        id: rightClickMenu

        RightClickMenuItem {
            text: "Open at RetroAchievements.org"
            externalLink: true
            onTriggered: {
                Qt.openUrlExternally("https://retroachievements.org/achievement/" + row.model.achievement_id)
            }
        }
    }

    contentItem: RowLayout {
        spacing: 8
        Image {
            id: pic
            Layout.preferredHeight: 80
            Layout.preferredWidth: 80
            fillMode: Image.PreserveAspectFit
            source: row.model.image_url
        }

        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft
            spacing: 4

            Text {
                id: titleText
                Layout.fillWidth: true
                text: row.model.name
                font.pointSize: 12
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                elide: Text.ElideRight
                maximumLineCount: 2
                color: "white"
            }

            Text {
                id: descriptionText
                Layout.fillWidth: true
                Layout.fillHeight: true
                text: row.model.description
                font.pointSize: 11
                font.family: Constants.regularFontFamily
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
                maximumLineCount: 3
                font.weight: Font.Normal
                color: "#c1c1c1"
            }
        }
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 1
            color: row.model.earned ? "#aeaeae" : "#4c4c4c"

        }
        ColumnLayout {
            Layout.fillHeight: true
            Layout.preferredWidth: 160
            Layout.maximumWidth: 160
            Layout.minimumWidth: 160

            Text {
                text: "Not earned"
                font.pointSize: 11
                visible: !row.model.earned
                font.family: Constants.regularFontFamily
                font.weight: Font.Normal
                color: "#c1c1c1"
            }

            Text {
                text: "Earned on"
                visible: row.model.earned
                font.pointSize: 10
                font.family: Constants.regularFontFamily
                font.weight: Font.Normal
                color: "white"
            }

            Text {
                text: row.model.earned_date_hardcore
                visible: row.model.earned
                font.pointSize: 11
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                color: "white"
            }
        }

        DetailsButton {
            Layout.alignment: Qt.AlignRight | Qt.AlignTop
            onClicked: function () {
                rightClickMenu.popup()
            }
        }

        // Row {
        //     Layout.fillHeight: true
        //     Layout.preferredWidth: 24
        //     Layout.rightMargin: 12
        //     spacing: 4
        //
        //     Text {
        //         id: pointsText
        //         anchors.verticalCenter: parent.verticalCenter
        //         text: row.model.points
        //         font.pointSize: 16
        //         font.family: Constants.regularFontFamily
        //         font.weight: Font.DemiBold
        //         verticalAlignment: Text.AlignVCenter
        //         horizontalAlignment: Text.AlignHCenter
        //         color: "white"
        //     }
        //
        //     Text {
        //         anchors.verticalCenter: parent.verticalCenter
        //         height: pointsText.height
        //         text: "pts"
        //         font.pointSize: 10
        //         font.family: Constants.regularFontFamily
        //         verticalAlignment: Text.AlignBottom
        //         horizontalAlignment: Text.AlignHCenter
        //         color: "white"
        //     }
        // }
        // Text {
        //     id: earnedSection
        //     Layout.fillHeight: true
        //     Layout.preferredWidth: 260
        //     text: row.model.earned ? "Earned on " + row.model.earned_date_hardcore : "Not earned"
        //     font.pointSize: 11
        //     font.family: Constants.regularFontFamily
        //     font.weight: Font.Normal
        //     color: "white"
        // }
    }
}