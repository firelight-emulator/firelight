import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

Pane {
    id: root
    required property GameActivity activity

    verticalPadding: 16
    horizontalPadding: 16
    background: Item {}

    contentItem: Flickable {
        contentHeight: overviewBanner.height + 16 + navigationTabs.height + 16 + content.height

        onContentHeightChanged: {
            console.log("Content height changed: " + contentHeight)
        }

        Rectangle {
            id: overviewBanner
            color: "#1E1E1E"
            radius: 8
            border.color: "#2A2A2A"
            border.width: 1
            opacity: 0.8
            width: Math.min(1000, parent.width)
            anchors.horizontalCenter: parent.horizontalCenter
            height: 260
        }

        NavigationTabBar {
            id: navigationTabs
            anchors.top: overviewBanner.bottom
            anchors.topMargin: 16
            anchors.horizontalCenter: parent.horizontalCenter

            tabWidth: 120
            tabs: ["Overview", "Achievements", "Activity", "Settings"]
        }

        ColumnLayout {
            id: content
            anchors.top: navigationTabs.bottom
            anchors.topMargin: 16
            width: Math.min(700, parent.width)
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 16

            Text {
                text: "Activity summary"
                font.pixelSize: 18
                font.family: Constants.regularFontFamily
                color: "white"
                font.weight: Font.DemiBold
                Layout.preferredHeight: 40
                verticalAlignment: Text.AlignVCenter
            }

            Pane {
                id: summary
                Layout.fillWidth: true
                Layout.preferredHeight: 160
                padding: 16
                background: Rectangle {
                    color: "#1E1E1E"
                    radius: 8
                    opacity: 0.8
                    border.color: "#2A2A2A"
                    border.width: 1
                }
                contentItem: RowLayout {
                     spacing: 16

                     Item {
                         Layout.fillHeight: true
                        Layout.fillWidth: true

                        Text {
                             id: timePlayed
                             text: "Total time played"
                             anchors.top: parent.top
                             anchors.bottom: timePlayedValue.top
                             anchors.bottomMargin: 8
                             font.pixelSize: 16
                             font.family: Constants.regularFontFamily
                             font.weight: Font.Medium
                             color: "white"
                             verticalAlignment: Text.AlignBottom
                         }

                         Text {
                             id: timePlayedValue
                             text: {
                                if (activity.totalSecondsPlayed === 0) {
                                    return "<b>0</b>s"
                                }

                                let hours = Math.floor(activity.totalSecondsPlayed / 3600)
                               let minutes = Math.floor((activity.totalSecondsPlayed % 3600) / 60)
                               let seconds = activity.totalSecondsPlayed % 60

                               if (hours > 0) {
                                   return ("<b>" + hours + "</b>h ") + ("<b>" + minutes + "</b>m ") + ("<b>" + seconds + "</b>s ")
                               } else if (minutes > 0) {
                                   return ("<b>" + minutes + "</b>m ") + ("<b>" + seconds + "</b>s ")
                               } else if (seconds > 0) {
                                   return ("<b>" + seconds + "</b>s ")
                               }

                               // return (hours > 0 ? ("<em>" + hours + "</em>h ") : "") + (minutes > 0 ? ("<em>" + minutes + "</em>m ") : "") + "<em>" + seconds + "</em>s "
                                // model.numUnpausedSeconds
                            }
                             anchors.bottom: parent.bottom
                             font.pixelSize: 30
                             font.family: Constants.regularFontFamily
                             elide: Text.ElideRight
                             width: parent.width
                             font.weight: Font.Bold
                             color: "white"
                             verticalAlignment: Text.AlignVCenter
                         }
                     }

                     Item {
                          Layout.fillHeight: true
                         Layout.fillWidth: true

                         Text {
                              id: timesPlayed
                              text: "Play count"
                              anchors.top: parent.top
                              anchors.bottom: timesPlayedValue.top
                              anchors.bottomMargin: 8
                              font.pixelSize: 16
                              font.family: Constants.regularFontFamily
                              font.weight: Font.Medium
                              color: "white"
                              verticalAlignment: Text.AlignBottom
                          }

                          Text {
                              id: timesPlayedValue
                              text: activity.timesPlayed
                              anchors.bottom: parent.bottom
                             elide: Text.ElideRight
                             width: parent.width
                              font.pixelSize: 30
                              font.family: Constants.regularFontFamily
                              font.weight: Font.Bold
                              color: "white"
                              verticalAlignment: Text.AlignVCenter
                          }
                      }



                     Item {
                         Layout.fillHeight: true
                        Layout.fillWidth: true

                        Text {
                             id: playTime
                             text: "Average time played"
                             anchors.top: parent.top
                             anchors.bottom: playTimeValue.top
                             anchors.bottomMargin: 8
                             font.pixelSize: 16
                             font.family: Constants.regularFontFamily
                             font.weight: Font.Medium
                             color: "white"
                             verticalAlignment: Text.AlignBottom
                         }

                         Text {
                             id: playTimeValue
                             text: {
                                 if (activity.timesPlayed === 0) {
                                     return "<b>0</b>s"
                                 }
                                 let val = activity.totalSecondsPlayed / activity.timesPlayed
                                if (val === 0) {
                                    return "<b>0</b>s"
                                }

                                let hours = Math.floor(val / 3600)
                               let minutes = Math.floor((val % 3600) / 60)
                               let seconds = Math.round(val % 60)

                               if (hours > 0) {
                                   return ("<b>" + hours + "</b>h ") + ("<b>" + minutes + "</b>m ") + ("<b>" + seconds + "</b>s ")
                               } else if (minutes > 0) {
                                   return ("<b>" + minutes + "</b>m ") + ("<b>" + seconds + "</b>s ")
                               } else if (seconds > 0) {
                                   return ("<b>" + seconds + "</b>s ")
                               }

                               // return (hours > 0 ? ("<em>" + hours + "</em>h ") : "") + (minutes > 0 ? ("<em>" + minutes + "</em>m ") : "") + "<em>" + seconds + "</em>s "
                                // model.numUnpausedSeconds
                            }
                             anchors.bottom: parent.bottom
                             font.pixelSize: 30
                             elide: Text.ElideRight
                             width: parent.width
                             font.family: Constants.regularFontFamily
                             font.weight: Font.Bold
                             color: "white"
                             verticalAlignment: Text.AlignVCenter
                         }
                     }
                 }
            }

            // Rectangle {
            //     id: summary
            //     color: "#1E1E1E"
            //     radius: 8
            //     Layout.fillWidth: true
            //     Layout.preferredHeight: 160
            //     border.color: "#2A2A2A"
            //     border.width: 1
            //
            //     RowLayout {
            //         anchors.fill: parent
            //         anchors.margins: 16
            //         spacing: 16
            //
            //         Text {
            //             text: activity.numUnpausedSeconds + " seconds"
            //             font.pixelSize: 16
            //             font.family: Constants.regularFontFamily
            //             color: "white"
            //             verticalAlignment: Text.AlignVCenter
            //         }
            //
            //         Text {
            //             text: activity.numSessions + " sessions"
            //             font.pixelSize: 16
            //             font.family: Constants.regularFontFamily
            //             color: "white"
            //             verticalAlignment: Text.AlignVCenter
            //         }
            //     }
            // }

            Text {
                id: title
                text: "Recent activity"
                font.pixelSize: 18
                font.family: Constants.regularFontFamily
                color: "white"
                font.weight: Font.DemiBold
                Layout.preferredHeight: 40
                verticalAlignment: Text.AlignVCenter
            }

            ListView {
                Layout.fillWidth: true
                Layout.preferredHeight: contentHeight
                interactive: false
                model: activity.playSessions
                delegate: Pane {
                    id: item
                    required property var model
                    height: 60
                    width: ListView.view.width
                    verticalPadding: 8
                    horizontalPadding: 16

                    background: Item {}

                    contentItem: RowLayout {
                        spacing: 16
                        Text {
                             text: {
                                 if (item.model.numUnpausedSeconds === 0) {
                                     return "<b>0</b>s"
                                 }

                                 let hours = Math.floor(item.model.numUnpausedSeconds / 3600)
                                let minutes = Math.floor((item.model.numUnpausedSeconds % 3600) / 60)
                                let seconds = item.model.numUnpausedSeconds % 60

                                if (hours > 0) {
                                    return ("<b>" + hours + "</b>h ") + ("<b>" + minutes + "</b>m ") + ("<b>" + seconds + "</b>s ")
                                } else if (minutes > 0) {
                                    return ("<b>" + minutes + "</b>m ") + ("<b>" + seconds + "</b>s ")
                                } else if (seconds > 0) {
                                    return ("<b>" + seconds + "</b>s ")
                                }

                                // return (hours > 0 ? ("<em>" + hours + "</em>h ") : "") + (minutes > 0 ? ("<em>" + minutes + "</em>m ") : "") + "<em>" + seconds + "</em>s "
                                 // model.numUnpausedSeconds
                             }
                            padding: 0
                            font.pixelSize: 20
                            font.weight: Font.Medium
                            font.family: Constants.regularFontFamily
                            verticalAlignment: Text.AlignVCenter
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                            color: "white"
                        }

                        FLDateTime {

                            Layout.fillHeight: true
                            Layout.preferredWidth: 200

                            epochSeconds: item.model.startTime

                        }

                        FLDateTime {
                            Layout.fillHeight: true
                            Layout.preferredWidth: 200

                            epochSeconds: item.model.endTime

                        }
                    }
                }

            }
        }
    }
}
