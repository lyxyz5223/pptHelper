
#include <qpushbutton.h>
#include <qcolordialog.h>
#include <qmessagebox.h>
#include <qevent.h>
#include <QPainter>
#include <QFile>
#include <QRegularExpression>
#include <qsvgrenderer.h>


class DoubleClickButton :
    public QPushButton
{

public:
    DoubleClickButton(QWidget* parent = nullptr);
    ~DoubleClickButton();

protected:
    void mouseDoubleClickEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void paintEvent(QPaintEvent* e);
private:
    //HWND ColorDialog1 = NULL;
    //QColor PointerColor1 = Qt::red;
    QColor iconColor = Qt::red;
public:
    void setIconColor(QColor IconColor)
    {
        iconColor = IconColor;
        repaint();
    }
    //QColor getPointerColor();
    //HWND getColorDialogHWND();
};

