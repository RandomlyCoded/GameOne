import QtQuick 2.15

Screen {
    signal screenFinished()

    color: "#00ff00"

    Text {
        id: hallo
        color: "#ff0000"
        anchors.centerIn: parent
        font { pixelSize: 50; bold: true }
        text: "Zum Starten klicken"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: screenFinished()
    }
}
