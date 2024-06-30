#pragma once
#include <qpushbutton.h>
#include <qpainter.h>
class Bt :
    public QPushButton
{
public:
    Bt(QWidget* parent = nullptr);

    ~Bt();
protected:

    void paintEvent(QPaintEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);

private:
    QBrush brush = QColor(255, 255, 0, 150);
    QString bttext;
};

