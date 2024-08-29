/**************************************************************************************
 * We believe in the power of notes to help us record ideas and thoughts.
 * We want people to have an easy, beautiful and simple way of doing that.
 * And so we have Notes.
 ***************************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qxtglobalshortcut.h"
#include "treeviewlogic.h"
#include "listviewlogic.h"
#include "noteeditorlogic.h"
#include "tagpool.h"
#include "splitterstyle.h"
#include "editorsettingsoptions.h"

#include <QScrollBar>
#include <QShortcut>
#include <QTextStream>
#include <QScrollArea>
#include <QtConcurrent>
#include <QProgressDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QList>
#include <QWidgetAction>
#include <QTimer>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QUuid>

#define DEFAULT_DATABASE_NAME "default_database"

/*!
 * \brief MainWindow::MainWindow
 * \param parent
 */
MainWindow::MainWindow(QWidget *parent)
    : MainWindowBase(parent),
      ui(new Ui::MainWindow),
      m_settingsDatabase(nullptr),
      m_clearButton(nullptr),
      m_searchButton(nullptr),
      m_newNoteButton(nullptr),
      m_globalSettingsButton(nullptr),
      m_noteEditorLogic(nullptr),
      m_searchEdit(nullptr),
      m_splitter(nullptr),
      m_trayIcon(new QSystemTrayIcon(this)),
#if !defined(Q_OS_MAC)
      m_restoreAction(new QAction(tr("&Hide Daino Notes"), this)),
      m_quitAction(new QAction(tr("&Quit"), this)),
#endif
      m_listView(nullptr),
      m_listModel(nullptr),
      m_listViewLogic(nullptr),
      m_treeView(nullptr),
      m_treeModel(new NodeTreeModel(this)),
      m_treeViewLogic(nullptr),
      m_blockEditorQuickView(nullptr),
      m_blockEditorWidget(this),
      m_tagPool(nullptr),
      m_dbManager(nullptr),
      m_dbThread(nullptr),
      m_aboutWindow(this),
      m_trashCounter(0),
      m_layoutMargin(10),
      m_shadowWidth(10),
      m_smallEditorWidth(420),
      m_largeEditorWidth(1250),
      m_canMoveWindow(false),
      m_canStretchWindow(false),
      m_isTemp(false),
      m_isListViewScrollBarHidden(true),
      m_isOperationRunning(false),
      m_dontShowUpdateWindow(false),
      m_alwaysStayOnTop(false),
      m_useNativeWindowFrame(true),
      m_hideToTray(false),
      m_listOfSerifFonts(
              { QStringLiteral("Ibarra Real Nova"), QStringLiteral("Trykker")}),
      m_listOfSansSerifFonts({ QStringLiteral("Inter") }),
      m_listOfMonoFonts({ QStringLiteral("iA Writer Quattro S"), QStringLiteral("iA Writer Mono S"), QStringLiteral("iA Writer Duo S") }),
      m_chosenSerifFontIndex(0),
      m_chosenSansSerifFontIndex(0),
      m_chosenMonoFontIndex(0),
      m_currentCharsLimitPerFont({ 64, // Mono    TODO: is this the proper way to initialize?
                                   80, // Serif
                                   80 }), // SansSerif
      m_currentFontTypeface(FontTypeface::SansSerif),
#ifdef __APPLE__
      m_displayFont(QFont(QFontInfo(QApplication::font()).family()).exactMatch()
                            ? QFontInfo(QApplication::font()).family()
                            : QStringLiteral("Inter")),
#elif _WIN32
      m_displayFont(QFont(QStringLiteral("Segoe UI")).exactMatch() ? QStringLiteral("Segoe UI")
                                                                   : QStringLiteral("Inter")),
#else
      m_displayFont(QStringLiteral("Inter")),
#endif
      m_currentTheme(Theme::Light),
      m_currentEditorTextColor(26, 26, 26),
      m_areNonEditorWidgetsVisible(true),
      m_isEditorSettingsFromQuickViewVisible(false),
      m_isProVersionActivated(false),
      m_localLicenseData(nullptr),
      m_blockModel(new BlockModel(this)),
      m_isTextFullWidth(false),
      m_isAnimatingSplitter(false),
      m_isDistractionFreeMode(false),
      m_paymentDetails(QJsonObject()),
      m_subscriptionStatus(SubscriptionStatus::NoSubscription),
      m_subscriptionWindowEngine(nullptr),
      m_subscriptionWindow(nullptr),
      m_purchaseDataAlt1(QStringLiteral("https://www.daino.io/daino_notes/daino_notes_purchase_data.json")),
      m_purchaseDataAlt2(
              QStringLiteral("https://raw.githubusercontent.com/nuttyartist/daino-notes-public/main/daino_notes_purchase_data.json")),
      m_dataBuffer(new QByteArray()),
      m_netManager(new QNetworkAccessManager(this)),
      m_reqAlt1(QNetworkRequest(QUrl(m_purchaseDataAlt1))),
      m_reqAlt2(QNetworkRequest(QUrl(m_purchaseDataAlt2))),
      m_netPurchaseDataReplyFirstAttempt(nullptr),
      m_netPurchaseDataReplySecondAttempt(nullptr),
      m_userLicenseKey(QStringLiteral("")),
      m_mainMenu(nullptr),
      m_buyOrManageSubscriptionAction(new QAction(this)),
      m_isLicensedCheckedAfterStartup(false),
      m_databaseFolderPath(""),
      m_groupHourlyTimer(new QTimer(this)),
      m_transitionToDainoNotesEngine(nullptr),
      m_transitionToDainoNotesWindow(nullptr),
      m_isFirstRun(false),
      m_currentThemeData(QJsonObject()),
      m_isFirstRunSubscriptionCheckDone(false),
      m_isUserAcceptedTermsAndPrivacy(false)
{
    ui->setupUi(this);
    setupBlockEditorView();
    setupMainWindow();
    setupSubscrirptionWindow();
    setupGlobalSettingsMenu();
    setupFonts();
    setupSplitter();
    setupSearchEdit();
    setupEditorSettings();
    setupKeyboardShortcuts();
    setupDatabases();
    setupAnonymousAnalytics();
    setupModelView();
    setupTextEdit();
    setupTransitionToDainoNotesWindow();
    restoreStates();
    setupButtons();
    setupSignalsSlots();
#if defined(UPDATE_CHECKER)
    autoCheckForUpdates();
#endif
    connect(&m_groupHourlyTimer, &QTimer::timeout, this, &MainWindow::groupHourlyTimerTriggered);
    // m_groupHourlyTimer.start(20 * 1000); // 20 seconds
    m_groupHourlyTimer.start(60 * 60 * 1000); // 1 hour
    checkProVersion();
    checkIfUserAcceptedTermsAndPrivacy();

    if (m_transitionToDainoNotesWindow != nullptr)
        m_transitionToDainoNotesWindow->show();

    QTimer::singleShot(200, this, SLOT(InitData()));
}

/*!
 * \brief MainWindow::InitData
 * Init the data from database and select the first note if there is one
 */
void MainWindow::InitData()
{
    QFileInfo fi(m_settingsDatabase->fileName());
    QDir dir(fi.absolutePath());
    QString oldNoteDBPath(dir.path() + QStringLiteral("/Notes.ini"));
    QString oldTrashDBPath(dir.path() + QStringLiteral("/Trash.ini"));

    bool isV0_9_0 = (QFile::exists(oldNoteDBPath) || QFile::exists(oldTrashDBPath));
    if (isV0_9_0) {
        QProgressDialog *pd =
                new QProgressDialog(tr("Migrating database, please wait."), QString(), 0, 0, this);
        pd->setCancelButton(nullptr);
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
        pd->setWindowFlags(Qt::Window);
#else
        pd->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);
#endif
        pd->setMinimumDuration(0);
        pd->show();

        setButtonsAndFieldsEnabled(false);

        QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
        connect(watcher, &QFutureWatcher<void>::finished, this, [&, pd]() {
            pd->deleteLater();
            setButtonsAndFieldsEnabled(true);
        });

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QFuture<void> migration = QtConcurrent::run(&MainWindow::migrateFromV0_9_0, this);
#else
        QFuture<void> migration = QtConcurrent::run(this, &MainWindow::migrateFromV0_9_0);
#endif
        watcher->setFuture(migration);
    }
    /// Check if it is running with an argument (ex. hide)
    if (qApp->arguments().contains(QStringLiteral("--autostart"))
        && QSystemTrayIcon::isSystemTrayAvailable()) {
        setMainWindowVisibility(false);
    }

    // init tree view
    emit requestNodesTree();
}

/*!
 * \brief Toggles visibility of the main window upon system tray activation
 * \param reason The reason the system tray was activated
 */
void MainWindow::onSystemTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        setMainWindowVisibility(!isVisible());
    }
}

/*!
 * \brief MainWindow::setMainWindowVisibility
 * \param state
 */
void MainWindow::setMainWindowVisibility(bool state)
{
    if (state) {
        show();
        raise();
        activateWindow();
#if !defined(Q_OS_MAC)
        m_restoreAction->setText(tr("&Hide Daino Notes"));
#endif
    } else {
#if !defined(Q_OS_MAC)
        m_restoreAction->setText(tr("&Show Daino Notes"));
#endif
        hide();
    }
}

void MainWindow::saveLastSelectedFolderTags(bool isFolder, const QString &folderPath,
                                            const QSet<int> &tagId)
{
    m_settingsDatabase->setValue("isSelectingFolder", isFolder);
    m_settingsDatabase->setValue("currentSelectFolder", folderPath);
    QStringList sl;
    for (const auto &id : tagId) {
        sl.append(QString::number(id));
    }
    m_settingsDatabase->setValue("currentSelectTagsId", sl);
}

void MainWindow::saveExpandedFolder(const QStringList &folderPaths)
{
    m_settingsDatabase->setValue("currentExpandedFolder", folderPaths);
}

void MainWindow::saveLastSelectedNote(const QSet<int> &notesId)
{
    QStringList sl;
    for (const auto &id : notesId) {
        sl.append(QString::number(id));
    }
    m_settingsDatabase->setValue("currentSelectNotesId", sl);
}

/*!
 * \brief MainWindow::paintEvent
 * \param event
 */
void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
}

/*!
 * \brief MainWindow::resizeEvent
 * \param event
 */
void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (m_splitter) {
        // restore note list width
        updateFrame();
    }

    QJsonObject dataToSendToView{ { "parentWindowHeight", height() },
                                  { "parentWindowWidth", width() } };
    emit mainWindowResized(QVariant(dataToSendToView));

    QMainWindow::resizeEvent(event);

    if (this->windowState() != Qt::WindowFullScreen) {
        updateFrame();
    }
}

/*!
 * \brief MainWindow::~MainWindow
 * Deconstructor of the class
 */
MainWindow::~MainWindow()
{
    delete ui;
    m_dbThread->quit();
    m_dbThread->wait();
    delete m_dbThread;
}

/*!
 * \brief MainWindow::setupMainWindow
 * Setting up main window prefrences like frameless window and the minimum size of the window
 * Setting the window background color to be white
 */
void MainWindow::setupMainWindow()
{
#if !defined(Q_OS_MAC)
    auto flags = Qt::Window | Qt::CustomizeWindowHint;
#  if defined(Q_OS_UNIX)
    //    flags |= Qt::FramelessWindowHint;
    flags = Qt::Window;
#  endif
    setWindowFlags(flags);
#endif

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    setAttribute(Qt::WA_TranslucentBackground);
#endif

#if defined(Q_OS_MAC)
    // Since Qt 6.8 we need this to ask the QLayout to ignore the contents marginsto to make the window frameless on macOS
    ui->centralWidget->setAttribute(Qt::WA_LayoutOnEntireRect, true);
#endif

    // load stylesheet
    QFile mainWindowStyleFile(QStringLiteral(":/styles/main-window.css"));
    mainWindowStyleFile.open(QFile::ReadOnly);
    m_styleSheet = QString::fromLatin1(mainWindowStyleFile.readAll());
    setStyleSheet(m_styleSheet);
    /**** Apply the stylesheet for all children we change classes for ****/

    // left frame
    ui->frameLeft->setStyleSheet(m_styleSheet);

    // middle frame
    ui->frameMiddle->setStyleSheet(m_styleSheet);
    ui->searchEdit->setStyleSheet(m_styleSheet);
    ui->verticalSpacer_upSearchEdit->setStyleSheet(m_styleSheet);
    ui->verticalSpacer_upSearchEdit2->setStyleSheet(m_styleSheet);
    ui->listviewLabel1->setStyleSheet(m_styleSheet);
    ui->listviewLabel2->setStyleSheet(m_styleSheet);

    // splitters
    ui->verticalSplitterLine_left->setStyleSheet(m_styleSheet);
    ui->verticalSplitterLine_middle->setStyleSheet(m_styleSheet);

    // buttons
    ui->toggleTreeViewButton->setStyleSheet(m_styleSheet);
    ui->newNoteButton->setStyleSheet(m_styleSheet);
    ui->globalSettingsButton->setStyleSheet(m_styleSheet);

    // custom scrollbars on Linux and Windows
//#if !defined(Q_OS_MACOS)
//    QFile scollBarStyleFile(QStringLiteral(":/styles/components/custom-scrollbar.css"));
//    scollBarStyleFile.open(QFile::ReadOnly);
//    QString scrollbarStyleSheet = QString::fromLatin1(scollBarStyleFile.readAll());
//#endif

#ifdef __APPLE__
    setCloseBtnQuit(false);
    m_layoutMargin = 0;
#endif

#if defined(Q_OS_WINDOWS)
    m_layoutMargin = 0;
#endif

    m_newNoteButton = ui->newNoteButton;
    m_globalSettingsButton = ui->globalSettingsButton;
    m_searchEdit = ui->searchEdit;
    m_splitter = ui->splitter;
    m_foldersWidget = ui->frameLeft;
    m_noteListWidget = ui->frameMiddle;
    m_toggleTreeViewButton = ui->toggleTreeViewButton;
    // don't resize first two panes when resizing
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 0);
    m_splitter->setStretchFactor(2, 1);

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    QMargins margins(m_layoutMargin, m_layoutMargin, m_layoutMargin, m_layoutMargin);
    setMargins(margins);
#endif
    ui->frame->installEventFilter(this);
    ui->centralWidget->setMouseTracking(true);
    setMouseTracking(true);
    QPalette pal(palette());
    pal.setColor(QPalette::Window, QColor(248, 248, 248));
    setAutoFillBackground(true);
    setPalette(pal);

    m_newNoteButton->setToolTip(tr("Create New Note"));
    m_globalSettingsButton->setToolTip(tr("Open App Settings"));
    m_toggleTreeViewButton->setToolTip(tr("Toggle Folders Pane"));

    ui->listviewLabel2->setMinimumSize({ 40, 25 });
    ui->listviewLabel2->setMaximumSize({ 40, 25 });

#ifdef __APPLE__
    QFont titleFont(m_displayFont, 13, QFont::Bold);
#else
    QFont titleFont(m_displayFont, 10, QFont::Bold);
#endif
    ui->listviewLabel1->setFont(titleFont);
    ui->listviewLabel2->setFont(titleFont);
    m_splitterStyle = new SplitterStyle();
    m_splitter->setStyle(m_splitterStyle);
    m_splitter->setHandleWidth(0);
    setNoteListLoading();
#ifdef __APPLE__
    ui->searchEdit->setFocus();
#endif
    setWindowIcon(QIcon(QStringLiteral(":images/notes_icon.ico")));
}

/*!
 * \brief MainWindow::setupFonts
 */
void MainWindow::setupFonts()
{
#ifdef __APPLE__
    m_searchEdit->setFont(QFont(m_displayFont, 12));
#else
    m_searchEdit->setFont(QFont(m_displayFont, 10));
#endif
}

/*!
 * \brief MainWindow::setupTrayIcon
 */
void MainWindow::setupTrayIcon()
{
#if !defined(Q_OS_MAC)
    auto trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(m_restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(m_quitAction);
    m_trayIcon->setContextMenu(trayIconMenu);
#endif

#if defined(Q_OS_MAC)
    QIcon icon(QStringLiteral(":images/notes_system_tray_icon_mac.png"));
    icon.setIsMask(true);
#else
    QIcon icon(QStringLiteral(":images/notes_system_tray_icon.png"));
#endif
    m_trayIcon->setIcon(icon);
    m_trayIcon->show();
}

/*!
 * \brief MainWindow::setupKeyboardShortcuts
 * Setting up the keyboard shortcuts
 */
void MainWindow::setupKeyboardShortcuts()
{
#if defined(Q_OS_MAC)
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_W), this), &QShortcut::activated, this,
            [=]() { setMainWindowVisibility(false); });
#endif
    //    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_N), this, SLOT(onNewNoteButtonClicked()));
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_N), this), &QShortcut::activated, this,
            [=]() { onNewNoteButtonClicked(true); });
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_D), this, SLOT(deleteSelectedNote()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), m_searchEdit, SLOT(setFocus()));
    // new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_E), m_searchEdit, SLOT(clear()));
    //    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Down), this, SLOT(selectNoteDown()));
    //    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Up), this, SLOT(selectNoteUp()));
    new QShortcut(QKeySequence(Qt::Key_Down), this, SLOT(selectNoteDown()));
    new QShortcut(QKeySequence(Qt::Key_Up), this, SLOT(selectNoteUp()));
    //    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Enter), this, SLOT(setFocusOnText()));
    //    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return), this, SLOT(setFocusOnText()));
    // new QShortcut(QKeySequence(Qt::Key_Enter), this, SLOT(setFocusOnText()));
    // new QShortcut(QKeySequence(Qt::Key_Return), this, SLOT(setFocusOnText()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F), this, SLOT(fullscreenWindow()));
    new QShortcut(Qt::Key_F11, this, SLOT(fullscreenWindow()));
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_L), this),
            &QShortcut::activated, this, [=]() { m_listView->setFocus(); });
    new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_M), this, SLOT(minimizeWindow()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q), this, SLOT(QuitApplication()));
