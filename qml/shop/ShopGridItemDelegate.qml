import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    id: myDelegate
    //
    // signal startGame(entryId: int)
    //
    // signal openDetails(entryId: int)
    //
    // signal manageSaveData(entryId: int)

    signal onClicked()

    required property var index
    required property var model

    required property real cellWidth
    required property real cellHeight
    required property real cellSpacing

    width: cellWidth
    height: cellHeight
    Button {
        id: button
        padding: 0
        anchors {
            fill: parent
            margins: myDelegate.cellSpacing / 2
        }
        focus: true

        onClicked: function () {
            myDelegate.onClicked()
        }

        hoverEnabled: true
        leftInset: 2
        rightInset: 2

        background: Rectangle {
            objectName: "background"
            color: button.hovered ? "#3a3e45" : "#25282C"
        }

        contentItem: ColumnLayout {
            Image {
                id: image
                Layout.preferredHeight: parent.width / (92 / 43)
                Layout.fillWidth: true
                fillMode: Image.Stretch

                source: myDelegate.model.capsule_image_url

            }
            Text {
                text: myDelegate.model.title
                font.pointSize: 11
                font.weight: Font.Bold
                font.family: Constants.regularFontFamily
                color: "white"
                Layout.fillWidth: true
                leftPadding: 4
                elide: Text.ElideRight
                maximumLineCount: 1
                // Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            }

            Text {
                // text: myDelegate.model.platform_name
                text: myDelegate.model.target_game_name + " (" + myDelegate.model.platform_name + ")"
                font.pointSize: 10
                font.weight: Font.Medium
                font.family: Constants.regularFontFamily
                leftPadding: 4
                color: "#C2BBBB"
                Layout.fillWidth: true
                // Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            }

            Text {
                // text: myDelegate.model.platform_name
                text: "by " + myDelegate.model.creator_name
                font.pointSize: 10
                font.weight: Font.Medium
                font.family: Constants.regularFontFamily
                leftPadding: 4
                color: "#C2BBBB"
                Layout.fillWidth: true
                // Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            }

            // Text {
            //     text: "Mario Kart 64 (Nintendo 64)"
            //     font.pointSize: 10
            //     font.weight: Font.Medium
            //     font.family: Constants.regularFontFamily
            //     leftPadding: 4
            //     color: "#C2BBBB"
            //     Layout.fillWidth: true
            //     // Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            // }
            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
        }
    }
}