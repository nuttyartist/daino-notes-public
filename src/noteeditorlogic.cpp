#include "noteeditorlogic.h"
#include "customMarkdownHighlighter.h"
#include "dbmanager.h"
#include "taglistview.h"
#include "taglistmodel.h"
#include "tagpool.h"
#include "taglistdelegate.h"
#include <QScrollBar>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QDebug>
#include <QCursor>

#define FIRST_LINE_MAX 80

NoteEditorLogic::NoteEditorLogic(QLineEdit *searchEdit, TagListView *tagListView, TagPool *tagPool,
                                 DBManager *dbManager, BlockModel *blockModel, QObject *parent)
    : QObject(parent),
      m_highlighter{ new CustomMarkdownHighlighter{ new QTextDocument() } },
      m_searchEdit{ searchEdit },
      m_tagListView{ tagListView },
      m_dbManager{ dbManager },
      m_isContentModified{ false },
      m_spacerColor{ 191, 191, 191 },
      m_currentAdaptableEditorPadding{ 0 },
      m_currentMinimumEditorPadding{ 0 },
      m_blockModel{ blockModel }
{
    connect(m_blockModel, &BlockModel::textChangeFinished, this,
            &NoteEditorLogic::onBlockModelTextChanged);
    connect(m_blockModel, &BlockModel::verticalScrollBarPositionChanged, this,
            [this](double scrollBarPosition, int itemIndexInView) {
                Q_UNUSED(scrollBarPosition);
                if (currentEditingNoteId() != SpecialNodeID::InvalidNodeId) {
                    // TODO: Crate seperate `scrollBarPosition` and `itemIndexInView` in NoteData
                    // and database
                    m_currentNotes[0].setScrollBarPosition(itemIndexInView);
                    emit updateNoteDataInList(m_currentNotes[0]);
                    m_isContentModified = true;
                    m_autoSaveTimer.start();
                    emit setVisibilityOfFrameRightWidgets(false);
                } else {
                    qDebug() << "NoteEditorLogic::onTextEditTextChanged() : m_currentNote is not "
                                "valid";
                }
            });
    connect(this, &NoteEditorLogic::requestCreateUpdateNote, m_dbManager,
            &DBManager::onCreateUpdateRequestedNoteContent, Qt::QueuedConnection);
    // auto save timer
    m_autoSaveTimer.setSingleShot(true);
    m_autoSaveTimer.setInterval(250);
    connect(&m_autoSaveTimer, &QTimer::timeout, this, [this]() { saveNoteToDB(); });
    m_tagListModel = new TagListModel{ this };
    m_tagListModel->setTagPool(tagPool);
    m_tagListView->setModel(m_tagListModel);
    m_tagListDelegate = new TagListDelegate{ this };
    m_tagListView->setItemDelegate(m_tagListDelegate);
    connect(tagPool, &TagPool::dataUpdated, this, [this](int) { showTagListForCurrentNote(); });
}

void NoteEditorLogic::closeEditor()
{
    m_blockModel->setNothingLoaded();
    if (currentEditingNoteId() != SpecialNodeID::InvalidNodeId) {
        saveNoteToDB();
        emit noteEditClosed(m_currentNotes[0], false);
    }
    m_currentNotes.clear();

    m_tagListModel->setModelData({});
}

bool NoteEditorLogic::markdownEnabled() const
{
    return m_highlighter->document() != nullptr;
}