#if defined(Q_OS_MACOS) || defined(Q_OS_WINDOWS)
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_K), this, SLOT(toggleStayOnTop()));
#endif
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_J), this, SLOT(toggleNoteList()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_J), this, SLOT(toggleFolderTree()));
    //    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_A), this, SLOT(selectAllNotesInList()));
    connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S), this),
            &QShortcut::activated, this,
            [=]() { emit toggleEditorSettingsKeyboardShorcutFired(); });
    QxtGlobalShortcut *shortcut = new QxtGlobalShortcut(this);
#if defined(Q_OS_MACOS)
    shortcut->setShortcut(QKeySequence(Qt::META | Qt::Key_N));
#else
    shortcut->setShortcut(QKeySequence(Qt::META | Qt::SHIFT | Qt::Key_N));
#endif
    connect(shortcut, &QxtGlobalShortcut::activated, this, [=]() {
        // workaround prevent textEdit and searchEdit
        // from taking 'N' from shortcut
        m_searchEdit->setDisabled(true);
        setMainWindowVisibility(isHidden() || windowState() == Qt::WindowMinimized
                                || qApp->applicationState() == Qt::ApplicationInactive);
        if (isHidden() || windowState() == Qt::WindowMinimized
            || qApp->applicationState() == Qt::ApplicationInactive)
#ifdef __APPLE__
            raise();
#else
            activateWindow();
#endif
        m_searchEdit->setDisabled(false);
    });
}

/*!
 * \brief MainWindow::setupSplitter
 * Set up the splitter that control the size of the scrollArea and the textEdit
 */
void MainWindow::setupSplitter()
{
    m_splitter->setCollapsible(0, false);
    m_splitter->setCollapsible(1, false);
    m_splitter->setCollapsible(2, false);
}

/*!
 * \brief MainWindow::setupButtons
 * Setting up the red (close), yellow (minimize), and green (maximize) buttons
 * Make only the buttons icon visible
 * And install this class event filter to them, to act when hovering on one of them
 */
void MainWindow::setupButtons()
{
    QString ss = QStringLiteral("QPushButton { "
                                "  border: none; "
                                "  padding: 0px; "
                                "}");

    QFont fontAwesomeIcon("Font Awesome 6 Free Solid");
    QFont materialSymbols("Material Symbols Outlined");
#if defined(Q_OS_MACOS)
    int pointSizeOffset = 0;
#else
    int pointSizeOffset = -4;
#endif

    fontAwesomeIcon.setPointSize(16 + pointSizeOffset);
    m_globalSettingsButton->setFont(fontAwesomeIcon);
    m_globalSettingsButton->setText(u8"\uf013"); // fa-gear

#if defined(Q_OS_MACOS)
    materialSymbols.setPointSize(30 + pointSizeOffset);
#else
    materialSymbols.setPointSize(30 + pointSizeOffset - 3);
#endif

#if defined(Q_OS_MACOS)
    materialSymbols.setPointSize(24 + pointSizeOffset);
#else
    materialSymbols.setPointSize(21 + pointSizeOffset);
#endif

    materialSymbols.setPointSize(20 + pointSizeOffset);
    m_toggleTreeViewButton->setFont(materialSymbols);
    if (m_foldersWidget->isHidden()) {
        m_toggleTreeViewButton->setText(u8"\ue31c"); // keyboard_tab_rtl
    } else {
        m_toggleTreeViewButton->setText(u8"\uec73"); // keyboard_tab_rtl
    }

    fontAwesomeIcon.setPointSize(17 + pointSizeOffset);
    m_newNoteButton->setFont(fontAwesomeIcon);
    m_newNoteButton->setText(u8"\uf067"); // fa_plus
}

/*!
 * \brief MainWindow::setupSignalsSlots
 * connect between signals and slots
 */
void MainWindow::setupSignalsSlots()
{
#if defined(UPDATE_CHECKER)
    connect(&m_updater, &UpdaterWindow::dontShowUpdateWindowChanged, this,
            [=](bool state) { m_dontShowUpdateWindow = state; });
#endif
    // actions
    // connect(rightToLeftActionion, &QAction::triggered, this, );
    // connect(checkForUpdatesAction, &QAction::triggered, this, );
    // new note button
    connect(m_newNoteButton, &QPushButton::clicked, this, &MainWindow::onNewNoteButtonClicked);
    // global settings button
    connect(m_globalSettingsButton, &QPushButton::clicked, this,
            &MainWindow::onGlobalSettingsButtonClicked);
    // line edit text changed
    connect(m_searchEdit, &QLineEdit::textChanged, m_listViewLogic,
            &ListViewLogic::onSearchEditTextChanged);
    // line edit enter key pressed
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &MainWindow::onSearchEditReturnPressed);
    // clear button
    connect(m_clearButton, &QToolButton::clicked, this, &MainWindow::onClearButtonClicked);
    // System tray activation
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onSystemTrayIconActivated);

#if !defined(Q_OS_MAC)
    // System tray context menu action: "Show/Hide Daino Notes"
    connect(m_restoreAction, &QAction::triggered, this, [this]() {
        setMainWindowVisibility(isHidden() || windowState() == Qt::WindowMinimized
                                || (qApp->applicationState() == Qt::ApplicationInactive));
    });
    // System tray context menu action: "Quit"
    connect(m_quitAction, &QAction::triggered, this, &MainWindow::QuitApplication);
    // Application state changed
    connect(qApp, &QApplication::applicationStateChanged, this,
            [this]() { m_listView->update(m_listView->currentIndex()); });
#endif

    // MainWindow <-> DBManager
    connect(this, &MainWindow::requestNodesTree, m_dbManager, &DBManager::onNodeTagTreeRequested,
            Qt::BlockingQueuedConnection);
    connect(this, &MainWindow::requestRestoreNotes, m_dbManager,
            &DBManager::onRestoreNotesRequested, Qt::BlockingQueuedConnection);
    connect(this, &MainWindow::requestImportNotes, m_dbManager, &DBManager::onImportNotesRequested,
            Qt::BlockingQueuedConnection);
    connect(this, &MainWindow::requestExportNotes, m_dbManager, &DBManager::onExportNotesRequested,
            Qt::BlockingQueuedConnection);
    connect(this, &MainWindow::requestMigrateNotesFromV0_9_0, m_dbManager,
            &DBManager::onMigrateNotesFromV0_9_0Requested, Qt::BlockingQueuedConnection);
    connect(this, &MainWindow::requestMigrateTrashFromV0_9_0, m_dbManager,
            &DBManager::onMigrateTrashFrom0_9_0Requested, Qt::BlockingQueuedConnection);

    //    connect(m_listViewLogic, &ListViewLogic::showNotesInEditor, m_noteEditorLogic,
    //            &NoteEditorLogic::showNotesInEditor);
    connect(m_listViewLogic, &ListViewLogic::showNotesInEditor,
            [this](const QList<NodeData> &notes) {
                m_noteEditorLogic->showNotesInEditor(
                        notes, false); // 'false' is the default value for the second parameter
            });

    connect(m_listViewLogic, &ListViewLogic::closeNoteEditor, m_noteEditorLogic,
            &NoteEditorLogic::closeEditor);
    connect(m_noteEditorLogic, &NoteEditorLogic::moveNoteToListViewTop, m_listViewLogic,
            &ListViewLogic::moveNoteToTop);
    connect(m_noteEditorLogic, &NoteEditorLogic::updateNoteDataInList, m_listViewLogic,
            &ListViewLogic::setNoteData);
    connect(m_noteEditorLogic, &NoteEditorLogic::deleteNoteRequested, m_listViewLogic,
            &ListViewLogic::deleteNoteRequested);
    connect(m_listViewLogic, &ListViewLogic::noteTagListChanged, m_noteEditorLogic,
            &NoteEditorLogic::onNoteTagListChanged);
    connect(m_noteEditorLogic, &NoteEditorLogic::noteEditClosed, m_listViewLogic,
            &ListViewLogic::onNoteEditClosed);
    connect(m_listViewLogic, &ListViewLogic::requestClearSearchUI, this, &MainWindow::clearSearch);
    // Handle search in block model
    connect(m_listViewLogic, &ListViewLogic::requestClearSearchUI, m_blockModel,
            &BlockModel::clearSearch);
    connect(m_searchEdit, &QLineEdit::textChanged, m_blockModel,
            &BlockModel::onSearchEditTextChanged);
    connect(m_treeViewLogic, &TreeViewLogic::addNoteToTag, m_listViewLogic,
            &ListViewLogic::onAddTagRequestD);
    connect(m_listViewLogic, &ListViewLogic::listViewLabelChanged, this,
            [this](const QString &l1, const QString &l2) {
                ui->listviewLabel1->setText(l1);
                ui->listviewLabel2->setText(l2);
                m_splitter->setHandleWidth(0);
            });
    connect(m_toggleTreeViewButton, &QPushButton::clicked, this, &MainWindow::toggleFolderTree);
    connect(m_dbManager, &DBManager::showErrorMessage, this, &MainWindow::showErrorMessage,
            Qt::QueuedConnection);
    //    connect(m_listViewLogic, &ListViewLogic::requestNewNote, this,
    //            &MainWindow::onNewNoteButtonClicked);
    connect(m_listViewLogic, &ListViewLogic::requestNewNote, this,
            [this]() { onNewNoteButtonClicked(false); });
    connect(m_listViewLogic, &ListViewLogic::moveNoteRequested, this, [this](int id, int target) {
        m_treeViewLogic->onMoveNodeRequested(id, target);
        m_treeViewLogic->openFolder(target);
    });
    connect(m_listViewLogic, &ListViewLogic::setNewNoteButtonVisible, this,
            [this](bool visible) { ui->newNoteButton->setVisible(visible); });
    connect(m_treeViewLogic, &TreeViewLogic::noteMoved, m_listViewLogic,
            &ListViewLogic::onNoteMovedOut);

    connect(m_listViewLogic, &ListViewLogic::requestClearSearchDb, this,
            &MainWindow::setNoteListLoading);
    connect(m_treeView, &NodeTreeView::loadNotesInTagsRequested, this,
            &MainWindow::setNoteListLoading);
    connect(m_treeView, &NodeTreeView::loadNotesInFolderRequested, this,
            &MainWindow::setNoteListLoading);
    connect(m_treeView, &NodeTreeView::saveExpand, this, &MainWindow::saveExpandedFolder);
    connect(m_treeView, &NodeTreeView::saveSelected, this, &MainWindow::saveLastSelectedFolderTags);
    connect(m_listView, &NoteListView::saveSelectedNote, this, &MainWindow::saveLastSelectedNote);
    connect(m_treeView, &NodeTreeView::saveLastSelectedNote, m_listViewLogic,
            &ListViewLogic::setLastSelectedNote);
    connect(m_treeView, &NodeTreeView::requestLoadLastSelectedNote, m_listViewLogic,
            &ListViewLogic::loadLastSelectedNoteRequested);
    connect(m_treeView, &NodeTreeView::loadNotesInFolderRequested, m_listViewLogic,
            &ListViewLogic::onNotesListInFolderRequested);
    connect(m_treeView, &NodeTreeView::loadNotesInTagsRequested, m_listViewLogic,
            &ListViewLogic::onNotesListInTagsRequested);
    connect(this, &MainWindow::requestChangeDatabasePath, m_dbManager,
            &DBManager::onChangeDatabasePathRequested, Qt::QueuedConnection);

#if defined(Q_OS_MACOS)
    connect(this, &MainWindowBase::toggleFullScreen, this, [this](bool isFullScreen) {
        if (isFullScreen) {
            ui->verticalSpacer_upSearchEdit->setMinimumHeight(0);
            ui->verticalSpacer_upSearchEdit->setMaximumHeight(0);
        } else {
            if (m_foldersWidget->isHidden()) {
                ui->verticalSpacer_upSearchEdit->setMinimumHeight(33);
                ui->verticalSpacer_upSearchEdit->setMaximumHeight(33);
            }
        }
    });
#endif
}

void MainWindow::sendAnonymousAnalytics()
{
    if (!m_isUserAcceptedTermsAndPrivacy)
        return;

    // QElapsedTimer timer;
    // timer.start();

    QNetworkRequest request(QUrl("https://www.daino.io/api/daino_notes/saveUserActivity"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject userActivityObject;
    userActivityObject["user_id"] = m_localAnalyticsData->value(QStringLiteral("id"), "NULL").toString();
    userActivityObject["app_version"] = qApp->applicationVersion();
    userActivityObject["os"] = QSysInfo::productType();
    userActivityObject["os_version"] = QSysInfo::productVersion();
    userActivityObject["is_pro"] = m_isProVersionActivated;
    userActivityObject["total_notes"] = m_dbManager->getTotalNumberOfNotesNotInTrash();
    userActivityObject["total_folders"] = m_dbManager->getTotalNumberOfFolders() - 3; // subtract 3 for default folders
    userActivityObject["total_tags"] = m_dbManager->getTotalNumberOfTags();

    // qint64 elapsed = timer.elapsed();
    // qDebug() << "Analytics, time taken to gather:" << elapsed << "ms";

    QJsonDocument userActivityDoc(userActivityObject);
    QByteArray postData = userActivityDoc.toJson();
    QNetworkReply *reply = m_netManager->post(request, postData);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonObject response = QJsonDocument::fromJson(responseData).object();
            qDebug() << "Analytics sent successfully:" << response;
        } else {
            QByteArray responseData = reply->readAll();
            QJsonObject response = QJsonDocument::fromJson(responseData).object();
            qDebug() << "Error sending analytics:" << response;
        }

        reply->deleteLater();
    });
}

/*!
 * \brief MainWindow::autoCheckForUpdates
 * Checks for updates, if an update is found, then the updater dialog will show
 * up, otherwise, no notification shall be showed
 */

#if defined(UPDATE_CHECKER)
void MainWindow::autoCheckForUpdates()
{
    m_updater.installEventFilter(this);
    m_updater.setShowWindowDisable(m_dontShowUpdateWindow);
    m_updater.checkForUpdates(false);
}
#endif

void MainWindow::groupHourlyTimerTriggered()
{
#if defined(UPDATE_CHECKER)
    autoCheckForUpdates();
#endif
    sendAnonymousAnalytics();
}

/*!
 * \brief MainWindow::setupSearchEdit
 * Set the lineedit to start a bit to the right and end a bit to the left (pedding)
 */
void MainWindow::setupSearchEdit()
{
    //    QLineEdit* searchEdit = m_searchEdit;

    m_searchEdit->setAttribute(Qt::WA_MacShowFocusRect, 0);

    QFont fontAwesomeIcon("Font Awesome 6 Free Solid");
#if defined(Q_OS_MACOS)
    int pointSizeOffset = 0;
#else
    int pointSizeOffset = -4;
#endif

    // clear button
    m_clearButton = new QToolButton(m_searchEdit);
    fontAwesomeIcon.setPointSize(15 + pointSizeOffset);
    m_clearButton->setStyleSheet("QToolButton { color: rgb(114, 114, 114) }");
    m_clearButton->setFont(fontAwesomeIcon);
    m_clearButton->setText(u8"\uf057"); // fa-circle-xmark
    m_clearButton->setCursor(Qt::ArrowCursor);
    m_clearButton->hide();

    // search button
    m_searchButton = new QToolButton(m_searchEdit);
    fontAwesomeIcon.setPointSize(9 + pointSizeOffset);
    m_searchButton->setStyleSheet("QToolButton { color: rgb(205, 205, 205) }");
    m_searchButton->setFont(fontAwesomeIcon);
    m_searchButton->setText(u8"\uf002"); // fa-magnifying-glass
    m_searchButton->setCursor(Qt::ArrowCursor);

    // layout
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::RightToLeft, m_searchEdit);
    layout->setContentsMargins(2, 0, 3, 0);
    layout->addWidget(m_clearButton);
    layout->addStretch();
    layout->addWidget(m_searchButton);
    m_searchEdit->setLayout(layout);

    m_searchEdit->installEventFilter(this);
}

void MainWindow::setupSubscrirptionWindow()
{
    // createSubscriptionWindow();
    // m_subscriptionWindow->hide();

    connect(this, &MainWindow::proVersionCheck, this, [this]() {
        m_buyOrManageSubscriptionAction->setVisible(true);
        if (m_isProVersionActivated) {
            m_buyOrManageSubscriptionAction->setText("&Manage Subscription");
        } else {
            m_buyOrManageSubscriptionAction->setText("&Become a Pro user"); //Upgrade to Daino Notes Pro
        }
    });

    verifyLicenseSignalsSlots();
}

