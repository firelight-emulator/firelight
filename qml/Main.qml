import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts 1.0
import QtQuick.Effects
import Firelight 1.0
import FirelightStyle 1.0

ApplicationWindow {
    id: window

    width: 1280
    height: 720

    visible: true

    title: qsTr("Firelight")
    background: Image {
        source: "file:orange.jpg"
        width: 2560
        height: 1440
        fillMode: Image.Stretch
    }

    Component {
        id: thingy

        Pane {
            id: header
            property alias text: headerLabel.text

            background: Item {
            }

            Column {
                anchors.fill: parent
                spacing: 8

                Text {
                    id: headerLabel
                    color: "#dadada"
                    font.pointSize: 24
                    font.family: Constants.semiboldFontFamily
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                }

                Rectangle {
                    width: parent.width
                    height: 1
                    opacity: 0.3
                    color: "#dadada"
                }
            }
        }
    }

    LibraryPage {
        id: libraryPage
        visible: false
    }

    ControllersPage {
        id: controllerPage
        visible: false
    }

    SettingsPage {
        id: settingsPage
        visible: false
    }

    NowPlayingPage {
        id: nowPlayingPage
        visible: false
    }


    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        // handle: FirelightSplitViewHandle {
        //     width: 1
        // }

        handle: Item {
        }

        Pane {
            id: drawer
            SplitView.preferredWidth: 220
            SplitView.maximumWidth: 220
            SplitView.minimumWidth: 48
            background: Rectangle {
                color: "black"
                opacity: 0.4
            }
            padding: 4

            Button {
                id: theButton
                width: 48
                height: 48
                hoverEnabled: true
                background: Rectangle {
                    color: "#dadada"
                    opacity: theButton.hovered ? 0.2 : 0
                    radius: 6
                }

                onPressed: function () {
                    if (drawer.SplitView.preferredWidth === 220) {
                        drawer.SplitView.preferredWidth = 48
                    } else {
                        drawer.SplitView.preferredWidth = 220
                    }
                }
                anchors.top: parent.top
                anchors.right: parent.right
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 4

                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 10
                }

                Text {
                    text: "Firelight"
                    opacity: parent.width > 48 ? 1 : 0
                    color: "#dadada"
                    font.pointSize: 16
                    font.family: Constants.semiboldFontFamily
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                }

                Text {
                    text: "alpha (0.4.0a)"
                    opacity: parent.width > 48 ? 1 : 0
                    color: "#dadada"
                    font.pointSize: 10
                    font.family: Constants.regularFontFamily
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                }

                Text {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 10
                }

                FirelightMenuItem {
                    labelText: "Home"
                    labelIcon: "\ue88a"
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 40
                    checked: true

                    onToggled: function () {
                        stackview.replace(thingy, {text: "Home"})
                    }
                }
                FirelightMenuItem {
                    labelText: "Library"
                    labelIcon: "\uf53e"
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 40

                    onToggled: function () {
                        stackview.replace(libraryPage)
                    }
                }
                FirelightMenuItem {
                    labelText: "Market"
                    labelIcon: "\uea12"
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 40

                    onToggled: function () {
                        stackview.replace(thingy, {text: "Market"})
                    }
                }
                FirelightMenuItem {
                    labelText: "Controllers"
                    labelIcon: "\uf135"
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 40

                    onToggled: function () {
                        stackview.replace(controllerPage)
                    }
                }
                Rectangle {
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 1
                    opacity: 0.3
                    color: "#dadada"
                }
                FirelightMenuItem {
                    labelText: "Now Playing"
                    labelIcon: "\ue037"
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 40

                    onToggled: function () {
                        stackview.replace(nowPlayingPage)
                    }
                }
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
                FirelightMenuItem {
                    labelText: "Settings"
                    labelIcon: "\ue8b8"
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 40

                    onToggled: function () {
                        stackview.replace(settingsPage)
                    }
                }
                Rectangle {
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 1
                    opacity: 0.3
                    color: "#dadada"
                }
                FirelightMenuItem {
                    labelText: "Profile"
                    labelIcon: "\ue853"
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 40

                    onToggled: function () {
                        stackview.replace(thingy, {text: "Profile"})
                    }
                }


            }


            // ListView {
            //     id: mainCategoryList
            //     interactive: false
            //
            //     anchors.fill: parent
            //
            //     implicitHeight: model.count * 40
            //     model: ListModel {
            //         ListElement {
            //             name: "Home"
            //             icon: "\ue88a"
            //             playlistId: -1
            //         }
            //         ListElement {
            //             name: "Library"
            //             icon: "\uf53e"
            //             playlistId: -1
            //         }
            //         ListElement {
            //             name: "Mods"
            //             icon: "\ue87a"
            //             playlistId: -1
            //         }
            //         ListElement {
            //             name: "Controllers"
            //             icon: "\uf135"
            //             playlistId: -1
            //         }
            //     }
            //
            //     delegate: FirelightMenuItem {
            //         labelText: model.name
            //         labelIcon: model.icon
            //         height: 40
            //         width: mainCategoryList.width
            //         leftPadding: 12
            //         rightPadding: 12
            //
            //         checked: model.name === "Home"
            //
            //         onClicked: function () {
            //             // library_short_model.filterOnPlaylistId(model.playlistId)
            //             if (model.name === "All games") {
            //                 library_short_model.sortByDisplayName()
            //             } else if (model.name === "Newly added") {
            //                 library_short_model.sortByCreatedAt()
            //             } else if (model.name === "Recently played") {
            //                 library_short_model.sortByLastPlayedAt()
            //             }
            //         }
            //
            //         // ButtonGroup.group: buttonGroup
            //     }
            // }
        }
        Pane {
            id: rightSide
            background: Rectangle {
                color: "black"
                opacity: 0.6
            }

            StackView {
                id: stackview
                anchors.fill: parent

                pushEnter: Transition {
                    PropertyAnimation {
                        property: "opacity"
                        from: 0
                        to: 1
                        duration: 200
                    }
                }
                pushExit: Transition {
                    PropertyAnimation {
                        property: "opacity"
                        from: 1
                        to: 0
                        duration: 200
                    }
                }
                popEnter: Transition {
                    PropertyAnimation {
                        property: "opacity"
                        from: 0
                        to: 1
                        duration: 200
                    }
                }
                popExit: Transition {
                    PropertyAnimation {
                        property: "opacity"
                        from: 1
                        to: 0
                        duration: 200
                    }
                }
                replaceEnter: Transition {
                    ParallelAnimation {
                        PropertyAnimation {
                            property: "opacity"
                            from: 0
                            to: 1
                            duration: 400
                        }
                        PropertyAnimation {
                            property: "x"
                            from: 20
                            to: 0
                            duration: 250
                        }
                    }
                }
                replaceExit: Transition {
                }
                // replaceExit: Transition {
                //     PropertyAnimation {
                //         property: "opacity"
                //         from: 1
                //         to: 0
                //         duration: 200
                //     }
                // }
            }

            // LibraryPage {
            //     id: libraryPage
            //     anchors.fill: parent
            // }
        }
        // SplitViewColumn {
        //     width: 300
        //
        //     ListView {
        //         id: listView
        //         anchors.fill: parent
        //         model: ListModel {
        //             ListElement {
        //                 name: "Home"; icon: "home"; page: "Home.qml"
        //             }
        //             ListElement {
        //                 name: "Settings"; icon: "settings"; page: "Settings.qml"
        //             }
        //         }
        //
        //         delegate: Item {
        //             width: listView.width
        //             height: 50
        //
        //             Rectangle {
        //                 anchors.fill: parent
        //                 color: listView.currentIndex === index ? "lightsteelblue" : "transparent"
        //
        //                 Text {
        //                     anchors.centerIn: parent
        //                     text: model.name
        //                 }
        //
        //                 MouseArea {
        //                     anchors.fill: parent
        //                     onClicked: {
        //                         listView.currentIndex = index
        //                         stackView.replaceItem(index, {page: model.page})
        //                     }
        //                 }
        //             }
        //         }
        //     }
        // }
        //
        // StackView {
        //     id: stackView
        //     anchors.fill: parent
        //
        //     initialItem: { page: "Home.qml" }
        // }
    }
}
