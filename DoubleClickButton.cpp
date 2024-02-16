#include "DoubleClickButton.h"

DoubleClickButton::DoubleClickButton(QWidget* parent)
{
}

DoubleClickButton::~DoubleClickButton()
{
}

void DoubleClickButton::mouseDoubleClickEvent(QMouseEvent* e)
{
    QPushButton::mouseDoubleClickEvent(e);
}
void DoubleClickButton::mouseReleaseEvent(QMouseEvent* e)
{
    QPushButton::mouseReleaseEvent(e);
}

//QColor DoubleClickButton::getPointerColor()
//{
//    return PointerColor1;
//}
//
//HWND DoubleClickButton::getColorDialogHWND()
//{
//    return ColorDialog1;
//}
