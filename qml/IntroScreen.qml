import QtQuick 2.15

Screen {
    signal screenFinished()

    color: "skyblue"

    Column {
        Debug { value: {" " } color: "skyblue" }
        Debug { value: {"Mit den <b>Pfeiltasten</b> bewegt man sich. Die <b> weißen Rechtecke</b>" } color: "orange" }
        Debug { value: {"stellen Moster dar, die man besiegen muss. Der <b>Kreis</b> ist der" } color: "orange" }
        Debug { value: {"Bossgegner. Wenn man kämpft, kann man mit der <b> A-Taste</b> angreifen." } color: "orange" }
        //Mosters sehen am Ende warscheinlich anders aus...
    }

    Text {
        id: aufforderung_fortfahren

        anchors.centerIn: parent

        text: "Zum Fortfahren erneut klicken"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: screenFinished()
    }
}