void MainWindow::createSubscriptionWindow()
{
    if (m_subscriptionWindow != nullptr)
        return;

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    const QUrl url("qrc:/qt/qml/SubscriptionWindow.qml");
#else
    const QUrl url("qrc:/qml/SubscriptionWindow.qml");
#endif
    m_subscriptionWindowEngine = new QQmlApplicationEngine(this);
    m_subscriptionWindowEngine->rootContext()->setContextProperty("mainWindow", this);
    m_subscriptionWindowEngine->load(url);
    QObject *rootObject = m_subscriptionWindowEngine->rootObjects().first();
    m_subscriptionWindow = qobject_cast<QWindow *>(rootObject);

    emit themeChanged(QVariant(m_currentThemeData));
    emit proVersionCheck(QVariant(m_isProVersionActivated));
    emit subscriptionStatusChanged(QVariant(m_subscriptionStatus));
    setCurrentFontBasedOnTypeface(m_currentFontTypeface);
#if defined(Q_OS_WINDOWS)
    emit platformSet(QVariant(QString("Windows")));
#elif defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    emit platformSet(QVariant(QString("Unix")));
#elif defined(Q_OS_MACOS)
    emit platformSet(QVariant(QString("Apple")));
#endif
    QJsonObject dataToSendToView{ { "displayFont",
                                    QFont(QFontInfo(QApplication::font()).family()).exactMatch()
                                            ? QFontInfo(QApplication::font()).family()
                                            : QStringLiteral("Inter") } };
    emit displayFontSet(QVariant(dataToSendToView));

    connect(m_subscriptionWindow, &QWindow::visibilityChanged, this, [this](bool isVisible) {
        if (!isVisible) {
            m_subscriptionWindow->deleteLater();
            m_subscriptionWindowEngine->deleteLater();
            m_subscriptionWindow = nullptr;
            m_subscriptionWindowEngine = nullptr;
        }
    });
}

void MainWindow::setupEditorSettings()
{
    // SubscriptionStatus::registerEnum("nuttyartist.notes", 1, 0);
    FontTypeface::registerEnum("nuttyartist.notes", 1, 0);
    FontSizeAction::registerEnum("nuttyartist.notes", 1, 0);
    EditorTextWidth::registerEnum("nuttyartist.notes", 1, 0);
    Theme::registerEnum("nuttyartist.notes", 1, 0);
    View::registerEnum("nuttyartist.notes", 1, 0);

    QJsonObject dataToSendToView{ { "displayFont",
                                    QFont(QFontInfo(QApplication::font()).family()).exactMatch()
                                            ? QFontInfo(QApplication::font()).family()
                                            : QStringLiteral("Inter") } };
    emit displayFontSet(QVariant(dataToSendToView));

#if defined(Q_OS_WINDOWS)
    emit platformSet(QVariant(QString("Windows")));
#elif defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    emit platformSet(QVariant(QString("Unix")));
#elif defined(Q_OS_MACOS)
    emit platformSet(QVariant(QString("Apple")));
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    emit qtVersionSet(QVariant(6));
#else
    emit qtVersionSet(QVariant(5));
#endif
}

/*!
 * \brief MainWindow::setCurrentFontBasedOnTypeface
 * Set the current font based on a given typeface
 */
void MainWindow::setCurrentFontBasedOnTypeface(FontTypeface::Value selectedFontTypeFace)
{
    m_currentFontTypeface = selectedFontTypeFace;
    switch (selectedFontTypeFace) {
    case FontTypeface::Mono:
        m_currentFontFamily = m_listOfMonoFonts.at(m_chosenMonoFontIndex);
        break;
    case FontTypeface::Serif:
        m_currentFontFamily = m_listOfSerifFonts.at(m_chosenSerifFontIndex);
        break;
    case FontTypeface::SansSerif:
        m_currentFontFamily = m_listOfSansSerifFonts.at(m_chosenSansSerifFontIndex);
        break;
    }

    m_currentFontPointSize = m_editorMediumFontSize;

    m_currentSelectedFont = QFont(m_currentFontFamily, m_currentFontPointSize, QFont::Normal);

    QJsonObject dataToSendToView;
    dataToSendToView["listOfSansSerifFonts"] = QJsonArray::fromStringList(m_listOfSansSerifFonts);
    dataToSendToView["listOfSerifFonts"] = QJsonArray::fromStringList(m_listOfSerifFonts);
    dataToSendToView["listOfMonoFonts"] = QJsonArray::fromStringList(m_listOfMonoFonts);
    dataToSendToView["chosenSansSerifFontIndex"] = m_chosenSansSerifFontIndex;
    dataToSendToView["chosenSerifFontIndex"] = m_chosenSerifFontIndex;
    dataToSendToView["chosenMonoFontIndex"] = m_chosenMonoFontIndex;
    dataToSendToView["currentFontTypeface"] = to_string(m_currentFontTypeface).c_str();
    dataToSendToView["currentFontPointSize"] = m_currentFontPointSize;
    dataToSendToView["currentEditorTextColor"] = m_currentEditorTextColor.name();
    QJsonObject currentCharsLimitPerFontType = { { "Mono", m_currentCharsLimitPerFont.mono },
                                                 { "Serif", m_currentCharsLimitPerFont.serif },
                                                 { "SansSerif",
                                                   m_currentCharsLimitPerFont.sansSerif } };
    dataToSendToView["currentCharsLimitPerFontTypeface"] = currentCharsLimitPerFontType;
    emit fontsChanged(QVariant(dataToSendToView));
}

/*!
 * \brief MainWindow::resetEditorSettings
 * Reset editor settings to default options
 */
void MainWindow::resetEditorSettings()
{
    m_currentFontTypeface = FontTypeface::SansSerif;
    m_chosenMonoFontIndex = 0;
    m_chosenSerifFontIndex = 0;
    m_chosenSansSerifFontIndex = 0;
#ifdef __APPLE__
    m_editorMediumFontSize = 15;
#else
    m_editorMediumFontSize = 11;
#endif
    m_currentFontPointSize = m_editorMediumFontSize;
    m_currentCharsLimitPerFont.mono = 64;
    m_currentCharsLimitPerFont.serif = 80;
    m_currentCharsLimitPerFont.sansSerif = 80;
    m_currentTheme = Theme::Light;

    setCurrentFontBasedOnTypeface(m_currentFontTypeface);
    setTheme(m_currentTheme);
}

void MainWindow::setupTransitionToDainoNotesWindow()
{
    bool isWhatsNewInDainoNotesShownAlready = m_settingsDatabase->value(QStringLiteral("isWhatsNewInDainoNotes3ShownAlready"), false).toBool();
    if (!isWhatsNewInDainoNotesShownAlready && !m_isFirstRun) {
        bool isFromPlume = false;
        QString exampleNotesAddedAtVersion = m_settingsDatabase->value(QStringLiteral("example_notes_added_at_version"), "FALSE").toString();
        if (exampleNotesAddedAtVersion != "FALSE") {
            int major = exampleNotesAddedAtVersion.split(".").first().toInt();
            if (major < 1) { // i.e. 0.x.x
                isFromPlume = true;
            }
        }

        createTransitionToDainoNotesWindow(isFromPlume);
#if defined(Q_OS_WINDOWS)
        emit platformSet(QVariant(QString("Windows")));
#elif defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
        emit platformSet(QVariant(QString("Unix")));
#elif defined(Q_OS_MACOS)
        emit platformSet(QVariant(QString("Apple")));
#endif

        m_settingsDatabase->setValue(QStringLiteral("isWhatsNewInDainoNotes3ShownAlready"), true);
    }

    if (m_isFirstRun) {
        m_settingsDatabase->setValue(QStringLiteral("isWhatsNewInDainoNotes3ShownAlready"), true);
    }
}

void MainWindow::createTransitionToDainoNotesWindow(bool isFromPlume=false)
{
    if (m_transitionToDainoNotesWindow != nullptr)
        return;

    const QUrl url("qrc:/qt/qml/TransitionToDainoNotesWindow.qml");
    m_transitionToDainoNotesEngine = new QQmlApplicationEngine(this);
    m_transitionToDainoNotesEngine->rootContext()->setContextProperty("mainWindow", this);
    m_transitionToDainoNotesEngine->load(url);
    if (isFromPlume)
        emit isPlumeSet(isFromPlume);
    QObject *rootObject = m_transitionToDainoNotesEngine->rootObjects().first();
    m_transitionToDainoNotesWindow = qobject_cast<QWindow *>(rootObject);

    connect(m_transitionToDainoNotesWindow, &QWindow::visibilityChanged, this, [this](bool isVisible) {
        if (!isVisible) {
            m_transitionToDainoNotesWindow->deleteLater();
            m_transitionToDainoNotesEngine->deleteLater();
            m_transitionToDainoNotesWindow = nullptr;
            m_transitionToDainoNotesEngine = nullptr;
        }
    });
}

/*!
 * \brief MainWindow::setupTextEdit
 * Setting up textEdit:
 * Setup the style of the scrollBar and set textEdit background to an image
 * Make the textEdit pedding few pixels right and left, to compel with a beautiful proportional grid
 * And install this class event filter to catch when text edit is having focus
 */
void MainWindow::setupTextEdit()
{
#ifdef __APPLE__
    if (QFont("Helvetica Neue").exactMatch()) {
        m_listOfSansSerifFonts.push_front("Helvetica Neue");
    } else if (QFont("Helvetica").exactMatch()) {
        m_listOfSansSerifFonts.push_front("Helvetica");
    }

    if (QFont(QFontInfo(QApplication::font()).family()).exactMatch()) {
        m_listOfSansSerifFonts.push_front(QFontInfo(QApplication::font()).family());
    }

    if (QFont("SF Pro Text").exactMatch()) {
        m_listOfSansSerifFonts.push_front("SF Pro Text");
    }

    if (QFont("Avenir Next").exactMatch()) {
        m_listOfSansSerifFonts.push_front("Avenir Next");
    } else if (QFont("Avenir").exactMatch()) {
        m_listOfSansSerifFonts.push_front("Avenir");
    }
#elif _WIN32
    if (QFont("Calibri").exactMatch())
        m_listOfSansSerifFonts.push_front("Calibri");

    if (QFont("Arial").exactMatch())
        m_listOfSansSerifFonts.push_front("Arial");

    if (QFont("Segoe UI").exactMatch())
        m_listOfSansSerifFonts.push_front("Segoe UI");
#endif
}

void MainWindow::setupBlockEditorView()
{
    qmlRegisterSingletonType<MouseEventSpy>("MouseEventSpy", 1, 0, "MouseEventSpy", MouseEventSpy::singletonProvider);
    qmlRegisterType<DeclarativeDropArea>("nuttyartist.notes.draganddrop", 1, 0, "DropArea");
    qmlRegisterType<DeclarativeDragArea>("nuttyartist.notes.draganddrop", 1, 0, "DragArea");
    SubscriptionStatus::registerEnum("nuttyartist.notes", 1, 0);
    qmlRegisterSingletonInstance("com.company.BlockModel", 1, 0, "BlockModel", m_blockModel);
    qmlRegisterType<BlockInfo>("nuttyartist.notes", 1, 0, "BlockInfo");
    qmlRegisterType<BlockModel>("nuttyartist.notes", 1, 0, "BlockModel");
    qmlRegisterType<MarkdownHighlighter>("MarkdownHighlighter", 1, 0, "MarkdownHighlighter");

    QUrl source("qrc:/qt/qml/EditorMain.qml");
    m_blockEditorQuickView.rootContext()->setContextProperty("noteEditorLogic", m_noteEditorLogic);
    m_blockEditorQuickView.rootContext()->setContextProperty("mainWindow", this);
    m_blockEditorQuickView.setSource(source);
    m_blockEditorQuickView.setResizeMode(QQuickView::SizeRootObjectToView);
    m_blockEditorWidget = QWidget::createWindowContainer(&m_blockEditorQuickView);
    m_blockEditorWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_blockEditorWidget->setFocusPolicy(Qt::StrongFocus);
    m_blockEditorWidget->show();
    ui->verticalLayout_textEdit->insertWidget(0, m_blockEditorWidget);
#if defined(Q_OS_WINDOWS)
    emit platformSet(QVariant(QString("Windows")));
#elif defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    emit platformSet(QVariant(QString("Unix")));
#elif defined(Q_OS_MACOS)
    emit platformSet(QVariant(QString("Apple")));
#endif

    QJsonObject dataToSendToView{ { "displayFont",
                                    QFont(QFontInfo(QApplication::font()).family()).exactMatch()
                                            ? QFontInfo(QApplication::font()).family()
                                            : QStringLiteral("Inter") } };
    emit displayFontSet(QVariant(dataToSendToView));

    connect(this, &MainWindow::proVersionCheck, m_blockModel, &BlockModel::onProVersionCheck);
}

/*!
 * \brief MainWindow::initializeSettingsDatabase
 */
void MainWindow::initializeSettingsDatabase()
{
    // Why are we not updating the app version in Settings?
    if (m_settingsDatabase->value(QStringLiteral("version"), "NULL") == "NULL")
        m_settingsDatabase->setValue(QStringLiteral("version"), qApp->applicationVersion());

#if defined(UPDATE_CHECKER)
    if (m_settingsDatabase->value(QStringLiteral("dontShowUpdateWindow"), "NULL") == "NULL")
        m_settingsDatabase->setValue(QStringLiteral("dontShowUpdateWindow"),
                                     m_dontShowUpdateWindow);
#endif

    if (m_settingsDatabase->value(QStringLiteral("windowGeometry"), "NULL") == "NULL") {
        int initWidth = 1106;
        int initHeight = 694;
        QPoint center = qApp->primaryScreen()->geometry().center();
        QRect rect(center.x() - initWidth / 2, center.y() - initHeight / 2, initWidth, initHeight);
        setGeometry(rect);
        m_settingsDatabase->setValue(QStringLiteral("windowGeometry"), saveGeometry());
    }

    if (m_settingsDatabase->value(QStringLiteral("splitterSizes"), "NULL") == "NULL") {
        m_splitter->resize(width() - 2 * m_layoutMargin, height() - 2 * m_layoutMargin);
        updateFrame();
        m_settingsDatabase->setValue(QStringLiteral("splitterSizes"), m_splitter->saveState());
    }
}

/*!
 * \brief MainWindow::setActivationSuccessful
 */
void MainWindow::setActivationSuccessful(QString licenseKey, bool removeGracePeriodStartedDate)
{
    m_isProVersionActivated = true;
    emit proVersionCheck(QVariant(m_isProVersionActivated));
    m_localLicenseData->setValue(QStringLiteral("isLicenseActivated"), true);
    m_localLicenseData->setValue(QStringLiteral("licenseKey"), licenseKey);
    if (removeGracePeriodStartedDate && m_localLicenseData->contains("gracePeriodStartedDate"))
        m_localLicenseData->remove("gracePeriodStartedDate");
    m_aboutWindow.setProVersion(m_isProVersionActivated);
    m_localLicenseData->sync();
}

/*!
 * \brief MainWindow::getPaymentDetails
 */
void MainWindow::getPaymentDetailsSignalsSlots()
{
    m_netPurchaseDataReplyFirstAttempt = m_netManager->get(m_reqAlt1);

    connect(m_netPurchaseDataReplyFirstAttempt, &QNetworkReply::readyRead, this,
            [this]() { m_dataBuffer->append(m_netPurchaseDataReplyFirstAttempt->readAll()); });
    connect(m_netPurchaseDataReplyFirstAttempt, &QNetworkReply::finished, this, [this]() {
        // Handle error
        if (m_netPurchaseDataReplyFirstAttempt->error() != QNetworkReply::NoError) {
            qDebug() << "Error : " << m_netPurchaseDataReplyFirstAttempt->errorString();
            qDebug() << "Failed first attempt at getting payment data. Trying second...";
            emit tryPurchaseDataSecondAlternative();
            m_netPurchaseDataReplyFirstAttempt->deleteLater();
            return;
        }

        // Handle success
        m_paymentDetails = QJsonDocument::fromJson(*m_dataBuffer).object();
        emit fetchingPaymentDetailsRemotelyFinished();
        m_netPurchaseDataReplyFirstAttempt->deleteLater();
    });
}

/*!
 * \brief MainWindow::getSubscriptionStatus
 */
