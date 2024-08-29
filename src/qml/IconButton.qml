import QtQuick 2.12

Item {
    id: root
    property string icon
    property string platform: "Other"
    property int iconPointSize: 19
    property int iconPointSizeOffset: root.platform === "Apple" ? 0 : -3
    property var themeData: {{theme: "Light"}}
    property bool enabled: true
    property string iconFontFamily: fontIconLoader.fa_solid
    property bool isHovered: !buttonMouseArea.isExited
    property string iconColor: themeData.theme === "Dark" ? "#5b94f5" : "black"
    property string backgroundHoveredColor: if (themeData.theme === "Dark")
                                                "#313131"
                                            else if (themeData.theme === "Light")
                                                "#EFEFEF"
                                            else // Sepia
                                                "#f1e7d2"
    property string backgroundPressedColor: if (themeData.theme === "Dark")
                                                "#2c2c2c"
                                            else if (themeData.theme === "Light")
                                                "#DFDFDE"
                                            else // Sepia
                                                "#dfd5c0"
    property int backgroundRadius: 5
    property bool usePointingHand: false
    property var mouseAreaAlias: buttonMouseArea
    property bool isEntered: false
    property var iconAlias: iconContent

    width: 30
    height: 28

    signal clicked
    signal pressed
    signal pressedAndHold
    signal released

    FontIconLoader {
        id: fontIconLoader
    }

    Rectangle {
        id: backgroundRect
        width: parent.width
        height: parent.height
        radius: root.backgroundRadius
        color: "transparent"

        Text {
            id: iconContent
            anchors.centerIn: parent
            text: root.icon
            font.family: root.iconFontFamily
            color: root.iconColor
            font.pointSize: root.iconPointSize + root.iconPointSizeOffset
            opacity: root.enabled ? 1.0 : 0.2
        }

        MouseArea {
            id: buttonMouseArea
            anchors.fill: parent
            hoverEnabled: true
            property bool isExited: false
            // pressAndHoldInterval: 100

            onEntered: {
                if (root.enabled) {
                    root.isEntered = true;
                    backgroundRect.color = root.backgroundHoveredColor;
                    isExited = false;
                    if (root.usePointingHand) {
                        buttonMouseArea.cursorShape = Qt.PointingHandCursor;
                    }
    //                cursorShape = Qt.PointingHandCursor;
                }
            }

            onExited: {
                if (root.enabled) {
                    root.isEntered = false;
                    backgroundRect.color = "transparent";
                    isExited = true;
                    cursorShape = Qt.ArrowCursor;
                }
            }

            onPressed: {
                if (root.enabled) {
                    root.pressed();
                    backgroundRect.color = root.backgroundPressedColor;
                }
            }

            onPressAndHold: {
                root.pressedAndHold();
            }

            onReleased: {
                if (root.enabled) {
                    root.released();
                    if (isExited) {
                        backgroundRect.color = "transparent";
                    } else {
                        backgroundRect.color = root.backgroundHoveredColor;
                    }
                }
            }

            onClicked: {
                if (root.enabled) {
                    root.clicked();
                }
            }
        }
    }
}
