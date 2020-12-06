import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    color: "black"
    width: 1750 //1750 || 800
    height: 1000 // 1000 || 600
    visible: true
    title: "GameOne"
// Bei nurnutzung der GameScreens "welcome"("start") durch "game" ersetzen
    property Item currentScreen: game // welcome

    WelcomeScreen {
        id: welcome

        width: parent.width
        height: parent.height

        enabled: currentScreen === welcome

        onScreenFinished: currentScreen = intro
    }

    IntroScreen {
        id: intro

        width: parent.width
        height: parent.height

        enabled: currentScreen === intro

        onScreenFinished: currentScreen = game
    }

    GameScreen {
        id: game

        width: parent.width
        height: parent.height

        enabled: currentScreen === game
    }
}