void NoteEditorLogic::showNotesInEditor(const QVector<NodeData> &notes, bool isCalledFromShortcut)
{
    auto currentId = currentEditingNoteId();
    if (notes.size() == 1 && notes[0].id() != SpecialNodeID::InvalidNodeId) {
        if (currentId != SpecialNodeID::InvalidNodeId && notes[0].id() != currentId) {
            emit noteEditClosed(m_currentNotes[0], false);
        }
        emit m_blockModel->numberOfSelectedNotesChanged(1);

        m_currentNotes = notes;
        showTagListForCurrentNote();

        QString content = notes[0].content();
        int scrollbarPos = notes[0].scrollBarPosition();

        m_blockModel->setVerticalScrollBarPosition(0, scrollbarPos);
        m_blockModel->loadText(content, isCalledFromShortcut);

        // QDateTime dateTime = notes[0].lastModificationdateTime();
        // QString noteDate = dateTime.toString(Qt::ISODate);
        // QString noteDateEditor = getNoteDateEditor(noteDate);
    } else if (notes.size() > 1) {
        emit m_blockModel->numberOfSelectedNotesChanged(notes.size());

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
        emit checkMultipleNotesSelected(QVariant(true));
#endif
        m_currentNotes = notes;
        m_tagListView->setVisible(false);
    } else {
        qDebug() << "NoteEditorLogic::showNotesInEditor() : Invalid node id";
        m_blockModel->setNothingLoaded();
    }
}

void NoteEditorLogic::onBlockModelTextChanged()
{
    if (currentEditingNoteId() != SpecialNodeID::InvalidNodeId) {
        QString content = m_currentNotes[0].content();
        QString sourceDocumentPlainText = m_blockModel->sourceDocument()->toPlainText();
        if (sourceDocumentPlainText != content) {
            // move note to the top of the list
            emit moveNoteToListViewTop(m_currentNotes[0]);

            // Get the new data
            QString firstline = getFirstLine(sourceDocumentPlainText);
            QDateTime dateTime = QDateTime::currentDateTime();
            QString noteDate = dateTime.toString(Qt::ISODate);
            // update note data
            m_currentNotes[0].setContent(sourceDocumentPlainText);
            m_currentNotes[0].setFullTitle(firstline);
            m_currentNotes[0].setLastModificationDateTime(dateTime);
            m_currentNotes[0].setIsTempNote(false);
            emit updateNoteDataInList(m_currentNotes[0]);
            m_isContentModified = true;
            m_autoSaveTimer.start();
            emit setVisibilityOfFrameRightWidgets(false);
        }
    } else {
        qDebug() << "NoteEditorLogic::onTextEditTextChanged() : m_currentNote is not valid";
    }
}

QDateTime NoteEditorLogic::getQDateTime(const QString &date)
{
    QDateTime dateTime = QDateTime::fromString(date, Qt::ISODate);
    return dateTime;
}

void NoteEditorLogic::showTagListForCurrentNote()
{
    if (currentEditingNoteId() != SpecialNodeID::InvalidNodeId) {
        auto tagIds = m_currentNotes[0].tagIds();
        if (tagIds.count() > 0) {
            m_tagListView->setVisible(true);
            m_tagListModel->setModelData(tagIds);
            return;
        }
        m_tagListModel->setModelData(tagIds);
    }
    m_tagListView->setVisible(false);
}

bool NoteEditorLogic::isInEditMode() const
{
    if (m_currentNotes.size() == 1) {
        return true;
    }
    return false;
}

int NoteEditorLogic::currentMinimumEditorPadding() const
{
    return m_currentMinimumEditorPadding;
}

void NoteEditorLogic::setCurrentMinimumEditorPadding(int newCurrentMinimumEditorPadding)
{
    m_currentMinimumEditorPadding = newCurrentMinimumEditorPadding;
}

int NoteEditorLogic::currentAdaptableEditorPadding() const
{
    return m_currentAdaptableEditorPadding;
}

void NoteEditorLogic::setCurrentAdaptableEditorPadding(int newCurrentAdaptableEditorPadding)
{
    m_currentAdaptableEditorPadding = newCurrentAdaptableEditorPadding;
}

int NoteEditorLogic::currentEditingNoteId() const
{
    if (isInEditMode()) {
        return m_currentNotes[0].id();
    }
    return SpecialNodeID::InvalidNodeId;
}

void NoteEditorLogic::saveNoteToDB()
{
    if (currentEditingNoteId() != SpecialNodeID::InvalidNodeId && m_isContentModified
        && !m_currentNotes[0].isTempNote()) {
        emit requestCreateUpdateNote(m_currentNotes[0]);
        m_isContentModified = false;
    }
}

