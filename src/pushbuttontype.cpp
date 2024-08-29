#include "pushbuttontype.h"
#include <QEvent>

PushButtonType::PushButtonType(QWidget *parent) : QPushButton(parent) { }

bool PushButtonType::event(QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        setIcon(pressedIcon);
    }
    if (event->type() == QEvent::MouseButtonRelease) {
        if (underMouse()) {
            setIcon(hoveredIcon);
        } else {
            setIcon(normalIcon);
        }
    }
    if (event->type() == QEvent::Enter) {
        setIcon(hoveredIcon);
    }

    if (event->type() == QEvent::Leave) {
        setIcon(normalIcon);
    }
    return QPushButton::event(event);
}

void PushButtonType::setPressedIcon(const QIcon &newPressedIcon)
{
    pressedIcon = newPressedIcon;
}

void PushButtonType::setHoveredIcon(const QIcon &newHoveredIcon)
{
    hoveredIcon = newHoveredIcon;
}

void PushButtonType::setNormalIcon(const QIcon &newNormalIcon)
{
    normalIcon = newNormalIcon;
    setIcon(newNormalIcon);
}

void PushButtonType::setTheme(Theme::Value theme)
{
    switch (theme) {
    case Theme::Light: {
        this->setStyleSheet(QStringLiteral(R"(QPushButton { )"
                                           R"(    border: none; )"
                                           R"(    padding: 0px; )"
                                           R"(    color: rgb(100, 100, 100); )"
                                           R"(})"
                                           R"(QPushButton:hover { )"
                                           R"(    border-radius: 5px;)"
                                           R"(    background-color: rgb(223, 223, 223); )"
                                           R"(})"
                                           R"(QPushButton:pressed { )"
                                           R"(    background-color: rgb(207, 207, 207); )"
                                           R"(})"));
        break;
    }
    case Theme::Dark: {
        this->setStyleSheet(QStringLiteral(R"(QPushButton { )"
                                           R"(    border: none; )"
                                           R"(    padding: 0px; )"
                                           R"(    color: rgb(162, 163, 164); )"
                                           R"(})"
                                           R"(QPushButton:hover { )"
                                           R"(    border-radius: 5px;)"
                                           R"(    background-color: rgb(65, 65, 65); )"
                                           R"(})"
                                           R"(QPushButton:pressed { )"
                                           R"(    background-color: rgb(61, 61, 61); )"
                                           R"(})"));
        break;
    }
    case Theme::Sepia: {
        this->setStyleSheet(QStringLiteral(R"(QPushButton { )"
                                           R"(    border: none; )"
                                           R"(    padding: 0px; )"
                                           R"(    color: rgb(100, 100, 100); )"
                                           R"(})"
                                           R"(QPushButton:hover { )"
                                           R"(    border-radius: 5px;)"
                                           R"(    background-color: rgb(223, 223, 223); )"
                                           R"(})"
                                           R"(QPushButton:pressed { )"
                                           R"(    background-color: rgb(207, 207, 207); )"
                                           R"(})"));
        break;
    }
    }
}
