#include "ScreenShotMagnifier.h"
#include <qpainter.h>
#include <qscreen.h>
#include <qmessagebox.h>
#include <Windows.h>
#include <qevent.h>
#include <qtimer.h>

ScreenShotMagnifier::ScreenShotMagnifier(QWidget* parent, QRect geometry, QWindow* magnificationWindow)
// : QWidget(parent)
{
	init(parent, geometry, magnificationWindow);
}
ScreenShotMagnifier::ScreenShotMagnifier(QWidget* parent, QRect geometry, WId winId)
{
	QWindow* sswindow = QWindow::fromWinId(winId);
	init(parent, geometry, sswindow);
}
ScreenShotMagnifier::ScreenShotMagnifier(QWidget* parent, QRect geometry, HWND hWnd)
{
	QWindow* sswindow = QWindow::fromWinId((WId)hWnd);
	init(parent, geometry, sswindow);
}
void ScreenShotMagnifier::init(QWidget* parent, QRect geometry, QWindow* magnificationWindow)
{
	setWindowFlags(Qt::WindowDoesNotAcceptFocus | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);
	setMouseTracking(true);
	installEventFilter(this);

	setWindowTitle("放大镜");
	if (_parent)
	{
		setWindowIcon(_parent->windowIcon());
		_parent = parent;
	}
	if (magnificationWindow)
		window = magnificationWindow;
	if (geometry.isValid())
		this->setGeometry(geometry);
	magnifierUpdateTimer = new QTimer(this);
	magnifierUpdateTimer->setSingleShot(false);
	magnifierUpdateTimer->setInterval(0);
	connect(magnifierUpdateTimer, &QTimer::timeout, this, &ScreenShotMagnifier::magUpdateProc);
	magnifierUpdateTimer->start();
}
ScreenShotMagnifier::~ScreenShotMagnifier()
{

}

void ScreenShotMagnifier::paintEvent(QPaintEvent* e)
{
	QPainter p(this);
	p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::LosslessImageRendering);
	p.setBrush(Qt::NoBrush);
	p.setPen(QPen(Qt::lightGray, borderWidth));
	QRect borderRect = rect();
	//borderRect.setTopLeft(QPoint(borderRect.left() + borderWidth / 2, borderRect.top() + borderWidth / 2));
	//borderRect.setBottomRight(QPoint(borderRect.right() - borderWidth / 2, borderRect.bottom() - borderWidth / 2));
	p.drawRect(borderRect);

	QRect picRect = rect();
	picRect.setTopLeft(QPoint(picRect.left() + borderWidth, picRect.top() + borderWidth));
	picRect.setBottomRight(QPoint(picRect.right() - borderWidth, picRect.bottom() - borderWidth));
	p.drawPixmap(picRect, screenShot);


	QWidget::paintEvent(e);
}

bool ScreenShotMagnifier::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
	MSG* msg = (MSG*)message;
	switch (msg->message)
	{
	case WM_NCHITTEST:
	{
		//QCursor cursor = this->cursor();
		//QPoint pos = mapFromGlobal(cursor.pos());
		//int xPos = pos.x();
		//int yPos = pos.y();
		//if (xPos < borderWidth && yPos < borderWidth)//左上角
		//	*result = HTTOPLEFT;
		//else if (xPos > width() - borderWidth && yPos < borderWidth)//右上角
		//	*result = HTTOPRIGHT;
		//else if (xPos < borderWidth && yPos > height() - borderWidth)//左下角
		//	*result = HTBOTTOMLEFT;
		//else if (xPos > width() - borderWidth && yPos > height() - borderWidth)//右下角
		//	*result = HTBOTTOMRIGHT;
		//else if (xPos < borderWidth)//左边
		//	*result = HTLEFT;
		//else if (xPos > width() - borderWidth)//右边
		//	*result = HTRIGHT;
		//else if (yPos < borderWidth)//上边
		//	*result = HTTOP;
		//else if (yPos > height() - borderWidth)//下边
		//	*result = HTBOTTOM;
		//else
		//	return false;
		//return true;
	}
	break;
	case WM_NCCALCSIZE:
	{
	}
	break;
	case WM_SIZE:   //要让窗体能够随着缩放改变，要响应WM_SIZE消息
	{

	}
	break;
	}
	return QWidget::nativeEvent(eventType, message, result);
}

