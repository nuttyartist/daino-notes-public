import QtQuick 2.15

Item {
    id: root
    width: root.backgroundSizeFitText ? root.textWidth !== -1 ? root.textWidth : buttonText.implicitWidth : root.backgroundWidth
    height: root.backgroundSizeFitText ? buttonText.implicitHeight : root.backgroundHeight
    required property string text
    signal clicked
    property var themeData: {{theme: "Light"}}
    property real backgroundOpacity: 1.0
    property string defaultBackgroundColor: "transparent"
    // property string highlightBackgroundColor: root.themeData.theme === "Dark" ? "#313131" : "#efefef"
    // property string pressedBackgroundColor: root.themeData.theme === "Dark" ? "#2c2c2c" : "#dfdfde"
    property string highlightBackgroundColor: if (themeData.theme === "Dark")
                                                "#313131"
                                            else if (themeData.theme === "Light")
                                                "#EFEFEF"
                                            else //if (themeData.theme === "Sepia")
                                                "#f1e7d2"
    property string pressedBackgroundColor: if (themeData.theme === "Dark")
                                                "#2c2c2c"
                                            else if (themeData.theme === "Light")
                                                "#DFDFDE"
                                            else //if (themeData.theme === "Sepia")
                                                "#f1e7d2"
    property string mainFontColorDefault: root.themeData.theme === "Dark" ? "#d6d6d6" : "#37352e"
    property string mainFontColorPressed: mainFontColorDefault
    property string mainFontColorHovered: mainFontColorDefault
    property string iconColorDefault: root.mainFontColorDefault
    property string iconColorPressed: root.mainFontColorPressed
    property string iconColorHovered: root.mainFontColorHovered
    property int textFontWeight: Font.Normal
    property string platform: ""
    property string displayFontFamily
    property int backgroundWidthOffset: 50
    property int backgroundHeightOffset: 15
    property int backgroundHeight: buttonText.implicitHeight + backgroundHeightOffset
    property int backgroundWidth: buttonText.implicitWidth + backgroundWidthOffset
    property string icon: ""
    property int pointSizeOffset: root.platform === "Apple" ? 0 : -3
    property int textFontPointSize: 14
    property int iconFontPointSize: 15
    property int textAlignment: TextButton.TextAlign.Left
    property int textLeftRightMargin: 32
    property int iconLeftRightMargin: 8
    property string iconFontFamily: fontIconLoader.fa_solid
    property bool usePointingHand: true
    property bool pressed: false
    property bool entered: false
    property bool backgroundSizeFitText: false
    property string shortcutKey: ""
    property bool shouldAnimateClicking: false
    property int backgroundRadius: 5
    property bool isEnabled: true
    property int textWidth: -1

    signal finishedAnimatingClicking()

    enum TextAlign {
        Left,
        Right,
        Middle
    }

    FontIconLoader {
        id: fontIconLoader
    }

    function animateClicking () {
        innerButtonRect.color = root.defaultBackgroundColor;
        animateClickingTimer1.start();

    }

    Timer {
        id: animateClickingTimer1
        interval: 50
        repeat: false
        onTriggered: {
            innerButtonRect.color = root.pressedBackgroundColor;
            animateClickingTimer2.start();
        }
    }

    Timer {
        id: animateClickingTimer2
        interval: 100
        onTriggered: {
            root.finishedAnimatingClicking();
        }
    }

    MouseArea {
        id: buttonMouseArea
        hoverEnabled: true
        anchors.fill: parent
        enabled: root.isEnabled

        onEntered: {
            root.entered = true;
            innerButtonRect.color = root.highlightBackgroundColor;
            buttonText.color = root.mainFontColorHovered;
            iconDisplay.color = root.iconColorHovered;
            if (root.usePointingHand) {
                buttonMouseArea.cursorShape = Qt.PointingHandCursor;
            }
        }

        onExited: {
            root.entered = false;
            iconDisplay.color = Qt.binding(function () { return root.iconColorDefault });
            buttonText.color = Qt.binding(function () { return root.mainFontColorDefault });
            innerButtonRect.color = Qt.binding(function () { return root.defaultBackgroundColor });
            buttonMouseArea.cursorShape = Qt.ArrowCursor;
        }

        onPressed: {
            root.pressed = true;
            innerButtonRect.color = root.pressedBackgroundColor;
            buttonText.color = root.mainFontColorPressed;
            iconDisplay.color = root.iconColorPressed;
        }

        onReleased: {
            root.pressed = false;
            buttonText.color = Qt.binding(function () { return root.mainFontColorDefault });
            iconDisplay.color = Qt.binding(function () { return root.iconColorDefault });
            if (buttonMouseArea.containsMouse) {
                innerButtonRect.color = root.highlightBackgroundColor;
            } else {
                innerButtonRect.color = Qt.binding(function () { return root.defaultBackgroundColor });
            }
        }

        onClicked: {
            root.clicked();
            if (root.shouldAnimateClicking) {
                root.animateClicking();
            }
        }

        Rectangle {
            id: innerButtonRect
            anchors.fill: parent
            radius: root.backgroundRadius
            color: root.defaultBackgroundColor
            opacity: root.backgroundOpacity
        }

        // Text {
        //     id: iconDisplay
        //     visible: root.icon !== "" && root.icon !== undefined
        //     text: root.icon
        //     font.family: root.iconFontFamily
        //     color: root.iconColorDefault
        //     font.pointSize: root.iconFontPointSize + root.pointSizeOffset
        //     anchors.verticalCenter: buttonText.verticalCenter
        //     anchors.right: root.textAlignment === TextButton.TextAlign.Right ? innerButtonRect.right : undefined
        //     anchors.rightMargin: root.iconLeftRightMargin
        //     anchors.left: root.textAlignment === TextButton.TextAlign.Left || root.textAlignment === TextButton.TextAlign.Middle ? innerButtonRect.left : undefined
        //     anchors.leftMargin: root.iconLeftRightMargin
        //     renderType: Text.NativeRendering
        // }

        Text {
            id: iconDisplay
            visible: root.icon !== "" && root.icon !== undefined
            text: root.icon
            font.family: root.iconFontFamily
            color: root.iconColorDefault
            font.pointSize: root.iconFontPointSize + root.pointSizeOffset
            anchors.verticalCenter: buttonText.verticalCenter
            anchors.right: if (root.textAlignment === TextButton.TextAlign.Right)
                               innerButtonRect.right
                           else if (root.textAlignment === TextButton.TextAlign.Middle)
                               buttonText.left
                           else
                               undefined
            anchors.rightMargin: root.iconLeftRightMargin
            anchors.left: root.textAlignment === TextButton.TextAlign.Left ? innerButtonRect.left : undefined
            anchors.leftMargin: root.iconLeftRightMargin
            renderType: Text.NativeRendering
            opacity: root.isEnabled ? 1.0 : 0.4
        }


        Text {
            id: buttonText
            text: root.text
            color: root.mainFontColorDefault
            font.pointSize: root.textFontPointSize + root.pointSizeOffset
            font.family: root.displayFontFamily
            font.weight: root.textFontWeight
            anchors.verticalCenter: innerButtonRect.verticalCenter
            anchors.left: root.textAlignment === TextButton.TextAlign.Left ? innerButtonRect.left : undefined
            anchors.leftMargin: root.textLeftRightMargin
            anchors.right: root.textAlignment === TextButton.TextAlign.Right ? iconDisplay.left : undefined
            anchors.rightMargin: root.textLeftRightMargin
            anchors.horizontalCenter: root.textAlignment === TextButton.TextAlign.Middle ? innerButtonRect.horizontalCenter : undefined
            renderType: Text.NativeRendering
            opacity: root.isEnabled ? 1.0 : 0.4
            width: root.textWidth === -1 ? undefined : root.textWidth
            elide: Text.ElideRight
        }

        Text {
            id: shortcutText
            visible: root.shortcutKey !== ""
            text: root.shortcutKey
            font.family: root.displayFontFamily
            anchors.verticalCenter: buttonText.verticalCenter
            anchors.right: root.textAlignment === TextButton.TextAlign.Left ? innerButtonRect.right : undefined
            anchors.rightMargin: 10
            anchors.left: root.textAlignment === TextButton.TextAlign.Right ? innerButtonRect.left : undefined
            anchors.leftMargin: 10
            color: root.themeData.theme === "Dark" ? "#636363" : "#9b9a97"
            font.pointSize: (root.textFontPointSize - 1) + root.pointSizeOffset
            renderType: Text.NativeRendering
        }
    }
}
