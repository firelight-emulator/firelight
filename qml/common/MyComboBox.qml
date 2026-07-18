import QtQuick
import QtQuick.Controls
import Firelight 1.0

// TODO
// Legacy name kept for existing call sites. It's now just FLComboBox with a
// stable, widest-text width so the control doesn't resize as the selection
// changes. New code should use FLComboBox directly
FLComboBox {
    implicitContentWidthPolicy: ComboBox.WidestText
}
