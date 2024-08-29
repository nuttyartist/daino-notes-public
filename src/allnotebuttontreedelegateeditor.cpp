#include "allnotebuttontreedelegateeditor.h"
#include <QPainter>
#include <QDebug>
#include <QTreeView>
#include "nodetreemodel.h"
#include "nodetreeview.h"
#include "notelistview.h"
#include "editorsettingsoptions.h"
#include <QPainterPath>

AllNoteButtonTreeDelegateEditor::AllNoteButtonTreeDelegateEditor(QTreeView *view,
                                                                 const QStyleOptionViewItem &option,
                                                                 const QModelIndex &index,
                                                                 QListView *listView,
                                                                 QWidget *parent)
    : QWidget(parent),
      m_option(option),
      m_index(index),
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
#ifdef __APPLE__
      m_titleFont(m_displayFont, 13, QFont::DemiBold),
      m_numberOfNotesFont(m_displayFont, 12, QFont::DemiBold),
#else
      m_titleFont(m_displayFont, 10, QFont::DemiBold),
      m_numberOfNotesFont(m_displayFont, 9, QFont::DemiBold),
#endif
      m_titleColor(69,67,69),
      m_titleSelectedColor(255, 255, 255),
      m_activeColor(68, 138, 201),
      m_hoverColor(237, 237, 237),
      m_folderIconColor(68, 138, 201),
      m_numberOfNotesColor(26, 26, 26, 127),
      m_numberOfNotesSelectedColor(255, 255, 255),
      m_view(view),
      m_listView(listView)
{
    setContentsMargins(0, 0, 0, 0);
}

void AllNoteButtonTreeDelegateEditor::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    auto iconRect = QRect(rect().x() + 27, rect().y() + (rect().height() - 20) / 2, 18, 20);
    auto iconPath = m_index.data(NodeItem::Roles::Icon).toString();
    auto displayName = m_index.data(NodeItem::Roles::DisplayText).toString();
    QRect nameRect(rect());
    nameRect.setLeft(iconRect.x() + iconRect.width() + 5);
    nameRect.setWidth(nameRect.width() - 22 - 50);

    int leftRightPadding = 10;
    int cornerRadius = 5;
    int topBottomPadding = 1;
    QRect adjustedRect = rect().adjusted(leftRightPadding, topBottomPadding, -leftRightPadding,
                                         -topBottomPadding);
    QPainterPath path;
    path.addRoundedRect(adjustedRect, cornerRadius, cornerRadius);

    if (m_view->selectionModel()->isSelected(m_index)) {
        //        painter.fillRect(rect(), QBrush(m_activeColor));
        painter.fillPath(path, QBrush(m_activeColor));
        painter.setPen(m_titleSelectedColor);
    } else {
        auto listView = dynamic_cast<NoteListView *>(m_listView);
        if (listView->isDragging()) {
            if (m_theme == Theme::Dark) {
                //                painter.fillRect(rect(), QBrush(QColor(35, 52, 69)));
                painter.fillPath(path, QBrush(QColor(35, 52, 69)));
            } else {
                //                painter.fillRect(rect(), QBrush(QColor(180, 208, 233)));
                painter.fillPath(path, QBrush(QColor(180, 208, 233)));
            }
        } else {
            //            painter.fillRect(rect(), QBrush(m_hoverColor));
            painter.fillPath(path, QBrush(m_hoverColor));
        }
        painter.setPen(m_folderIconColor);
    }
#ifdef __APPLE__
    int iconPointSizeOffset = 0;
#else
    int iconPointSizeOffset = -4;
#endif
    painter.setFont(QFont("Material Symbols Outlined", 16 + iconPointSizeOffset));
    painter.drawText(iconRect, iconPath); // folder

    if (m_view->selectionModel()->isSelected(m_index)) {
        painter.setPen(m_titleSelectedColor);
    } else {
        painter.setPen(m_titleColor);
    }

    painter.setFont(m_titleFont);
    painter.drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, displayName);
    auto childCountRect = rect();
    childCountRect.setLeft(nameRect.right() + 22);
    childCountRect.setWidth(childCountRect.width() - 5);
    childCountRect.setRight(childCountRect.right() - 13);
    auto childCount = m_index.data(NodeItem::Roles::ChildCount).toInt();
    if (m_view->selectionModel()->isSelected(m_index)) {
        painter.setPen(m_numberOfNotesSelectedColor);
    } else {
        painter.setPen(m_numberOfNotesColor);
    }
    painter.setFont(m_numberOfNotesFont);
    painter.drawText(childCountRect, Qt::AlignRight | Qt::AlignVCenter,
                     QString::number(childCount));
    QWidget::paintEvent(event);
}

void AllNoteButtonTreeDelegateEditor::setTheme(Theme::Value theme)
{
    m_theme = theme;
    switch (theme) {
    case Theme::Light: {
        m_hoverColor = QColor(237, 237, 237);
        m_titleColor = QColor(69,67,69);
        m_numberOfNotesColor = QColor(26, 26, 26, 127);
        break;
    }
    case Theme::Dark: {
        m_hoverColor = QColor(51, 51, 51);
        m_titleColor = QColor(212, 212, 212);
        m_numberOfNotesColor = QColor(212, 212, 212, 127);
        break;
    }
    case Theme::Sepia: {
        m_hoverColor = QColor(237, 237, 237);
        m_titleColor = QColor(69,67,69);
        m_numberOfNotesColor = QColor(26, 26, 26, 127);
        break;
    }
    }
}
