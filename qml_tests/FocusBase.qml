import QtQuick
import Firelight 1.0

// Stands in for FLButtonBase: a base component that declares focus metadata its
// derived components must inherit
Item {
    id: root

    FLFocus.showCursor: true
    FLFocus.spacing: -4
}
