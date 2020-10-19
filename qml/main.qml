import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    color: "black"
    minimumWidth: 800 //1750
    minimumHeight: 600 // 1000
    visible: true
    title: qsTr("Testspiel")
// Bei nurnutzung der GameScreens "welcome" durch "game" ersetzen
    property Item currentScreen: game// welcome

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

        onBattle: currentScreen = battle
    }


    BattleScreen {
        id: battle

        width: parent.width
        height: parent.height

        enabled: currentScreen === battle
    }
}
