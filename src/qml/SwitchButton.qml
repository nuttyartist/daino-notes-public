import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal 2.15

Switch {
    id: buttonContainer
    property var themeData: {{thme: "Light"}}
    signal switched()
    Universal.theme: themeData.theme === "Dark" ? Universal.Dark : Universal.Light

    onClicked: {
        buttonContainer.switched();
    }
}
