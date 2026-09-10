import QtQuick
import QtQuick.Controls

Rectangle {
    id: separator
    implicitWidth: 1
    gradient: Gradient {
        GradientStop { position: 0.0; color: "transparent" }
        GradientStop { position: 0.05; color: Theme.border }
        GradientStop { position: 0.95; color: Theme.border }
        GradientStop { position: 1.0; color: "transparent" }
    }
}