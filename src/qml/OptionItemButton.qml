import QtQuick 2.15
import QtQuick.Layouts 2.15

MouseArea {
    id: optionItemMouseArea
    property int contentWidth
    property string displayText
    property string displayFontFamily: "Inter"
    property string platform: "Other"
    property string mainFontColor: themeData.theme === "Dark" ? "#d6d6d6" : "#37352e"
    property string highlightBackgroundColor: themeData.theme === "Dark" ? "#313131" : "#efefef"
    property string pressedBackgroundColor: themeData.theme === "Dark" ? "#2c2c2c" : "#dfdfde"
    property bool isContainingMouse: optionItemMouseArea.containsMouse
    property bool checked: optionSwitch.checked
    property var themeData: {{thme: "Light"}}
    property bool isEnabled: true
    property int pointSizeOffset: -4
    property bool isProFeature: false
    property bool isProVersion: false

    hoverEnabled: true
    width: innerRectangle.width
    height: innerRectangle.height

    signal switched
    signal unswitched
    signal clickedWhenDisabled

    onEntered: {
         if (optionItemMouseArea.enabled) {
             innerRectangle.color = optionItemMouseArea.highlightBackgroundColor;
             optionItemMouseArea.cursorShape = Qt.PointingHandCursor;
         }
    }

    onExited: {
        if (optionItemMouseArea.enabled) {
            innerRectangle.color = "transparent";
            optionItemMouseArea.cursorShape = Qt.ArrowCursor;
        }
    }

    onPressed: {
        if (optionItemMouseArea.enabled) {
            innerRectangle.color = optionItemMouseArea.pressedBackgroundColor;
        }
    }

    onReleased: {
        if (optionItemMouseArea.enabled) {
            if (optionItemMouseArea.containsMouse) {
                innerRectangle.color = optionItemMouseArea.highlightBackgroundColor;
            } else {
                innerRectangle.color = "transparent";
            }
        }
    }

    onClicked: {
        if (optionItemMouseArea.enabled && optionItemMouseArea.isEnabled) {
            // optionItemMouseArea.checked = !optionSwitch.checked;
            optionSwitch.checked = !optionSwitch.checked;
            if (optionSwitch.checked) {
                optionItemMouseArea.switched();
            } else {
                optionItemMouseArea.unswitched();
            }
        }

        if (!optionItemMouseArea.isEnabled) {
            optionItemMouseArea.clickedWhenDisabled();
        }
    }

    onCheckedChanged: {
        optionSwitch.checked = optionItemMouseArea.checked;
    }

    function setOptionSelected (isSelected) {
        optionItemMouseArea.checked = isSelected;
    }

    Rectangle {
        id: innerRectangle
        width: optionItemMouseArea.contentWidth
        height: 30
        radius: 5
        color: "transparent"

        RowLayout {
            id: rowItems
            anchors.verticalCenter: parent.verticalCenter
            x: 0
            property int leftRightMargins: 10
            width: optionItemMouseArea.contentWidth

            Item {
                height: 1
                width: rowItems.leftRightMargins
            }

            Text {
                id: optionText
                text: optionItemMouseArea.displayText
                color: optionItemMouseArea.mainFontColor
                font.pointSize: optionItemMouseArea.platform === "Apple" ? 14 : 14 + optionItemMouseArea.pointSizeOffset
                font.family: optionItemMouseArea.displayFontFamily
                opacity: optionItemMouseArea.enabled ? 1.0 : 0.2
                renderType: Text.NativeRendering
            }

            // Item {
            //     height: 1
            //     width: innerRectangle.width - optionText.width - optionSwitch.width - parent.x - rowItems.leftRightMargins
            // }

            Item {
                Layout.fillWidth: optionItemMouseArea.isProFeature && !optionItemMouseArea.isProVersion ? false : true
            }

            Rectangle {
                Layout.leftMargin: -6
                visible: optionItemMouseArea.isProFeature && !optionItemMouseArea.isProVersion
                width: 35
                height: 25
                radius: 5
                color: optionItemMouseArea.themeData.theme === "Dark" ? "#342a3a" : "#f0f0f0"

                Text {
                    anchors.centerIn: parent
                    text: "PRO"
                    color: optionItemMouseArea.themeData.theme === "Dark" ? "#a6a6a6" : "#787878"
                    font.pointSize: optionItemMouseArea.platform === "Apple" ? 13 : 13 + optionItemMouseArea.pointSizeOffset
                    font.family: optionItemMouseArea.displayFontFamily
                    renderType: Text.NativeRendering
                }
            }

            Item {
                Layout.fillWidth: optionItemMouseArea.isProFeature && !optionItemMouseArea.isProVersion ? false : true
            }

            SwitchButton {
                // x: optionItemMouseArea.isProFeature && !optionItemMouseArea.isProVersion ? -20 : 0
                Layout.leftMargin: -12
                id: optionSwitch
                checkable: true
                themeData: optionItemMouseArea.themeData
                checked: optionItemMouseArea.checked
                enabled: optionItemMouseArea.enabled && optionItemMouseArea.isEnabled

                onClicked: {
                    if (optionItemMouseArea.enabled && optionItemMouseArea.isEnabled) {
                        if (checked) {
                            optionItemMouseArea.switched();
                        } else {
                            optionItemMouseArea.unswitched();
                        }
                    }
                    if (!optionItemMouseArea.isEnabled) {
                        optionItemMouseArea.clickedWhenDisabled();
                    }
                }
            }
        }
    }
}
