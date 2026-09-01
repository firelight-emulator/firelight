import QtQuick

FLButtonBase {
    id: control

    objectName: "FLButton|" + text

    property string imageSource: ""
    property string iconName: ""
    property string trailingIconName: ""

    property var leadingIcon: null

    readonly property int _hpad: compact ? AppStyle.spacingMd : AppStyle.spacingLg

    leftPadding: iconName !== "" ? AppStyle.spacingSm : _hpad
    rightPadding: trailingIconName !== "" ? AppStyle.spacingSm : _hpad
    implicitWidth: contentItem.implicitWidth + leftPadding + rightPadding
    rounded: false

    contentItem: Item {
        implicitWidth: row.implicitWidth
        implicitHeight: row.implicitHeight

        Row {
            id: row
            anchors.centerIn: parent
            spacing: AppStyle.spacingSm

            Loader {
                id: leadingIconLoader
                sourceComponent: control.leadingIcon
                visible: control.leadingIcon !== null
                anchors.verticalCenter: parent.verticalCenter
            }

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
                font.family: AppStyle.fontFamily
                font.weight: Font.Normal
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
