import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Button {
    id: control

    property string label: ""
    property alias tooltipLabel: control.text
    property string iconCode: ""
    property bool rounded: true
    property real radius: 0
    property bool circle: false
    property bool tooltipOnTop: false
    property bool tooltipOnRight: false

    implicitHeight: AppStyle.buttonStandardHeight
    implicitWidth: control.circle ? AppStyle.buttonStandardHeight : AppStyle.buttonStandardWidth

    property bool showGlobalCursor: true

    property var inputHints: [
        {
            input: Qt.Key_Select,
            label: "Select"
        }
    ]

    checkable: false

    background: Rectangle {
        color: "white"
        opacity: !enabled ? 0 : parent.activeFocus && !InputMethodManager.usingMouse ? AppStyle.buttonBackgroundOpacityFocused : parent.pressed ? AppStyle.buttonBackgroundOpacityPressed : hover.hovered ? AppStyle.buttonBackgroundOpacityHovered : (!control.flat ? AppStyle.buttonBackgroundOpacityInactive : 0)
        Behavior on opacity {
            NumberAnimation {
                duration: 32
            }
        }
        radius: control.radius !== 0 ? control.radius : control.rounded ? width / 2 : 0
    }

    // background: Rectangle {
    //     color: control.flat ? "white" : !enabled ? AppStyle.buttonBackgroundColorDisabled : control.activeFocus && !InputMethodManager.usingMouse ? AppStyle.buttonBackgroundColorFocused : control.pressed ? AppStyle.buttonBackgroundColorPressed : control.hovered ? AppStyle.buttonBackgroundColorHovered : AppStyle.buttonBackgroundColorInactive
    //     opacity: !control.flat ? 1 : !enabled ? 0 : parent.activeFocus && !InputMethodManager.usingMouse ? 1 : parent.pressed ? 0.1 : parent.hovered ? 0.2 : 0
    //     Behavior on opacity {
    //         NumberAnimation {
    //             duration: 32
    //         }
    //     }
    //     radius: control.radius !== 0 ? control.radius : control.rounded ? width / 2 : 0
    // }

    HoverHandler {
        id: hover
        cursorShape: Qt.PointingHandCursor
    }

    contentItem: RowLayout {
        Text {
            visible: control.iconCode !== ""
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            text: control.iconCode
            font.family: Constants.symbolFontFamily

            font.pixelSize: AppStyle.buttonIconSize
            font.weight: AppStyle.buttonIconWeight
            color: !enabled ? AppStyle.buttonTextColorDisabled : control.activeFocus && !InputMethodManager.usingMouse ? AppStyle.buttonTextColorFocused : AppStyle.buttonTextColorInactive

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        Text {
            text: control.label
            visible: control.label !== ""
            Layout.fillHeight: true
            font.pixelSize: AppStyle.buttonTextFontSize
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            font.family: Constants.regularFontFamily
            font.weight: AppStyle.buttonTextFontWeight
            color: !enabled ? AppStyle.buttonTextColorDisabled : control.activeFocus && !InputMethodManager.usingMouse ? AppStyle.buttonTextColorFocused : AppStyle.buttonTextColorInactive

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    Text {
        text: control.text
        visible: control.enabled && ((hover.hovered && InputMethodManager.usingMouse) || (control.activeFocus && !InputMethodManager.usingMouse)) && control.text
        font.pixelSize: 15
        font.family: Constants.regularFontFamily
        font.weight: Font.Normal
        color: "white"
        anchors {
            top: control.tooltipOnTop || control.tooltipOnRight ? undefined : parent.bottom
            topMargin: control.tooltipOnTop || control.tooltipOnRight ? 0 : 4
            bottom: control.tooltipOnTop && !control.tooltipOnRight ? parent.top : undefined
            bottomMargin: control.tooltipOnTop && !control.tooltipOnRight ? 8 : 0
            horizontalCenter: !control.tooltipOnRight ? parent.horizontalCenter : undefined
            left: control.tooltipOnRight ? parent.right : undefined
            leftMargin: control.tooltipOnRight ? 12 : 0
            verticalCenter: control.tooltipOnRight ? parent.verticalCenter : undefined
        }
    }
}