import QtQuick

FLButtonBase {
    id: control

    objectName: "FLButton|" + text

    property string iconName: ""
    property string trailingIconName: ""

    readonly property int _hpad: compact ? AppStyle.spacingSm : AppStyle.spacingMd

    leftPadding: iconName !== "" ? AppStyle.spacingSm : _hpad
    rightPadding: trailingIconName !== "" ? AppStyle.spacingSm : _hpad
    implicitWidth: contentItem.implicitWidth + leftPadding + rightPadding

    contentItem: Item {
        implicitWidth: row.implicitWidth
        implicitHeight: row.implicitHeight

        Row {
            id: row
            anchors.centerIn: parent
            spacing: AppStyle.spacingSm

            Icon {
                visible: control.iconName !== ""
                anchors.verticalCenter: parent.verticalCenter
                name: control.iconName
                size: AppStyle.iconSizeButton
                color: control._fg
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: control.text
                color: control._fg
                font.pixelSize: AppStyle.fontSizeMedium
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
            }
            Icon {
                visible: control.trailingIconName !== ""
                anchors.verticalCenter: parent.verticalCenter
                name: control.trailingIconName
                size: AppStyle.iconSizeButton
                color: control._fg
            }
        }
    }
}
