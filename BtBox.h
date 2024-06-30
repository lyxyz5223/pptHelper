#pragma once
#include <qwidget.h>
#include <qpainter.h>

class BtBox :
    public QWidget
{
public:
    BtBox(QWidget* parent = nullptr);
    ~BtBox();


private:

protected:
    void paintEvent(QPaintEvent* e);
};

