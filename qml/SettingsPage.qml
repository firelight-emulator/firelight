import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FirelightStyle 1.0

Pane {
    id: root
    background: Item {
    }

    SplitView {
        id: page
        orientation: Qt.Horizontal
        anchors.fill: parent

        handle: FirelightSplitViewHandle {
        }

        ContentPane {
            id: navigation
            SplitView.minimumWidth: 350
            SplitView.maximumWidth: 350

            ListView {
                id: categoryList

                clip: true
                boundsBehavior: Flickable.StopAtBounds
                anchors.fill: parent
                model: ListModel {
                    ListElement {
                        display_name: "Appearance"
                        icon: "\ue40a"
                        enabled: false
                    }
                    ListElement {
                        display_name: "Video"
                        icon: "\ue333"
                        enabled: true
                    }
                    ListElement {
                        display_name: "Sound"
                        icon: "\ue050"
                        enabled: false
                    }
                    // ListElement {
                    //     display_name: "Menu Navigation"
                    //     icon: "\ue89f"
                    // }
                    ListElement {
                        display_name: "System"
                        icon: "\uf522"
                        enabled: false
                    }
                    ListElement {
                        display_name: "About"
                        icon: "\ue88e"
                        enabled: false
                    }
                }

                delegate: Button {
                    id: settingsCategoryButton
                    background: Rectangle {
                        color: enabled ? handler.hovered || checked ? "white" : "transparent" : "transparent"
                        opacity: checked ? 0.2 : 0.1
                        radius: 4
                    }

                    enabled: model.enabled

                    implicitHeight: 48

                    checkable: true
                    autoExclusive: true

                    width: ListView.view.width

                    onClicked: function () {
                        stackView.replace(controls.pages[model.index])
                    }

                    contentItem: Item {
                        anchors.fill: parent

                        Text {
                            id: buttonIcon
                            text: model.icon
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            visible: model.icon !== ""
                            leftPadding: 8
                            // width: 24

                            font.family: Constants.symbolFontFamily
                            font.pixelSize: 28
                            color: settingsCategoryButton.enabled ? "#b3b3b3" : "#666666"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        Text {
                            id: buttonText
                            text: model.display_name
                            anchors.left: model.icon !== "" ? buttonIcon.right : parent.left
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            leftPadding: model.icon !== "" ? 8 : 0
                            font.pointSize: 11
                            font.family: Constants.semiboldFontFamily
                            color: settingsCategoryButton.enabled ? "#b3b3b3" : "#666666"
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        HoverHandler {
                            id: handler
                            cursorShape: Qt.PointingHandCursor
                        }
                    }
                }
            }
        }

        ContentPane {
            id: controls

            property var pages: [appearancePage, videoPage, soundPage, systemPage, aboutPage]

            StackView {
                id: stackView
                anchors.fill: parent

                initialItem: appearancePage

                popEnter: Transition {
                }
                popExit: Transition {
                }
                pushEnter: Transition {
                }
                pushExit: Transition {
                }

                replaceEnter: Transition {
                    PropertyAnimation {
                        property: "opacity"
                        from: 0
                        to: 1
                        duration: 400
                        easing.type: Easing.InOutQuad
                    }
                }

                replaceExit: Transition {
                }

            }

            Component {
                id: appearancePage
                AppearanceSettings {
                }
            }

            Component {
                id: videoPage
                VideoSettings {
                }
            }

            Component {
                id: soundPage
                SoundSettings {
                }
            }

            Component {
                id: systemPage
                SystemSettings {
                }
            }

            Component {
                id: aboutPage
                About {
                }
            }
        }
    }
}

// Component {
//     id: settingsPage
//     Item {
//         Rectangle {
//             id: person
//             width: 800
//             height: 600
//
//             property var pages: [page1, page2, page3]
//
//             ListView {
//                 id: categoryList
//                 width: parent.width / 2
//                 height: parent.height
//                 model: ListModel {
//                     ListElement {
//                         name: "Category 1"
//                     }
//                     ListElement {
//                         name: "Category 2"
//                     }
//                     ListElement {
//                         name: "Category 3"
//                     }
//                 }
//                 clip: true
//                 delegate: Rectangle {
//                     width: categoryList.width
//                     height: 50
//                     color: "lightgray"
//
//                     Text {
//                         text: model.name
//                         font.pixelSize: 20
//                         color: "black"
//                         anchors.centerIn: parent
//                     }
//
//                     MouseArea {
//                         anchors.fill: parent
//                         onClicked: {
//                             stackView.push(person.pages[model.index])
//                         }
//                     }
//                 }
//             }
//
//             StackView {
//                 id: stackView
//                 width: parent.width / 2
//                 height: parent.height
//                 anchors.left: categoryList.right
//             }
//
//             Component {
//                 id: page1
//                 Rectangle {
//                     color: "red"
//                     Text {
//                         text: "This is Category 1"
//                         anchors.centerIn: parent
//                     }
//                 }
//             }
//
//             Component {
//                 id: page2
//                 Rectangle {
//                     color: "green"
//                     Text {
//                         text: "This is Category 2"
//                         anchors.centerIn: parent
//                     }
//                 }
//             }
//
//             Component {
//                 id: page3
//                 Rectangle {
//                     color: "blue"
//                     Text {
//                         text: "This is Category 3"
//                         anchors.centerIn: parent
//                     }
//                 }
//             }
//         }
//     }
// }