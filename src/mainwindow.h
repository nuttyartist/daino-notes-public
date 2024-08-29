/*********************************************************************************************
 * Mozilla License
 * Just a meantime project to see the ability of qt, the framework that my OS might be based on
 * And for those linux users that believe in the power of notes
 *********************************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QtCore>
#include <QGroupBox>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QSplitter>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QProgressDialog>
#include <QAction>
#include <QAutostart>
#include <QtGlobal>
#include <QWidget>
#include <QQuickView>
#include <QQmlContext>
#include <QVariant>
#include <QJsonObject>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QByteArray>
#include <QQmlApplicationEngine>

#include "nodedata.h"
#include "notelistmodel.h"
#include "notelistview.h"
#include "nodetreemodel.h"

#if defined(UPDATE_CHECKER)
#include "updaterwindow.h"
#endif

#include "dbmanager.h"
#include "aboutwindow.h"
#include "framelesswindow.h"
#include "nodetreeview.h"
#include "editorsettingsoptions.h"
#include "mouseeventspy.h"
#include "custommessagebox.h"

#include "markdownhighlighter.h"
#include "blockmodel.h"

#include "DeclarativeDragArea.h"
#include "DeclarativeDragDropEvent.h"
#include "DeclarativeDropArea.h"
#include "DeclarativeMimeData.h"

L_DECLARE_ENUM(SubscriptionStatus, NoSubscription, Active, ActivationLimitReached, Expired, Invalid,
               EnteredGracePeriod, GracePeriodOver, NoInternetConnection, UnknownError)

namespace Ui {
class MainWindow;
}
class TreeViewLogic;
class ListViewLogic;
class NoteEditorLogic;
class TagPool;
class SplitterStyle;

#if defined(Q_OS_WINDOWS) || defined(Q_OS_WIN)
// #if defined(__MINGW32__) || defined(__GNUC__)
using MainWindowBase = QMainWindow;
// #else
//  using MainWindowBase = CFramelessWindow;
// #endif
#elif defined(Q_OS_MACOS)
using MainWindowBase = CFramelessWindow;
#else
using MainWindowBase = QMainWindow;
#endif
class MainWindow : public MainWindowBase
{
    Q_OBJECT

public:
    enum class ShadowType { Linear = 0, Radial };

    enum class ShadowSide {
        Left = 0,
        Right,
        Top,
        Bottom,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    enum class StretchSide {
        None = 0,
        Left,
        Right,
        Top,
        Bottom,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    Q_ENUM(ShadowType)
    Q_ENUM(ShadowSide)
    Q_ENUM(StretchSide)

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void setMainWindowVisibility(bool state);

public slots:
    void changeEditorTextWidthFromStyleButtons(EditorTextWidth::Value editorTextWidth);
    void createNewNote(bool isCalledFromShortcut = false);
    void saveLastSelectedFolderTags(bool isFolder, const QString &folderPath,
                                    const QSet<int> &tagId);
    void saveExpandedFolder(const QStringList &folderPaths);
    void saveLastSelectedNote(const QSet<int> &notesId);
    void changeEditorFontTypeFromStyleButtons(FontTypeface::Value fontType, int chosenFontIndex);
    void changeEditorFontSizeFromStyleButtons(FontSizeAction::Value fontSizeAction);
    void changeEditorDistractionFreeModeFromStyleButtons(bool isDistractionFreeMode);
    void resetEditorSettings();
    void setTheme(Theme::Value theme);
    void collapseNoteList();
    void expandNoteList();
    void collapseFolderTree();
    void expandFolderTree();
    void stayOnTop(bool checked);
    void moveCurrentNoteToTrash();
    void setEditorSettingsFromQuickViewVisibility(bool isVisible);
    void setEditorSettingsScrollBarPosition(double position);
    void setActivationSuccessful(QString licenseKey, bool removeGracePeriodStartedDate = true);
    void checkProVersion();
    QVariant getUserLicenseKey();
    void callStartSystemMove();
    void handleMaximizeWindow();
    void expandPanes();
    void collapsePanes();
    void openSubscriptionWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *) override;
    void changeEvent(QEvent *event) override;
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    Ui::MainWindow *ui;

    QSettings *m_settingsDatabase;
    QToolButton *m_clearButton;
    QToolButton *m_searchButton;
    QHBoxLayout m_trafficLightLayout;
    QPushButton *m_newNoteButton;
    QPushButton *m_globalSettingsButton;
    QPushButton *m_toggleTreeViewButton;
    NoteEditorLogic *m_noteEditorLogic;
    QLineEdit *m_searchEdit;
    QSplitter *m_splitter;
    QWidget *m_noteListWidget;
    QWidget *m_foldersWidget;
    QSystemTrayIcon *m_trayIcon;
#if !defined(Q_OS_MAC)
    QAction *m_restoreAction;
    QAction *m_quitAction;
#endif

    NoteListView *m_listView;
    NoteListModel *m_listModel;
    ListViewLogic *m_listViewLogic;
    NodeTreeView *m_treeView;
    NodeTreeModel *m_treeModel;
    TreeViewLogic *m_treeViewLogic;
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    QQuickView m_blockEditorQuickView;
    QWidget *m_blockEditorWidget;
#endif
    QWidget *m_editorSettingsWidget;
    TagPool *m_tagPool;
    DBManager *m_dbManager;
    QThread *m_dbThread;
    SplitterStyle *m_splitterStyle;
#if defined(UPDATE_CHECKER)
    UpdaterWindow m_updater;
#endif
    AboutWindow m_aboutWindow;
    StretchSide m_stretchSide;
    Autostart m_autostart;
    int m_mousePressX;
    int m_mousePressY;
    int m_trashCounter;
    int m_layoutMargin;
    int m_shadowWidth;
    int m_nodeTreeWidth;
    int m_smallEditorWidth;
    int m_largeEditorWidth;
    bool m_canMoveWindow;
    bool m_canStretchWindow;
    bool m_isTemp;
    bool m_isListViewScrollBarHidden;
    bool m_isOperationRunning;
    bool m_dontShowUpdateWindow;
    bool m_alwaysStayOnTop;
    bool m_useNativeWindowFrame;
    bool m_hideToTray;

    QString m_styleSheet;

    QStringList m_listOfSerifFonts;
    QStringList m_listOfSansSerifFonts;
    QStringList m_listOfMonoFonts;
    int m_chosenSerifFontIndex;
    int m_chosenSansSerifFontIndex;
    int m_chosenMonoFontIndex;
    int m_editorMediumFontSize;
    int m_currentFontPointSize;
    struct m_charsLimitPerFont
    {
        int mono;
        int serif;
        int sansSerif;
    } m_currentCharsLimitPerFont;
    FontTypeface::Value m_currentFontTypeface;
    QString m_currentFontFamily;
    QFont m_currentSelectedFont;
    QString m_displayFont;
    Theme::Value m_currentTheme;
    QColor m_currentEditorTextColor;
    bool m_areNonEditorWidgetsVisible;
    bool m_isEditorSettingsFromQuickViewVisible;
    bool m_isProVersionActivated;
    QSettings *m_localLicenseData;
    BlockModel *m_blockModel;
    bool m_isTextFullWidth;
    bool m_isAnimatingSplitter;
    bool m_isDistractionFreeMode;
    QJsonObject m_paymentDetails;
    SubscriptionStatus::Value m_subscriptionStatus;
    QQmlApplicationEngine *m_subscriptionWindowEngine;
    QWindow *m_subscriptionWindow;
    QString m_purchaseDataAlt1;
    QString m_purchaseDataAlt2;
    QByteArray *m_dataBuffer;
    QNetworkAccessManager *m_netManager;
    QNetworkRequest m_reqAlt1;
    QNetworkRequest m_reqAlt2;
    QNetworkReply *m_netPurchaseDataReplyFirstAttempt;
    QNetworkReply *m_netPurchaseDataReplySecondAttempt;
    QString m_userLicenseKey;
    QMenu m_mainMenu;
    QAction *m_buyOrManageSubscriptionAction;
    bool m_isLicensedCheckedAfterStartup;
    QString m_databaseFolderPath;
    QTimer m_groupHourlyTimer;
    QQmlApplicationEngine *m_transitionToDainoNotesEngine;
    QWindow *m_transitionToDainoNotesWindow;
    bool m_isFirstRun;
    QJsonObject m_currentThemeData;
    QSettings *m_localAnalyticsData;
    bool m_isFirstRunSubscriptionCheckDone;
    bool m_isUserAcceptedTermsAndPrivacy;

    void setupMainWindow();
    void setupFonts();
    void setupTrayIcon();
    void setupKeyboardShortcuts();
    void setupSplitter();
    void setupButtons();
    void setupSignalsSlots();
    void setupSearchEdit();
    void setupEditorSettings();
    void setupTextEdit();
    void setupTransitionToDainoNotesWindow();
    void setupBlockEditorView();
    void setupDatabases();
    void setupAnonymousAnalytics();
    void setupModelView();
    void initializeSettingsDatabase();
    void setLayoutForScrollArea();
    void checkIfUserAcceptedTermsAndPrivacy();
    void setButtonsAndFieldsEnabled(bool doEnable);
    void restoreStates();
    void migrateFromV0_9_0();
    void executeImport(const bool replace);
    void migrateNoteFromV0_9_0(const QString &notePath);
    void migrateTrashFromV0_9_0(const QString &trashPath);
    void setCurrentFontBasedOnTypeface(FontTypeface::Value selectedFontTypeFace);
    void setWindowButtonsVisible(bool isVisible);
    void updateSelectedOptionsEditorSettings();
    void dropShadow(QPainter &painter, ShadowType type, ShadowSide side);
    void fillRectWithGradient(QPainter &painter, QRect rect, QGradient &gradient);
    double gaussianDist(double x, const double center, double sigma) const;
    void setMargins(QMargins margins);
    void animateHidePane(int paneIndex, QWidget* pane, int duration=500);
    void animateShowPane(int paneIndex, QWidget* pane, int duration=500);
    void setupSubscrirptionWindow();
    void setupGlobalSettingsMenu();
    void getPaymentDetailsSignalsSlots();
    void verifyLicenseSignalsSlots();
    void getSubscriptionStatus();
    void createTransitionToDainoNotesWindow(bool isFromPlume);
    void createSubscriptionWindow();
#if defined(UPDATE_CHECKER)
    void autoCheckForUpdates();
#endif
    void sendAnonymousAnalytics();

private slots:
    void InitData();
    void groupHourlyTimerTriggered();
    void onSystemTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onNewNoteButtonClicked(bool isCalledFromShortcut = false);
    void onGlobalSettingsButtonClicked();
    void onClearButtonClicked();
    void selectNoteDown();
    void selectNoteUp();
    void setFocusOnText();
    void fullscreenWindow();
    void maximizeWindow();
    void minimizeWindow();
    void QuitApplication();
    void checkForUpdates();
    void toggleNoteList();
    void toggleFolderTree();
    void importPlainTextFiles();
    void exportToPlainTextFiles(const QString &extension);
    void importNotesFile();
    void exportNotesFile();
    void restoreNotesFile();
    void setUseNativeWindowFrame(bool useNativeWindowFrame);
    void setHideToTray(bool enabled);
    void toggleStayOnTop();
    void onSearchEditReturnPressed();
    void deleteSelectedNote();
    void clearSearch();
    void showErrorMessage(const QString &title, const QString &content);
    void setNoteListLoading();
    void selectAllNotesInList();
    void updateFrame();
    bool isTitleBar(int x, int y) const;

signals:
    void paneToggled();
    void windowFocusChanged(bool isFocused);
    void focusOnEditor();
    void mainWindowDeactivated();
    void requestNodesTree();
    void requestOpenDBManager(const QString &path, bool doCreate);
    void requestRestoreNotes(const QString &filePath);
    void requestImportNotes(const QString &filePath);
    void requestExportNotes(QString fileName);
    void requestMigrateNotesFromV0_9_0(QVector<NodeData> &noteList);
    void requestMigrateTrashFromV0_9_0(QVector<NodeData> &noteList);
    void requestMigrateNotesFromV1_5_0(const QString &path);
    void requestChangeDatabasePath(const QString &newPath);
    void showPopupWithText(QVariant text);
    void themeChanged(QVariant theme);
    void platformSet(QVariant platform);
    void qtVersionSet(QVariant qtVersion);
    void editorSettingsShowed(QVariant data);
    void mainWindowResized(QVariant data);
    void mainWindowMoved(QVariant data);
    void displayFontSet(QVariant data);
    void settingsChanged(QVariant data);
    void fontsChanged(QVariant data);
    void toggleEditorSettingsKeyboardShorcutFired();
    void editorSettingsScrollBarPositionChanged(QVariant data);
    void proVersionCheck(QVariant data);
    void tryPurchaseDataSecondAlternative();
    void fetchingPaymentDetailsRemotelyFinished();
    void gettingPaymentDetailsFinished();
    void subscriptionStatusChanged(QVariant subscriptionStatus);
    void isPlumeSet(QVariant data);
};

#endif // MAINWINDOW_H