void MainWindow::getSubscriptionStatus()
{
    // qDebug() << "Checking subscription status...";
    QString validateLicenseEndpoint = m_paymentDetails["purchaseApiBase"].toString()
            + m_paymentDetails["validateLicenseEndpoint"].toString();
    QNetworkRequest request{ QUrl(validateLicenseEndpoint) };
    QJsonObject licenseDataObject;
    licenseDataObject["license_key"] = m_userLicenseKey;
    QJsonDocument licenseDataDoc(licenseDataObject);
    QByteArray postData = licenseDataDoc.toJson();
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = m_netManager->post(request, postData);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        bool skipSettingNotProOnError = false;
        bool showSubscriptionWindowWhenNotPro = true;

        if (reply->error() != QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonObject response = QJsonDocument::fromJson(responseData).object();

            if (response.isEmpty()) {
                // No Internet
                if (!m_localLicenseData->contains(QStringLiteral("gracePeriodStartedDate"))) {
                    // gracePeriodStartedDate not found, so entering new one now
                    m_localLicenseData->setValue(QStringLiteral("gracePeriodStartedDate"),
                                                 QDateTime::currentDateTime());
                    skipSettingNotProOnError = true;
                    setActivationSuccessful(m_userLicenseKey, false);
                    m_subscriptionStatus = SubscriptionStatus::EnteredGracePeriod;
                    // qDebug() << "Entered grace period";
                } else {
                    QDateTime gracePeriodStartedDate =
                            m_localLicenseData
                                    ->value(QStringLiteral("gracePeriodStartedDate"),
                                            QDateTime::currentDateTime())
                                    .toDateTime();
                    QDateTime dateAfterSevenDays =
                            gracePeriodStartedDate.addDays(7); // 7 days grace period offline usage
                    if (QDateTime::currentDateTime() > dateAfterSevenDays) {
                        // Show grace period is over window, you need internt connection
                        m_subscriptionStatus = SubscriptionStatus::GracePeriodOver;
                        // qDebug() << "Grace period over";
                    } else {
                        qDebug() << "Inside grace period";
                        // Inside grace period - make Pro
                        m_subscriptionStatus = SubscriptionStatus::Active;
                        skipSettingNotProOnError = true;
                        setActivationSuccessful(m_userLicenseKey, false);
                        // qDebug() << "Inside grace period";
                    }
                }
            } else if (response.contains("valid") && response["valid"] == false
                       && response.contains("license_key")
                       && response[QStringLiteral("license_key")]
                                       .toObject()[QStringLiteral("status")]
                                       .toString()
                               == "expired") {
                // Show license expired window
                m_subscriptionStatus = SubscriptionStatus::Expired;
                showSubscriptionWindowWhenNotPro = false;
                // qDebug() << "License expired";
            } else if (response.contains("valid") && response["valid"] == false) {
                // Show license not valid window
                m_subscriptionStatus = SubscriptionStatus::Invalid;
                // qDebug() << "License not valid";
            } else {
                // Handle error
                m_subscriptionStatus = SubscriptionStatus::UnknownError;
                // qDebug() << "Unknown error";
            }

            if (!skipSettingNotProOnError) {
                m_isProVersionActivated = false;
                emit proVersionCheck(QVariant(m_isProVersionActivated));
                m_aboutWindow.setProVersion(false);
            }
        } else {
            QByteArray responseData = reply->readAll();
            QJsonObject response = QJsonDocument::fromJson(responseData).object();

            if (response.contains("license_key") && response.contains("valid")
                && response["valid"] == true) {
                if (response[QStringLiteral("license_key")]
                            .toObject()[QStringLiteral("status")]
                            .toString()
                    == "inactive") {
                    int activationUsage = response[QStringLiteral("license_key")]
                                                  .toObject()[QStringLiteral("activation_usage")]
                                                  .toInt();
                    int activationLimit = response[QStringLiteral("license_key")]
                                                  .toObject()[QStringLiteral("activation_limit")]
                                                  .toInt();
                    if (activationUsage >= activationLimit) {
                        // Over activation limit
                        m_subscriptionStatus = SubscriptionStatus::ActivationLimitReached;
                        m_isProVersionActivated = false;
                        emit proVersionCheck(QVariant(m_isProVersionActivated));
                        m_aboutWindow.setProVersion(false);
                        showSubscriptionWindowWhenNotPro = false;
                        // qDebug() << "Activation limit reached";
                    } else {
                        // License valid but not activated yet. Activate
                        // TODO: verify against some device ID
                        QString activateLicenseEndpoint =
                                m_paymentDetails["purchaseApiBase"].toString()
                                + m_paymentDetails["activateLicenseEndpoint"].toString();
                        QNetworkRequest requestActivate{ QUrl(activateLicenseEndpoint) };
                        QJsonObject licenseDataObject2;
                        licenseDataObject2["license_key"] = m_userLicenseKey;
                        licenseDataObject2["instance_name"] = "Daino_Notes_Pro";
                        QJsonDocument licenseDataDoc2(licenseDataObject2);
                        QByteArray postData2 = licenseDataDoc2.toJson();
                        requestActivate.setHeader(QNetworkRequest::ContentTypeHeader,
                                                  "application/json");
                        m_netManager->post(requestActivate, postData2);

                        m_subscriptionStatus = SubscriptionStatus::Active;
                        setActivationSuccessful(m_userLicenseKey);
                        // qDebug() << "Activated and verifying";
                    }
                } else if (response[QStringLiteral("license_key")]
                                   .toObject()[QStringLiteral("status")]
                                   .toString()
                           == "active") {
                    // Lincense is active
                    // TODO: verify against device ID as well
                    m_subscriptionStatus = SubscriptionStatus::Active;
                    setActivationSuccessful(m_userLicenseKey);
                    // qDebug() << "License is valid and active";
                }
            } else {
                // Handle error
                m_subscriptionStatus = SubscriptionStatus::UnknownError;
                m_isProVersionActivated = false;
                emit proVersionCheck(QVariant(m_isProVersionActivated));
                m_aboutWindow.setProVersion(false);
                // qDebug() << "Unknown error";
            }
        }

        // qDebug() << "m_subscriptionStatus: " << m_subscriptionStatus;
        emit subscriptionStatusChanged(QVariant(m_subscriptionStatus));

        if (!m_isProVersionActivated && showSubscriptionWindowWhenNotPro) {
            if (m_subscriptionWindow != nullptr) {
                m_subscriptionWindow->show();
                m_subscriptionWindow->raise();
            } else {
                createSubscriptionWindow();
                m_subscriptionWindow->show();
                m_subscriptionWindow->raise();
            }
        }

        m_localLicenseData->sync();

        if (!m_isFirstRunSubscriptionCheckDone) {
            m_isFirstRunSubscriptionCheckDone = true;
            sendAnonymousAnalytics();
        }
    });
}

/*!
 * \brief MainWindow::verifyLicenseSignalsSlots
 */
void MainWindow::verifyLicenseSignalsSlots()
{
    connect(this, &MainWindow::tryPurchaseDataSecondAlternative, this, [this]() {
        m_dataBuffer->clear();
        m_netPurchaseDataReplySecondAttempt = m_netManager->get(m_reqAlt2);

        connect(m_netPurchaseDataReplySecondAttempt, &QNetworkReply::readyRead, this,
                [this]() { m_dataBuffer->append(m_netPurchaseDataReplySecondAttempt->readAll()); });
        connect(m_netPurchaseDataReplySecondAttempt, &QNetworkReply::finished, this, [this]() {
            // Handle success
            if (m_netPurchaseDataReplySecondAttempt->error() == QNetworkReply::NoError) {
                m_paymentDetails = QJsonDocument::fromJson(*m_dataBuffer).object();
            } else {
                // Handle error - ignore and use defaulte hard-coded/embedded payment data
                qDebug() << "Failed second attempt at getting payment data. Using default embedded "
                            "payment data...";
            }
            emit fetchingPaymentDetailsRemotelyFinished();
            m_netPurchaseDataReplySecondAttempt->deleteLater();
        });
    });

    connect(this, &MainWindow::fetchingPaymentDetailsRemotelyFinished, this, [this]() {
        if (m_paymentDetails.isEmpty()) {
            qDebug() << "Using default embedded payment data";
            QJsonObject paymentDetailsDefault;
            paymentDetailsDefault["purchase_pro_url"] = "https://www.get-notes.com/pricing";
            paymentDetailsDefault["purchaseApiBase"] = "https://www.daino.io/api/daino_notes";
            paymentDetailsDefault["activateLicenseEndpoint"] = "/activateLicense";
            paymentDetailsDefault["validateLicenseEndpoint"] = "/validateLicense";
            // paymentDetailsDefault["deactivateLicenseEndpoint"] = "/none"; // Not used - we use /cancelSubscriptionDainoNotes instead
            m_paymentDetails = paymentDetailsDefault;
        }
        emit gettingPaymentDetailsFinished();
    });

    connect(this, &MainWindow::gettingPaymentDetailsFinished, this,
            [this]() { getSubscriptionStatus(); });
}


/*!
 * \brief MainWindow::checkProVersion
 */
void MainWindow::checkProVersion()
{
#if defined(PRO_VERSION)
    m_isProVersionActivated = true;
    emit proVersionCheck(QVariant(m_isProVersionActivated));
    m_aboutWindow.setProVersion(m_isProVersionActivated);
#else
    m_userLicenseKey = m_localLicenseData->value(QStringLiteral("licenseKey"), "NULL").toString();
    // m_userLicenseKey = "NULL";
    if (m_userLicenseKey != "NULL") {
        if (!m_isLicensedCheckedAfterStartup) {
            emit proVersionCheck(QVariant(true)); // This is so that if the user has a license key, the PRO feature will show until the license is verified
            m_isLicensedCheckedAfterStartup = true;
        }
        m_dataBuffer->clear();

        m_netPurchaseDataReplyFirstAttempt = m_netManager->get(m_reqAlt1);

        connect(m_netPurchaseDataReplyFirstAttempt, &QNetworkReply::readyRead, this,
                [this]() { m_dataBuffer->append(m_netPurchaseDataReplyFirstAttempt->readAll()); });

        connect(m_netPurchaseDataReplyFirstAttempt, &QNetworkReply::finished, this, [this]() {
            if (m_netPurchaseDataReplyFirstAttempt->error() != QNetworkReply::NoError) {
                qDebug() << "Error : " << m_netPurchaseDataReplyFirstAttempt->errorString();
                qDebug() << "Failed first attempt at getting data. Trying second...";
                emit tryPurchaseDataSecondAlternative();
                m_netPurchaseDataReplyFirstAttempt->deleteLater();
                return;
            }

            m_paymentDetails = QJsonDocument::fromJson(*m_dataBuffer).object();
            emit fetchingPaymentDetailsRemotelyFinished();
            m_netPurchaseDataReplyFirstAttempt->deleteLater();
        });
    } else {
        m_isProVersionActivated = false;
        emit proVersionCheck(QVariant(m_isProVersionActivated));
        m_aboutWindow.setProVersion(m_isProVersionActivated);
    }
#endif
}

QVariant MainWindow::getUserLicenseKey()
{
    return QVariant(m_userLicenseKey);
}

void MainWindow::openSubscriptionWindow()
{
    if (m_subscriptionWindow != nullptr) {
        m_subscriptionWindow->show();
        m_subscriptionWindow->raise();
    } else {
        createSubscriptionWindow();
        m_subscriptionWindow->show();
        m_subscriptionWindow->raise();
    }
}


/*!
 * \brief MainWindow::setupDatabases
 * Setting up the database:
 */
void MainWindow::setupDatabases()
{
    m_settingsDatabase =
            new QSettings(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("Awesomeness"),
                          QStringLiteral("Settings"), this);

#if !defined(PRO_VERSION)
#  if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    m_localLicenseData =
            new QSettings(QSettings::NativeFormat, QSettings::UserScope,
                          QStringLiteral("Awesomeness"), QStringLiteral(".dainoNotesLicenseData"), this);
#  else
    m_localLicenseData =
            new QSettings(QSettings::NativeFormat, QSettings::UserScope,
                          QStringLiteral("Awesomeness"), QStringLiteral("dainoNotesLicenseData"), this);

    if (m_localLicenseData->value(QStringLiteral("licenseKey"), "").toString().isEmpty()) {
        // No license key found, check in old Notes
        QSettings oldNotesLicenseData(QSettings::NativeFormat, QSettings::UserScope, QStringLiteral("Awesomeness"), QStringLiteral("notesLicenseData"));
        if (!oldNotesLicenseData.value(QStringLiteral("licenseKey"), "").toString().isEmpty()) {
            // Found license key in old Notes
            // Copy all keys/values to m_localLicenseData
            if (oldNotesLicenseData.contains(QStringLiteral("licenseKey")))
                m_localLicenseData->setValue(QStringLiteral("licenseKey"), oldNotesLicenseData.value(QStringLiteral("licenseKey")));
            if (oldNotesLicenseData.contains(QStringLiteral("isLicenseActivated")))
                m_localLicenseData->setValue(QStringLiteral("isLicenseActivated"), oldNotesLicenseData.value(QStringLiteral("isLicenseActivated")));
            if (oldNotesLicenseData.contains(QStringLiteral("gracePeriodStartedDate")))
                m_localLicenseData->setValue(QStringLiteral("gracePeriodStartedDate"), oldNotesLicenseData.value(QStringLiteral("gracePeriodStartedDate")));
        } /*else { // Don't check Plume license for now
            QSettings plumeLicenseData(QSettings::NativeFormat, QSettings::UserScope, QStringLiteral("Awesomeness"), QStringLiteral("plumeLicenseData"));
            if (!plumeLicenseData.value(QStringLiteral("licenseKey"), "").toString().isEmpty()) {
                // Found license key in Plume
                // Copy all keys/values to m_localLicenseData
                if (plumeLicenseData.contains(QStringLiteral("licenseKey")))
                    m_localLicenseData->setValue(QStringLiteral("licenseKey"), plumeLicenseData.value(QStringLiteral("licenseKey")));
                if (plumeLicenseData.contains(QStringLiteral("isLicenseActivated")))
                    m_localLicenseData->setValue(QStringLiteral("isLicenseActivated"), plumeLicenseData.value(QStringLiteral("isLicenseActivated")));
                if (plumeLicenseData.contains(QStringLiteral("gracePeriodStartedDate")))
                    m_localLicenseData->setValue(QStringLiteral("gracePeriodStartedDate"), plumeLicenseData.value(QStringLiteral("gracePeriodStartedDate")));
            }
        }*/
    }

    m_localLicenseData->sync();
#  endif
#endif

#  if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    m_localAnalyticsData =
            new QSettings(QSettings::NativeFormat, QSettings::UserScope,
                          QStringLiteral("Awesomeness"), QStringLiteral(".dainoNotesAnalyticsData"), this);

#  else
    m_localAnalyticsData =
            new QSettings(QSettings::NativeFormat, QSettings::UserScope,
                          QStringLiteral("Awesomeness"), QStringLiteral("dainoNotesAnalyticsData"), this);
#  endif

    m_settingsDatabase->setFallbacksEnabled(false);
    bool needMigrateFromV1_5_0 = false;
    if (m_settingsDatabase->value(QStringLiteral("version"), "NULL") == "NULL") {
        needMigrateFromV1_5_0 = true;
    }
    auto versionString = m_settingsDatabase->value(QStringLiteral("version")).toString();
    auto major = versionString.split(".").first().toInt();
    if (major < 2) {
        needMigrateFromV1_5_0 = true;
    }
    initializeSettingsDatabase();

    bool doCreate = false;
    QFileInfo fi(m_settingsDatabase->fileName());
    QDir dir(fi.absolutePath());
    bool folderCreated = dir.mkpath(QStringLiteral("."));
    if (!folderCreated)
        qFatal("ERROR: Can't create settings folder : %s",
               dir.absolutePath().toStdString().c_str());
    QString defaultDBPath = dir.path() + QDir::separator() + QStringLiteral("notes.db");

    QString noteDBFilePath =
            m_settingsDatabase->value(QStringLiteral("noteDBFilePath"), QString()).toString();

    if (!QFile::exists(defaultDBPath) && (noteDBFilePath.isEmpty() || !QFile::exists(noteDBFilePath))) {
        m_isFirstRun = true;
    }

    if (noteDBFilePath.isEmpty()) {
        noteDBFilePath = defaultDBPath;
    }
    QFileInfo noteDBFilePathInf(noteDBFilePath);
    QFileInfo defaultDBPathInf(defaultDBPath);
    if ((!noteDBFilePathInf.exists()) && (defaultDBPathInf.exists())) {
        QDir().mkpath(noteDBFilePathInf.absolutePath());
        QFile defaultDBFile(defaultDBPath);
        defaultDBFile.rename(noteDBFilePath);
    }
    if (QFile::exists(noteDBFilePath) && needMigrateFromV1_5_0) {
        {
            auto m_db = QSqlDatabase::addDatabase("QSQLITE", DEFAULT_DATABASE_NAME);
            m_db.setDatabaseName(noteDBFilePath);
            if (m_db.open()) {
                QSqlQuery query(m_db);
                if (query.exec("SELECT name FROM sqlite_master WHERE type='table' AND "
                               "name='tag_table';")) {
                    if (query.next() && query.value(0).toString() == "tag_table") {
                        needMigrateFromV1_5_0 = false;
                    }
                }
                m_db.close();
            }
            m_db = QSqlDatabase::database();
        }
        QSqlDatabase::removeDatabase(DEFAULT_DATABASE_NAME);
    }
    if (!QFile::exists(noteDBFilePath)) {
        QFile noteDBFile(noteDBFilePath);
        if (!noteDBFile.open(QIODevice::WriteOnly))
            qFatal("ERROR : Can't create database file");

        noteDBFile.close();
        doCreate = true;
        needMigrateFromV1_5_0 = false;
    } else if (needMigrateFromV1_5_0) {
        QFile noteDBFile(noteDBFilePath);
        noteDBFile.rename(dir.path() + QDir::separator() + QStringLiteral("oldNotes.db"));
        noteDBFile.setFileName(noteDBFilePath);
        if (!noteDBFile.open(QIODevice::WriteOnly))
            qFatal("ERROR : Can't create database file");

        noteDBFile.close();
        doCreate = true;
    }

    if (needMigrateFromV1_5_0) {
        m_settingsDatabase->setValue(QStringLiteral("version"), qApp->applicationVersion());
    }
    m_dbManager = new DBManager;
    m_dbThread = new QThread;
    m_dbThread->setObjectName(QStringLiteral("dbThread"));
    m_dbManager->moveToThread(m_dbThread);
    connect(m_dbThread, &QThread::started, this, [=]() {
        setTheme(m_currentTheme);
        emit requestOpenDBManager(noteDBFilePath, doCreate);
        if (needMigrateFromV1_5_0) {
            emit requestMigrateNotesFromV1_5_0(dir.path() + QDir::separator()
                                               + QStringLiteral("oldNotes.db"));
        }
    });
    connect(this, &MainWindow::requestOpenDBManager, m_dbManager,
            &DBManager::onOpenDBManagerRequested, Qt::QueuedConnection);
    connect(this, &MainWindow::requestMigrateNotesFromV1_5_0, m_dbManager,
            &DBManager::onMigrateNotesFrom1_5_0Requested, Qt::QueuedConnection);
    connect(m_dbThread, &QThread::finished, m_dbManager, &QObject::deleteLater);

    connect(
            m_dbManager, &DBManager::databaseOpened, this,
            [=]() {
                QString versionExampleNotesAdded =
                        m_settingsDatabase
                                ->value(QStringLiteral("example_notes_added_at_version"), "FALSE")
                                .toString();
                if (!m_dbManager->IsDatabaseHasNotes() || versionExampleNotesAdded == "FALSE") {
                    // If the database has no notes or it has but the example notes were not added
                    // yet then add the example notes
                    m_settingsDatabase->setValue(QStringLiteral("example_notes_added_at_version"),
                                                 qApp->applicationVersion());
                    m_settingsDatabase->sync();
                    m_dbManager->addExampleNotes();
                }
            },
            Qt::QueuedConnection);

    m_dbThread->start();
}

