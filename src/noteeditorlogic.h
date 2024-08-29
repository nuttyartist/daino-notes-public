#ifndef NOTEEDITORLOGIC_H
#define NOTEEDITORLOGIC_H

#include <QObject>
#include <QTimer>
#include <QColor>
#include <QVector>
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
#  include <QWidget>
#  include <QVariant>
#  include <QJsonArray>
#  include <QJsonObject>
#  include <QRegularExpression>
#endif

#include "nodedata.h"
#include "editorsettingsoptions.h"
#include "blockmodel.h"

class CustomMarkdownHighlighter;
class QLabel;
class QLineEdit;
class DBManager;
class TagListView;
class TagListModel;
class TagPool;
class TagListDelegate;
class QListWidget;
class NoteEditorLogic : public QObject
{
    Q_OBJECT
public:
    explicit NoteEditorLogic(QLineEdit *searchEdit, TagListView *tagListView, TagPool *tagPool,
                             DBManager *dbManager, BlockModel *blockModel,
                             QObject *parent = nullptr);

    bool markdownEnabled() const;
    static QString getNoteDateEditor(const QString &dateEdited);
    void highlightSearch() const;
    bool isTempNote() const;
    void saveNoteToDB();
    int currentEditingNoteId() const;
    void deleteCurrentNote();

    static QString getNthLine(const QString &str, int lineNumber = 0);
    static QString getFirstLine(const QString &str);
    static QString getSecondLine(const QString &str);
    void setTheme(Theme::Value theme, QColor textColor, qreal fontSize);

    int currentAdaptableEditorPadding() const;
    void setCurrentAdaptableEditorPadding(int newCurrentAdaptableEditorPadding);

    int currentMinimumEditorPadding() const;
    void setCurrentMinimumEditorPadding(int newCurrentMinimumEditorPadding);

public slots:
    void showNotesInEditor(const QVector<NodeData> &notes, bool isCalledFromShortcut = false);
    void onBlockModelTextChanged();
    void closeEditor();
    void onNoteTagListChanged(int noteId, const QSet<int> &tagIds);
signals:
    void requestCreateUpdateNote(const NodeData &note);
    void noteEditClosed(const NodeData &note, bool selectNext);
    void setVisibilityOfFrameRightWidgets(bool);
    void setVisibilityOfFrameRightNonEditor(bool);
    void moveNoteToListViewTop(const NodeData &note);
    void updateNoteDataInList(const NodeData &note);
    void deleteNoteRequested(const NodeData &note);
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    void tasksFoundInEditor(QVariant data);
    void clearKanbanModel();
    void resetKanbanSettings();
    void checkMultipleNotesSelected(QVariant isMultipleNotesSelected);
#endif

private:
    static QDateTime getQDateTime(const QString &date);
    void showTagListForCurrentNote();
    bool isInEditMode() const;
    QString moveTextToNewLinePosition(const QString &inputText, int startLinePosition,
                                      int endLinePosition, int newLinePosition,
                                      bool isColumns = false);
    QMap<QString, int> getTaskDataInLine(const QString &line);
    void replaceTextBetweenLines(int startLinePosition, int endLinePosition, QString &newText);
    void removeTextBetweenLines(int startLinePosition, int endLinePosition);
    void appendNewColumn(QJsonArray &data, QJsonObject &currentColumn, QString &currentTitle,
                         QJsonArray &tasks);
    void addUntitledColumnToTextEditor(int startLinePosition);

private:
    CustomMarkdownHighlighter *m_highlighter;
    QLineEdit *m_searchEdit;
    TagListView *m_tagListView;
    DBManager *m_dbManager;
    QVector<NodeData> m_currentNotes;
    bool m_isContentModified;
    QTimer m_autoSaveTimer;
    TagListDelegate *m_tagListDelegate;
    TagListModel *m_tagListModel;
    QColor m_spacerColor;
    int m_currentAdaptableEditorPadding;
    int m_currentMinimumEditorPadding;
    BlockModel *m_blockModel;
};

#endif // NOTEEDITORLOGIC_H
