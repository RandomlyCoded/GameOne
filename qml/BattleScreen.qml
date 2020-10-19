import QtQuick 2.15

Screen {
    Rectangle {
        color: "white"

        height: parent.height
        width: parent.width - height
    }

    Text {
        id: test

        anchors.centerIn: parent

        color: "black"

        font { pixelSize: 50; bold: true; italic: true }

        text: "text"
    }
}
