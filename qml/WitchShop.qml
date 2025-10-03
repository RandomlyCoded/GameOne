import GameOne 1.0
import QtQuick 2.15

Rectangle {
    visible: false
    id: witchShop
    width: mainWindow.width * 4/3
    height: mainWindow.height * 4/3

    Image {
        id: background
        source: "image://assets/WitchBackground.svg"
    }
}
