import QtQuick 2.15

Item {
    id: debug

    property alias color: label.color
    property var value

    width: label.width
    height: label.height + 12.5

    Rectangle {
        anchors.fill: parent
        border { color: debug.color; width: 1 }
        color: Qt.rgba(debug.color.r/2, debug.color.g/2, debug.color.b/2, 0.5)

        Text {
            id: label

            anchors.centerIn: parent
            color: "lime"
            text: typeof(debug.value) === "string" ? debug.value : JSON.stringify(debug.value)
        }
    }
}