bool ScreenShotMagnifier::eventFilter(QObject* watched, QEvent* event)
{
	return false;
}

void ScreenShotMagnifier::mouseMoveEvent(QMouseEvent* e)
{
	if (e->buttons() == Qt::MouseButton::NoButton)
	{
		QPoint pos = e->pos();
		if (pos.x() < borderWidth)//左边界
		{
			if (pos.y() < borderWidth)//左上角
				setCursor(Qt::SizeFDiagCursor);//左上角或右下角
			else if (pos.y() > height() - borderWidth)//左下角
				setCursor(Qt::SizeBDiagCursor);//左下角或右上角
			else//左边
				setCursor(Qt::SizeHorCursor);
		}
		else if (pos.x() > width() - borderWidth)//右边界
		{
			if (pos.y() < borderWidth)//右上角
				setCursor(Qt::SizeBDiagCursor);
			else if (pos.y() > height() - borderWidth)//右下角
				setCursor(Qt::SizeFDiagCursor);
			else//右边
				setCursor(Qt::SizeHorCursor);
		}
		else if (pos.y() < borderWidth)//上边界
			setCursor(Qt::SizeVerCursor);
		else if (pos.y() > height() - borderWidth)//下边界
			setCursor(Qt::SizeVerCursor);
		else
		{
			resizeDirection = None;
			setCursor(Qt::CustomCursor);
		}
	}
	if (e->buttons() & Qt::LeftButton)//鼠标左键拖动
	{
		if (resizeDirection == None)
		{
			//此处移动放大镜的父亲窗口
			move(mapToParent(mapFromGlobal(e->globalPos())) - mouseDownOffset);
			update();
		}
		QRect geo = this->geometry();
		QCursor cursor;
		int left = originGeo.x() + cursor.pos().x() - mouseDownPos.x();
		int top = originGeo.y() + cursor.pos().y() - mouseDownPos.y();
		//int right = originGeo.right() + cursor.pos().x() - mouseDownPos.x();
		//int bottom = originGeo.bottom() + cursor.pos().y() - mouseDownPos.y();
		int wid = originGeo.width() + cursor.pos().x() - mouseDownPos.x();
		int hei = originGeo.height() + cursor.pos().y() - mouseDownPos.y();
		if (resizeDirection & Left)
			geo.setX((left < geo.right() - 2 * borderWidth) ? left : (geo.right() - 2 * borderWidth));
		if (resizeDirection & Top)
			geo.setY((top < geo.bottom() - 2 * borderWidth) ? top : (geo.bottom() - 2 * borderWidth));
		if (resizeDirection & Right)
			geo.setWidth((wid > 2 * borderWidth) ? wid : (2 * borderWidth));
		if (resizeDirection & Bottom)
			geo.setHeight((hei > 2 * borderWidth) ? hei : (2 * borderWidth));
		setGeometry(geo);
	}
	QWidget::mouseMoveEvent(e);
}


void ScreenShotMagnifier::mousePressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
	{
		mouseDownOffset = mapToParent(e->pos()) - geometry().topLeft();
		originGeo = this->geometry();
		QCursor cursor;
		mouseDownPos = cursor.pos();
		QPoint pos = e->pos();
		if (pos.x() < borderWidth)//左边界
		{
			if (pos.y() < borderWidth)//左上角
				resizeDirection = TopLeft;
			else if (pos.y() > height() - borderWidth)//左下角
				resizeDirection = BottomLeft;
			else//左边
				resizeDirection = Left;
		}
		else if (pos.x() > width() - borderWidth)//右边界
		{
			if (pos.y() < borderWidth)//右上角
				resizeDirection = TopRight;
			else if (pos.y() > height() - borderWidth)//右下角
				resizeDirection = BottomRight;
			else//右边
				resizeDirection = Right;
		}
		else if (pos.y() < borderWidth)//上边界
			resizeDirection = Top;
		else if (pos.y() > height() - borderWidth)//下边界
			resizeDirection = Bottom;
	}
	QWidget::mousePressEvent(e);
}

