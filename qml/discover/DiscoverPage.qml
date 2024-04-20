import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import QtQml.Models
import FirelightStyle 1.0

Item {
    id: root

    Pane {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        background: Item {
        }

        horizontalPadding: 0
        verticalPadding: 6

        Column {
            anchors.fill: parent
            spacing: 8

            Text {
                text: "Mods"
                color: "#353535"
                font.pointSize: 24
                font.family: Constants.semiboldFontFamily
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    Item {
        id: content
        anchors.topMargin: 4
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        ListView {
            id: list
            anchors.fill: parent
            boundsBehavior: Flickable.StopAtBounds
            clip: true
            spacing: 8
            model: mod_model

            delegate: Button {
                id: button
                height: 100
                width: 200
                padding: 4

                scale: pressed ? 0.99 : 1

                Behavior on scale {
                    NumberAnimation {
                        duration: 50
                        easing.type: Easing.InOutBounce
                    }
                }

                // layer.enabled: true
                // layer.effect: MultiEffect {
                //     id: shadowEffect
                //     source: button
                //     // anchors.fill: button
                //     shadowEnabled: true
                //     shadowBlur: 0.4
                //     shadowVerticalOffset: button.pressed ? 2 : 6
                //     shadowScale: 1
                //     shadowOpacity: 0.3
                //
                //     Behavior on shadowVerticalOffset {
                //         NumberAnimation {
                //             duration: 50
                //             easing.type: Easing.InOutBounce
                //         }
                //     }
                // }

                background: Rectangle {
                    id: bg
                    color: "white"
                    radius: 8

                    // Behavior on opacity {
                    //     NumberAnimation {
                    //         duration: 100
                    //         easing.type: Easing.InOutQuad
                    //     }
                    // }
                }

                // MultiEffect {
                //     source: bg
                //     anchors.fill: bg
                //     shadowEnabled: true
                // }

                HoverHandler {
                    id: hov
                    cursorShape: Qt.PointingHandCursor
                }

                TapHandler {
                    onTapped: {
                        contentPopup.modId = model.id
                        contentPopup.name = model.name
                        contentPopup.author = model.primary_author
                        contentPopup.description = model.description
                        contentPopup.targetGameName = model.target_game_name
                        contentPopup.modInLibrary = library_model.isModInLibrary(model.id)

                        contentPopup.targetInLibrary = false
                        for (let i = 0; i < model.rom_ids.length; i++) {
                            if (library_model.isRomInLibrary(model.rom_ids[i])) {
                                contentPopup.targetInLibrary = true
                                break
                            }
                        }

                        contentPopup.open()
                    }
                }

                contentItem: RowLayout {
                    spacing: 8

                    Image {
                        id: img
                        Layout.alignment: Qt.AlignTop
                        Layout.fillWidth: true
                        Layout.preferredHeight: width / 2
                        // visible: false
                        source: model.image_source
                        fillMode: Image.Stretch
                    }
                    //
                    // ColumnLayout {
                    //     Layout.topMargin: 4
                    //     Layout.bottomMargin: 4
                    //     Layout.fillHeight: true
                    //     Layout.fillWidth: true
                    //     spacing: 4
                    //
                    //     Text {
                    //         Layout.fillWidth: true
                    //         text: model.name
                    //         font.family: Constants.semiboldFontFamily
                    //         font.pointSize: 14
                    //         verticalAlignment: Text.AlignVCenter
                    //         color: "black"
                    //         opacity: 0.9
                    //     }
                    //
                    //     Text {
                    //         Layout.fillWidth: true
                    //         text: "by " + model.primary_author
                    //         font.family: Constants.regularFontFamily
                    //         font.pointSize: 10
                    //         verticalAlignment: Text.AlignVCenter
                    //         color: "black"
                    //         opacity: 0.7
                    //     }
                    //     Item {
                    //         Layout.fillWidth: true
                    //         Layout.fillHeight: true
                    //     }
                    //
                    //     Text {
                    //         Layout.fillWidth: true
                    //         text: "Mod for " + model.target_game_name + " (" + model.platform_name + ")"
                    //         font.family: Constants.regularFontFamily
                    //         font.pointSize: 10
                    //         verticalAlignment: Text.AlignVCenter
                    //         color: "black"
                    //         opacity: 0.7
                    //     }
                    // }
                }
            }
        }
    }


    FirelightDialog {
        id: contentPopup

        property int modId;
        property int patchId;
        property string name;
        property string author;
        property string description;
        property bool targetInLibrary;
        property bool modInLibrary;
        property string targetGameName;
        property int romId;
        property int gameReleaseId;

        width: parent.width * 0.75
        height: parent.height * 0.9
        parent: Overlay.overlay
        x: (parent.width / 2) - (width / 2)
        y: (parent.height / 2) - (height / 2)
        contentItem: StoreContent {
            id: contentThing
            anchors.centerIn: parent
            modId: contentPopup.modId
            name: contentPopup.name
            author: contentPopup.author
            description: contentPopup.description
            targetInLibrary: contentPopup.targetInLibrary
            targetGameName: contentPopup.targetGameName
            romId: contentPopup.romId
            gameReleaseId: contentPopup.gameReleaseId
            modInLibrary: contentPopup.modInLibrary
        }

        onAboutToShow: {
            contentThing.modId = contentPopup.modId
            contentThing.name = contentPopup.name
            contentThing.author = contentPopup.author
            contentThing.description = contentPopup.description
            contentThing.targetInLibrary = contentPopup.targetInLibrary
            contentThing.targetGameName = contentPopup.targetGameName
            contentThing.romId = contentPopup.romId
            contentThing.gameReleaseId = contentPopup.gameReleaseId
            contentThing.modInLibrary = contentPopup.modInLibrary
        }

        onClosed: {
            contentThing.modId = -1
            contentThing.name = ""
            contentThing.author = ""
            contentThing.description = ""
            contentThing.targetInLibrary = false
            contentThing.targetGameName = ""
            contentThing.modInLibrary = false
            contentThing.romId = -1
            contentThing.gameReleaseId = -1
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


}