#include "Bt.h"

Bt::Bt(QWidget* parent)
    : QPushButton(parent)
{
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setWindowFlags(Qt::FramelessWindowHint);
    bttext = this->text();
}

Bt::~Bt()
{

}

void Bt::paintEvent(QPaintEvent* e)
{
    QPainter p(this);
    p.setPen(QPen(QColor(0,0,0,255)));
    p.setBrush(brush);
    QRect qrect;
    qrect.setRect(this->rect().x(), rect().y(), rect().width() - 1, rect().height() - 1);
    //p.drawRect();
    p.setRenderHint(QPainter::Antialiasing);
    p.drawRoundedRect(qrect, 50, 50, Qt::RelativeSize);
    
    p.drawText(rect(),bttext);
    QWidget::paintEvent(e);
    //QPushButton::paintEvent(e);
}

void Bt::mousePressEvent(QMouseEvent* e)
{
    //brush.setColor(QColor(brush.color().red() - 10, brush.color().green() - 10, brush.color().blue() - 10,brush.color().alpha()));
    QPushButton::mousePressEvent(e);
}

void Bt::mouseReleaseEvent(QMouseEvent* e)
{
    //brush.setColor(QColor(brush.color().red() + 10, brush.color().green() + 10, brush.color().blue() + 10, brush.color().alpha()));
    QPushButton::mouseReleaseEvent(e);
}

