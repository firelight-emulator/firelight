import QtQuick
import QtQuick.Controls
import Firelight 1.0

FLTwoColumnPage {
    id: root
    objectName: "ConfigureControllerPage"

    headerText: qsTr("Configure Controller")
    largeMenuRows: true

    model: [
        {
            "type": "page",
            "key": "suspend-points",
            "label": qsTr("General Settings"),
            "page": temp
        },
        {
            "type": "page",
            "key": "suspend-points2",
            "label": qsTr("Shortcuts"),
            "page": temp
        },
        {
            "type": "page",
            "key": "suspend-points3",
            "label": qsTr("Analog Stick Settings"),
            "page": temp
        },
        {
            "type": "page",
            "key": "suspend-points5",
            "label": qsTr("Button Mappings"),
            "page": temp
        },
        {
            "type": "page",
            "key": "suspend-points4",
            "label": qsTr("Test Controller"),
            "page": temp
        },
    ]

    Component {
        id: temp

        Text {
            text: "placeholder"
            color: "white"
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeMedium
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}