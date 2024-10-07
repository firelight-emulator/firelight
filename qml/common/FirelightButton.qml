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

    implicitHeight: AppStyle.buttonStandardHeight
    implicitWidth: control.circle ? AppStyle.buttonStandardHeight : AppStyle.buttonStandardWidth

    property bool showGlobalCursor: true

    hoverEnabled: true
    checkable: false

    background: Rectangle {
        color: "white"
        opacity: !enabled ? 0 : parent.activeFocus && !InputMethodManager.usingMouse ? AppStyle.buttonBackgroundOpacityFocused : parent.pressed ? AppStyle.buttonBackgroundOpacityPressed : parent.hovered ? AppStyle.buttonBackgroundOpacityHovered : (!control.flat ? AppStyle.buttonBackgroundOpacityInactive : 0)
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
        visible: control.enabled && ((control.hovered && InputMethodManager.usingMouse) || (control.activeFocus && !InputMethodManager.usingMouse)) && control.text
        font.pixelSize: 15
        font.family: Constants.regularFontFamily
        font.weight: Font.Light
        color: "white"
        anchors {
            top: parent.bottom
            topMargin: 4
            horizontalCenter: parent.horizontalCenter
        }
    }
}