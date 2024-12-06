#include "DoubleClickButton.h"
QFile penIconFile;
QString penIconStr;
QRegularExpression QRE("fill=\".*?\"");
QRegularExpressionMatch QREM;
QString matchStr;
//样式表！
//QPushButton{background-repeat: no-repeat;background-position: center;background-image: url(":/svg/svgs/edit-outline.svg");}

DoubleClickButton::DoubleClickButton(QWidget* parent)
{
    penIconFile.setFileName(QString(":/svg/svgs/填充画笔.svg"));
    penIconFile.open(QIODevice::ReadOnly | QIODevice::Text);
    penIconStr = penIconFile.readAll();
    QREM = QRE.match(penIconStr);
    matchStr = QREM.captured();

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
void DoubleClickButton::paintEvent(QPaintEvent* e)
{
    QPushButton::paintEvent(e);
    //this->setIcon(QIcon());
    /*
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><g data-name="Layer 2"><g data-name="edit"><rect width="24" height="24" opacity="0"/>    <!--This is to be changed!--><circle cx="7" cy="17" r="2.5" fill="#00ff0000" /><path d="M19.4 7.34L16.66 4.6A2 2 0 0 0 14 4.53l-9 9a2 2 0 0 0-.57 1.21L4 18.91a1 1 0 0 0 .29.8A1 1 0 0 0 5 20h.09l4.17-.38a2 2 0 0 0 1.21-.57l9-9a1.92 1.92 0 0 0-.07-2.71zM9.08 17.62l-3 .28.27-3L12 9.32l2.7 2.7zM16 10.68L13.32 8l1.95-2L18 8.73z"/></g><g gata-name="backg"></g></g></svg>
    */
    QPainter penIconPainter(this);
    QSvgRenderer penIconRenderer;
    QString ColorHex = iconColor.name();
    penIconStr.replace(QRE, "fill=\"" + ColorHex + "\"");
    penIconRenderer.load(penIconStr.toLocal8Bit());
    penIconPainter.setRenderHint(QPainter::Antialiasing);
    QSize size = QSize(width() > height() ? height(), height() : width(), width());
    size = QSize(height(), height());
    QSize penBtnIconSize(size.height() * 3 / 5, size.height() * 3 / 5);//相对按钮的图标大小
    QPoint penBtnIconPos(QPoint((this->size().width() - penBtnIconSize.width()) / 2, (this->size().height() - penBtnIconSize.height()) / 2));
    QRect thisBtnIconR(penBtnIconPos, penBtnIconSize);
    penIconRenderer.render(&penIconPainter, thisBtnIconR);
    //penIconPainter.fillRect(this->rect(), iconColor);
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