void MainWindow::setupAnonymousAnalytics()
{
    if (m_localAnalyticsData->contains(QStringLiteral("id"))) {
        QUuid id(m_localAnalyticsData->value(QStringLiteral("id")).toString());
        if (id.isNull()) {
            QString newUUID = QUuid::createUuid().toString(QUuid::WithoutBraces);
            m_localAnalyticsData->setValue(QStringLiteral("id"), newUUID);
            m_localAnalyticsData->sync();
        }
    } else {
        QString newUUID = QUuid::createUuid().toString(QUuid::WithoutBraces);
        m_localAnalyticsData->setValue(QStringLiteral("id"), newUUID);
        m_localAnalyticsData->sync();
    }
}

/*!
 * \brief MainWindow::setupModelView
 */
void MainWindow::setupModelView()
{
    m_listView = ui->listView;
    m_tagPool = new TagPool(m_dbManager);
    m_listModel = new NoteListModel(m_listView);
    m_listView->setTagPool(m_tagPool);
    m_listView->setModel(m_listModel);
    m_listViewLogic = new ListViewLogic(m_listView, m_listModel, m_searchEdit, m_clearButton,
                                        m_tagPool, m_dbManager, this);
    m_treeView = static_cast<NodeTreeView *>(ui->treeView);
    m_treeView->setModel(m_treeModel);
    m_treeView->setBlockModel(m_blockModel);
    m_treeViewLogic = new TreeViewLogic(m_treeView, m_treeModel, m_dbManager, m_listView, this);
    m_noteEditorLogic =
            new NoteEditorLogic(m_searchEdit, static_cast<TagListView *>(ui->tagListView),
                                m_tagPool, m_dbManager, m_blockModel, this);
}

/*!
 * \brief MainWindow::restoreStates
 * Restore the latest states (if there are any) of the window and the splitter from
 * the settings database
 */
void MainWindow::restoreStates()
{
    m_isUserAcceptedTermsAndPrivacy = m_settingsDatabase->value(QStringLiteral("isUserAcceptedTermsAndPrivacy"), false).toBool();

    QFileInfo fi(m_settingsDatabase->fileName());
    m_databaseFolderPath = fi.absolutePath();
    m_blockModel->setDatabaseFolderPath(m_databaseFolderPath);

// #if defined(Q_OS_MACOS)
//     bool nativeByDefault = false;
// #else
//     bool nativeByDefault = true;
// #endif
//     setUseNativeWindowFrame(
//             m_settingsDatabase->value(QStringLiteral("useNativeWindowFrame"), nativeByDefault)
//                     .toBool());
    setUseNativeWindowFrame(true);

    setHideToTray(m_settingsDatabase->value(QStringLiteral("hideToTray"), true).toBool());
    if (m_hideToTray) {
        setupTrayIcon();
    }

    if (m_settingsDatabase->value(QStringLiteral("windowGeometry"), "NULL") != "NULL")
        restoreGeometry(m_settingsDatabase->value(QStringLiteral("windowGeometry")).toByteArray());

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    // Set margin to zero if the window is maximized
    if (isMaximized()) {
        setMargins(QMargins());
    }
#endif

#if defined(UPDATE_CHECKER)
    if (m_settingsDatabase->value(QStringLiteral("dontShowUpdateWindow"), "NULL") != "NULL")
        m_dontShowUpdateWindow =
                m_settingsDatabase->value(QStringLiteral("dontShowUpdateWindow")).toBool();
#endif

    m_splitter->setCollapsible(0, true);
    m_splitter->setCollapsible(1, true);
    m_splitter->resize(width() - m_layoutMargin, height() - m_layoutMargin);

    if (m_settingsDatabase->contains(QStringLiteral("splitterSizes"))) {
        m_splitter->restoreState(
                m_settingsDatabase->value(QStringLiteral("splitterSizes")).toByteArray());
        // in rare cases, the splitter sizes can be zero, which causes bugs (issue #531)
        auto splitterSizes = m_splitter->sizes();
        splitterSizes[0] = std::max(splitterSizes[0], m_foldersWidget->minimumWidth());
        splitterSizes[1] = std::max(splitterSizes[1], m_noteListWidget->minimumWidth());
        m_splitter->setSizes(splitterSizes);
    }

    m_foldersWidget->setHidden(
            m_settingsDatabase->value(QStringLiteral("isTreeCollapsed")).toBool());
    m_noteListWidget->setHidden(
            m_settingsDatabase->value(QStringLiteral("isNoteListCollapsed")).toBool());

#if defined(Q_OS_MACOS)
    if (m_foldersWidget->isHidden()) {
        ui->verticalSpacer_upSearchEdit->setMinimumHeight(33);
        ui->verticalSpacer_upSearchEdit->setMaximumHeight(33);
    }
#else
    if (!m_useNativeWindowFrame && m_foldersWidget->isHidden()) {
        ui->verticalSpacer_upSearchEdit->setMinimumHeight(25);
        ui->verticalSpacer_upSearchEdit->setMaximumHeight(25);
    }
#endif

    m_splitter->setCollapsible(0, false);
    m_splitter->setCollapsible(1, false);

    QString selectedFontTypefaceFromDatabase =
            m_settingsDatabase->value(QStringLiteral("selectedFontTypeface"), "NULL").toString();
    if (selectedFontTypefaceFromDatabase != "NULL") {
        if (selectedFontTypefaceFromDatabase == "Mono") {
            m_currentFontTypeface = FontTypeface::Mono;
        } else if (selectedFontTypefaceFromDatabase == "Serif") {
            m_currentFontTypeface = FontTypeface::Serif;
        } else if (selectedFontTypefaceFromDatabase == "SansSerif") {
            m_currentFontTypeface = FontTypeface::SansSerif;
        }
    }

    if (m_settingsDatabase->value(QStringLiteral("editorMediumFontSize"), "NULL") != "NULL") {
        m_editorMediumFontSize =
                m_settingsDatabase->value(QStringLiteral("editorMediumFontSize")).toInt();
    } else {
#ifdef __APPLE__
        m_editorMediumFontSize = 15;
#else
        m_editorMediumFontSize = 11;
#endif
    }
    m_currentFontPointSize = m_editorMediumFontSize;

    if (m_settingsDatabase->value(QStringLiteral("isTextFullWidth"), "NULL") != "NULL") {
        m_isTextFullWidth = m_settingsDatabase->value(QStringLiteral("isTextFullWidth")).toBool();
    } else {
        // Default option, Focus Mode
        m_isTextFullWidth = false;
    }

    if (m_settingsDatabase->value(QStringLiteral("isDistractionFreeMode"), "NULL") != "NULL") {
        m_isDistractionFreeMode = m_settingsDatabase->value(QStringLiteral("isDistractionFreeMode")).toBool();
    } else {
        m_isDistractionFreeMode = false;
    }

    if (m_settingsDatabase->value(QStringLiteral("charsLimitPerFontMono"), "NULL") != "NULL")
        m_currentCharsLimitPerFont.mono =
                m_settingsDatabase->value(QStringLiteral("charsLimitPerFontMono")).toInt();
    if (m_settingsDatabase->value(QStringLiteral("charsLimitPerFontSerif"), "NULL") != "NULL")
        m_currentCharsLimitPerFont.serif =
                m_settingsDatabase->value(QStringLiteral("charsLimitPerFontSerif")).toInt();
    if (m_settingsDatabase->value(QStringLiteral("charsLimitPerFontSansSerif"), "NULL") != "NULL")
        m_currentCharsLimitPerFont.sansSerif =
                m_settingsDatabase->value(QStringLiteral("charsLimitPerFontSansSerif")).toInt();

    if (m_settingsDatabase->value(QStringLiteral("chosenMonoFont"), "NULL") != "NULL") {
        QString fontName = m_settingsDatabase->value(QStringLiteral("chosenMonoFont")).toString();
        int fontIndex = m_listOfMonoFonts.indexOf(fontName);
        if (fontIndex != -1) {
            m_chosenMonoFontIndex = fontIndex;
        }
    }
    if (m_settingsDatabase->value(QStringLiteral("chosenSerifFont"), "NULL") != "NULL") {
        QString fontName = m_settingsDatabase->value(QStringLiteral("chosenSerifFont")).toString();
        int fontIndex = m_listOfSerifFonts.indexOf(fontName);
        if (fontIndex != -1) {
            m_chosenSerifFontIndex = fontIndex;
        }
    }
    if (m_settingsDatabase->value(QStringLiteral("chosenSansSerifFont"), "NULL") != "NULL") {
        QString fontName =
                m_settingsDatabase->value(QStringLiteral("chosenSansSerifFont")).toString();
        int fontIndex = m_listOfSansSerifFonts.indexOf(fontName);
        if (fontIndex != -1) {
            m_chosenSansSerifFontIndex = fontIndex;
        }
    }

    if (m_settingsDatabase->value(QStringLiteral("theme"), "NULL") != "NULL") {
        QString chosenTheme = m_settingsDatabase->value(QStringLiteral("theme")).toString();
        if (chosenTheme == "Light") {
            m_currentTheme = Theme::Light;
        } else if (chosenTheme == "Dark") {
            m_currentTheme = Theme::Dark;
        } else if (chosenTheme == "Sepia") {
            m_currentTheme = Theme::Sepia;
        }
    }

    setCurrentFontBasedOnTypeface(m_currentFontTypeface);
    setTheme(m_currentTheme);

    auto expandedFolder =
            m_settingsDatabase->value(QStringLiteral("currentExpandedFolder"), QStringList{})
                    .toStringList();
    auto isSelectingFolder =
            m_settingsDatabase->value(QStringLiteral("isSelectingFolder"), true).toBool();
    auto currentSelectFolder =
            m_settingsDatabase->value(QStringLiteral("currentSelectFolder"), QString{}).toString();
    auto currentSelectTagsId =
            m_settingsDatabase->value(QStringLiteral("currentSelectTagsId"), QStringList{})
                    .toStringList();
    QSet<int> tags;
    for (const auto &tagId : qAsConst(currentSelectTagsId)) {
        tags.insert(tagId.toInt());
    }
    m_treeViewLogic->setLastSavedState(isSelectingFolder, currentSelectFolder, tags,
                                       expandedFolder);
    auto currentSelectNotes =
            m_settingsDatabase->value(QStringLiteral("currentSelectNotesId"), QStringList{})
                    .toStringList();
    QSet<int> notesId;
    for (const auto &id : qAsConst(currentSelectNotes)) {
        notesId.insert(id.toInt());
    }
    m_listViewLogic->setLastSavedState(notesId);

    updateSelectedOptionsEditorSettings();
    updateFrame();
}

void MainWindow::changeEditorTextWidthFromStyleButtons(EditorTextWidth::Value editorTextWidth)
{
    switch (editorTextWidth) {
    case EditorTextWidth::TextWidthFullWidth:
        m_isTextFullWidth = !m_isTextFullWidth;
        break;
    case EditorTextWidth::TextWidthIncrease:
        switch (m_currentFontTypeface) {
        case FontTypeface::Mono:
            m_currentCharsLimitPerFont.mono = m_currentCharsLimitPerFont.mono + 1;
            break;
        case FontTypeface::Serif:
            m_currentCharsLimitPerFont.serif = m_currentCharsLimitPerFont.serif + 1;
            break;
        case FontTypeface::SansSerif:
            m_currentCharsLimitPerFont.sansSerif = m_currentCharsLimitPerFont.sansSerif + 1;
            break;
        }
        break;
    case EditorTextWidth::TextWidthDecrease:
        switch (m_currentFontTypeface) {
        case FontTypeface::Mono:
            m_currentCharsLimitPerFont.mono -= 1;
            break;
        case FontTypeface::Serif:
            m_currentCharsLimitPerFont.serif -= 1;
            break;
        case FontTypeface::SansSerif:
            m_currentCharsLimitPerFont.sansSerif -= 1;
            break;
        }
        break;
    }

    setCurrentFontBasedOnTypeface(m_currentFontTypeface);

    updateSelectedOptionsEditorSettings();
}

void MainWindow::checkIfUserAcceptedTermsAndPrivacy()
{
    if (!m_isUserAcceptedTermsAndPrivacy) {
        QString message = "By continuing you acknowledge that you have read and agree to Daino Notes "
                          "<a href='https://www.get-notes.com/notes-app-terms'>Terms of Use</a> and "
                          "<a href='https://www.get-notes.com/notes-app-privacy-policy'>Privacy Policy</a>";
        CustomMessageBox *msgBox = new CustomMessageBox(message, this);
        msgBox->setWindowModality(Qt::ApplicationModal);
        connect(msgBox, &CustomMessageBox::messageBoxClosed, [=](bool accepted) {
            if (accepted) {
                qDebug() << "User accepted the terms of use and privacy policy";
                m_isUserAcceptedTermsAndPrivacy = true;
                QTimer::singleShot(2000, this, [=]() {
                    sendAnonymousAnalytics();
                });
                m_settingsDatabase->setValue("isUserAcceptedTermsAndPrivacy", true);
                m_settingsDatabase->sync();
                msgBox->deleteLater();
            } else {
                qDebug() << "User declined the terms of use and privacy policy";
                m_settingsDatabase->setValue("isUserAcceptedTermsAndPrivacy", false);
                m_settingsDatabase->sync();
                QCoreApplication::quit();
            }
        });
        msgBox->show();
    }
}

/*!
 * \brief MainWindow::setButtonsAndFieldsEnabled
 * \param doEnable
 */
void MainWindow::setButtonsAndFieldsEnabled(bool doEnable)
{
    m_newNoteButton->setEnabled(doEnable);
    m_searchEdit->setEnabled(doEnable);
    m_globalSettingsButton->setEnabled(doEnable);
}

/*!
 * \brief MainWindow::setEditorSettingsFromQuickViewVisibility
 * \param isVisible
 */
void MainWindow::setEditorSettingsFromQuickViewVisibility(bool isVisible)
{
    m_isEditorSettingsFromQuickViewVisible = isVisible;
}

/*!
 * \brief MainWindow::setEditorSettingsScrollBarPosition
 * \param isVisible
 */
void MainWindow::setEditorSettingsScrollBarPosition(double position)
{
    emit editorSettingsScrollBarPositionChanged(QVariant(position));
}

/*!
 * \brief MainWindow::onNewNoteButtonClicked
 * Create a new note when clicking the 'new note' button
 */
void MainWindow::onNewNoteButtonClicked(bool isCalledFromShortcut)
{
    if (!m_newNoteButton->isVisible()) {
        return;
    }
    if (m_listViewLogic->isAnimationRunning()) {
        return;
    }

    // save the data of the previous selected
    m_noteEditorLogic->saveNoteToDB();

    if (!m_searchEdit->text().isEmpty()) {
        m_listViewLogic->clearSearch(true);
    } else {
        createNewNote(isCalledFromShortcut);
    }
}

/*!
 * \brief MainWindow::setupGlobalSettingsMenu
 */
