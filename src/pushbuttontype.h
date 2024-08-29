#ifndef PUSHBUTTONTYPE_H
#define PUSHBUTTONTYPE_H

#include <QPushButton>

#include "editorsettingsoptions.h"

class PushButtonType : public QPushButton
{
    Q_OBJECT
public:
    explicit PushButtonType(QWidget *parent = nullptr);

    void setNormalIcon(const QIcon &newNormalIcon);
    void setHoveredIcon(const QIcon &newHoveredIcon);
    void setPressedIcon(const QIcon &newPressedIcon);
    void setTheme(Theme::Value theme);

protected:
    bool event(QEvent *event) override;

private:
    QIcon normalIcon;
    QIcon hoveredIcon;
    QIcon pressedIcon;
};

#endif // PUSHBUTTONTYPE_H
