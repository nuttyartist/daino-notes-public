import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import nuttyartist.notes 1.0
import QtQuick.Dialogs
//import notes

Item {
    id: settingsContainer
    width: settingsPane.width
    height: settingsPane.height
    property var themeData: {{theme: "Light"}}
    property string displayFontFamily: "Inter"
    property string platform: ""
    property string categoriesFontColor: settingsContainer.themeData.theme === "Dark"  ? "#868686" : "#7d7c78"
    property string mainFontColor: settingsContainer.themeData.theme === "Dark" ? "#d6d6d6" : "#37352e"
    property string highlightBackgroundColor: settingsContainer.themeData.theme === "Dark" ? "#313131" : "#efefef"
    property string pressedBackgroundColor: settingsContainer.themeData.theme === "Dark" ? "#2c2c2c" : "#dfdfde"
    property string separatorLineColors: settingsContainer.themeData.theme === "Dark" ? "#3a3a3a" : "#ededec"
    property string mainBackgroundColor: settingsContainer.themeData.theme === "Dark" ? "#252525" : "white"
    signal themeChanged
    property int paddingRightLeft: 12
    property var listOfSansSerifFonts: []
    property var listOfSerifFonts: []
    property var listOfMonoFonts: []
    property int chosenSansSerifFontIndex: 0
    property int chosenSerifFontIndex: 0
    property int chosenMonoFontIndex: 0
    property string currentFontTypeface
    property real latestScrollBarPosition: 0.0
    property int pointSizeOffset: -4
    property int qtVersion: 6
    property bool showBlockEditor: true
    property var rootContainer
    property bool isProVersion: false

    signal setPlainTextEditorVisibility (isVisible: bool)
    signal subscriptionRequiredPopupRequested(text: string)

//    signal changeFontType(fontType : EditorSettingsOptions) // TODO: It's better to use signal & slots for calling C++ functions
//    to change the editor settings rather than calling public slots directly. But I couldn't make it work between QML and C++
//    (QObject::connect: Error: No such signal EditorSettings_QMLTYPE_45::changeFontType(int))
//    So, currently following this:
//    https://scythe-studio.com/en/blog/how-to-integrate-c-and-qml-registering-enums

    Connections {
        target: mainWindow

        function onEditorSettingsShowed (data) {
            var settingsPaneHeightByParentWindow = 0.80 * data.parentWindowHeight; // 80 percent of the parent window's height
            settingsPane.height = scrollViewControl.contentHeight > settingsPaneHeightByParentWindow ? settingsPaneHeightByParentWindow : scrollViewControl.contentHeight;
            revealSettingsAnimation.start();
            settingsContainer.upadteScrollBarPosition();
        }

        function onMainWindowResized (data) {
            var settingsPaneHeightByParentWindow = 0.80 * data.parentWindowHeight; // 80 percent of the parent window's height
            settingsPane.height = scrollViewControl.contentHeight > settingsPaneHeightByParentWindow ? settingsPaneHeightByParentWindow : scrollViewControl.contentHeight;
        }

        function onDisplayFontSet (data) {
            settingsContainer.displayFontFamily = data.displayFont;
        }

        function onPlatformSet (data) {
            settingsContainer.platform = data;
        }

        function onQtVersionSet (data) {
            settingsContainer.qtVersion = data;
        }

        function onThemeChanged (data) {
            settingsContainer.themeData = data;
            themeChanged();
        }

        function onEditorSettingsScrollBarPositionChanged (data) {
            settingsContainer.latestScrollBarPosition = data;
        }

        function onSettingsChanged (data) {
            if (data.currentFontTypeface === "SansSerif") {
                fontChooserSans.checked = true;
                fontChooserSerif.checked = false;
                fontChooserMono.checked = false;
            } else if (data.currentFontTypeface === "Serif") {
                fontChooserSerif.checked = true;
                fontChooserSans.checked = false;
                fontChooserMono.checked = false;
            } else if (data.currentFontTypeface === "Mono") {
                fontChooserMono.checked = true;
                fontChooserSans.checked = false;
                fontChooserSerif.checked = false;
            }

            if (data.currentTheme === "Light") {
                lightThemeChooserButton.themeSelected(true);
                darkThemeChooserButton.themeSelected(false);
                sepiaThemeChooserButton.themeSelected(false);
            } else if (data.currentTheme === "Dark") {
                darkThemeChooserButton.themeSelected(true);
                sepiaThemeChooserButton.themeSelected(false);
                lightThemeChooserButton.themeSelected(false);
            } else if (data.currentTheme === "Sepia") {
                sepiaThemeChooserButton.themeSelected(true);
                darkThemeChooserButton.themeSelected(false);
                lightThemeChooserButton.themeSelected(false);
            }

            focusModeOption.setOptionSelected(!data.isTextFullWidth);
            distractionFreeModeOption.setOptionSelected(data.isDistractionFreeMode);

            notesListCollapsedOption.setOptionSelected(!data.isNoteListCollapsed);
            foldersTreeCollapsedOption.setOptionSelected(!data.isFoldersTreeCollapsed);
            plainTextEnabledOption.setOptionSelected(!settingsContainer.showBlockEditor);
            stayOnTopOption.setOptionSelected(data.isStayOnTop);
        }

        function onFontsChanged (data) {
            settingsContainer.listOfSansSerifFonts = data.listOfSansSerifFonts;            
            settingsContainer.listOfSansSerifFonts = settingsContainer.listOfSansSerifFonts.map(item => item === '.AppleSystemUIFont' ? 'System Font' : item);
            settingsContainer.listOfSerifFonts = data.listOfSerifFonts;
            settingsContainer.listOfMonoFonts = data.listOfMonoFonts;
            settingsContainer.chosenSansSerifFontIndex = data.chosenSansSerifFontIndex;
            settingsContainer.chosenSerifFontIndex = data.chosenSerifFontIndex;
            settingsContainer.chosenMonoFontIndex = data.chosenMonoFontIndex;
            settingsContainer.currentFontTypeface = data.currentFontTypeface;
        }
    }

    FontIconLoader {
        id: fontIconLoader
    }

    PropertyAnimation {
        id: revealSettingsAnimation
        target: settingsContainer
        property: "opacity"
        from: 0.3
        to: 1.0
        duration: 100
        easing.type: Easing.InOutQuad
    }

    function upadteScrollBarPosition () {
        editorSettingsVerticalScrollBar.position = settingsContainer.latestScrollBarPosition;
    }

    Pane {
        id: settingsPane
        visible: true
        padding: 0
        width: 240
        contentWidth: 240 - 2 // for the border rectangle
        height: 500
        Material.elevation: 10
        Material.background: settingsContainer.mainBackgroundColor
        Material.roundedScale: Material.MediumScale

        Rectangle {
            anchors.fill: parent
            border.width: 1
            color: settingsContainer.mainBackgroundColor
            border.color: settingsContainer.themeData.theme === "Dark" ? "#545052" : "#9a9a9a"
            radius: 10
            antialiasing: true
        }

        // Rectangle {
        //     visible: settingsContainer.qtVersion < 6
        //     anchors.fill: parent
        //     color: settingsContainer.mainBackgroundColor
        // }

        ScrollView {
            id: scrollViewControl
            anchors.fill: parent
            clip: true

            ScrollBar.vertical: CustomVerticalScrollBar {
                id: editorSettingsVerticalScrollBar
                themeData: settingsContainer.themeData
                isDarkGray: true
                showBackground: true

                onPositionChanged: {
                    mainWindow.setEditorSettingsScrollBarPosition(editorSettingsVerticalScrollBar.position);

                }
            }

            Column {
                Item {
                    width: 1
                    height: settingsContainer.paddingRightLeft
                }

                Text {
                    text: qsTr("Style")
                    color: settingsContainer.categoriesFontColor
                    font.pointSize: settingsContainer.platform === "Apple" ? 12 : 12 + settingsContainer.pointSizeOffset
                    font.family: settingsContainer.displayFontFamily
                    x: settingsContainer.paddingRightLeft
                    renderType: TextArea.NativeRendering
                }

                Item {
                    width: 1
                    height: 5
                }

                Row {
                    id: fontTypefaceRow
                    x: (settingsPane.contentWidth - fontTypefaceRow.width) / 2
                    FontChooserButton {
                        id: fontChooserSans
                        displayFontFamily: settingsContainer.displayFontFamily
                        platform: settingsContainer.platform
                        mainFontColor: settingsContainer.mainFontColor
                        highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                        pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                        themeData: settingsContainer.themeData
                        categoriesFontColor: settingsContainer.categoriesFontColor
                        fontTypeface: "Sans"
                        fontsModel: settingsContainer.listOfSansSerifFonts
                        currentlyChosenFontIndex: settingsContainer.chosenSansSerifFontIndex
                        qtVersion: settingsContainer.qtVersion
                        mainBackgroundColor: settingsContainer.mainBackgroundColor

                        onClicked: (chosenFontIndex) => {
                            if (chosenFontIndex === -1) {
                                if (settingsContainer.currentFontTypeface === "SansSerif") {
                                    chosenFontIndex = currentlyChosenFontIndex < fontsModel.length - 1 ? currentlyChosenFontIndex + 1 : 0;
                                } else {
                                   chosenFontIndex = currentlyChosenFontIndex;
                                }
                            }
                            mainWindow.changeEditorFontTypeFromStyleButtons(FontTypeface.SansSerif, chosenFontIndex);
                            fontChooserSerif.checked = false;
                            fontChooserMono.checked = false;
                        }

                        Connections {
                            target: settingsContainer

                            function onThemeChanged () {
                                fontChooserSans.themeChanged();
                            }
                        }
                    }

                    FontChooserButton {
                        id: fontChooserSerif
                        displayFontFamily: settingsContainer.displayFontFamily
                        platform: settingsContainer.platform
                        mainFontColor: settingsContainer.mainFontColor
                        highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                        pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                        themeData: settingsContainer.themeData
                        categoriesFontColor: settingsContainer.categoriesFontColor
                        fontTypeface: "Serif"
                        fontsModel: settingsContainer.listOfSerifFonts
                        currentlyChosenFontIndex: settingsContainer.chosenSerifFontIndex
                        qtVersion: settingsContainer.qtVersion
                        mainBackgroundColor: settingsContainer.mainBackgroundColor

                        onClicked: (chosenFontIndex) => {
                           if (chosenFontIndex === -1) {
                               if (settingsContainer.currentFontTypeface === "Serif") {
                                   chosenFontIndex = currentlyChosenFontIndex < fontsModel.length - 1 ? currentlyChosenFontIndex + 1 : 0;
                               } else {
                                  chosenFontIndex = currentlyChosenFontIndex;
                               }
                           }
                            mainWindow.changeEditorFontTypeFromStyleButtons(FontTypeface.Serif, chosenFontIndex);
                            fontChooserSans.checked = false;
                            fontChooserMono.checked = false;
                        }

                        Connections {
                            target: settingsContainer

                            function onThemeChanged () {
                                fontChooserSerif.themeChanged();
                            }
                        }
                    }

                    FontChooserButton {
                        id: fontChooserMono
                        displayFontFamily: settingsContainer.displayFontFamily
                        platform: settingsContainer.platform
                        mainFontColor: settingsContainer.mainFontColor
                        highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                        pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                        themeData: settingsContainer.themeData
                        categoriesFontColor: settingsContainer.categoriesFontColor
                        fontTypeface: "Mono"
                        fontsModel: settingsContainer.listOfMonoFonts
                        currentlyChosenFontIndex: settingsContainer.chosenMonoFontIndex
                        qtVersion: settingsContainer.qtVersion
                        mainBackgroundColor: settingsContainer.mainBackgroundColor

                        onClicked: (chosenFontIndex) => {
                           if (chosenFontIndex === -1) {
                               if (settingsContainer.currentFontTypeface === "Mono") {
                                   chosenFontIndex = currentlyChosenFontIndex < fontsModel.length - 1 ? currentlyChosenFontIndex + 1 : 0;
                               } else {
                                  chosenFontIndex = currentlyChosenFontIndex;
                               }
                           }
                            mainWindow.changeEditorFontTypeFromStyleButtons(FontTypeface.Mono, chosenFontIndex);
                            fontChooserSans.checked = false;
                            fontChooserSerif.checked = false;
                        }

                        Connections {
                            target: settingsContainer

                            function onThemeChanged () {
                                fontChooserMono.themeChanged();
                            }
                        }
                    }
                }

                Item {
                    width: 1
                    height: 10
                }

                Rectangle {
                    height: 1
                    width: settingsPane.contentWidth
                    color: settingsContainer.separatorLineColors
                    x: 1
                }

                Column {
                    id: textSettingsGroup

                    Item {
                        width: 1
                        height: 10
                    }

                    Text {
                        text: qsTr("Text")
                        color: settingsContainer.categoriesFontColor
                        font.pointSize: settingsContainer.platform === "Apple" ? 12 : 12 + settingsContainer.pointSizeOffset
                        font.family: settingsContainer.displayFontFamily
                        x: settingsContainer.paddingRightLeft
                        renderType: TextArea.NativeRendering
                    }

                    Item {
                        width: 1
                        height: 10
                    }

                    // Full width
                    OptionItemButton {
                        id: focusModeOption
                        x: settingsContainer.paddingRightLeft
                        contentWidth: settingsPane.contentWidth - settingsContainer.paddingRightLeft*2
                        displayText: qsTr("Focus mode")
                        displayFontFamily: settingsContainer.displayFontFamily
                        platform: settingsContainer.platform
                        mainFontColor: settingsContainer.mainFontColor
                        highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                        pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                        themeData: settingsContainer.themeData

                        onSwitched: {
                            mainWindow.changeEditorTextWidthFromStyleButtons(EditorTextWidth.TextWidthFullWidth)
                        }

                        onUnswitched: {
                            mainWindow.changeEditorTextWidthFromStyleButtons(EditorTextWidth.TextWidthFullWidth)
                        }
                    }

                    // OptionItemButton {
                    //     id: typewriterModeOption
                    //     x: settingsContainer.paddingRightLeft
                    //     contentWidth: settingsPane.contentWidth - settingsContainer.paddingRightLeft*2
                    //     displayText: qsTr("Typewriter")
                    //     displayFontFamily: settingsContainer.displayFontFamily
                    //     platform: settingsContainer.platform
                    //     mainFontColor: settingsContainer.mainFontColor
                    //     highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                    //     pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                    //     themeData: settingsContainer.themeData

                    //     onSwitched: {
                    //     }

                    //     onUnswitched: {
                    //     }
                    // }

                    OptionItemButton {
                        id: distractionFreeModeOption
                        x: settingsContainer.paddingRightLeft
                        contentWidth: settingsPane.contentWidth - settingsContainer.paddingRightLeft*2
                        displayText: qsTr("Distraction-free")
                        displayFontFamily: settingsContainer.displayFontFamily
                        platform: settingsContainer.platform
                        mainFontColor: settingsContainer.mainFontColor
                        highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                        pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                        themeData: settingsContainer.themeData
                        isProFeature: true
                        isEnabled: settingsContainer.isProVersion
                        isProVersion: settingsContainer.isProVersion

                        onSwitched: {
                            if (settingsContainer.isProVersion) {
                                mainWindow.changeEditorDistractionFreeModeFromStyleButtons(true);
                            } else {
                                settingsContainer.subscriptionRequiredPopupRequested("Thanks for using Daino Notes. A license is required to use the distraction-free mode. Would you like to purchase a license?");
                            }
                        }

                        onUnswitched: {
                            if (settingsContainer.isProVersion) {
                                mainWindow.changeEditorDistractionFreeModeFromStyleButtons(false);
                            } else {
                                settingsContainer.subscriptionRequiredPopupRequested("Thanks for using Daino Notes. A license is required to use the distraction-free mode. Would you like to purchase a license?");
                            }
                        }

                        onClickedWhenDisabled: {
                            settingsContainer.subscriptionRequiredPopupRequested("Thanks for using Daino Notes. A license is required to use the distraction-free mode. Would you like to purchase a license?");
                        }
                    }

                    Item {
                        width: 1
                        height: 10
                    }

                    // Font size
                    Row {
                        id: fontSizeRow
                        x: settingsContainer.paddingRightLeft + 15
                        Text {
                            id: fontSizeText
                            x: 20
                            text: qsTr("Font size")
                            color: settingsContainer.mainFontColor
                            font.pointSize: settingsContainer.platform === "Apple" ? 14 : 14 + settingsContainer.pointSizeOffset
                            font.family: settingsContainer.displayFontFamily
                            renderType: TextArea.NativeRendering
                        }

                        Item {
                            width: settingsPane.contentWidth - fontSizePlusButton.width - fontSizeText.width - fontSizeMinusButton.width - 25 - fontSizeRow.x
                            height: 1
                        }

                        IconButton {
                            id: fontSizeMinusButton
                            icon: fontIconLoader.icons.fa_minus
                            anchors.verticalCenter: fontSizeText.verticalCenter
                            themeData: settingsContainer.themeData
                            platform: settingsContainer.platform
                            iconPointSizeOffset: settingsContainer.platform === "Apple" ? 0 : -7

                            onClicked: {
                                mainWindow.changeEditorFontSizeFromStyleButtons(FontSizeAction.FontSizeDecrease);
                            }
                        }

                        Item {
                            width: 10
                            height: 1
                        }

                        IconButton {
                            id: fontSizePlusButton
                            icon: fontIconLoader.icons.fa_plus
                            anchors.verticalCenter: fontSizeText.verticalCenter
                            themeData: settingsContainer.themeData
                            platform: settingsContainer.platform
                             iconPointSizeOffset: settingsContainer.platform === "Apple" ? 0 : -7

                            onClicked: {
                                mainWindow.changeEditorFontSizeFromStyleButtons(FontSizeAction.FontSizeIncrease);
                            }
                        }
                    }

                    Item {
                        width: 1
                        height: 5
                    }

                    // Text width
                    Row {
                        id: textWidthRow
                        x: settingsContainer.paddingRightLeft + 15
                        visible: focusModeOption.checked
                        Text {
                            id: textWidthText
                            text: qsTr("Text width")
                            color: settingsContainer.mainFontColor
                            font.pointSize: settingsContainer.platform === "Apple" ? 14 : 14 + settingsContainer.pointSizeOffset
                            font.family: settingsContainer.displayFontFamily
                            renderType: TextArea.NativeRendering
                        }

                        Item {
                            width: settingsPane.contentWidth - textWidthPlusButton.width - textWidthText.width - textWidthMinusButton.width - 25 - textWidthRow.x
                            height: 1
                        }

                        IconButton {
                            id: textWidthMinusButton
                            icon: fontIconLoader.icons.fa_minus
                            anchors.verticalCenter: textWidthText.verticalCenter
                            themeData: settingsContainer.themeData
                            platform: settingsContainer.platform
                            iconPointSizeOffset: settingsContainer.platform === "Apple" ? 0 : -7

                            onClicked: {
                                mainWindow.changeEditorTextWidthFromStyleButtons(EditorTextWidth.TextWidthDecrease)
                            }
                        }

                        Item {
                            width: 10
                            height: 1
                        }

                        IconButton {
                            id: textWidthPlusButton
                            icon: fontIconLoader.icons.fa_plus
                            anchors.verticalCenter: textWidthText.verticalCenter
                            themeData: settingsContainer.themeData
                            platform: settingsContainer.platform
                            iconPointSizeOffset: settingsContainer.platform === "Apple" ? 0 : -7

                            onClicked: {
                                mainWindow.changeEditorTextWidthFromStyleButtons(EditorTextWidth.TextWidthIncrease)
                            }
                        }
                    }

                    Item {
                        width: 1
                        height: 7
                    }

                    Rectangle {
                        height: 1
                        width: settingsPane.contentWidth
                        color: settingsContainer.separatorLineColors
                        x: 1
                    }
                }



                Item {
                    width: 1
                    height: 10
                }

                Text {
                    text: qsTr("Theme")
                    x: settingsContainer.paddingRightLeft
                    color: settingsContainer.categoriesFontColor
                    font.pointSize: settingsContainer.platform === "Apple" ? 12 : 12 + settingsContainer.pointSizeOffset
                    font.family: settingsContainer.displayFontFamily
                    renderType: TextArea.NativeRendering
                }

                Item {
                    width: 1
                    height: 5
                }

                // Theme buttons
                Row {
                    x: (settingsPane.contentWidth - fontTypefaceRow.width) / 2
                    ThemeChooserButton {
                        id: lightThemeChooserButton
                        displayFontFamily: settingsContainer.displayFontFamily
                        platform: settingsContainer.platform
                        mainFontColor: settingsContainer.mainFontColor
                        highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                        pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                        themeColor: "#f7f7f7"
                        themeName: "Light"
                        themeData: settingsContainer.themeData
                        qtVersion: settingsContainer.qtVersion

                        onClicked: {
                            mainWindow.setTheme(Theme.Light);
                            darkThemeChooserButton.unclicked();
                            sepiaThemeChooserButton.unclicked();
                        }

                        Connections {
                            target: settingsContainer

                            function onThemeChanged () {
                                lightThemeChooserButton.themeChanged();
                            }
                        }
                    }

                    ThemeChooserButton {
                        id: darkThemeChooserButton
                        displayFontFamily: settingsContainer.displayFontFamily
                        platform: settingsContainer.platform
                        mainFontColor: settingsContainer.mainFontColor
                        highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                        pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                        themeColor: "#191919"
                        themeName: "Dark"
                        themeData: settingsContainer.themeData
                        qtVersion: settingsContainer.qtVersion

                        onClicked: {
                            mainWindow.setTheme(Theme.Dark);
                            lightThemeChooserButton.unclicked();
                            sepiaThemeChooserButton.unclicked();
                        }

                        Connections {
                            target: settingsContainer

                            function onThemeChanged () {
                                darkThemeChooserButton.themeChanged();
                            }
                        }
                    }

                    ThemeChooserButton {
                        id: sepiaThemeChooserButton
                        displayFontFamily: settingsContainer.displayFontFamily
                        platform: settingsContainer.platform
                        mainFontColor: settingsContainer.mainFontColor
                        highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                        pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                        themeColor: "#f7cc6f"
                        themeName: "Sepia"
                        themeData: settingsContainer.themeData
                        qtVersion: settingsContainer.qtVersion

                        onClicked: {
                            mainWindow.setTheme(Theme.Sepia);
                            lightThemeChooserButton.unclicked();
                            darkThemeChooserButton.unclicked();
                        }

                        Connections {
                            target: settingsContainer

                            function onThemeChanged () {
                                sepiaThemeChooserButton.themeChanged();
                            }
                        }
                    }
                }

                Item {
                    width: 1
                    height: 15
                }

                Rectangle {
                    height: 1
                    width: settingsPane.contentWidth
                    color: settingsContainer.separatorLineColors
                    x: 1
                }

                Item {
                    width: 1
                    height: 10
                }

                Text {
                    text: qsTr("Options")
                    x: settingsContainer.paddingRightLeft
                    color: settingsContainer.categoriesFontColor
                    font.pointSize: settingsContainer.platform === "Apple" ? 12 : 12 + settingsContainer.pointSizeOffset
                    font.family: settingsContainer.displayFontFamily
                    renderType: TextArea.NativeRendering
                }

                Item {
                    width: 1
                    height: 10
                }

                Column {
                    x: settingsContainer.paddingRightLeft
                    OptionItemButton {
                        id: notesListCollapsedOption
                        contentWidth: settingsPane.contentWidth - settingsContainer.paddingRightLeft*2
                        displayText: qsTr("Show notes list")
                        displayFontFamily: settingsContainer.displayFontFamily
                        platform: settingsContainer.platform
                        mainFontColor: settingsContainer.mainFontColor
                        highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                        pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                        themeData: settingsContainer.themeData

                        onSwitched: {
                            console.log("Switched");
                            mainWindow.expandNoteList();
                        }

                        onUnswitched: {
                            console.log("Unswitched");
                            mainWindow.collapseNoteList();
                        }
                    }

                    OptionItemButton {
                        id: foldersTreeCollapsedOption
                        contentWidth: settingsPane.contentWidth - settingsContainer.paddingRightLeft*2
                        displayText: qsTr("Show folders tree")
                        displayFontFamily: settingsContainer.displayFontFamily
                        platform: settingsContainer.platform
                        mainFontColor: settingsContainer.mainFontColor
                        highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                        pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                        themeData: settingsContainer.themeData

                        onSwitched: {
                            mainWindow.expandFolderTree();
                        }

                        onUnswitched: {
                            mainWindow.collapseFolderTree();
                        }
                    }

                    OptionItemButton {
                        id: plainTextEnabledOption
                        contentWidth: settingsPane.contentWidth - settingsContainer.paddingRightLeft*2
                        displayText: qsTr("Show plain text")
                        displayFontFamily: settingsContainer.displayFontFamily
                        platform: settingsContainer.platform
                        mainFontColor: settingsContainer.mainFontColor
                        highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                        pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                        themeData: settingsContainer.themeData

                        onSwitched: {
                            settingsContainer.setPlainTextEditorVisibility(true);
                        }

                        onUnswitched: {
                            settingsContainer.setPlainTextEditorVisibility(false);
                        }
                    }

                    OptionItemButton {
                        id: stayOnTopOption
                        visible: settingsContainer.platform === "Apple"
                        contentWidth: settingsPane.contentWidth - settingsContainer.paddingRightLeft*2
                        displayText: qsTr("Always stay on top")
                        displayFontFamily: settingsContainer.displayFontFamily
                        platform: settingsContainer.platform
                        mainFontColor: settingsContainer.mainFontColor
                        highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                        pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                        themeData: settingsContainer.themeData

                        onSwitched: {
                            mainWindow.stayOnTop(true);
                        }

                        onUnswitched: {
                            mainWindow.stayOnTop(false);
                        }
                    }
                }

                Item {
                    width: 1
                    height: 15
                }

                Rectangle {
                    height: 1
                    width: settingsPane.contentWidth
                    color: settingsContainer.separatorLineColors
                    x: 1
                }

                Item {
                    width: 1
                    height: 10
                }

                TextButton {
                    id: deleteNoteButton
                    text: qsTr("Move to Trash")
                    icon: fontIconLoader.icons.fa_trash
                    highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                    pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                    backgroundWidth: settingsPane.contentWidth - settingsContainer.paddingRightLeft*2
                    mainFontColorDefault: settingsContainer.mainFontColor
                    platform: settingsContainer.platform
                    displayFontFamily: settingsContainer.displayFontFamily
                    x: settingsContainer.paddingRightLeft
                    pointSizeOffset: settingsContainer.platform === "Apple" ? 0 : -4

                    onClicked: {
                        mainWindow.moveCurrentNoteToTrash();
                    }
                }

                Item {
                    width: 1
                    height: 10
                }

                Rectangle {
                    height: 1
                    width: settingsPane.contentWidth
                    color: settingsContainer.separatorLineColors
                    x: 1
                }

                Item {
                    width: 1
                    height: 10
                }

                TextButton {
                    id: resetButton
                    text: qsTr("Reset all settings")
                    icon: fontIconLoader.icons.fa_undo_alt
                    highlightBackgroundColor: settingsContainer.highlightBackgroundColor
                    pressedBackgroundColor: settingsContainer.pressedBackgroundColor
                    backgroundWidth: settingsPane.contentWidth - settingsContainer.paddingRightLeft*2
                    mainFontColorDefault: settingsContainer.mainFontColor
                    platform: settingsContainer.platform
                    displayFontFamily: settingsContainer.displayFontFamily
                    x: settingsContainer.paddingRightLeft
                    pointSizeOffset: settingsContainer.platform === "Apple" ? 0 : -4

                    onClicked: {
                        if (settingsContainer.platform === "Apple") {
                            console.log("Apple");
                            resetEditorSettingsDialog.visible = true
                        } else {
                            console.log("Non-Apple");
                            resetEditorSettingsDialogNonApple.visible = true;
                        }
                    }
                }

                Item {
                    width: 1
                    height: 10
                }
            }
        }
    }

    MessageDialog {
        id: resetEditorSettingsDialog
        visible: false
        text: "Reset settings?"
        informativeText: "Reset all editor settings to their defaults? This will not affect your data, only the app appearance."
        buttons: MessageDialog.Cancel | MessageDialog.Yes

        onButtonClicked: function (button, role) {
            switch (button) {
            case MessageDialog.Yes:
                mainWindow.resetEditorSettings();
                break;
            case MessageDialog.Cancel:
                break;
            }
        }
    }

     Dialog {
         id: resetEditorSettingsDialogNonApple
         parent: Overlay.overlay
         width: settingsContainer.rootContainer ? settingsContainer.rootContainer.width/1.5 : 0
         x: settingsContainer.rootContainer ? Math.round((settingsContainer.rootContainer.width - width) / 2) : 0
         y: settingsContainer.rootContainer ? Math.round((settingsContainer.rootContainer.height - height) / 2) : 0
         visible: false
         title: qsTr("Reset settings?")
         standardButtons: Dialog.Yes | Dialog.Cancel
         Material.accent: "#2383e2";
         Material.theme: settingsContainer.themeData.theme === "Dark" ? Material.Dark : Material.Light
 //        Material.roundedScale: Material.SmallScale
         font.family: settingsContainer.displayFontFamily

         Text {
             width: settingsContainer.rootContainer ? settingsContainer.rootContainer.width/2 - 20 : 0
             wrapMode: Text.WordWrap
             text: qsTr("Reset all editor settings to their defaults? This will not affect your data, only the app appearance.")
             font.family: settingsContainer.displayFontFamily
             color: settingsContainer.mainFontColor
             renderType: TextArea.NativeRendering
         }

         onAccepted: {
             mainWindow.resetEditorSettings();
         }
         onRejected: {
         }
         onDiscarded: {
         }
     }
}