void MainWindow::setupGlobalSettingsMenu()
{
    QMenu *importExportNotesMenu = m_mainMenu.addMenu(tr("&Import/Export Notes"));
    importExportNotesMenu->setToolTipsVisible(true);
    m_mainMenu.setToolTipsVisible(true);

    QShortcut *closeMenu = new QShortcut(Qt::Key_F10, &m_mainMenu);
    closeMenu->setContext(Qt::ApplicationShortcut);
    connect(closeMenu, &QShortcut::activated, &m_mainMenu, &QMenu::close);

#if defined(Q_OS_WINDOWS) || defined(Q_OS_WIN)
    setStyleSheet(m_styleSheet);
    setCSSClassesAndUpdate(&m_mainMenu, "menu");
#endif

#ifdef __APPLE__
    m_mainMenu.setFont(QFont(m_displayFont, 13));
    importExportNotesMenu->setFont(QFont(m_displayFont, 13));
#else
        m_mainMenu.setFont(QFont(m_displayFont, 10, QFont::Normal));
        importExportNotesMenu->setFont(QFont(m_displayFont, 10, QFont::Normal));
#endif

#if defined(UPDATE_CHECKER)
    // Check for update action
    QAction *checkForUpdatesAction = m_mainMenu.addAction(tr("Check For &Updates"));
    connect(checkForUpdatesAction, &QAction::triggered, this, &MainWindow::checkForUpdates);
#endif

           // Autostart
    QAction *autostartAction = m_mainMenu.addAction(tr("&Start automatically"));
    connect(autostartAction, &QAction::triggered, this,
            [=]() { m_autostart.setAutostart(autostartAction->isChecked()); });
    autostartAction->setCheckable(true);
    autostartAction->setChecked(m_autostart.isAutostart());

           // hide to tray
    QAction *hideToTrayAction = m_mainMenu.addAction(tr("&Hide to tray"));
    connect(hideToTrayAction, &QAction::triggered, this, [=]() {
        m_settingsDatabase->setValue(QStringLiteral("hideToTray"), hideToTrayAction->isChecked());
    });
    hideToTrayAction->setCheckable(true);
    hideToTrayAction->setChecked(m_hideToTray);
    connect(hideToTrayAction, &QAction::triggered, this, [this]() {
        setHideToTray(!m_hideToTray);
        if (m_hideToTray) {
            setupTrayIcon();
        } else {
            m_trayIcon->hide();
        }
    });

    QAction *changeDBPathAction = m_mainMenu.addAction(tr("&Change database path"));
    connect(changeDBPathAction, &QAction::triggered, this, [=]() {
        auto btn = QMessageBox::question(this, "Are you sure you want to change the database path?",
                                         "Are you sure you want to change the database path?");
        if (btn == QMessageBox::Yes) {
            auto newDbPath = QFileDialog::getSaveFileName(this, "New Database path", "notes.db");
            if (!newDbPath.isEmpty()) {
                m_settingsDatabase->setValue(QStringLiteral("noteDBFilePath"), newDbPath);
                QFileInfo noteDBFilePathInf(newDbPath);
                QDir().mkpath(noteDBFilePathInf.absolutePath());
                emit requestChangeDatabasePath(newDbPath);
            }
        }
    });

    QAction *whatsNewInDainoNotesAction = m_mainMenu.addAction(tr("&See what's new in Daino Notes"));
    connect(whatsNewInDainoNotesAction, &QAction::triggered, this, [&]() {
        emit isPlumeSet(false);
        if (m_transitionToDainoNotesWindow != nullptr) {
            m_transitionToDainoNotesWindow->show();
            m_transitionToDainoNotesWindow->raise();
        } else {
            createTransitionToDainoNotesWindow(false);
            emit themeChanged(QVariant(m_currentThemeData));
            emit proVersionCheck(QVariant(m_isProVersionActivated));
            setCurrentFontBasedOnTypeface(m_currentFontTypeface);
#if defined(Q_OS_WINDOWS)
            emit platformSet(QVariant(QString("Windows")));
#elif defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
            emit platformSet(QVariant(QString("Unix")));
#elif defined(Q_OS_MACOS)
            emit platformSet(QVariant(QString("Apple")));
#endif
            m_transitionToDainoNotesWindow->show();
        }
    });

           // About Notes
    QAction *aboutAction = m_mainMenu.addAction(tr("&About Daino Notes"));
    connect(aboutAction, &QAction::triggered, this, [&]() { m_aboutWindow.show(); });

    m_mainMenu.addSeparator();

#if !defined(PRO_VERSION)
    // Buy/Manage subscription
    m_buyOrManageSubscriptionAction = m_mainMenu.addAction(
            tr(m_isProVersionActivated ? "&Manage Subscription" : "&Become a Pro user")); //Upgrade to Daino Notes Pro"));
    m_buyOrManageSubscriptionAction->setVisible(false);
    connect(m_buyOrManageSubscriptionAction, &QAction::triggered, this,
            &MainWindow::openSubscriptionWindow);

    m_mainMenu.addSeparator();
#endif

           // Close the app
    QAction *quitAppAction = m_mainMenu.addAction(tr("&Quit"));
    connect(quitAppAction, &QAction::triggered, this, &MainWindow::QuitApplication);

           // Import notes from plain text actions
    QAction *importNotesPlainTextAction = importExportNotesMenu->addAction(tr("&Import .txt/.md"));
    importNotesPlainTextAction->setToolTip(tr("Import notes from .txt or .md files"));
    connect(importNotesPlainTextAction, &QAction::triggered, this, &MainWindow::importPlainTextFiles);

    QAction *exportNotesToPlainTextAction = importExportNotesMenu->addAction(tr("&Export to .txt"));
    exportNotesToPlainTextAction->setToolTip(tr("Export notes to .txt files\nNote: If you wish to backup your notes,\nuse the .nbk file format instead of .txt/.md"));
    connect(exportNotesToPlainTextAction, &QAction::triggered, this, [this](){
        exportToPlainTextFiles(".txt");
    });

    QAction *exportNotesToMarkdownAction = importExportNotesMenu->addAction(tr("&Export to .md"));
    exportNotesToMarkdownAction->setToolTip(tr("Export notes to .md files\nNote: If you wish to backup your notes,\nuse the .nbk file format instead of .txt/.md"));
    connect(exportNotesToMarkdownAction, &QAction::triggered, this, [this](){
        exportToPlainTextFiles(".md");
    });

    importExportNotesMenu->addSeparator();

           // Export notes action
    QAction *exportNotesFileAction = importExportNotesMenu->addAction(tr("&Export to .nbk"));
    exportNotesFileAction->setToolTip(tr("Save all notes to a .nbk file"));
    connect(exportNotesFileAction, &QAction::triggered, this, &MainWindow::exportNotesFile);

           // Import notes action
    QAction *importNotesFileAction = importExportNotesMenu->addAction(tr("&Import .nbk"));
    importNotesFileAction->setToolTip(tr("Add notes from a .nbk file"));
    connect(importNotesFileAction, &QAction::triggered, this, &MainWindow::importNotesFile);

           // Restore notes action
    QAction *restoreNotesFileAction = importExportNotesMenu->addAction(tr("&Restore from .nbk"));
    restoreNotesFileAction->setToolTip(tr("Replace all notes with notes from a .nbk file"));
    connect(restoreNotesFileAction, &QAction::triggered, this, &MainWindow::restoreNotesFile);
}

/*!
 * \brief MainWindow::onGlobalSettingsButtonClicked
 * Open up the menu when clicking the global settings button
 */
void MainWindow::onGlobalSettingsButtonClicked()
{
    m_mainMenu.exec(
            m_globalSettingsButton->mapToGlobal(QPoint(0, m_globalSettingsButton->height())));
}

/*!
 * \brief MainWindow::updateSelectedOptionsEditorSettings
 */
void MainWindow::updateSelectedOptionsEditorSettings()
{
    QJsonObject dataToSendToView;
    dataToSendToView["currentFontTypeface"] = to_string(m_currentFontTypeface).c_str();
    dataToSendToView["currentTheme"] = to_string(m_currentTheme).c_str();
    dataToSendToView["isTextFullWidth"] = m_isTextFullWidth;
    dataToSendToView["isNoteListCollapsed"] = m_noteListWidget->isHidden();
    dataToSendToView["isFoldersTreeCollapsed"] = m_foldersWidget->isHidden();
    dataToSendToView["isMarkdownDisabled"] = !m_noteEditorLogic->markdownEnabled();
    dataToSendToView["isStayOnTop"] = m_alwaysStayOnTop;
    dataToSendToView["isDistractionFreeMode"] = m_isDistractionFreeMode;
    emit settingsChanged(QVariant(dataToSendToView));
}

/*!
 * \brief MainWindow::changeEditorFontTypeFromStyleButtons
 * Change the font based on the type passed from the Style Editor Window
 */
void MainWindow::changeEditorFontTypeFromStyleButtons(FontTypeface::Value fontTypeface,
                                                      int chosenFontIndex)
{
    if (chosenFontIndex < 0)
        return;

    switch (fontTypeface) {
    case FontTypeface::Mono:
        if (chosenFontIndex > m_listOfMonoFonts.size() - 1)
            return;
        m_chosenMonoFontIndex = chosenFontIndex;
        break;
    case FontTypeface::Serif:
        if (chosenFontIndex > m_listOfSerifFonts.size() - 1)
            return;
        m_chosenSerifFontIndex = chosenFontIndex;
        break;
    case FontTypeface::SansSerif:
        if (chosenFontIndex > m_listOfSansSerifFonts.size() - 1)
            return;
        m_chosenSansSerifFontIndex = chosenFontIndex;
        break;
    }

    setCurrentFontBasedOnTypeface(fontTypeface);

    updateSelectedOptionsEditorSettings();
}

/*!
 * \brief MainWindow::changeEditorFontSizeFromStyleButtons
 * Change the font size based on the button pressed in the Style Editor Window
 * Increase / Decrease
 */
void MainWindow::changeEditorFontSizeFromStyleButtons(FontSizeAction::Value fontSizeAction)
{
    switch (fontSizeAction) {
    case FontSizeAction::FontSizeIncrease:
        m_editorMediumFontSize += 1;
        setCurrentFontBasedOnTypeface(m_currentFontTypeface);
        break;
    case FontSizeAction::FontSizeDecrease:
        m_editorMediumFontSize -= 1;
        setCurrentFontBasedOnTypeface(m_currentFontTypeface);
        break;
    }
    setTheme(m_currentTheme);
}

void MainWindow::changeEditorDistractionFreeModeFromStyleButtons(bool isDistractionFreeMode)
{
    m_isDistractionFreeMode = isDistractionFreeMode;
    updateSelectedOptionsEditorSettings();
}

/*!
 * \brief MainWindow::setTheme
 * Changes the app theme
 */
void MainWindow::setTheme(Theme::Value theme)
{
    m_currentTheme = theme;

    setCSSThemeAndUpdate(this, theme);
    setCSSThemeAndUpdate(ui->verticalSpacer_upSearchEdit, theme);
    setCSSThemeAndUpdate(ui->verticalSpacer_upSearchEdit2, theme);
    setCSSThemeAndUpdate(ui->listviewLabel1, theme);
    setCSSThemeAndUpdate(ui->searchEdit, theme);
    setCSSThemeAndUpdate(ui->verticalSplitterLine_left, theme);
    setCSSThemeAndUpdate(ui->verticalSplitterLine_middle, theme);
    setCSSThemeAndUpdate(ui->globalSettingsButton, theme);
    setCSSThemeAndUpdate(ui->toggleTreeViewButton, theme);
    setCSSThemeAndUpdate(ui->newNoteButton, theme);
    setCSSThemeAndUpdate(ui->frameLeft, theme);
    setCSSThemeAndUpdate(ui->frameMiddle, theme);

    m_blockModel->setTheme(theme);

    switch (theme) {
    case Theme::Light: {
        QJsonObject themeData{ { "theme", QStringLiteral("Light") },
                               { "backgroundColor", "#ffffff" } };
        m_currentThemeData = themeData;
        emit themeChanged(QVariant(themeData));
        m_currentEditorTextColor = QColor(26, 26, 26);
        m_searchButton->setStyleSheet("QToolButton { color: rgb(205, 205, 205) }");
        m_clearButton->setStyleSheet("QToolButton { color: rgb(114, 114, 114) }");
        break;
    }
    case Theme::Dark: {
        QJsonObject themeData{ { "theme", QStringLiteral("Dark") },
                               { "backgroundColor", "#191919" } };
        m_currentThemeData = themeData;
        emit themeChanged(QVariant(themeData));
        m_currentEditorTextColor = QColor(223, 224, 224);
        m_searchButton->setStyleSheet("QToolButton { color: rgb(68, 68, 68) }");
        m_clearButton->setStyleSheet("QToolButton { color: rgb(147, 144, 147) }");
        break;
    }
    case Theme::Sepia: {
        QJsonObject themeData{ { "theme", QStringLiteral("Sepia") },
                               { "backgroundColor", "#fbf0d9" } };
        m_currentThemeData = themeData;
        emit themeChanged(QVariant(themeData));
        m_currentEditorTextColor = QColor(50, 30, 3);
        m_searchButton->setStyleSheet("QToolButton { color: rgb(205, 205, 205) }");
        m_clearButton->setStyleSheet("QToolButton { color: rgb(114, 114, 114) }");
        break;
    }
    }
    m_noteEditorLogic->setTheme(theme, m_currentEditorTextColor, m_editorMediumFontSize);
    m_listViewLogic->setTheme(theme);
    m_aboutWindow.setTheme(theme);
    m_treeViewLogic->setTheme(theme);
    ui->tagListView->setTheme(theme);

    updateSelectedOptionsEditorSettings();
}

void MainWindow::deleteSelectedNote()
{
    m_noteEditorLogic->deleteCurrentNote();
    if (m_listModel->rowCount() == 1) {
        emit m_listViewLogic->closeNoteEditor();
    }
}

/*!
 * \brief MainWindow::onClearButtonClicked
 * clears the search and
 * select the note that was selected before searching if it is still valid.
 */
void MainWindow::onClearButtonClicked()
{
    m_listViewLogic->clearSearch();
}

/*!
 * \brief MainWindow::createNewNote
 * create a new note
 * add it to the database
 * add it to the scrollArea
 */
void MainWindow::createNewNote(bool isCalledFromShortcut)
{
    m_listView->scrollToTop();
    QModelIndex newNoteIndex;
    if (!m_noteEditorLogic->isTempNote()) {
        // clear the textEdit
        m_blockModel->blockSignals(true);
        m_noteEditorLogic->closeEditor();
        m_blockModel->blockSignals(false);

        NodeData tmpNote;
        tmpNote.setNodeType(NodeData::Note);
        QDateTime noteDate = QDateTime::currentDateTime();
        tmpNote.setCreationDateTime(noteDate);
        tmpNote.setLastModificationDateTime(noteDate);
        tmpNote.setFullTitle(QStringLiteral("New Note"));
        auto inf = m_listViewLogic->listViewInfo();
        if ((!inf.isInTag) && (inf.parentFolderId > SpecialNodeID::RootFolder)) {
            NodeData parent;
            QMetaObject::invokeMethod(m_dbManager, "getNode", Qt::BlockingQueuedConnection,
                                      Q_RETURN_ARG(NodeData, parent),
                                      Q_ARG(int, inf.parentFolderId));
            if (parent.nodeType() == NodeData::Folder) {
                tmpNote.setParentId(parent.id());
                tmpNote.setParentName(parent.fullTitle());
            } else {
                tmpNote.setParentId(SpecialNodeID::DefaultNotesFolder);
                tmpNote.setParentName("Notes");
            }
        } else {
            tmpNote.setParentId(SpecialNodeID::DefaultNotesFolder);
            tmpNote.setParentName("Notes");
        }
        int noteId = SpecialNodeID::InvalidNodeId;
        QMetaObject::invokeMethod(m_dbManager, "nextAvailableNodeId", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(int, noteId));
        tmpNote.setId(noteId);
        tmpNote.setIsTempNote(true);
        if (inf.isInTag) {
            tmpNote.setTagIds(inf.currentTagList);
        }
        // insert the new note to NoteListModel
        newNoteIndex = m_listModel->insertNote(tmpNote, 0);

        // update the editor
        m_noteEditorLogic->showNotesInEditor({ tmpNote }, isCalledFromShortcut);
    } else {
        newNoteIndex = m_listModel->getNoteIndex(m_noteEditorLogic->currentEditingNoteId());
        m_listView->animateAddedRow({ newNoteIndex });
    }
    // update the current selected index
    m_listView->setCurrentIndexC(newNoteIndex);
    m_blockEditorWidget->setFocus();
    emit focusOnEditor();
}

void MainWindow::selectNoteDown()
{
    if (m_listView->hasFocus()) {
        m_listViewLogic->selectNoteDown();
    }
}

/*!
 * \brief MainWindow::setFocusOnText
 * Set focus on editor
 */
void MainWindow::setFocusOnText()
{
    if (m_noteEditorLogic->currentEditingNoteId() != SpecialNodeID::InvalidNodeId) {
        m_listView->setCurrentRowActive(true);
        m_blockEditorWidget->setFocus();
        emit focusOnEditor();
    }
}

void MainWindow::selectNoteUp()
{
    if (m_listView->hasFocus()) {
        m_listViewLogic->selectNoteUp();
    }
}

/*!
 * \brief MainWindow::fullscreenWindow
 * Switch to fullscreen mode
 */
void MainWindow::fullscreenWindow()
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    if (isFullScreen()) {
        if (!isMaximized()) {
            QMargins margins(m_layoutMargin, m_layoutMargin, m_layoutMargin, m_layoutMargin);
            setMargins(margins);
        }

        setWindowState(windowState() & ~Qt::WindowFullScreen);
    } else {
        setWindowState(windowState() | Qt::WindowFullScreen);
        setMargins(QMargins());
    }

#elif _WIN32
    if (isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }
#endif
}

/*!
 * \brief MainWindow::maximizeWindow
 * Maximize the window
 */
void MainWindow::maximizeWindow()
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    if (isMaximized()) {
        if (!isFullScreen()) {
            QMargins margins(m_layoutMargin, m_layoutMargin, m_layoutMargin, m_layoutMargin);

            setMargins(margins);
            setWindowState(windowState() & ~Qt::WindowMaximized);
        } else {
            setWindowState(windowState() & ~Qt::WindowFullScreen);
        }

    } else {
        setWindowState(windowState() | Qt::WindowMaximized);
        setMargins(QMargins());
    }
#elif _WIN32
    if (isMaximized()) {
        setWindowState(windowState() & ~Qt::WindowMaximized);
        setWindowState(windowState() & ~Qt::WindowFullScreen);
    } else if (isFullScreen()) {
        setWindowState((windowState() | Qt::WindowMaximized) & ~Qt::WindowFullScreen);
        setGeometry(qApp->primaryScreen()->availableGeometry());
    } else {
        setWindowState(windowState() | Qt::WindowMaximized);
        setGeometry(qApp->primaryScreen()->availableGeometry());
    }
#endif
}

/*!
 * \brief MainWindow::minimizeWindow
 * Minimize the window
 */
void MainWindow::minimizeWindow()
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    QMargins margins(m_layoutMargin, m_layoutMargin, m_layoutMargin, m_layoutMargin);
    setMargins(margins);
#endif

    // BUG : QTBUG-57902 minimize doesn't store the window state before minimizing
    showMinimized();
}

/*!
 * \brief MainWindow::QuitApplication
 * Exit the application
 * Save the geometry of the app to the settings
 * Save the current note if it's note temporary one otherwise remove it from DB
 */
