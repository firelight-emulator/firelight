import QtQuick
import QtQuick.Controls
// import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

FirelightDialog {
    id: control
    headerText: "Manage save data"

    // width: Math.min(parent.width, 800)

    contentItem: ListView {
        interactive: false
        currentIndex: 0
        model: ListModel {
            id: saveDataModel
            ListElement { name: "Slot 1"; size: "1 MB" }
            ListElement { name: "Slot 2"; size: "2 MB" }
            ListElement { name: "Slot 3"; size: "3 MB" }
            ListElement { name: "Slot 4"; size: "3 MB" }
            ListElement { name: "Slot 5"; size: "3 MB" }
            ListElement { name: "Slot 6"; size: "3 MB" }
            ListElement { name: "Slot 7"; size: "3 MB" }
            ListElement { name: "Slot 8"; size: "3 MB" }
        }
        delegate: Button {
            height: 80
            width: ListView.view.width
            background: Item{}
            contentItem: RowLayout {
                spacing: 16
                // Text {
                //     text: model.name
                //     font.pixelSize: 16
                //     color: ColorPalette.neutral400
                //     Layout.alignment: Qt.AlignVCenter
                // }
                Rectangle {
                    implicitWidth: height * 16 / 9
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignVCenter
                    color: "red"
                }

                ColumnLayout {

                }

                Text {
                    text: model.size
                    font.pixelSize: 14
                    color: ColorPalette.neutral300
                }
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }
    }
}