void ScreenShotMagnifier::mouseReleaseEvent(QMouseEvent* e)
{
	resizeDirection = None;
	mouseDownOffset = QPoint();
	QWidget::mouseReleaseEvent(e);
}

#include <iostream>
void ScreenShotMagnifier::magUpdateProc()
{
	if (window)
	{
		QRect picRect = rect();
		picRect.setTopLeft(QPoint(picRect.left() + borderWidth, picRect.top() + borderWidth));
		picRect.setBottomRight(QPoint(picRect.right() - borderWidth, picRect.bottom() - borderWidth));

		QRect thisWindowRect = rect();//相对于屏幕的坐标
		thisWindowRect.moveTo(mapToGlobal(thisWindowRect.topLeft()));
		QPoint sourceCenterPoint;//放大来源中心位置
		QRect slideShowWindowRect = window->geometry();
		slideShowWindowRect.moveTo(window->mapToGlobal(slideShowWindowRect.topLeft()));
		int sx = slideShowWindowRect.width();
		int sy = slideShowWindowRect.height();
		if (sx - thisWindowRect.width())
			sourceCenterPoint.setX(slideShowWindowRect.left() + ((thisWindowRect.left() - slideShowWindowRect.left()) * sx / (sx - thisWindowRect.width())));
		if (sy - thisWindowRect.height())
			sourceCenterPoint.setY(slideShowWindowRect.top() + ((thisWindowRect.top() - slideShowWindowRect.top()) * sy / (sy - thisWindowRect.height())));
		//放大资源的矩形宽高
		int width = (int)(picRect.width() / MAGFACTOR);
		int height = (int)(picRect.height() / MAGFACTOR);
		QRect sourceRect;//放大资源的矩形
		sourceRect.setLeft(sourceCenterPoint.x() - width / 2);
		sourceRect.setTop(sourceCenterPoint.y() - height / 2);

		// Don't scroll outside desktop area.
		// Don't scroll outside Client area.
		if (sourceRect.left() < slideShowWindowRect.left())
			sourceRect.setLeft(slideShowWindowRect.left());
		if (sourceRect.left() > slideShowWindowRect.right() - width)
			sourceRect.setLeft(slideShowWindowRect.right() - width);
		sourceRect.setRight(sourceRect.left() + width);
		if (sourceRect.top() < slideShowWindowRect.top())
			sourceRect.setTop(slideShowWindowRect.top());
		if (sourceRect.top() > slideShowWindowRect.bottom() - height)
			sourceRect.setTop(slideShowWindowRect.bottom() - height);
		sourceRect.setBottom(sourceRect.top() + height);
		sourceRect.moveTo(window->mapFromGlobal(sourceRect.topLeft()));

		//截图
		QScreen* screen = window->screen();
		//std::cout << (long)window->winId() << std::endl;
		
		if (!isHidden() && !isMinimized() && isVisible())
		{
			screenShot = screen->grabWindow(window->winId(), sourceRect.x(), sourceRect.y(), sourceRect.width(), sourceRect.height());
			
			//screenShot = screen->grabWindow(window->winId());

			//slideShowWindowRect.moveTo(window->mapToGlobal(slideShowWindowRect.topLeft()));
			//screenShot = screen->grabWindow(0, slideShowWindowRect.x(), slideShowWindowRect.y(), slideShowWindowRect.width(), slideShowWindowRect.height());

		}


		//screenShot.save("ScreenShot.png");
		//if (!screenShot.save("ScreenShot.png"))
		//	QMessageBox::critical(this, "错误", "截图保存失败！");
		//else
		//	QMessageBox::information(this, "提示", "截图保存成功！");
		update();
	}

}