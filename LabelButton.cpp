#include "LabelButton.h"
#include <qevent.h>
LabelButton::LabelButton(QWidget* parent) : QLabel(parent)
{
	setSvgFile((":/svg/svgs/arrow-ios-back-outline.svg"));
}

void LabelButton::mouseReleaseEvent(QMouseEvent* e)
{
	if (e->globalPos() == pressPos && e->button() & Qt::MouseButton::LeftButton)
		emit clicked();
	pressPos = QPoint();
	e->ignore();
}
void LabelButton::mousePressEvent(QMouseEvent* e)
{
	if (underMouse() && e->button() & Qt::MouseButton::LeftButton)
		pressPos = e->globalPos();
	e->ignore();
}
#include <qpainter.h>
#include <qsvgrenderer.h>
void LabelButton::paintEvent(QPaintEvent* e)
{
	QPainter p(this);
	QSize size = QSize(width() > height() ? height(),height() : width(),width());
	size = QSize(height(), height());//
	QSize pixmapSize(size.width() * 3 / 5, size.height() * 3 / 5);//相对按钮的图标大小
	QPoint pixmapPos(QPoint((this->size().width() - pixmapSize.width()) / 2, (this->size().height() - pixmapSize.height()) / 2));
	QRect pixmapRect(pixmapPos, pixmapSize);
	//p.drawPixmap(pixmapRect, pixmap());
	//p.drawPixmap(pixmapRect, icon.pixmap(pixmapSize, QIcon::Mode::Normal, QIcon::State::Off));
	//p.drawImage(pixmapRect, image);
	QSvgRenderer penIconRenderer;
	penIconRenderer.load(svgStr.toLocal8Bit());
	p.setRenderHint(QPainter::Antialiasing);
	penIconRenderer.render(&p, pixmapRect);

}

