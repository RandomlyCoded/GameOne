import "." as GameOne
import QtQuick 2.15

GameOne.Screen {
    signal screenFinished()

    color: "#00ff00"

    Text {
        id: hello
        color: "#ff0000"
        anchors.centerIn: parent
        font { pixelSize: 50; bold: true }
        text: "Press to continue"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: screenFinished()
    }
}
