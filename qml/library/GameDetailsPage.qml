import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root

    required property int entryId
    property int tabWidth: 150
    // required property string title
    // required property string contentId
    // required property int parentId
    //
    property var entryData: {}

    Component.onCompleted: function () {
        entryData = library_database.getLibraryEntryJson(entryId)
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 12

        Text {
            Layout.fillWidth: true
            padding: 10
            text: root.entryData.display_name
            color: "white"
            font.pointSize: 22
            font.family: Constants.regularFontFamily
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        LibraryEntryComboBox {
            id: entryComboBox
            visible: root.entryData.is_patch
            entryId: root.entryData.parent_entry_id
        }
    }

    Button {
        Layout.alignment: Qt.AlignCenter
        text: "Play"
        onClicked: {
            console.log("Play button clicked")
        }
    }

    TabBar {
        id: bar
        Layout.topMargin: 16
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredHeight: 40
        currentIndex: -1

        onCurrentIndexChanged: function () {
            view.setCurrentIndex(currentIndex)
        }

        background: Rectangle {
            width: root.tabWidth
            visible: bar.contentChildren[bar.currentIndex].enabled
            height: 2
            radius: 1
            color: "white"
            x: root.tabWidth * bar.currentIndex
            y: bar.height

            Behavior on x {
                NumberAnimation {
                    duration: 120
                    easing.type: Easing.InOutQuad
                }
            }
        }

        TabButton {
            width: root.tabWidth
            contentItem: Text {
                text: "Details"
                color: parent.enabled ? "#ffffff" : "#666666"
                font.family: Constants.regularFontFamily
                font.pointSize: 11
                font.weight: parent.enabled && parent.checked ? Font.Bold : Font.Normal
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            enabled: false

            background: Item {
            }

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
        }
        TabButton {
            width: root.tabWidth
            contentItem: Text {
                text: "Achievements"
                color: parent.enabled ? "#ffffff" : "#666666"
                font.family: Constants.regularFontFamily
                font.pointSize: 11
                font.weight: parent.enabled && parent.checked ? Font.Bold : Font.Normal
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            enabled: false

            background: Item {
            }

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
        }
        TabButton {
            width: root.tabWidth
            contentItem: Text {
                text: "Activity"
                color: parent.enabled ? "#ffffff" : "#666666"
                font.family: Constants.regularFontFamily
                font.pointSize: 11
                font.weight: parent.enabled && parent.checked ? Font.Bold : Font.Normal
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            enabled: false

            background: Item {
            }

            HoverHandler {
                cursorShape: parent.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
            }
        }
    }

    // Item {
    //     Layout.fillWidth: true
    //     Layout.fillHeight: true
    //
    //     Text {
    //         text: "There will be cool stuff here soon :)"
    //         color: "white"
    //         font.family: Constants.regularFontFamily
    //         font.pointSize: 10
    //         anchors.centerIn: parent
    //         horizontalAlignment: Text.AlignHCenter
    //         verticalAlignment: Text.AlignVCenter
    //     }
    // }

    SwipeView {
        id: view
        Layout.fillWidth: true
        Layout.fillHeight: true
        currentIndex: 0

        clip: true

        onCurrentIndexChanged: function () {
            bar.setCurrentIndex(currentIndex)
        }

        // Image {
        //     source: "https://t4.ftcdn.net/jpg/00/89/02/67/360_F_89026793_eyw5a7WCQE0y1RHsizu41uhj7YStgvAA.jpg"
        //     fillMode: Image.PreserveAspectFit
        // }
        //
        // Image {
        //     source: "https://t4.ftcdn.net/jpg/00/89/02/67/360_F_89026793_eyw5a7WCQE0y1RHsizu41uhj7YStgvAA.jpg"
        //     fillMode: Image.PreserveAspectFit
        // }
        //
        // Image {
        //     source: "https://t4.ftcdn.net/jpg/00/89/02/67/360_F_89026793_eyw5a7WCQE0y1RHsizu41uhj7YStgvAA.jpg"
        //     fillMode: Image.PreserveAspectFit
        // }

        Text {
            text: "There will be cool stuff here soon :)"
            color: "white"
            font.family: Constants.regularFontFamily
            font.pointSize: 10
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        Text {
            text: "and here"
            color: "white"
            font.family: Constants.regularFontFamily
            font.pointSize: 10
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        Text {
            text: "and here too!"
            color: "white"
            font.family: Constants.regularFontFamily
            font.pointSize: 10
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}