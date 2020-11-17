import QtQuick 2.15

Item {
    id: button

    property color borderColor: textColor
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

        Text {
            id: label

            anchors.fill: parent
            font.pixelSize: 20
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: button.activated()
    }
}
