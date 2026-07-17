import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts 1.0
import Firelight 1.0

Pane {
    id: root

    signal resumeGame()
    signal resetGame()
    signal rewindPressed()
    signal closeGame()
    signal backToMenu()

    LibraryEntry {
        id: entry
        entryId: EmulationService.currentEntryId
    }

    horizontalPadding: 16
    verticalPadding: 0
    background: Item {}

    contentItem: ColumnLayout {
        spacing: 0
        RowLayout {
            id: navBanner
            Layout.fillWidth: true
            Layout.maximumHeight: Constants.standardTitleBarHeight
            Layout.minimumHeight: Constants.standardTitleBarHeight
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            TabBar {
                id: navTabBar
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                Layout.preferredHeight: parent.height
                spacing: 18

                background: Item {}

                Repeater {
                    focus: true
                    model: ["Quick Menu", "Achievements", "Settings"]
                    delegate: TabButton {
                          HoverHandler {
                              id: hoverHandler
                              cursorShape: Qt.PointingHandCursor
                          }
                          anchors.verticalCenter: parent.verticalCenter
                          padding: 10
                          property bool showGlobalCursor: true
                          contentItem: Text {
                              text: modelData
                              font.family: Constants.mainFontFamily
                              font.pixelSize: AppStyle.fontSizeMedium
                              font.weight: Font.Medium
                              color: Theme.textPrimary
                              verticalAlignment: Text.AlignVCenter
                              horizontalAlignment: Text.AlignHCenter
                          }
                          background: Rectangle {
                              color: ColorPalette.neutral100
                              opacity: parent.TabBar.index === navTabBar.currentIndex ? 0.16 : parent.pressed ? 0.04 : hoverHandler.hovered ? 0.08 : 0
                              radius: 6
                          }
                          width: 130
                      }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            opacity: 0.3
            color: "#dadada"
        }

        StackView {
            id: quickMenuStack
            Layout.fillHeight: true
            Layout.fillWidth: true
            focus: true

            property var lastIndex: 0
            property bool movingRight: true

            Keys.onPressed: function(event) {
                if (event.key === Qt.Key_PageDown && navTabBar.currentIndex > 0) {
                    navTabBar.currentIndex -= 1
                    event.accept = true
                    return
                }

                if (event.key === Qt.Key_PageUp && navTabBar.currentIndex < navTabBar.count - 1) {
                    navTabBar.currentIndex += 1
                    event.accept = true
                    return
                }

                event.accept = false
            }

            Connections {
                target: navTabBar
                function onCurrentIndexChanged() {
                    quickMenuStack.movingRight = navTabBar.currentIndex > quickMenuStack.lastIndex;

                    quickMenuStack.lastIndex = navTabBar.currentIndex

                    if (navTabBar.currentIndex === 0) {
                        quickMenuStack.replaceCurrentItem(emulationView, {}, StackView.ReplaceTransition)
                    } else if (navTabBar.currentIndex === 1) {
                        quickMenuStack.replaceCurrentItem(achievementsView, {}, StackView.ReplaceTransition)
                    } else if (navTabBar.currentIndex === 2) {
                        quickMenuStack.replaceCurrentItem(gameSettingsView, {}, StackView.ReplaceTransition)
                    }

                    if (navTabBar.activeFocus) {
                        quickMenuStack.forceActiveFocus()
                    }
                }
            }

            initialItem: emulationView
            clip: true


            replaceEnter: Transition {
                NumberAnimation {
                    duration: 100
                    from: 0.0
                    property: "opacity"
                    to: 1.0
                }
                NumberAnimation {
                    duration: 200
                    easing.type: Easing.InOutQuad
                    from: 30 * (quickMenuStack.movingRight ? 1 : -1)
                    property: "x"
                    to: 0
                }
            }
            replaceExit: Transition {
                NumberAnimation {
                    duration: 100
                    from: 1.0
                    property: "opacity"
                    to: 0.0
                }
                NumberAnimation {
                    duration: 200
                    easing.type: Easing.InOutQuad
                    from: 0
                    property: "x"
                    to: 30 * (quickMenuStack.movingRight ? -1 : 1)
                }
            }


        }

    FirelightDialog {
        id: closeGameDialog
        property int numUnsynced: AchievementService.numCurrentSessionHardcoreUnlocks
        text: {
            if (numUnsynced > 0) {
                return "You have " + numUnsynced + " unsynced hardcore achievement unlock" + (numUnsynced > 1 ? "s" : "") + ". If you close the game, " + (numUnsynced > 1 ? "they" : "it") + " will be demoted to " + (numUnsynced > 1 ? "" : "a") + " non-hardcore unlock" + (numUnsynced > 1 ? "s" : "") + ".\n\nAre you sure you want to close the game?"
            }
            return "Are you sure you want to close the game?"
        }
        onAboutToShow: {
            numUnsynced = AchievementService.numCurrentSessionHardcoreUnlocks
        }
        onAccepted: {
            root.closeGame()
        }
    }

    FirelightDialog {
        id: resetGameDialog
        text: "Are you sure you want to reset the game?"
        onAccepted: {
            root.resetGame()
        }
    }

    Component {
        id: achievementsView
        FocusScope {
            RetroAchievementsGame {
                id: rcheevosGame
                contentHash: EmulationService.currentContentHash
                hardcore: AchievementService.inHardcoreSession
            }
            Text {
                visible: AchievementService.loggedIn && rcheevosGame.achievementSets.length === 0
                anchors.centerIn: parent
                text: "No achievements found for this game"
                color: Theme.textPrimary
                font.family: Constants.mainFontFamily
                font.pixelSize: AppStyle.fontSizeMedium
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            Pane {
                id: setListContainer
                visible: rcheevosGame.achievementSets.length >= 1
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                KeyNavigation.right: achievementListContainer
                width: (parent.width - 700) > 280 ? 280 : 108
                // width: Math.max(80, Math.min(300, parent.width - 1000))
                // Behavior on width {
                //     NumberAnimation {
                //         duration: 50
                //         easing.type: Easing.InOutQuad
                //     }
                // }
                background: Item {}
                contentItem: ListView {
                        id: actualList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: rcheevosGame.achievementSets
                        spacing: 6
                         highlightMoveDuration: 80
                         highlightMoveVelocity: -1
                         highlightRangeMode: InputMethodManager.usingMouse ? ListView.NoHighlightRange : ListView.ApplyRange
                         preferredHighlightBegin: 200
                         preferredHighlightEnd: height - 200
                         boundsBehavior: Flickable.StopAtBounds
                         keyNavigationEnabled: true
                         focus: true
                        delegate: Button {
                            id: control
                            width: setListContainer.width > 108 ? actualList.width : implicitWidth
                            TapHandler {
                                onTapped: {
                                    actualList.currentIndex = index
                                }
                            }
                            property bool showGlobalCursor: true
                            padding: 8
                          background: Rectangle {
                              color: "white"
                              opacity: control.hovered ? 0.20 : actualList.currentIndex === index ? 0.15 : 0
                              radius: 6
                          }
                          contentItem: RowLayout {
                              spacing: 12
                              Image {
                                  id: gameIcon
                                  source: model.iconUrl
                                  sourceSize.width: 64
                                  fillMode: Image.PreserveAspectFit
                                  Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                                  smooth: true
                              }

                              ColumnLayout {
                                  Layout.fillWidth: true
                                  Layout.fillHeight: true
                                  visible: setListContainer.width > 108

                                  RowLayout {
                                      Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                                      Layout.fillWidth: true
                                      spacing: 16

                                      Text {
                                          text: model.name
                                          color: Theme.textPrimary
                                          font.family: Constants.mainFontFamily
                                          font.pixelSize: AppStyle.fontSizeMedium
                                          horizontalAlignment: Text.AlignLeft
                                          verticalAlignment: Text.AlignVCenter
                                      }

                                      Item {
                                          Layout.fillWidth: true
                                          Layout.fillHeight: true
                                      }
                                  }



                                  RowLayout {
                                      Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                                      Layout.fillWidth: true

                                      Text {
                                          text: (AchievementService.inHardcoreSession ? model.numEarnedHardcore : model.numEarned) + "/" + model.numAchievements + " earned"
                                          color: Theme.textMuted
                                          font.family: Constants.mainFontFamily
                                          font.pixelSize: AppStyle.fontSizeSmall
                                          horizontalAlignment: Text.AlignLeft
                                          verticalAlignment: Text.AlignVCenter
                                      }
                                  }

                                  Item {
                                      Layout.fillWidth: true
                                      Layout.fillHeight: true
                                  }

                                  Rectangle {
                                      color: Theme.surfaceElevated
                                      implicitHeight: 14
                                      Layout.fillWidth: true
                                      radius: 4

                                      Rectangle {
                                          color: "#FFD700"
                                          anchors.left: parent.left
                                          anchors.top: parent.top
                                          anchors.bottom: parent.bottom
                                          width: parent.width * ((AchievementService.inHardcoreSession ? model.numEarnedHardcore : model.numEarned) / model.numAchievements)
                                          radius: 4
                                      }
                                  }
                              }
                          }
                      }
                        // delegate: Row {
                        //     TapHandler {
                        //         onTapped: {
                        //             actualList.currentIndex = index
                        //         }
                        //     }
                        //     Image {
                        //         source: model.iconUrl
                        //         sourceSize.width: 60
                        //         sourceSize.height: 60
                        //         fillMode: Image.PreserveAspectFit
                        //         smooth: true
                        //         anchors.verticalCenter: parent.verticalCenter
                        //     }
                        //     Text {
                        //         text: model.name
                        //         color: "white"
                        //         font.family: Constants.mainFontFamily
                        //         font.pixelSize: 16
                        //         horizontalAlignment: Text.AlignLeft
                        //         verticalAlignment: Text.AlignVCenter
                        //         anchors.verticalCenter: parent.verticalCenter
                        //         padding: 8
                        //     }
                        // }
                    }
            }
            Pane {
                id: achievementListContainer
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: setListContainer.right
                anchors.right: sortBar.left
                KeyNavigation.right: sortBar
                clip: true
                focus: true
                background: Item {}
                contentItem: ColumnLayout {
                    spacing: 16
                    // Text {
                    //     text: "Achievements"
                    //     color: ColorPalette.neutral200
                    //     font.family: Constants.mainFontFamily
                    //     font.pixelSize: 16
                    //     Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    // }
                    ListView {
                         id: achievementList
                         Layout.fillWidth: true
                         Layout.fillHeight: true
                         model: rcheevosGame.achievementSets[actualList.currentIndex].achievements
                         highlightMoveDuration: 80
                         highlightMoveVelocity: -1
                         highlightRangeMode: InputMethodManager.usingMouse ? ListView.NoHighlightRange : ListView.ApplyRange
                         preferredHighlightBegin: 200
                         preferredHighlightEnd: height - 200
                         boundsBehavior: Flickable.StopAtBounds
                         keyNavigationEnabled: true
                         contentY: 0
                         focus: true
                         cacheBuffer: 1000

                         ScrollBar.vertical: ScrollBar {
                             // parent: achievementList.parent
                             anchors.top: achievementList.top
                             anchors.left: achievementList.right
                             anchors.leftMargin: 4
                             anchors.bottom: achievementList.bottom
                          }

                         // header: Pane {
                         //     width: Math.min(ListView.view.width, 1000)
                         //     anchors.horizontalCenter: parent.horizontalCenter
                         //     background: Item {}
                         //     leftPadding: 0
                         //     rightPadding: 0
                         //     topPadding: 8
                         //     bottomPadding: 24
                         //     contentItem: ColumnLayout {
                         //         spacing: 16
                         //         // Pane {
                         //         //     Layout.fillWidth: true
                         //         //     background: Rectangle {
                         //         //         color: "transparent"
                         //         //         border.color: "#FFD700"
                         //         //         radius: 6
                         //         //     }
                         //         //     contentItem: Text {
                         //         //         text: "Hardcore mode is active"
                         //         //         color: "white"
                         //         //         font.family: Constants.mainFontFamily
                         //         //         font.pixelSize: 16
                         //         //         horizontalAlignment: Text.AlignHCenter
                         //         //         verticalAlignment: Text.AlignVCenter
                         //         //     }
                         //         // }
                         //         RowLayout {
                         //             Layout.fillWidth: true
                         //             spacing: 24
                         //             Image {
                         //                 id: gameIcon
                         //                 source: rcheevosGame.achievementSets[actualList.currentIndex].iconUrl
                         //                 sourceSize.width: 80
                         //                 fillMode: Image.PreserveAspectFit
                         //                 Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                         //                 smooth: true
                         //             }
                         //
                         //             ColumnLayout {
                         //                 Layout.fillWidth: true
                         //                 Layout.fillHeight: true
                         //
                         //                 RowLayout {
                         //                     Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                         //                     Layout.fillWidth: true
                         //                     spacing: 16
                         //
                         //                     Text {
                         //                         text: rcheevosGame.achievementSets[actualList.currentIndex].name
                         //                         color: ColorPalette.neutral100
                         //                         font.family: Constants.mainFontFamily
                         //                         font.pixelSize: 18
                         //                         horizontalAlignment: Text.AlignLeft
                         //                         verticalAlignment: Text.AlignVCenter
                         //                     }
                         //
                         //                     Pane {
                         //                         visible: AchievementService.inHardcoreSession
                         //                         verticalPadding: 4
                         //                         background: Rectangle {
                         //                             color: "transparent"
                         //                             border.color: "#FFD700"
                         //                             radius: 6
                         //                         }
                         //                         contentItem: Text {
                         //                             text: "Hardcore mode active"
                         //                             color: "white"
                         //                             font.family: Constants.mainFontFamily
                         //                             font.pixelSize: 16
                         //                             horizontalAlignment: Text.AlignHCenter
                         //                             verticalAlignment: Text.AlignVCenter
                         //                         }
                         //                     }
                         //
                         //                     Item {
                         //                         Layout.fillWidth: true
                         //                         Layout.fillHeight: true
                         //                     }
                         //                 }
                         //
                         //
                         //
                         //                 RowLayout {
                         //                     Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                         //                     Layout.fillWidth: true
                         //
                         //                     Text {
                         //                         text: rcheevosGame.achievementSets[actualList.currentIndex].platformName
                         //                         color: ColorPalette.neutral300
                         //                         font.family: Constants.mainFontFamily
                         //                         font.pixelSize: 17
                         //                         horizontalAlignment: Text.AlignLeft
                         //                         verticalAlignment: Text.AlignVCenter
                         //                     }
                         //
                         //                     Item {
                         //                         Layout.fillWidth: true
                         //                         Layout.fillHeight: true
                         //                     }
                         //
                         //                     Text {
                         //                         text: (AchievementService.inHardcoreSession ? rcheevosGame.achievementSets[actualList.currentIndex].numEarnedHardcore : rcheevosGame.achievementSets[actualList.currentIndex].numEarned) + "/" + rcheevosGame.achievementSets[actualList.currentIndex].numAchievements + " earned"
                         //                         color: ColorPalette.neutral300
                         //                         font.family: Constants.mainFontFamily
                         //                         font.pixelSize: 16
                         //                         horizontalAlignment: Text.AlignLeft
                         //                         verticalAlignment: Text.AlignVCenter
                         //                     }
                         //                 }
                         //                 Item {
                         //                     Layout.fillWidth: true
                         //                     Layout.fillHeight: true
                         //                 }
                         //
                         //                 Rectangle {
                         //                     color: "#1e1e1e"
                         //                     implicitHeight: 14
                         //                     Layout.fillWidth: true
                         //                     Layout.alignment: Qt.AlignBottom | Qt.AlignLeft
                         //                     Layout.bottomMargin: 4
                         //                     radius: 4
                         //
                         //                     Rectangle {
                         //                         color: "#FFD700"
                         //                         anchors.left: parent.left
                         //                         anchors.top: parent.top
                         //                         anchors.bottom: parent.bottom
                         //                         width: parent.width * ((AchievementService.inHardcoreSession ? rcheevosGame.achievementSets[actualList.currentIndex].numEarnedHardcore : rcheevosGame.achievementSets[actualList.currentIndex].numEarned) / rcheevosGame.achievementSets[actualList.currentIndex].numAchievements)
                         //                         radius: 4
                         //                     }
                         //                 }
                         //             }
                         //         }
                         //     }
                         // }

                         spacing: 12

                         delegate: AchievementListButton {
                             required property var model
                             required property var index

                             name: model.name
                             description: model.description
                             type: model.type
                             points: model.points
                             iconUrl: model.iconUrl
                             earned: model.earned
                             achievementId: model.achievementId
                         }

                         // delegate: Text {
                         //     required property var model
                         //     required property var index
                         //     text: model.name
                         //     color: "white"
                         //     font.family: Constants.mainFontFamily
                         //     font.pixelSize: 16
                         //     horizontalAlignment: Text.AlignLeft
                         //     verticalAlignment: Text.AlignVCenter
                         //     padding: 8
                         // }
                     }
                 }
            }

            Pane {
                id: sortBar
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                width: 64
                background: Item {}
                contentItem: ColumnLayout {
                    RadioIconButton {
                         Layout.preferredWidth: 42
                         Layout.preferredHeight: 42
                         iconSize: 24
                         Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                         focus: true
                         iconName: "sort"
                          // icon.height: 42
                          // icon.width: 42
                          // KeyNavigation.right: mainArea
                          model: [
                                { label: "Default order", value: "default" },
                                { label: "A-Z", value: "a-z" },
                                { label: "Z-A", value: "z-a" },
                                { label: "Points (most first)", value: "points_most" },
                                { label: "Points (least first)", value: "points_least" },
                                { label: "Earned date", value: "earned_date" },
                                { label: "Type", value: "type" }
                            ]

                            onSelectionChanged: function(index, value, text) {
                                rcheevosGame.achievementSets[actualList.currentIndex].achievements.sortMethod = value
                            }

                      }
                }
            }

            // Pane {
            //         id: setList
            //     anchors.left: parent.left
            //     anchors.top: parent.top
            //     anchors.bottom: parent.bottom
            //     width: 400
            //     background: Item {}
            //     contentItem: ListView {
            //         id: actualList
            //         model: rcheevosGame.achievementSets
            //         spacing: 16
            //         delegate: Pane {
            //             TapHandler {
            //                 onTapped: {
            //                     actualList.currentIndex = index
            //                 }
            //             }
            //             padding: 0
            //           background: Item {}
            //           contentItem: ColumnLayout {
            //               spacing: 16
            //               // Pane {
            //               //     Layout.fillWidth: true
            //               //     background: Rectangle {
            //               //         color: "transparent"
            //               //         border.color: "#FFD700"
            //               //         radius: 6
            //               //     }
            //               //     contentItem: Text {
            //               //         text: "Hardcore mode is active"
            //               //         color: "white"
            //               //         font.family: Constants.mainFontFamily
            //               //         font.pixelSize: 16
            //               //         horizontalAlignment: Text.AlignHCenter
            //               //         verticalAlignment: Text.AlignVCenter
            //               //     }
            //               // }
            //               RowLayout {
            //                   Layout.fillWidth: true
            //                   spacing: 12
            //                   Image {
            //                       id: gameIcon
            //                       source: model.iconUrl
            //                       sourceSize.width: 64
            //                       fillMode: Image.PreserveAspectFit
            //                       Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
            //                       smooth: true
            //                   }
            //
            //                   ColumnLayout {
            //                       Layout.fillWidth: true
            //                       Layout.fillHeight: true
            //
            //                       RowLayout {
            //                           Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
            //                           Layout.fillWidth: true
            //                           spacing: 16
            //
            //                           Text {
            //                               text: model.name
            //                               color: ColorPalette.neutral100
            //                               font.family: Constants.mainFontFamily
            //                               font.pixelSize: 16
            //                               horizontalAlignment: Text.AlignLeft
            //                               verticalAlignment: Text.AlignVCenter
            //                           }
            //
            //                           Item {
            //                               Layout.fillWidth: true
            //                               Layout.fillHeight: true
            //                           }
            //                       }
            //
            //
            //
            //                       RowLayout {
            //                           Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
            //                           Layout.fillWidth: true
            //
            //                           Text {
            //                               text: (AchievementService.inHardcoreSession ? model.numEarnedHardcore : model.numEarned) + "/" + model.numAchievements + " earned"
            //                               color: ColorPalette.neutral400
            //                               font.family: Constants.mainFontFamily
            //                               font.pixelSize: 14
            //                               horizontalAlignment: Text.AlignLeft
            //                               verticalAlignment: Text.AlignVCenter
            //                           }
            //                       }
            //
            //                       Item {
            //                           Layout.fillWidth: true
            //                           Layout.fillHeight: true
            //                       }
            //
            //                       Rectangle {
            //                           color: "#1e1e1e"
            //                           implicitHeight: 14
            //                           Layout.fillWidth: true
            //                           radius: 4
            //
            //                           Rectangle {
            //                               color: "#FFD700"
            //                               anchors.left: parent.left
            //                               anchors.top: parent.top
            //                               anchors.bottom: parent.bottom
            //                               width: parent.width * ((AchievementService.inHardcoreSession ? model.numEarnedHardcore : model.numEarned) / model.numAchievements)
            //                               radius: 4
            //                           }
            //                       }
            //                   }
            //               }
            //           }
            //       }
            //         // delegate: Row {
            //         //     TapHandler {
            //         //         onTapped: {
            //         //             actualList.currentIndex = index
            //         //         }
            //         //     }
            //         //     Image {
            //         //         source: model.iconUrl
            //         //         sourceSize.width: 60
            //         //         sourceSize.height: 60
            //         //         fillMode: Image.PreserveAspectFit
            //         //         smooth: true
            //         //         anchors.verticalCenter: parent.verticalCenter
            //         //     }
            //         //     Text {
            //         //         text: model.name
            //         //         color: "white"
            //         //         font.family: Constants.mainFontFamily
            //         //         font.pixelSize: 16
            //         //         horizontalAlignment: Text.AlignLeft
            //         //         verticalAlignment: Text.AlignVCenter
            //         //         anchors.verticalCenter: parent.verticalCenter
            //         //         padding: 8
            //         //     }
            //         // }
            //     }
            // }
            //
            //
            //
            // // Text {
            // //     visible: AchievementService.loggedIn && !rcheevosGame.achievementSets[0].hasAchievements
            // //     anchors.centerIn: parent
            // //     text: "No achievements found for this game."
            // //     color: ColorPalette.neutral100
            // //     font.family: Constants.mainFontFamily
            // //     font.pixelSize: 18
            // //     horizontalAlignment: Text.AlignHCenter
            // //     verticalAlignment: Text.AlignVCenter
            // // }
            // //
            // // Text {
            // //     visible: !AchievementService.loggedIn
            // //     anchors.centerIn: parent
            // //     text: "Not logged in to RetroAchievements. You can login from the Settings menu.\n\nAfter you log in, restart the game to load achievements."
            // //     color: ColorPalette.neutral100
            // //     font.family: Constants.mainFontFamily
            // //     font.pixelSize: 18
            // //     horizontalAlignment: Text.AlignHCenter
            // //     verticalAlignment: Text.AlignVCenter
            // // }
            // //
            // // Column {
            // //     visible: AchievementService.loggedIn && rcheevosGame.achievementSets[0].hasAchievements
            // //     anchors.right: mainArea.left
            // //     anchors.rightMargin: 16
            // //     anchors.topMargin: 24
            // //     anchors.top: parent.top
            // //
            // //     RadioIconButton {
            // //         icon.source: "qrc:/icons/sort"
            // //         icon.width: 60
            // //         icon.height: 60
            // //         KeyNavigation.right: mainArea
            // //         model: [
            // //               { label: "Default order", value: "default" },
            // //               { label: "A-Z", value: "a-z" },
            // //               { label: "Z-A", value: "z-a" },
            // //               { label: "Points (most first)", value: "points_most" },
            // //               { label: "Points (least first)", value: "points_least" },
            // //               { label: "Earned date", value: "earned_date" },
            // //               { label: "Type", value: "type" }
            // //           ]
            // //
            // //           onSelectionChanged: function(index, value, text) {
            // //               rcheevosGame.achievementSets[0].achievements.sortMethod = value
            // //           }
            // //
            // //     }
            // // }
            // //
            // //
            // Pane {
            //     id: mainArea
            //     visible: AchievementService.loggedIn && rcheevosGame.achievementSets[actualList.currentIndex].hasAchievements
            //     anchors.top: parent.top
            //     anchors.left: setList.right
            //     anchors.right: parent.right
            //     // anchors.bottom: parent.bottom
            //     // width: Math.min(parent.width, 1000)
            //     // anchors.horizontalCenter: parent.horizontalCenter
            //     anchors.bottom: parent.bottom
            //     clip: true
            //     focus: true
            //     leftPadding: 16
            //     rightPadding: 16
            //     topPadding: 16
            //     bottomPadding: 32
            //     background: Item{}
            //     contentItem: ListView {
            //         id: achievementList
            //         model: rcheevosGame.achievementSets[actualList.currentIndex].achievements
            //         highlightMoveDuration: 80
            //         highlightMoveVelocity: -1
            //         highlightRangeMode: InputMethodManager.usingMouse ? ListView.NoHighlightRange : ListView.ApplyRange
            //         preferredHighlightBegin: 200
            //         preferredHighlightEnd: height - 200
            //         boundsBehavior: Flickable.StopAtBounds
            //         contentY: 0
            //         focus: true
            //         cacheBuffer: 1000
            //
            //         ScrollBar.vertical: ScrollBar { }
            //
            //         // header: Pane {
            //         //     width: Math.min(ListView.view.width, 1000)
            //         //     anchors.horizontalCenter: parent.horizontalCenter
            //         //     background: Item {}
            //         //     leftPadding: 0
            //         //     rightPadding: 0
            //         //     topPadding: 8
            //         //     bottomPadding: 24
            //         //     contentItem: ColumnLayout {
            //         //         spacing: 16
            //         //         // Pane {
            //         //         //     Layout.fillWidth: true
            //         //         //     background: Rectangle {
            //         //         //         color: "transparent"
            //         //         //         border.color: "#FFD700"
            //         //         //         radius: 6
            //         //         //     }
            //         //         //     contentItem: Text {
            //         //         //         text: "Hardcore mode is active"
            //         //         //         color: "white"
            //         //         //         font.family: Constants.mainFontFamily
            //         //         //         font.pixelSize: 16
            //         //         //         horizontalAlignment: Text.AlignHCenter
            //         //         //         verticalAlignment: Text.AlignVCenter
            //         //         //     }
            //         //         // }
            //         //         RowLayout {
            //         //             Layout.fillWidth: true
            //         //             spacing: 24
            //         //             Image {
            //         //                 id: gameIcon
            //         //                 source: rcheevosGame.achievementSets[actualList.currentIndex].iconUrl
            //         //                 sourceSize.width: 80
            //         //                 fillMode: Image.PreserveAspectFit
            //         //                 Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
            //         //                 smooth: true
            //         //             }
            //         //
            //         //             ColumnLayout {
            //         //                 Layout.fillWidth: true
            //         //                 Layout.fillHeight: true
            //         //
            //         //                 RowLayout {
            //         //                     Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
            //         //                     Layout.fillWidth: true
            //         //                     spacing: 16
            //         //
            //         //                     Text {
            //         //                         text: rcheevosGame.achievementSets[actualList.currentIndex].name
            //         //                         color: ColorPalette.neutral100
            //         //                         font.family: Constants.mainFontFamily
            //         //                         font.pixelSize: 18
            //         //                         horizontalAlignment: Text.AlignLeft
            //         //                         verticalAlignment: Text.AlignVCenter
            //         //                     }
            //         //
            //         //                     Pane {
            //         //                         visible: AchievementService.inHardcoreSession
            //         //                         verticalPadding: 4
            //         //                         background: Rectangle {
            //         //                             color: "transparent"
            //         //                             border.color: "#FFD700"
            //         //                             radius: 6
            //         //                         }
            //         //                         contentItem: Text {
            //         //                             text: "Hardcore mode active"
            //         //                             color: "white"
            //         //                             font.family: Constants.mainFontFamily
            //         //                             font.pixelSize: 16
            //         //                             horizontalAlignment: Text.AlignHCenter
            //         //                             verticalAlignment: Text.AlignVCenter
            //         //                         }
            //         //                     }
            //         //
            //         //                     Item {
            //         //                         Layout.fillWidth: true
            //         //                         Layout.fillHeight: true
            //         //                     }
            //         //                 }
            //         //
            //         //
            //         //
            //         //                 RowLayout {
            //         //                     Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
            //         //                     Layout.fillWidth: true
            //         //
            //         //                     Text {
            //         //                         text: rcheevosGame.achievementSets[actualList.currentIndex].platformName
            //         //                         color: ColorPalette.neutral300
            //         //                         font.family: Constants.mainFontFamily
            //         //                         font.pixelSize: 17
            //         //                         horizontalAlignment: Text.AlignLeft
            //         //                         verticalAlignment: Text.AlignVCenter
            //         //                     }
            //         //
            //         //                     Item {
            //         //                         Layout.fillWidth: true
            //         //                         Layout.fillHeight: true
            //         //                     }
            //         //
            //         //                     Text {
            //         //                         text: (AchievementService.inHardcoreSession ? rcheevosGame.achievementSets[actualList.currentIndex].numEarnedHardcore : rcheevosGame.achievementSets[actualList.currentIndex].numEarned) + "/" + rcheevosGame.achievementSets[actualList.currentIndex].numAchievements + " earned"
            //         //                         color: ColorPalette.neutral300
            //         //                         font.family: Constants.mainFontFamily
            //         //                         font.pixelSize: 16
            //         //                         horizontalAlignment: Text.AlignLeft
            //         //                         verticalAlignment: Text.AlignVCenter
            //         //                     }
            //         //                 }
            //         //                 Item {
            //         //                     Layout.fillWidth: true
            //         //                     Layout.fillHeight: true
            //         //                 }
            //         //
            //         //                 Rectangle {
            //         //                     color: "#1e1e1e"
            //         //                     implicitHeight: 14
            //         //                     Layout.fillWidth: true
            //         //                     Layout.alignment: Qt.AlignBottom | Qt.AlignLeft
            //         //                     Layout.bottomMargin: 4
            //         //                     radius: 4
            //         //
            //         //                     Rectangle {
            //         //                         color: "#FFD700"
            //         //                         anchors.left: parent.left
            //         //                         anchors.top: parent.top
            //         //                         anchors.bottom: parent.bottom
            //         //                         width: parent.width * ((AchievementService.inHardcoreSession ? rcheevosGame.achievementSets[actualList.currentIndex].numEarnedHardcore : rcheevosGame.achievementSets[actualList.currentIndex].numEarned) / rcheevosGame.achievementSets[actualList.currentIndex].numAchievements)
            //         //                         radius: 4
            //         //                     }
            //         //                 }
            //         //             }
            //         //         }
            //         //     }
            //         // }
            //
            //         spacing: 12
            //
            //         delegate: AchievementListButton {
            //             required property var model
            //             required property var index
            //
            //             name: model.name
            //             description: model.description
            //             type: model.type
            //             points: model.points
            //             iconUrl: model.iconUrl
            //             earned: model.earned
            //             achievementId: model.achievementId
            //         }
            //
            //         // delegate: Text {
            //         //     required property var model
            //         //     required property var index
            //         //     text: model.name
            //         //     color: "white"
            //         //     font.family: Constants.mainFontFamily
            //         //     font.pixelSize: 16
            //         //     horizontalAlignment: Text.AlignLeft
            //         //     verticalAlignment: Text.AlignVCenter
            //         //     padding: 8
            //         // }
            //     }
            // }
        }
    }

    Component {
        id: emulationView
        FocusScope {
            id: emuMenuScope

            // Bumped whenever the running game's device selection changes, to
            // re-evaluate the invokable-backed properties below.
            property int deviceRefreshTick: 0
            // Ports that actually offer a device choice (e.g. Gamepad / Zapper);
            // empty for standard-controller-only games, so the Controls entry
            // stays hidden and nothing extra clutters the menu.
            property var deviceChoicePorts: {
                emuMenuScope.deviceRefreshTick; // re-evaluate on device changes
                var list = []
                var count = EmulationService.controllerPortCount()
                for (var p = 0; p < count; p++) {
                    if (EmulationService.controllerVariantsForPort(p).length > 1)
                        list.push(p)
                }
                return list
            }
            Connections {
                target: EmulationService
                function onControllerDevicesChanged() {
                    emuMenuScope.deviceRefreshTick++
                }
            }

            RowLayout {
                anchors.fill: parent
                spacing: 32
                 ColumnLayout {
                    id: menuColumn
                    KeyNavigation.right: quickMenuStack
                     Layout.maximumWidth: 400
                     Layout.alignment: Qt.AlignCenter
                     FirelightMenuItem {
                         id: resumeGameButton
                         labelText: "Resume Game"
                         focus: true
                         Layout.fillWidth: true
                         KeyNavigation.down: restartGameButton
                         // Layout.preferredWidth: parent.width / 2
                         Layout.preferredHeight: 40
                         Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                         checkable: false
                         alignRight: true
                         onClicked: function () {
                             root.resumeGame()
                         }
                     }
                     FirelightMenuItem {
                         id: restartGameButton
                         labelText: "Restart Game"
                         Layout.fillWidth: true
                         KeyNavigation.down: rewindButton
                         // Layout.preferredWidth: parent.width / 2
                         Layout.preferredHeight: 40
                         Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                         checkable: false
                         alignRight: true
                         onClicked: {
                             resetGameDialog.open()
                         }
                     }
                     FirelightMenuItem {
                         id: rewindButton
                         labelText: "Rewind"
                         KeyNavigation.down: suspendPointButton
                         Layout.fillWidth: true
                         // Layout.preferredWidth: parent.width / 2
                         Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                         Layout.preferredHeight: 40
                         checkable: false
                         enabled: EmulationService.rewindEnabled
                         alignRight: true
                         onClicked: {
                             root.rewindPressed()
                         }
                     }
                     // Rectangle {
                     //     Layout.fillWidth: true
                     //     // Layout.preferredWidth: parent.width / 2
                     //     Layout.preferredHeight: 1
                     //     Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                     //     opacity: 0.3
                     //     color: "#dadada"
                     // }
                     FirelightMenuItem {
                         id: suspendPointButton
                         labelText: "Suspend Points"
                         Layout.fillWidth: true
                         KeyNavigation.down: controlsButton.visible ? controlsButton : backToMenuButton
                         // Layout.preferredWidth: parent.width / 2
                         Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                         Layout.preferredHeight: 40
                         checkable: false
                         alignRight: true
                         // enabled: false
                         onClicked: {
                             quickMenuStack.replaceCurrentItem(suspendPointMenu, {}, StackView.Immediate)
                             quickMenuStack.forceActiveFocus()
                         }
                     }
                     // These route through the dispatcher rather than doing the
                     // work here, so the menu and the hotkey can't drift: same
                     // save slot, same hardcore gate, same toast.
                     FirelightMenuItem {
                         id: screenshotButton
                         labelText: "Screenshot"
                         Layout.fillWidth: true
                         KeyNavigation.down: muteButton
                         Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                         Layout.preferredHeight: 40
                         checkable: false
                         alignRight: true
                         onClicked: ShortcutDispatcher.trigger("screenshot")
                     }
                     SettingBinding {
                         id: muteBinding
                         key: "audio-muted"
                     }

                     FirelightMenuItem {
                         id: muteButton
                         // Follows the setting, so it stays right when the
                         // hotkey is what changed it.
                         labelText: muteBinding.value === "true" ? "Unmute" : "Mute"
                         Layout.fillWidth: true
                         KeyNavigation.down: controlsButton.visible ? controlsButton : backToMenuButton
                         Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                         Layout.preferredHeight: 40
                         checkable: false
                         alignRight: true
                         onClicked: ShortcutDispatcher.trigger("toggle_mute")
                     }
                     FirelightMenuItem {
                         id: controlsButton
                         labelText: "Controls"
                         Layout.fillWidth: true
                         KeyNavigation.down: backToMenuButton
                         Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                         Layout.preferredHeight: 40
                         checkable: false
                         alignRight: true
                         // Only when the game offers a real device choice
                         // (Gamepad / Mouse / Light Gun).
                         visible: emuMenuScope.deviceChoicePorts.length > 0
                         onClicked: {
                             quickMenuStack.replaceCurrentItem(controlsMenu, {}, StackView.Immediate)
                             quickMenuStack.forceActiveFocus()
                         }
                     }
                     FirelightMenuItem {
                         id: backToMenuButton
                         labelText: "Back to Menu"
                         Layout.fillWidth: true
                         KeyNavigation.down: closeGameButton
                         Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                         Layout.preferredHeight: 40
                         checkable: false
                         alignRight: true
                         onClicked: {
                             root.backToMenu()
                         }
                     }
                     FirelightMenuItem {
                         id: closeGameButton
                         labelText: "Close Game"
                         Layout.fillWidth: true
                         // Layout.preferredWidth: parent.width / 2
                         Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                         Layout.preferredHeight: 40
                         checkable: false
                         alignRight: true

                         onClicked: {
                             closeGameDialog.open()
                         }
                     }
                 }

                 StackView {
                     id: quickMenuStack
                     Layout.fillWidth: true
                     Layout.fillHeight: true

                     KeyNavigation.left: resumeGameButton

                     Keys.onBackPressed: {
                        if (quickMenuStack.depth >= 1) {
                            quickMenuStack.pop()
                            resumeGameButton.forceActiveFocus()
                        }
                     }
                 }
             }
        }

    }

    Component {
        id: gameSettingsView
        GameSettingsView {
            contentHash: EmulationService.currentContentHash
            platformId: EmulationService.currentPlatformId
            // Default to editing this game's own overrides; the tab lets the
            // user switch to the platform tier. (Values inherit at runtime.)
            level: SettingsLevel.Game
            platformName: EmulationService.currentPlatformName
        }
    }

    Component {
        id: controlsMenu
        FocusScope {
            id: controlsScope

            property int refreshTick: 0
            property var ports: {
                controlsScope.refreshTick; // re-evaluate on device changes
                var list = []
                var count = EmulationService.controllerPortCount()
                for (var p = 0; p < count; p++) {
                    if (EmulationService.controllerVariantsForPort(p).length > 1)
                        list.push(p)
                }
                return list
            }
            Connections {
                target: EmulationService
                function onControllerDevicesChanged() { controlsScope.refreshTick++ }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 14

                Text {
                    text: "Controllers"
                    color: "white"
                    font.pixelSize: AppStyle.fontSizeLarge
                    font.family: Constants.regularFontFamily
                    font.weight: Font.DemiBold
                }
                Text {
                    Layout.fillWidth: true
                    text: "Choose the device each player uses. Aim a Mouse or Light Gun with your mouse or the left analog stick; fire with the trigger (A by default)."
                    color: ColorPalette.neutral300
                    font.pixelSize: AppStyle.fontSizeSmall
                    font.family: Constants.regularFontFamily
                    wrapMode: Text.WordWrap
                }

                Repeater {
                    model: controlsScope.ports
                    delegate: ColumnLayout {
                        id: portRow
                        required property var modelData
                        property int portIndex: modelData
                        Layout.fillWidth: true
                        Layout.topMargin: 6
                        spacing: 6

                        Text {
                            text: "Player " + (portRow.portIndex + 1)
                            color: "white"
                            font.pixelSize: AppStyle.fontSizeMedium
                            font.family: Constants.regularFontFamily
                            font.weight: Font.DemiBold
                        }
                        Flow {
                            Layout.fillWidth: true
                            spacing: 8
                            Repeater {
                                model: {
                                    controlsScope.refreshTick;
                                    EmulationService.controllerVariantsForPort(portRow.portIndex)
                                }
                                delegate: Rectangle {
                                    id: optionButton
                                    required property var modelData
                                    property bool current: modelData.isCurrent
                                    implicitWidth: optionText.implicitWidth + 28
                                    implicitHeight: 36
                                    radius: 6
                                    color: optionButton.current ? ColorPalette.neutral100 : "transparent"
                                    border.color: optionButton.current ? ColorPalette.neutral100 : ColorPalette.neutral500
                                    border.width: 1
                                    Text {
                                        id: optionText
                                        anchors.centerIn: parent
                                        text: optionButton.modelData.name
                                              + (optionButton.modelData.deviceClass === 3 ? "  (Light Gun)"
                                                 : optionButton.modelData.deviceClass === 2 ? "  (Mouse)" : "")
                                        color: optionButton.current ? "black" : "white"
                                        font.pixelSize: AppStyle.fontSizeSmall
                                        font.family: Constants.regularFontFamily
                                        font.weight: Font.Medium
                                    }
                                    HoverHandler { cursorShape: Qt.PointingHandCursor }
                                    TapHandler {
                                        onTapped: EmulationService.setControllerVariant(
                                                      portRow.portIndex, optionButton.modelData.coreDeviceId)
                                    }
                                }
                            }
                        }
                    }
                }
                Item { Layout.fillHeight: true }
            }
        }
    }

    Component {
        id: rewindMenu
        RewindMenu {
            onRewindPointSelected: function(point) {
                emulatorLoader.item.loadRewindPoint(point)
                root.currentItem.close()
                // root.popCurrentItem(StackView.Immediate)
            }
        }
    }

    Component {
        id: suspendPointMenu
        Item {
            SuspendPoints {
                id: suspendData
                contentHash: emulatorLoader.item ? emulatorLoader.item.contentHash : ""
                saveSlotNumber: EmulationService.currentSaveSlotNumber
            }

            FirelightDialog {
                id: deleteSuspendPointDialog
                property int index: 0
                text: "Are you sure you want to delete the Suspend Point in slot " + (index + 1) + "?"
                onAccepted: {
                    suspendData.deleteSuspendPoint(index)
                    index = 0
                }
            }

            FirelightDialog {
                id: overwriteSuspendPointDialog
                property int index: 0
                text: "Are you sure you want to overwrite the data in slot " + (index + 1) + "?"
                onAccepted: {
                    emulatorLoader.item.writeSuspendPoint(index)
                    index = 0
                }
            }

            GridView {
                id: suspendPointGrid
                anchors.fill: parent
                anchors.leftMargin: (parent.width % cellWidth) / 2
                anchors.rightMargin: (parent.width % cellWidth) / 2
                model: suspendData.suspendPoints
                cellWidth: 300
                cellHeight: 360

                header: Pane {
                    width: GridView.view.width
                    verticalPadding: 0
                    horizontalPadding: 8
                    background: Item {}
                    contentItem: ColumnLayout {
                        spacing: 8
                        RowLayout {
                            spacing: 12
                            Layout.preferredHeight: 40
                            Layout.fillWidth: true
                            Text {
                                Layout.fillHeight: true
                                text: "Suspend Points"
                                color: "white"
                                font.family: Constants.regularFontFamily
                                font.pixelSize: AppStyle.fontSizeLarge
                                font.weight: Font.DemiBold
                                verticalAlignment: Text.AlignVCenter
                            }
                            Item {
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                            }
                            Button {
                                id: undoButton
                                Layout.fillHeight: true
                                horizontalPadding: 12
                                background: Rectangle {
                                    color: "white"
                                    opacity: parent.pressed ? 0.16 : 0.1
                                    radius: 4
                                    visible: createHoverHandler.hovered && parent.enabled
                                }
                                HoverHandler {
                                    id: createHoverHandler
                                    cursorShape: parent.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                }
                                contentItem: RowLayout {
                                    spacing: 8
                                    FLIcon {
                                        id: createIcon
                                        icon: "undo"
                                        Layout.fillHeight: true
                                        size: height
                                        color: undoButton.enabled ? "white" : "#737373"
                                    }
                                    Text {
                                        Layout.fillHeight: true
                                        Layout.fillWidth: true
                                         text: "Undo last load"
                                         color: undoButton.enabled ? "white" : "#737373"
                                         font.family: Constants.regularFontFamily
                                         font.pixelSize: AppStyle.fontSizeMedium
                                         font.weight: Font.Medium
                                         verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment: Text.AlignHCenter
                                     }
                                 }

                                 enabled: emulatorLoader.item ? emulatorLoader.item.canUndoLoadSuspendPoint : false

                                 onClicked: {
                                    emulatorLoader.item.undoLoadSuspendPoint()
                                 }
                            }

                        }
                        // RowLayout {
                        //     spacing: 12
                        //     Layout.preferredHeight: 40
                        //     Layout.fillWidth: true
                        //     Button {
                        //         Layout.fillHeight: true
                        //         horizontalPadding: 12
                        //         background: Rectangle {
                        //             color: "white"
                        //             opacity: parent.pressed ? 0.16 : 0.1
                        //             radius: 4
                        //             visible: createHoverHandler.hovered && parent.enabled
                        //         }
                        //         HoverHandler {
                        //             id: createHoverHandler
                        //             cursorShape: Qt.PointingHandCursor
                        //         }
                        //         contentItem: RowLayout {
                        //             spacing: 8
                        //             FLIcon {
                        //                 id: createIcon
                        //                 icon: "undo"
                        //                 Layout.fillHeight: true
                        //                 size: height
                        //                 color: "white"
                        //             }
                        //             Text {
                        //                 Layout.fillHeight: true
                        //                 Layout.fillWidth: true
                        //                  text: "Undo last load"
                        //                  color: "white"
                        //                  font.family: Constants.regularFontFamily
                        //                  font.pixelSize: 16
                        //                  font.weight: Font.Medium
                        //                  verticalAlignment: Text.AlignVCenter
                        //                     horizontalAlignment: Text.AlignHCenter
                        //              }
                        //          }
                        //
                        //          // onClicked: {
                        //          //    emulatorLoader.item.writeSuspendPoint(suspendPointGrid.count)
                        //          // }
                        //     }
                        //     Item {
                        //         Layout.fillHeight: true
                        //         Layout.fillWidth: true
                        //     }
                        //
                        // }
                    }
                }

                populate: FLGridViewPopulateTransition {}
                add: FLGridViewAddTransition {}

                delegate: Pane {
                    id: theThing
                    required property var model
                    required property var index
                    width: GridView.view.cellWidth
                    height: GridView.view.cellHeight
                    padding: 8
                    background: Item {}

                    property bool showGlobalCursor: true
                    property var globalCursorProxy: contentItem

                    contentItem: FocusScope {
                        FLSuspendPointCard {
                            anchors.fill: parent
                            visible: model.has_data
                            imageUrl: theThing.model.image_url
                            dateTimeString: theThing.model.timestamp
                            index: theThing.index

                            onDeleteClicked: function() {
                                deleteSuspendPointDialog.index = index
                                deleteSuspendPointDialog.open()
                            }

                            onOverwriteClicked: function() {
                                overwriteSuspendPointDialog.index = index
                                overwriteSuspendPointDialog.open()
                            }
                        }
                        Button {
                            anchors.fill: parent
                            visible: !model.has_data
                            HoverHandler {
                                id: theHoverHandler
                                cursorShape: Qt.PointingHandCursor
                            }
                            background: Rectangle {
                                color: "white"
                                opacity: parent.pressed ? 0.08 : theHoverHandler.hovered ? 0.24 : 0.16
                                border.color: "#737373"
                                radius: 4
                            }
                            contentItem: Text {
                                color: "white"
                                text: "Create in slot " + (theThing.index + 1)
                                font.pixelSize: AppStyle.fontSizeMedium
                                font.family: Constants.regularFontFamily
                                font.weight: Font.DemiBold
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: {
                                console.log("clicked create suspend point")
                                emulatorLoader.item.writeSuspendPoint(theThing.index)
                            }
                        }
                    }
                }
            }
        }
    }
}
}