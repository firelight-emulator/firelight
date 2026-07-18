import QtQuick
import Firelight 1.0

// TODO
// A brief message over everything else. Deliberately not interactive and never
// focusable: it reports what just happened, and a hotkey firing must not steal
// input from the game
Rectangle {
    id: root

    property int duration: 2000

    function show(message) {
        label.text = message;
        opacity = 1;
        // Restart rather than queue: the last thing you did is the thing worth
        // reading, and holding speed_up spams these
        hideTimer.restart();
    }

    anchors.horizontalCenter: parent.horizontalCenter
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 64

    width: label.implicitWidth + 32
    height: 40
    radius: 20
    color: Theme.surface
    border.width: 1
    border.color: Theme.border

    opacity: 0
    visible: opacity > 0

    Behavior on opacity {
        NumberAnimation { duration: 160; easing.type: Easing.InOutQuad }
    }

    Text {
        id: label
        anchors.centerIn: parent
        color: Theme.textPrimary
        font.pixelSize: AppStyle.fontSizeSmall
        font.family: Constants.regularFontFamily
        font.weight: Font.DemiBold
    }

    Timer {
        id: hideTimer
        interval: root.duration
        onTriggered: root.opacity = 0
    }
}
