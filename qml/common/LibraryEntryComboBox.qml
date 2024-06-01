import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ComboBox {
    id: root
    model: library_short_model
    textRole: "display_name"
    valueRole: "id"

    editable: true

    property bool firstTime: true

    required property int entryId
    property var currentEntry: entryId !== -1 ? library_database.getLibraryEntryJson(entryId) : {
        "display_name": "none",
        "platform_name": "none"
    }

    onCurrentValueChanged: function () {
        if (firstTime) {
            firstTime = false
            return
        }
        console.log("changed")
        entryId = currentValue
        currentEntry: library_database.getLibraryEntryJson(entryId)
    }

    Component.onCompleted: function () {
        if (entryId !== -1) {
            currentEntry = library_database.getLibraryEntryJson(entryId)
        }
    }

    delegate: ItemDelegate {
        id: delegate

        required property var model
        required property int index

        width: root.width
        contentItem: ColumnLayout {
            Text {
                text: model.display_name
                font.pointSize: 10
                font.weight: Font.Bold
                font.family: Constants.regularFontFamily
                color: "white"
                Layout.fillWidth: true
                elide: Text.ElideRight
                maximumLineCount: 1
                // Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            }
            Text {
                text: model.platform_name
                font.pointSize: 9
                font.weight: Font.Medium
                font.family: Constants.regularFontFamily
                color: "#C2BBBB"
                Layout.fillWidth: true
                // Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            }
        }

        background: Rectangle {
            color: delegate.highlighted ? "#1E1E1E" : "#000000"
        }

        highlighted: root.highlightedIndex === index
    }

    // indicator: Canvas {
    //     id: canvas
    //     x: control.width - width - control.rightPadding
    //     y: control.topPadding + (control.availableHeight - height) / 2
    //     width: 12
    //     height: 8
    //     contextType: "2d"
    //
    //     Connections {
    //         target: control
    //
    //         function onPressedChanged() {
    //             canvas.requestPaint();
    //         }
    //     }
    //
    //     onPaint: {
    //         context.reset();
    //         context.moveTo(0, 0);
    //         context.lineTo(width, 0);
    //         context.lineTo(width / 2, height);
    //         context.closePath();
    //         context.fillStyle = control.pressed ? "#17a81a" : "#21be2b";
    //         context.fill();
    //     }
    // }
    //
    // contentItem: Pane {
    //     padding: 12
    //     background: Item {
    //     }
    //     ColumnLayout {
    //         visible: root.entryId === -1
    //         Text {
    //             text: "Select a game..."
    //             font.pointSize: 10
    //             font.weight: Font.Bold
    //             font.family: Constants.regularFontFamily
    //             color: "white"
    //             Layout.fillWidth: true
    //             elide: Text.ElideRight
    //             maximumLineCount: 1
    //             Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    //         }
    //     }
    //     ColumnLayout {
    //         visible: root.entryId !== -1
    //         Text {
    //             text: root.currentEntry.display_name
    //             font.pointSize: 10
    //             font.weight: Font.Bold
    //             font.family: Constants.regularFontFamily
    //             color: "white"
    //             Layout.fillWidth: true
    //             elide: Text.ElideRight
    //             maximumLineCount: 1
    //             // Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
    //         }
    //         Text {
    //             text: root.currentEntry.platform_name
    //             font.pointSize: 9
    //             font.weight: Font.Medium
    //             font.family: Constants.regularFontFamily
    //             color: "#C2BBBB"
    //             Layout.fillWidth: true
    //             // Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
    //         }
    //     }
    // }


    // contentItem: Text {
    //     text: root.displayText
    //     font.pointSize: 10
    //     font.weight: Font.Bold
    //     font.family: Constants.regularFontFamily
    //     color: "white"
    //     Layout.fillWidth: true
    //     elide: Text.ElideRight
    //     maximumLineCount: 1
    //     // Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
    // }

    background: Rectangle {
        implicitWidth: 240
        implicitHeight: 40
        color: "black"
        radius: 8
    }

    popup: Popup {
        y: root.height - 1
        width: root.width
        implicitHeight: contentItem.implicitHeight
        padding: 1

        contentItem: ListView {
            id: theList
            clip: true
            implicitHeight: contentHeight
            model: root.popup.visible ? root.delegateModel : null
            currentIndex: root.highlightedIndex

            highlight: Item {
            }

            ScrollIndicator.vertical: ScrollIndicator {
            }
        }

        background: Rectangle {
            color: "black"
            radius: 2
        }
    }
}