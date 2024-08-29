#include "aboutwindow.h"
#include "ui_aboutwindow.h"

#include <QDebug>
#include <QFile>
#include <QScrollBar>
#include <qstyle.h>
#include <sstream>
#include "editorsettingsoptions.h"

/**
 * Initializes the window components and configures the AboutWindow
 */
AboutWindow::AboutWindow(QWidget *parent)
    : QDialog(parent), m_ui(new Ui::AboutWindow), m_isProVersion(false)
{
    m_ui->setupUi(this);
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
#endif
    setWindowTitle(tr("About") + " " + QCoreApplication::applicationName());

    setAboutText();

    m_ui->aboutText->setTextColor(QColor(26, 26, 26));

#ifdef __APPLE__
    QFont fontToUse = QFont(QFontInfo(QApplication::font()).family()).exactMatch()
            ? QFontInfo(QApplication::font()).family()
            : QStringLiteral("Inter");
    m_ui->aboutText->setFont(fontToUse);
#elif _WIN32
    QFont fontToUse = QFont(QStringLiteral("Segoe UI")).exactMatch() ? QStringLiteral("Segoe UI")
                                                                     : QStringLiteral("Inter");
    m_ui->aboutText->setFont(fontToUse);
#else
    m_ui->aboutText->setFont(QFont(QStringLiteral("Inter")));
#endif

    // load stylesheet for aboutText
    QFile cssFile(":/styles/about-window.css");
    cssFile.open(QFile::ReadOnly);
    m_ui->aboutText->setStyleSheet(cssFile.readAll());

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    QFile scollBarStyleFile(QStringLiteral(":/styles/components/custom-scrollbar.css"));
    scollBarStyleFile.open(QFile::ReadOnly);
    QString scrollbarStyleSheet = QString::fromLatin1(scollBarStyleFile.readAll());
    m_ui->aboutText->verticalScrollBar()->setStyleSheet(scrollbarStyleSheet);
#endif
}

AboutWindow::~AboutWindow()
{
    /* Delete UI controls */
    delete m_ui;
}

void AboutWindow::setAboutText()
{
    QString proVersionText = m_isProVersion ? " (Pro Version)" : "";

    m_ui->aboutText->setText(
            "<strong>Version:</strong> " + QCoreApplication::applicationVersion() + proVersionText
              +
              "<br/><br/><strong>Daino Notes:</strong> "
              "<br/><a href='https://www.get-notes.com?utm_source=daino_notes_app_about_window'>Daino Notes Website</a>"
              "<br/><a href='https://www.get-notes.com/notes-app-terms?utm_source=daino_notes_app_about_window'>Terms of Use</a>"
              "<br/><a href='https://www.get-notes.com/notes-app-privacy-policy?utm_source=daino_notes_app_about_window'>Privacy Policy</a>"
              "<br/><a href='https://discord.gg/rDP7xSQZW9'>Join our community on Discord</a>"
              "<br/><br/><strong>Daino:</strong> "
              "<br/><a href='https://www.daino.io?utm_source=daino_notes_app_about_window'>Daino Website</a>"
              "<br/><br/><strong>Daino Notes makes use of the following "
              "third-party libraries:</strong><br/>"
#if defined(UPDATE_CHECKER)
              "<br/>QSimpleUpdater"
#endif
              ""
              "<br/>QAutostart<br/>QXT<br/>MD4C<br/>html2md (by Tim Gromeyer)"
              "<br/>QBasicHtmlExporter (by Doug Beney)<br/>Krita's draganddrop plugin"
              "<br/>fts_fuzzy_match (by Forrest Smith)"
              // "<br>qml-flexbox"
              // "<br/>Yoga Layout"
              "<br/><br/><strong>Daino Notes "
              "makes use of the following open source "
              "fonts:</strong><br/><br/>Inter<br/>Trykker<br/>Ibarra Real Nova<br/>iA "
              "Writer Mono<br/>iA Writer Duo<br/>iA Writer "
              "Quattro<br/>Font Awesome<br/>Material Symbols"
              ""
              "<br/><br/>Daino Notes is based on the open source <a href='https://github.com/nuttyartist/notes'>Notes</a> app. Thanks to all the contributors who made this possible:"
              "<br/><br/>Alex Spataru<br/>Ali Diouri"
              "<br/>David Planella<br/>Diep Ngoc<br/>Guilherme "
              "Silva<br/>Kevin Doyle<br/>Thorbjørn Lindeijer<br/>Tuur Vanhoutte<br/>Waqar "
              "Ahmed"
              "<br/><br/><a href='https://github.com/nuttyartist/daino-notes-public'>Daino Notes GitHub page</a>"
              "<br/><br/><strong>Qt version:</strong> "
            + qVersion() + " (built with " + QT_VERSION_STR + ")");
}

void AboutWindow::setTheme(Theme::Value theme)
{
    setCSSThemeAndUpdate(m_ui->aboutText, theme);
}

void AboutWindow::setProVersion(bool isProVersion)
{
    m_isProVersion = isProVersion;
    setAboutText();
}