void NoteEditorLogic::onNoteTagListChanged(int noteId, const QSet<int> &tagIds)
{
    if (currentEditingNoteId() == noteId) {
        m_currentNotes[0].setTagIds(tagIds);
        showTagListForCurrentNote();
    }
}

void NoteEditorLogic::deleteCurrentNote()
{
    if (isTempNote()) {
        auto noteNeedDeleted = m_currentNotes[0];
        m_currentNotes.clear();
        emit noteEditClosed(noteNeedDeleted, true);
    } else if (currentEditingNoteId() != SpecialNodeID::InvalidNodeId) {
        auto noteNeedDeleted = m_currentNotes[0];
        saveNoteToDB();
        m_currentNotes.clear();
        emit noteEditClosed(noteNeedDeleted, false);
        emit deleteNoteRequested(noteNeedDeleted);
    }
}

QString NoteEditorLogic::getNthLine(const QString &str, int targetLineNumber)
{
    if (targetLineNumber < 1) {
        return tr("Invalid line number");
    }

    int previousLineBreakIndex = -1;
    int lineCount = 0;
    for (int i = 0; i <= str.length(); i++) {
        if (i == str.length() || str[i] == '\n') {
            lineCount++;
            if (lineCount >= targetLineNumber && (i - previousLineBreakIndex > 1 || (i > 0 && i == str.length() && str[i-1] != '\n'))) {
                QString line = str.mid(previousLineBreakIndex + 1, i - previousLineBreakIndex - 1);
                line = line.trimmed();
                if (!line.isEmpty() && !line.startsWith("{{") && !line.startsWith("---") && !line.startsWith("```")) {
                    line.replace("<br />", "\n");
                    QTextDocument doc;
                    doc.setMarkdown(line);
                    QString text = doc.toPlainText();
                    if (text.length() > 1 && text.first(1) == "^") {
                        text = text.mid(1);
                    }
                    if (text.isEmpty()) {
                        return tr("No additional text");
                    }
                    QTextStream ts(&text);
                    return ts.readLine(FIRST_LINE_MAX);
                }
            }
            previousLineBreakIndex = i;
        }
    }

    return tr("No additional text");
}

/*!
 * \brief NoteEditorLogic::getFirstLine
 * Get a string 'str' and return only the first line of it
 * If the string contain no text, return "New Note"
 * TODO: We might make it more efficient by not loading the entire string into the memory
 * \param str
 * \return
 */
QString NoteEditorLogic::getFirstLine(const QString &str)
{
    return getNthLine(str, 1);
}

QString NoteEditorLogic::getSecondLine(const QString &str)
{
    return getNthLine(str, 2);
}

void NoteEditorLogic::setTheme(Theme::Value theme, QColor textColor, qreal fontSize)
{
    m_tagListDelegate->setTheme(theme);
    m_highlighter->setTheme(theme, textColor, fontSize);
    switch (theme) {
    case Theme::Light: {
        m_spacerColor = QColor(191, 191, 191);
        break;
    }
    case Theme::Dark: {
        m_spacerColor = QColor(212, 212, 212);
        break;
    }
    case Theme::Sepia: {
        m_spacerColor = QColor(191, 191, 191);
        break;
    }
    }
}

QString NoteEditorLogic::getNoteDateEditor(const QString &dateEdited)
{
    QDateTime dateTimeEdited(getQDateTime(dateEdited));
    QLocale usLocale(QLocale(QStringLiteral("en_US")));

    return usLocale.toString(dateTimeEdited, QStringLiteral("MMMM d, yyyy, h:mm A"));
}

void NoteEditorLogic::highlightSearch() const
{
    QString searchString = m_searchEdit->text();

    if (searchString.isEmpty())
        return;
}

bool NoteEditorLogic::isTempNote() const
{
    if (currentEditingNoteId() != SpecialNodeID::InvalidNodeId && m_currentNotes[0].isTempNote()) {
        return true;
    }
    return false;
}