void MainWindow::QuitApplication()
{
    if (windowState() != Qt::WindowFullScreen) {
        m_settingsDatabase->setValue(QStringLiteral("windowGeometry"), saveGeometry());
    }

    m_noteEditorLogic->saveNoteToDB();

#if defined(UPDATE_CHECKER)
    m_settingsDatabase->setValue(QStringLiteral("dontShowUpdateWindow"), m_dontShowUpdateWindow);
#endif
    m_settingsDatabase->setValue(QStringLiteral("splitterSizes"), m_splitter->saveState());

    m_settingsDatabase->setValue(QStringLiteral("isTreeCollapsed"), m_foldersWidget->isHidden());
    m_settingsDatabase->setValue(QStringLiteral("isNoteListCollapsed"),
                                 m_noteListWidget->isHidden());

    m_settingsDatabase->setValue(QStringLiteral("selectedFontTypeface"),
                                 to_string(m_currentFontTypeface).c_str());
    m_settingsDatabase->setValue(QStringLiteral("editorMediumFontSize"), m_editorMediumFontSize);
    m_settingsDatabase->setValue(QStringLiteral("isTextFullWidth"), m_isTextFullWidth);
    m_settingsDatabase->setValue(QStringLiteral("isDistractionFreeMode"), m_isDistractionFreeMode);
    m_settingsDatabase->setValue(QStringLiteral("charsLimitPerFontMono"),
                                 m_currentCharsLimitPerFont.mono);
    m_settingsDatabase->setValue(QStringLiteral("charsLimitPerFontSerif"),
                                 m_currentCharsLimitPerFont.serif);
    m_settingsDatabase->setValue(QStringLiteral("charsLimitPerFontSansSerif"),
                                 m_currentCharsLimitPerFont.sansSerif);
    m_settingsDatabase->setValue(QStringLiteral("theme"), to_string(m_currentTheme).c_str());
    m_settingsDatabase->setValue(QStringLiteral("chosenMonoFont"),
                                 m_listOfMonoFonts.at(m_chosenMonoFontIndex));
    m_settingsDatabase->setValue(QStringLiteral("chosenSerifFont"),
                                 m_listOfSerifFonts.at(m_chosenSerifFontIndex));
    m_settingsDatabase->setValue(QStringLiteral("chosenSansSerifFont"),
                                 m_listOfSansSerifFonts.at(m_chosenSansSerifFontIndex));

    m_settingsDatabase->sync();

    m_noteEditorLogic->closeEditor();

    QCoreApplication::quit();
}

/*!
 * \brief MainWindow::checkForUpdates
 * Called when the "Check for Updates" menu item is clicked, this function
 * instructs the updater window to check if there are any updates available
 */
void MainWindow::checkForUpdates()
{
#if defined(UPDATE_CHECKER)
    m_updater.checkForUpdates(true);
#endif
}

void MainWindow::importPlainTextFiles()
{
    QList<QPair<QString, QDateTime>> fileDatas;
    QFileDialog dialog(this);

    // Set filters and options
    dialog.setNameFilter("Text Files (*.txt *.md)");
    dialog.setFileMode(QFileDialog::ExistingFiles);

    // Open the dialog and check if user has selected files
    if (dialog.exec()) {
        // Get list of selected files
        QStringList files = dialog.selectedFiles();

        // Loop over the files and read their content
        for (const QString &fileName : files) {
            QFile file(fileName);

            // Open the file and check for errors
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QMessageBox::warning(this, "File error", "Can't open file " + fileName);
                continue;
            }

            // Read the file content
            QTextStream in(&file);
            QString fileContent = in.readAll();

            QFileInfo fileInfo(file);
            QDateTime lastModified = fileInfo.lastModified();

            fileDatas.append(qMakePair(fileContent, lastModified));

            // Close the file
            file.close();
        }
    }

    QMessageBox msgBox;
    if (!fileDatas.isEmpty()) {
        m_dbManager->addNotesToNewImportedFolder(fileDatas);
        msgBox.setText("Notes imported successfully!");
        msgBox.exec();
    } else {
        msgBox.setText("No files selected. Please select one or more files to import.");
        msgBox.exec();
    }
}

void MainWindow::exportToPlainTextFiles(const QString &extension)
{
    QString dir = QFileDialog::getExistingDirectory(nullptr, tr("Select Export Directory"),
                                                    "/home",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty()) {
        return;
    }

    m_dbManager->exportNotes(dir, extension);

    QMessageBox msgBox;
    msgBox.setText("Notes exported successfully!");
    msgBox.exec();
}

/*!
 * \brief MainWindow::importNotesFile
 * Called when the "Import Notes" menu button is clicked. this function will
 * prompt the user to select a file, attempt to load the file, and update the DB
 * if valid.
 * The user is presented with a dialog box if the upload/import fails for any reason.
 */
void MainWindow::importNotesFile()
{
    executeImport(false);
}

/*!
 * \brief MainWindow::restoreNotesFile
 * Called when the "Restore Notes" menu button is clicked. this function will
 * prompt the user to select a file, attempt to load the file, and update the DB
 * if valid.
 * The user is presented with a dialog box if the upload/import/restore fails for any reason.
 */
void MainWindow::restoreNotesFile()
{
    if (m_listModel->rowCount() > 0) {
        QMessageBox msgBox;
        msgBox.setText(tr("Warning: All current notes will be lost. Make sure to create a backup "
                          "copy before proceeding."));
        msgBox.setInformativeText(tr("Would you like to continue?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if (msgBox.exec() != QMessageBox::Yes) {
            return;
        }
    }
    executeImport(true);
}

/*!
 * \brief MainWindow::executeImport
 * Executes the note import process. if replace is true all current notes will be
 * removed otherwise current notes will be kept.
 * \param replace
 */
void MainWindow::executeImport(const bool replace)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Notes Backup File"), "",
                                                    tr("Notes Backup File (*.nbk)"));
    if (fileName.isEmpty()) {
        return;
    } else {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(this, tr("Unable to open file"), file.errorString());
            return;
        }
        file.close();

        setButtonsAndFieldsEnabled(false);
        if (replace) {
            emit requestRestoreNotes(fileName);
        } else {
            emit requestImportNotes(fileName);
        }
        setButtonsAndFieldsEnabled(true);
        //        emit requestNotesList(SpecialNodeID::RootFolder, true);
    }
}

/*!
 * \brief MainWindow::exportNotesFile
 * Called when the "Export Notes" menu button is clicked. this function will
 * prompt the user to select a location for the export file, and then builds
 * the file.
 * The user is presented with a dialog box if the file cannot be opened for any reason.
 * \param clicked
 */
void MainWindow::exportNotesFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Notes"), "notes.nbk",
                                                    tr("Notes Backup File (*.nbk)"));
    if (fileName.isEmpty()) {
        return;
    } else {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to open file"), file.errorString());
            return;
        }
        file.close();
        emit requestExportNotes(fileName);
    }
}

void MainWindow::toggleFolderTree()
{
    if (m_foldersWidget->isHidden()) {
        expandFolderTree();
    } else {
        collapseFolderTree();
    }
    updateSelectedOptionsEditorSettings();
}

void MainWindow::animateHidePane(int paneIndex, QWidget* pane, int duration)
{
    m_isAnimatingSplitter = true;
    QTimeLine* timeLine = new QTimeLine(duration, m_splitter);
    timeLine->setFrameRange(0, 100);
    timeLine->setEasingCurve(QEasingCurve::OutExpo);

    QList<int> initialSizes = m_splitter->sizes();
    int totalSize = 0;
    for (int size : initialSizes) {
        totalSize += size;
    }

    QList<int> targetSizes = initialSizes;
    targetSizes[paneIndex] = 0;

    int originalMinimumWidth = pane->minimumWidth();
    // int originalMaximumWidth = pane->maximumWidth();
    int originalWidth = pane->width();
    QRect originalGeometry = pane->geometry();
    pane->setMinimumWidth(0);

    connect(timeLine, &QTimeLine::frameChanged, this, [this, initialSizes, targetSizes, paneIndex, pane](int frame) {
        QList<int> currentSizes;
        float progress = frame / 100.0f;
        int maxWidth = initialSizes[paneIndex] * (1 - progress);

        for (int i = 0; i < initialSizes.size(); ++i) {
            int delta = targetSizes[i] - initialSizes[i];
            currentSizes.append(initialSizes[i] + delta * progress);
        }

        pane->setMaximumWidth(maxWidth);
        m_splitter->setSizes(currentSizes);
    });

    QObject::connect(timeLine, &QTimeLine::finished, this, [this, timeLine, pane, originalGeometry, originalWidth, originalMinimumWidth, initialSizes]() {
        timeLine->deleteLater();
        pane->setHidden(true);
        m_splitter->setSizes(initialSizes);
        pane->setMinimumWidth(originalMinimumWidth);
        pane->setMaximumWidth(QWIDGETSIZE_MAX);
        pane->setGeometry(QRect(originalGeometry.x(), originalGeometry.y(), originalWidth, originalGeometry.height()));
        updateFrame();
        updateSelectedOptionsEditorSettings();
        m_isAnimatingSplitter = false;
    });

    timeLine->start();
}

void MainWindow::animateShowPane(int paneIndex, QWidget* pane, int duration)
{
    m_isAnimatingSplitter = true;
    QTimeLine* timeLine = new QTimeLine(duration, m_splitter);
    timeLine->setFrameRange(0, 100);
    timeLine->setEasingCurve(QEasingCurve::OutExpo);

    QList<int> initialSizes = m_splitter->sizes();
    int totalSize = 0;
    for (int size : initialSizes) {
        totalSize += size;
    }

    QList<int> targetSizes = initialSizes;

    int originalMinimumWidth = pane->minimumWidth();
    // int originalMaximumWidth = pane->maximumWidth();
    QRect originalGeometry = pane->geometry();
    int originalSize = pane->width();
    targetSizes[paneIndex] = originalSize;

    pane->setHidden(false);
    pane->setMinimumWidth(0);

    connect(timeLine, &QTimeLine::frameChanged, this, [this, initialSizes, targetSizes, pane, originalSize](int frame) {
        QList<int> currentSizes;
        float progress = frame / 100.0f;
        int newWidth = originalSize * progress;

        for (int i = 0; i < initialSizes.size(); ++i) {
            int delta = targetSizes[i] - initialSizes[i];
            currentSizes.append(initialSizes[i] + delta * progress);
        }

        pane->setMaximumWidth(newWidth);
        m_splitter->setSizes(currentSizes);
    });

    QObject::connect(timeLine, &QTimeLine::finished, this, [this, timeLine, pane, originalMinimumWidth, originalGeometry]() {
        pane->setMinimumWidth(originalMinimumWidth);
        pane->setMaximumWidth(QWIDGETSIZE_MAX);
        pane->setGeometry(originalGeometry);
        timeLine->deleteLater();
        updateFrame();
        updateSelectedOptionsEditorSettings();
        m_isAnimatingSplitter = false;
    });

    timeLine->start();
}

void MainWindow::expandPanes()
{
    if (m_isAnimatingSplitter)
        return;

    if (m_foldersWidget->isHidden() && m_noteListWidget->isHidden()) {
        expandNoteList();
    } else if (m_foldersWidget->isHidden() && !m_noteListWidget->isHidden()) {
        expandFolderTree();
    } else if (!m_foldersWidget->isHidden() && m_noteListWidget->isHidden()) {
        expandNoteList();
    }
}

void MainWindow::collapsePanes()
{
    if (m_isAnimatingSplitter)
        return;

    if (!m_foldersWidget->isHidden() && !m_noteListWidget->isHidden()) {
        collapseFolderTree();
    } else if (m_foldersWidget->isHidden() && !m_noteListWidget->isHidden()) {
        collapseNoteList();
    } else if (!m_foldersWidget->isHidden() && m_noteListWidget->isHidden()) {
        collapseFolderTree();
    }
}

void MainWindow::collapseFolderTree()
{
    if (m_isAnimatingSplitter || m_foldersWidget->isHidden())
        return;

    emit paneToggled();

    m_toggleTreeViewButton->setText(u8"\ue31c"); // keyboard_tab
    m_foldersWidget->setHidden(true);
    // animateHidePane(0, m_foldersWidget, 500);

    updateFrame();
    updateSelectedOptionsEditorSettings();

#if defined(Q_OS_MACOS)
    if (windowState() != Qt::WindowFullScreen) {
        ui->verticalSpacer_upSearchEdit->setMinimumHeight(33);
        ui->verticalSpacer_upSearchEdit->setMaximumHeight(33);
    }
#else
    if (!m_useNativeWindowFrame && windowState() != Qt::WindowFullScreen) {
        ui->verticalSpacer_upSearchEdit->setMinimumHeight(25);
        ui->verticalSpacer_upSearchEdit->setMaximumHeight(25);
    }
#endif
}

void MainWindow::expandFolderTree()
{
    if (m_isAnimatingSplitter || !m_foldersWidget->isHidden())
        return;

    emit paneToggled();

    m_toggleTreeViewButton->setText(u8"\uec73"); // keyboard_tab_rtl
    m_foldersWidget->setHidden(false);
    // animateShowPane(0, m_foldersWidget, 500);

    updateFrame();
    updateSelectedOptionsEditorSettings();

#if defined(Q_OS_MACOS)
    ui->verticalSpacer_upSearchEdit->setMinimumHeight(0);
    ui->verticalSpacer_upSearchEdit->setMaximumHeight(0);
#else
    if (!m_useNativeWindowFrame) {
        ui->verticalSpacer_upSearchEdit->setMinimumHeight(0);
        ui->verticalSpacer_upSearchEdit->setMaximumHeight(0);
    }
#endif
}

void MainWindow::toggleNoteList()
{
    if (m_noteListWidget->isHidden()) {
        expandNoteList();
    } else {
        collapseNoteList();
    }
    updateSelectedOptionsEditorSettings();
}

void MainWindow::collapseNoteList()
{
    if (m_isAnimatingSplitter || m_noteListWidget->isHidden())
        return;

    emit paneToggled();

    // TODO: Animating the note list widget doesn't work well
    // animateHidePane(0, m_noteListWidget, 500);
    m_noteListWidget->setHidden(true);

    updateFrame();
    updateSelectedOptionsEditorSettings();
}

void MainWindow::expandNoteList()
{
    if (m_isAnimatingSplitter || !m_noteListWidget->isHidden())
        return;

    emit paneToggled();

    // TODO: Animating the note list widget doesn't work well
    // animateShowPane(0, m_noteListWidget, 500);
    m_noteListWidget->setHidden(false);
    m_isAnimatingSplitter = true;
    QTimer::singleShot(500, this, [this]() {
        m_isAnimatingSplitter = false;
    });

    updateFrame();
    updateSelectedOptionsEditorSettings();
}

/*!
 * \brief MainWindow::closeEvent
 * Called when the window is about to close
 * \param event
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!event->spontaneous() || !isVisible()) {
        QuitApplication();
        return;
    }
    if (m_hideToTray && m_trayIcon->isVisible() && QSystemTrayIcon::isSystemTrayAvailable()) {
        // don't close the application, just hide to tray
        setMainWindowVisibility(false);
        event->ignore();
    } else {
        // save states and quit application
        QuitApplication();
    }
}

/*!
 * \brief MainWindow::moveEvent
 * \param event
 */
void MainWindow::moveEvent(QMoveEvent *event)
{
    QJsonObject dataToSendToView{ { "parentWindowX", x() }, { "parentWindowY", y() } };
    emit mainWindowMoved(QVariant(dataToSendToView));

    event->accept();
}

#ifndef _WIN32
/*!
 * \brief MainWindow::mousePressEvent
 * Set variables to the position of the window when the mouse is pressed
 * \param event
 */
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (m_useNativeWindowFrame) {
        MainWindowBase::mousePressEvent(event);
        return;
    }

    m_mousePressX = event->pos().x();
    m_mousePressY = event->pos().y();

    if (event->buttons() == Qt::LeftButton) {
        if (isTitleBar(m_mousePressX, m_mousePressY)) {

#  if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            m_canMoveWindow = !window()->windowHandle()->startSystemMove();
#  else
            m_canMoveWindow = true;
#  endif
        }

        event->accept();

    } else {
        MainWindowBase::mousePressEvent(event);
    }
}

/*!
 * \brief MainWindow::mouseMoveEvent
 * Move the window according to the mouse positions
 * \param event
 */
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_useNativeWindowFrame) {
        MainWindowBase::mouseMoveEvent(event);
        return;
    }

    if (m_canMoveWindow) {
        int dx = event->globalX() - m_mousePressX;
        int dy = event->globalY() - m_mousePressY;
        move(dx, dy);
    }

    event->accept();
}

/*!
 * \brief MainWindow::mouseReleaseEvent
 * Initialize flags
 * \param event
 */
void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_useNativeWindowFrame) {
        MainWindowBase::mouseReleaseEvent(event);
        return;
    }

    m_canMoveWindow = false;
    m_canStretchWindow = false;
    QApplication::restoreOverrideCursor();
    event->accept();
}
#else
/*!
 * \brief MainWindow::mousePressEvent
 * Set variables to the position of the window when the mouse is pressed
 * \param event
 */
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (m_useNativeWindowFrame) {
        MainWindowBase::mousePressEvent(event);
        return;
    }

    if (event->button() == Qt::LeftButton) {
        if (isTitleBar(event->pos().x(), event->pos().y())) {

#  if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            m_canMoveWindow = !window()->windowHandle()->startSystemMove();
#  else
            m_canMoveWindow = true;
#  endif
            m_mousePressX = event->pos().x();
            m_mousePressY = event->pos().y();
        }
    }

    event->accept();
}

/*!
 * \brief MainWindow::mouseMoveEvent
 * Move the window according to the mouse positions
 * \param event
 */
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_useNativeWindowFrame) {
        MainWindowBase::mouseMoveEvent(event);
        return;
    }

    if (m_canMoveWindow) {
        //        setCursor(Qt::ClosedHandCursor);
        int dx = event->globalPos().x() - m_mousePressX;
        int dy = event->globalY() - m_mousePressY;
        move(dx, dy);
    }
}

/*!
 * \brief MainWindow::mouseReleaseEvent
 * Initialize flags
 * \param event
 */
