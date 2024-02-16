
#include <qpushbutton.h>
#include <qcolordialog.h>
#include <qmessagebox.h>
#include <qevent.h>

class DoubleClickButton :
    public QPushButton
{

public:
    DoubleClickButton(QWidget* parent = nullptr);
    ~DoubleClickButton();

protected:
    void mouseDoubleClickEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
private:
    //HWND ColorDialog1 = NULL;
    //QColor PointerColor1 = Qt::red;

public:
    //QColor getPointerColor();
    //HWND getColorDialogHWND();
};

