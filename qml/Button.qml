import QtQuick 2.15

Item {
    id: button

    property bool checked: false
    property color borderColor: textColor
    property alias elide: label.elide
    property alias color: background.color
    property alias text: label.text
    property alias textColor: label.color

    signal activated()

    implicitWidth: label.implicitWidth + 10
    implicitHeight: label.implicitHeight + 6

    Rectangle {
        id: background

        anchors.fill: parent
        border { width: 1; color: button.borderColor }
        color: button.checked ? "#fff" : "#ccc"
        opacity: button.checked ? 0.8 : 0.5
    }

    Text {
        id: label

        anchors.centerIn: background
        font.pixelSize: 20

        width: Math.min(button.width - 10, implicitWidth)
    }

    MouseArea {
        anchors.fill: parent
        onClicked: button.activated()
    }
}
