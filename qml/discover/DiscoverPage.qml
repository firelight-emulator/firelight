import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models
import FirelightStyle 1.0

Pane {
    id: root

    background: Item {
    }

    FirelightDialog {
        id: contentPopup

        property string name;
        property string author;
        property string description;

        width: parent.width * 0.75
        height: parent.height * 0.9
        parent: Overlay.overlay
        x: (parent.width / 2) - (width / 2)
        y: (parent.height / 2) - (height / 2)
        contentItem: StoreContent {
            anchors.centerIn: parent
            name: contentPopup.name
            author: contentPopup.author
            description: contentPopup.description
        }

        header: Item {
            height: 0
            width: 0
        }
        footer: Item {
            height: 0
            width: 0
        }
    }

    Pane {
        id: content
        anchors.fill: parent

        background: Item {
        }

        ListView {
            id: list
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            boundsBehavior: Flickable.StopAtBounds
            width: 600
            clip: true
            spacing: 8
            model: mod_model

            delegate: Pane {
                height: 100
                width: parent.width
                padding: 4

                background: Rectangle {
                    color: "white"
                    opacity: hov.hovered ? 0.2 : 0.1
                    radius: 4

                    Behavior on opacity {
                        NumberAnimation {
                            duration: 100
                            easing.type: Easing.InOutQuad
                        }
                    }
                }

                HoverHandler {
                    id: hov
                    cursorShape: Qt.PointingHandCursor
                }

                TapHandler {
                    onTapped: {
                        contentPopup.name = model.name
                        contentPopup.author = model.primary_author
                        contentPopup.description = model.description
                        contentPopup.open()
                    }
                }

                RowLayout {
                    anchors.fill: parent
                    spacing: 8

                    Image {
                        Layout.fillHeight: true
                        Layout.preferredWidth: height * 2
                        source: model.image_source
                        fillMode: Image.Stretch
                    }

                    ColumnLayout {
                        Layout.topMargin: 4
                        Layout.bottomMargin: 4
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            Layout.fillWidth: true
                            text: model.name
                            font.family: Constants.semiboldFontFamily
                            font.pointSize: 16
                            verticalAlignment: Text.AlignVCenter
                            color: "white"
                        }

                        Text {
                            Layout.fillWidth: true
                            text: "by " + model.primary_author
                            font.family: Constants.regularFontFamily
                            font.pointSize: 10
                            verticalAlignment: Text.AlignVCenter
                            color: "#cfcfcf"
                        }
                        Item {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                        }

                        Text {
                            Layout.fillWidth: true
                            text: "Mod for " + model.target_game_name + " (" + model.platform_name + ")"
                            font.family: Constants.regularFontFamily
                            font.pointSize: 10
                            verticalAlignment: Text.AlignVCenter
                            color: "#cfcfcf"
                        }
                    }
                }
            }
        }

        // HackPage {
        //     anchors.left: list.right
        //     anchors.top: parent.top
        //     anchors.bottom: parent.bottom
        //     anchors.right: parent.right
        // }
    }
}