void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_useNativeWindowFrame) {
        MainWindowBase::mouseReleaseEvent(event);
        return;
    }

    m_canMoveWindow = false;
    //    unsetCursor();
    event->accept();
}
#endif

/*!
 * \brief MainWindow::clearSearch
 */
void MainWindow::clearSearch()
{
    m_listView->setFocusPolicy(Qt::ClickFocus);

    m_searchEdit->blockSignals(true);
    m_searchEdit->clear();
    m_searchEdit->blockSignals(false);

    m_clearButton->hide();
    m_searchEdit->setFocus();
}

void MainWindow::showErrorMessage(const QString &title, const QString &content)
{
    QMessageBox::information(this, title, content);
}

void MainWindow::setNoteListLoading()
{
    ui->listviewLabel1->setText("Loading…");
    ui->listviewLabel2->setText("");
}

void MainWindow::selectAllNotesInList()
{
    m_listViewLogic->selectAllNotes();
}

void MainWindow::updateFrame()
{
#if defined(Q_OS_MACOS)
    if (m_foldersWidget->isHidden() && m_noteListWidget->isHidden()) {
       setWindowButtonsVisible(false);
    } else {
       setWindowButtonsVisible(true);
    }
#endif
}

/*!
 * \brief MainWindow::checkMigration
 */
void MainWindow::migrateFromV0_9_0()
{
    QFileInfo fi(m_settingsDatabase->fileName());
    QDir dir(fi.absolutePath());

    QString oldNoteDBPath(dir.path() + QDir::separator() + "Notes.ini");
    if (QFile::exists(oldNoteDBPath)) {
        migrateNoteFromV0_9_0(oldNoteDBPath);
    }

    QString oldTrashDBPath(dir.path() + QDir::separator() + "Trash.ini");
    if (QFile::exists(oldTrashDBPath)) {
        migrateTrashFromV0_9_0(oldTrashDBPath);
    }
}

/*!
 * \brief MainWindow::migrateNote
 * \param notePath
 */
void MainWindow::migrateNoteFromV0_9_0(const QString &notePath)
{
    QSettings notesIni(notePath, QSettings::IniFormat);
    QStringList dbKeys = notesIni.allKeys();
    QVector<NodeData> noteList;

    auto it = dbKeys.begin();
    for (; it < dbKeys.end() - 1; it += 3) {
        QString noteName = it->split(QStringLiteral("/"))[0];
        int id = noteName.split(QStringLiteral("_"))[1].toInt();
        NodeData newNote;
        newNote.setId(id);
        QString createdDateDB =
                notesIni.value(noteName + QStringLiteral("/dateCreated"), "Error").toString();
        newNote.setCreationDateTime(QDateTime::fromString(createdDateDB, Qt::ISODate));
        QString lastEditedDateDB =
                notesIni.value(noteName + QStringLiteral("/dateEdited"), "Error").toString();
        newNote.setLastModificationDateTime(QDateTime::fromString(lastEditedDateDB, Qt::ISODate));
        QString contentText =
                notesIni.value(noteName + QStringLiteral("/content"), "Error").toString();
        newNote.setContent(contentText);
        QString firstLine = NoteEditorLogic::getFirstLine(contentText);
        newNote.setFullTitle(firstLine);
        noteList.append(newNote);
    }

    if (!noteList.isEmpty()) {
        emit requestMigrateNotesFromV0_9_0(noteList);
    }

    QFile oldNoteDBFile(notePath);
    oldNoteDBFile.rename(QFileInfo(notePath).dir().path() + QDir::separator()
                         + QStringLiteral("oldNotes.ini"));
}

/*!
 * \brief MainWindow::migrateTrash
 * \param trashPath
 */
void MainWindow::migrateTrashFromV0_9_0(const QString &trashPath)
{
    QSettings trashIni(trashPath, QSettings::IniFormat);
    QStringList dbKeys = trashIni.allKeys();

    QVector<NodeData> noteList;

    auto it = dbKeys.begin();
    for (; it < dbKeys.end() - 1; it += 3) {
        QString noteName = it->split(QStringLiteral("/"))[0];
        int id = noteName.split(QStringLiteral("_"))[1].toInt();
        NodeData newNote;
        newNote.setId(id);
        QString createdDateDB =
                trashIni.value(noteName + QStringLiteral("/dateCreated"), "Error").toString();
        newNote.setCreationDateTime(QDateTime::fromString(createdDateDB, Qt::ISODate));
        QString lastEditedDateDB =
                trashIni.value(noteName + QStringLiteral("/dateEdited"), "Error").toString();
        newNote.setLastModificationDateTime(QDateTime::fromString(lastEditedDateDB, Qt::ISODate));
        QString contentText =
                trashIni.value(noteName + QStringLiteral("/content"), "Error").toString();
        newNote.setContent(contentText);
        QString firstLine = NoteEditorLogic::getFirstLine(contentText);
        newNote.setFullTitle(firstLine);
        noteList.append(newNote);
    }

    if (!noteList.isEmpty()) {
        emit requestMigrateTrashFromV0_9_0(noteList);
    }
    QFile oldTrashDBFile(trashPath);
    oldTrashDBFile.rename(QFileInfo(trashPath).dir().path() + QDir::separator()
                          + QStringLiteral("oldTrash.ini"));
}

/*!
 * \brief MainWindow::dropShadow
 * \param painter
 * \param type
 * \param side
 */
void MainWindow::dropShadow(QPainter &painter, ShadowType type, MainWindow::ShadowSide side)
{
    int resizedShadowWidth = m_shadowWidth > m_layoutMargin ? m_layoutMargin : m_shadowWidth;

    QRect mainRect = rect();

    QRect innerRect(m_layoutMargin, m_layoutMargin, mainRect.width() - 2 * resizedShadowWidth + 1,
                    mainRect.height() - 2 * resizedShadowWidth + 1);
    QRect outerRect(innerRect.x() - resizedShadowWidth, innerRect.y() - resizedShadowWidth,
                    innerRect.width() + 2 * resizedShadowWidth,
                    innerRect.height() + 2 * resizedShadowWidth);

    QPoint center;
    QPoint topLeft;
    QPoint bottomRight;
    QPoint shadowStart;
    QPoint shadowStop;
    QRadialGradient radialGradient;
    QLinearGradient linearGradient;

    switch (side) {
    case ShadowSide::Left:
        topLeft = QPoint(outerRect.left(), innerRect.top() + 1);
        bottomRight = QPoint(innerRect.left(), innerRect.bottom() - 1);
        shadowStart = QPoint(innerRect.left(), innerRect.top() + 1);
        shadowStop = QPoint(outerRect.left(), innerRect.top() + 1);
        break;
    case ShadowSide::Top:
        topLeft = QPoint(innerRect.left() + 1, outerRect.top());
        bottomRight = QPoint(innerRect.right() - 1, innerRect.top());
        shadowStart = QPoint(innerRect.left() + 1, innerRect.top());
        shadowStop = QPoint(innerRect.left() + 1, outerRect.top());
        break;
    case ShadowSide::Right:
        topLeft = QPoint(innerRect.right(), innerRect.top() + 1);
        bottomRight = QPoint(outerRect.right(), innerRect.bottom() - 1);
        shadowStart = QPoint(innerRect.right(), innerRect.top() + 1);
        shadowStop = QPoint(outerRect.right(), innerRect.top() + 1);
        break;
    case ShadowSide::Bottom:
        topLeft = QPoint(innerRect.left() + 1, innerRect.bottom());
        bottomRight = QPoint(innerRect.right() - 1, outerRect.bottom());
        shadowStart = QPoint(innerRect.left() + 1, innerRect.bottom());
        shadowStop = QPoint(innerRect.left() + 1, outerRect.bottom());
        break;
    case ShadowSide::TopLeft:
        topLeft = outerRect.topLeft();
        bottomRight = innerRect.topLeft();
        center = innerRect.topLeft();
        break;
    case ShadowSide::TopRight:
        topLeft = QPoint(innerRect.right(), outerRect.top());
        bottomRight = QPoint(outerRect.right(), innerRect.top());
        center = innerRect.topRight();
        break;
    case ShadowSide::BottomRight:
        topLeft = innerRect.bottomRight();
        bottomRight = outerRect.bottomRight();
        center = innerRect.bottomRight();
        break;
    case ShadowSide::BottomLeft:
        topLeft = QPoint(outerRect.left(), innerRect.bottom());
        bottomRight = QPoint(innerRect.left(), outerRect.bottom());
        center = innerRect.bottomLeft();
        break;
    }

    QRect zone(topLeft, bottomRight);
    radialGradient = QRadialGradient(center, resizedShadowWidth, center);

    linearGradient.setStart(shadowStart);
    linearGradient.setFinalStop(shadowStop);

    switch (type) {
    case ShadowType::Radial:
        fillRectWithGradient(painter, zone, radialGradient);
        break;
    case ShadowType::Linear:
        fillRectWithGradient(painter, zone, linearGradient);
        break;
    }
}

/*!
 * \brief MainWindow::fillRectWithGradient
 * \param painter
 * \param rect
 * \param gradient
 */
void MainWindow::fillRectWithGradient(QPainter &painter, QRect rect, QGradient &gradient)
{
    double variance = 0.2;
    double xMax = 1.10;
    double q = 70 / gaussianDist(0, 0, sqrt(variance));
    double nPt = 100.0;

    for (int i = 0; i <= nPt; i++) {
        double v = gaussianDist(i * xMax / nPt, 0, sqrt(variance));

        QColor c(168, 168, 168, int(q * v));
        gradient.setColorAt(i / nPt, c);
    }

    painter.fillRect(rect, gradient);
}

/*!
 * \brief MainWindow::gaussianDist
 * \param x
 * \param center
 * \param sigma
 * \return
 */
double MainWindow::gaussianDist(double x, const double center, double sigma) const
{
    return (1.0 / (2 * M_PI * pow(sigma, 2)) * exp(-pow(x - center, 2) / (2 * pow(sigma, 2))));
}

/*!
 * \brief MainWindow::mouseDoubleClickEvent
 * When the blank area at the top of window is double-clicked the window get maximized
 * \param event
 */
void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (m_useNativeWindowFrame || event->buttons() != Qt::LeftButton
        || !isTitleBar(event->pos().x(), event->pos().y())) {
        MainWindowBase::mouseDoubleClickEvent(event);
        return;
    }

#ifndef __APPLE__
    maximizeWindow();
#else
    maximizeWindowMac();
#endif
    event->accept();
}

/*!
 * \brief MainWindow::leaveEvent
 */
void MainWindow::leaveEvent(QEvent *)
{
    unsetCursor();
}

/*!
 * \brief MainWindow::changeEvent
 */
void MainWindow::changeEvent(QEvent *event)
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    if (!m_useNativeWindowFrame && event->type() == QEvent::WindowStateChange) {
        if (isMaximized())
            setMargins(QMargins());
        else
            setMargins(QMargins(m_layoutMargin, m_layoutMargin, m_layoutMargin, m_layoutMargin));
    }

#elif __APPLE__
    if (event->type() == QEvent::WindowStateChange) {
        if (this->windowState() != Qt::WindowFullScreen) {
            setStandardWindowButtonsMacVisibility(true);
        }
    }

#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (event->type() == QEvent::ThemeChange) {
        if (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark && m_currentTheme != Theme::Dark) {
            setTheme(Theme::Dark);
        } else if (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Light && m_currentTheme == Theme::Dark){
            setTheme(Theme::Light);
        }
#ifdef __APPLE__
        QTimer::singleShot(100, this, &MainWindow::updateFrame);
#endif
    }
#endif

    MainWindowBase::changeEvent(event);
}

void MainWindow::setWindowButtonsVisible(bool isVisible)
{
#ifdef __APPLE__
    setStandardWindowButtonsMacVisibility(isVisible);
#else
    Q_UNUSED(isVisible)
#endif
}

/*!
 * \brief MainWindow::eventFilter
 * Mostly take care on the event happened on widget whose filter installed to the mainwindow
 * \param object
 * \param event
 * \return
 */
bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type()) {
    case QEvent::Enter: {
        if (object == ui->frame) {
            ui->centralWidget->setCursor(Qt::ArrowCursor);
        }

        break;
    }
    case QEvent::Leave: {
        break;
    }
    case QEvent::WindowDeactivate: {
        emit windowFocusChanged(false);
        emit mainWindowDeactivated();
        m_canMoveWindow = false;
        m_canStretchWindow = false;
        QApplication::restoreOverrideCursor();
        break;
    }
    case QEvent::WindowActivate: {
        emit windowFocusChanged(true);
        break;
    }
        //    case QEvent::FocusIn: {
        //        if (object == m_textEdit) {
        //            if (!m_isOperationRunning) {
        //                if (m_listModel->rowCount() == 0) {
        //                    if (!m_searchEdit->text().isEmpty()) {
        //                        m_listViewLogic->clearSearch(true);
        //                    } else {
        //                        createNewNote();
        //                    }
        //                }
        //            }
        //            m_listView->setCurrentRowActive(true);
        //            m_textEdit->setFocus();
        //        }
        //        break;
        //    }
    case QEvent::WindowStateChange: {
        if (isFullScreen()) {
            qDebug() << "MainWindow::eventFilter: window state changed to fullscreen";
        } else {
            qDebug() << "MainWindow::eventFilter: window state changed to normal";
        }
        break;
    }
    case QEvent::FocusOut: {
        break;
    }
    case QEvent::Show:
#if defined(UPDATE_CHECKER)
        if (object == &m_updater) {

            QRect rect = m_updater.geometry();
            QRect appRect = geometry();
            int titleBarHeight = 28;

            int x = int(appRect.x() + (appRect.width() - rect.width()) / 2.0);
            int y = int(appRect.y() + titleBarHeight + (appRect.height() - rect.height()) / 2.0);

            m_updater.setGeometry(QRect(x, y, rect.width(), rect.height()));
        }
#endif
        break;
    case QEvent::KeyPress: {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return && m_searchEdit->text().isEmpty()) {
            setFocusOnText();
        } else if (keyEvent->key() == Qt::Key_Return
                   && keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
            setFocusOnText();
            return true;
        } else if (keyEvent->key() == Qt::Key_Escape && isFullScreen()) {
            // exit fullscreen
            fullscreenWindow();
        }
        break;
    }
    default:
        break;
    }

    return QObject::eventFilter(object, event);
}

/*!
 * \brief MainWindow::stayOnTop
 * \param checked
 */
void MainWindow::stayOnTop(bool checked)
{
    m_alwaysStayOnTop = checked;

#if defined(Q_OS_MACOS)
    setWindowAlwaysOnTopMac(checked);
    updateSelectedOptionsEditorSettings();
#endif
}

/*!
 * \brief MainWindow::moveCurrentNoteToTrash
 */
void MainWindow::moveCurrentNoteToTrash()
{
    m_noteEditorLogic->deleteCurrentNote();
}

/*!
 * \brief MainWindow::setUseNativeWindowFrame
 * \param useNativeWindowFrame
 */
void MainWindow::setUseNativeWindowFrame(bool useNativeWindowFrame)
{
    m_useNativeWindowFrame = useNativeWindowFrame;
    m_settingsDatabase->setValue(QStringLiteral("useNativeWindowFrame"), useNativeWindowFrame);

#ifndef __APPLE__
    // Reset window flags to its initial state.
    Qt::WindowFlags flags = Qt::Window;

    if (!useNativeWindowFrame) {
        flags |= Qt::CustomizeWindowHint;
#  if defined(Q_OS_UNIX)
        flags |= Qt::FramelessWindowHint;
#  endif
    }

    setWindowFlags(flags);
#endif

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    if (useNativeWindowFrame || isMaximized()) {
        ui->centralWidget->layout()->setContentsMargins(QMargins());
    } else {
        QMargins margins(m_layoutMargin, m_layoutMargin, m_layoutMargin, m_layoutMargin);
        ui->centralWidget->layout()->setContentsMargins(margins);
    }
#endif

    setMainWindowVisibility(true);
}

void MainWindow::setHideToTray(bool enabled)
{
    m_hideToTray = enabled;
    m_settingsDatabase->setValue(QStringLiteral("hideToTray"), enabled);
}

/*!
 * \brief MainWindow::toggleStayOnTop
 */
void MainWindow::toggleStayOnTop()
{
    stayOnTop(!m_alwaysStayOnTop);
}

/*!
 * \brief MainWindow::onSearchEditReturnPressed
 */
void MainWindow::onSearchEditReturnPressed()
{
    if (m_searchEdit->text().isEmpty())
        return;

    //    QString searchText = m_searchEdit->text();
}

/*!
 * \brief MainWindow::setMargins
 * \param margins
 */
void MainWindow::setMargins(QMargins margins)
{
    if (m_useNativeWindowFrame)
        return;

    ui->centralWidget->layout()->setContentsMargins(margins);
}

bool MainWindow::isTitleBar(int x, int y) const
{
    if (m_useNativeWindowFrame)
        return false;

    // The width of the title bar is essentially the width of the main window.
    int titleBarWidth = width();
    int titleBarHeight = ui->globalSettingsButton->height();

    int adjustedX = x;
    int adjustedY = y;

    if (!isMaximized() && !isFullScreen()) {
        titleBarWidth -= m_layoutMargin * 2;
        adjustedX -= m_layoutMargin;
        adjustedY -= m_layoutMargin;
    }

    return (adjustedX >= 0 && adjustedX <= titleBarWidth && adjustedY >= 0
            && adjustedY <= titleBarHeight);
}

void MainWindow::callStartSystemMove()
{
    window()->windowHandle()->startSystemMove();
}

void MainWindow::handleMaximizeWindow()
{
#ifndef __APPLE__
    maximizeWindow();
#else
    maximizeWindowMac();
#endif
}
