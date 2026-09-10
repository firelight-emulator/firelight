// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Layouts
import Firelight 1.0

// TODO
// The mirror of FLColumnLayout. It describes itself and moves nothing
RowLayout {
    id: root

    // TODO
    // Answers for its children when a press asks what it can reach
    FLFocus.container: true
}
