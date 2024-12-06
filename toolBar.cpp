#include "toolBar.h"
#include <qpainter.h>
#include <qlayout.h>
toolBar::toolBar(QWidget* parent) : QWidget(parent)
{
	//setMouseTracking(true);
	FoldAni = new QPropertyAnimation(this);
	FoldAni->setTargetObject(this);
	FoldAni->setDuration(500);
	FoldAni->setPropertyName("geometry");
	FoldAni->setEasingCurve(QEasingCurve::OutQuart);
}

void toolBar::mousePressEvent(QMouseEvent* e)
{
	if (FoldAni->state() == FoldAni->Running)
		FoldAni->stop();
	if (e->buttons() & Qt::MouseButton::LeftButton)
	{
		previousMousePos = e->globalPos();
		substractPos.setX(previousMousePos.x() - geometry().x());
		substractPos.setY(previousMousePos.y() - geometry().y());
	}
}

void toolBar::mouseReleaseEvent(QMouseEvent* e)
{
	previousMousePos = QPoint();
}

void toolBar::mouseMoveEvent(QMouseEvent* event)
{
	qDebug() << event->globalPos().x() << "," << event->globalPos().y();
	if (previousMousePos != QPoint() && event->buttons() & Qt::MouseButton::LeftButton)
	{
		QWidget::move(event->globalPos().x() - substractPos.x(), event->globalPos().y() - substractPos.y());
		geometryRect.setTopLeft(QWidget::geometry().topLeft());
		Customized = true;
		bkWindowRect = this->parentWidget()->geometry();
	}
}

void toolBar::paintEvent(QPaintEvent* e)
{
	QPainter p(this);
	QWidget::paintEvent(e);
}