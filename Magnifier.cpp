#include "Magnifier.h"
#include <qpainter.h>
#include <qtimer.h>
#include <qevent.h>
#include <qpropertyanimation.h>
#include <qwindow.h>
#include <Windowsx.h>
#include <magnification.h>
#pragma comment(lib, "Magnification.lib")

MyMagnifier::MyMagnifier(QWidget* parent, QRect geometry)// : QWidget(parent)
{
	if (!parent)
	{
		showErrorMessageBox();
		return;
	}
	else
		_parent = parent;
	init(geometry, Mode::customize);
}

MyMagnifier::MyMagnifier(QWidget* parent, Mode mode, QRect geometry) : QWidget(parent)
{
	if (!parent)
	{
		showErrorMessageBox();
		return;
	}
	else
		_parent = parent;
	this->mode = mode;
	init(geometry, mode);
}

MyMagnifier::~MyMagnifier()
{
	MagUninitialize();

}

bool MyMagnifier::init(QRect geometry, Mode mode)
{
	if (!_parent)
	{
		showErrorMessageBox();
		return false;
	}

	setWindowFlag(Qt::WindowDoesNotAcceptFocus);
	setWindowFlag(Qt::WindowStaysOnTopHint);
	setWindowFlag(Qt::FramelessWindowHint);
	//setAttribute(Qt::WA_TranslucentBackground);
	SetWindowLong((HWND)this->winId(), GWL_EXSTYLE, GetWindowLong((HWND)this->winId(), GWL_EXSTYLE) | WS_EX_LAYERED);
	SetWindowLong((HWND)winId(), GWL_EXSTYLE , GetWindowLong((HWND)winId(), GWL_EXSTYLE) & (~WS_EX_TRANSPARENT));
	SetWindowPos((HWND)winId(), HWND_TOPMOST, 0, 0, 0, 0,
		SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

	setMouseTracking(true);
	installEventFilter(this);
	setWindowIcon(_parent->windowIcon());
	if (geometry.isValid())
	{
		resize(geometry.size());
		move(geometry.topLeft());
		//ratio = {
		//	(double)parent->width() / geometry.left(),
		//	(double)parent->height() / geometry.top(),
		//	(double)parent->rect().width() / geometry.width(),
		//	(double)parent->rect().height() / geometry.height()
		//};
	}

	// 初始化 Magnification API
	if (!MagInitialize())
		MessageBox(NULL, L"Failed to initialize Magnification API", L"Error", MB_OK | MB_ICONERROR);


	hwndHost = (HWND)this->winId();//父窗口句柄(当前窗口句柄)
	hwndMag = NULL;//放大镜窗口句柄
	RECT magWindowRect;//放大镜窗口矩形
	// Get the host window rectangle.
	GetWindowRect(hwndHost, &magWindowRect);
	// Get the instance of the application.
	HANDLE hInst = GetModuleHandle(NULL);

	// Make the window opaque.
	SetLayeredWindowAttributes(hwndHost, 0, 255, LWA_ALPHA);

	// Create a magnifier control that fills the client area.
	GetClientRect(hwndHost, &magWindowRect);
	hwndMag = CreateWindow(WC_MAGNIFIER, TEXT("MagnifierWindow"),
		WS_CHILD | MS_SHOWMAGNIFIEDCURSOR | WS_VISIBLE,
		magWindowRect.left, magWindowRect.top, magWindowRect.right, magWindowRect.bottom, hwndHost, NULL, (HINSTANCE)hInst, NULL);

	if (!hwndMag)
	{
		MessageBox(hwndHost, L"Failed to magnifier window", L"Error", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	SetWindowLong((HWND)winId(), GWL_EXSTYLE, GetWindowLong((HWND)winId(), GWL_EXSTYLE) & (~WS_EX_TRANSPARENT));

	// Set the magnification factor.
	MAGTRANSFORM matrix;
	memset(&matrix, 0, sizeof(matrix));
	matrix.v[0][0] = MAGFACTOR;
	matrix.v[1][1] = MAGFACTOR;
	matrix.v[2][2] = 1.0f;

	BOOL ret = MagSetWindowTransform(hwndMag, &matrix);

	if (ret)
	{
		MAGCOLOREFFECT magEffectInvert =
		{
			{ // MagEffectInvert
				//{ -1.0f,  0.0f,  0.0f,  0.0f,  0.0f },
				//{  0.0f, -1.0f,  0.0f,  0.0f,  0.0f },
				//{  0.0f,  0.0f, -1.0f,  0.0f,  0.0f },
				//{  0.0f,  0.0f,  0.0f,  1.0f,  0.0f },
				//{  1.0f,  1.0f,  1.0f,  0.0f,  1.0f }

				{1.0f, 0.0f, 0.0f, 0.0f, 0.0f},  // 红色分量
				{0.0f, 1.0f, 0.0f, 0.0f, 0.0f},  // 绿色分量
				{0.0f, 0.0f, 1.0f, 0.0f, 0.0f},  // 蓝色分量
				{0.0f, 0.0f, 0.0f, 1.0f, 0.0f},  // Alpha 分量
				{0.0f, 0.0f, 0.0f, 0.0f, 1.0f}   // 常量偏移量
			}
		};

		ret = MagSetColorEffect(hwndMag, &magEffectInvert);
	}
	else
	{
		MessageBox(hwndHost, L"Failed to set magnification transform", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}
	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &MyMagnifier::UpdateMagWindow);
	timer->setSingleShot(false);
	timer->setInterval(0);//刷新率60hz对应的间隔时间为16ms
	timer->start();

	return true;
}

void MyMagnifier::paintEvent(QPaintEvent* e)
{
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);
	//p.fillRect(this->rect(), Qt::red);
	p.setPen(Qt::NoPen);
	p.fillRect(this->rect(), Qt::white);
	QPen pen;
	pen.setColor(Qt::GlobalColor::darkGray);
	pen.setWidth(borderWidth);
	p.setPen(pen);
	QRect paintRect = this->rect();
	//paintRect.setCoords(
	//	paintRect.left() + borderWidth / 2,
	//	paintRect.top() + borderWidth / 2,
	//	paintRect.right() - borderWidth / 2,
	//	paintRect.bottom() - borderWidth / 2
	//);
	p.drawRect(paintRect);
	QWidget::paintEvent(e);
}

void MyMagnifier::mouseMoveEvent(QMouseEvent* e)
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
			//此处需要移动放大镜控件
			RECT thisWindowRect;
			RECT magWindowRect;
			GetClientRect((HWND)this->winId(), &thisWindowRect);
			GetClientRect(hwndMag, &magWindowRect);
			RECT parentWindowRect;
			GetWindowRect((HWND)_parent->winId(), &parentWindowRect);

			MoveWindow(hwndMag,
				borderWidth + padding.paddingLeft,
				borderWidth + padding.paddingTop,
				thisWindowRect.right - thisWindowRect.left - 2 * borderWidth - padding.paddingLeft - padding.paddingRight,
				thisWindowRect.bottom - thisWindowRect.top - 2 * borderWidth - padding.paddingTop - padding.paddingBottom,
				0
			);

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


void MyMagnifier::mousePressEvent(QMouseEvent* e)
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

void MyMagnifier::mouseReleaseEvent(QMouseEvent* e)
{
	resizeDirection = None;
	mouseDownOffset = QPoint();
	QWidget::mouseReleaseEvent(e);
}

bool MyMagnifier::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
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
	case WM_SIZE:   //要让窗体能够随着缩放改变，要响应WM-SIZE消息
	{

	}
	break;
	}
	return QWidget::nativeEvent(eventType, message, result);
}

bool MyMagnifier::eventFilter(QObject* watched, QEvent* event)
{
	//if ((HWND)((QWidget*)watched)->winId() == hwndHost)
	//{
	//	if (event->type() == QEvent::MouseButtonPress)
	//	{
	//		QMouseEvent* e = static_cast<QMouseEvent*>(event);
	//		if (e->button() == Qt::LeftButton)
	//		{
	//			mouseDownOffset = e->pos() - geometry().topLeft();
	//		}
	//	}
	//	else if (event->type() == QEvent::MouseMove)
	//	{
	//		QMouseEvent* e = static_cast<QMouseEvent*>(event);
	//		if (e->buttons() & Qt::LeftButton)
	//		{
	//			move(e->globalPos() - mouseDownOffset);
	//		}
	//	}
	//	else if (event->type() == QEvent::MouseButtonRelease)
	//	{
	//		QMouseEvent* e = static_cast<QMouseEvent*>(event);
	//		if (e->button() == Qt::LeftButton)
	//		{

	//		}
	//	}
	//}
	return false;
}

void MyMagnifier::resizeEvent(QResizeEvent* event)
{
	
}


//
// FUNCTION: UpdateMagWindow()
//
// PURPOSE: Sets the source rectangle and updates the window. Called by a timer.
//
void MyMagnifier::UpdateMagWindow()
{
	//if (geometryBackup != this->geometry())
	//{
	//	if (ratio.valid())
	//	{
	//		move(
	//			parentWidget()->width() / ratio.left,
	//			parentWidget()->height() / ratio.top
	//		);
	//		resize(
	//			parentWidget()->geometry().width() / ratio.width,
	//			parentWidget()->geometry().height() / ratio.height
	//		);
	//		geometryBackup = this->geometry();
	//	}
	//}

	RECT thisWindowRect;
	RECT magWindowRect;
	GetClientRect((HWND)this->winId(), &thisWindowRect);
	GetClientRect(hwndMag, &magWindowRect);
	RECT parentWindowRect;
	GetWindowRect((HWND)_parent->winId(), &parentWindowRect);

	//QRect thisWindowQRect = rect();
	//QWindow* window = QWindow::fromWinId((WId)hwndHost);
	//window->setGeometry(
	//	borderWidth + padding.paddingLeft,
	//	borderWidth + padding.paddingTop,
	//	thisWindowQRect.width() - 2 * borderWidth - padding.paddingLeft - padding.paddingRight,
	//	thisWindowQRect.height() - 2 * borderWidth - padding.paddingTop - padding.paddingBottom
	//);
	//window->deleteLater();
	MoveWindow(hwndMag,
		borderWidth + padding.paddingLeft,
		borderWidth + padding.paddingTop,
		thisWindowRect.right - thisWindowRect.left - 2 * borderWidth - padding.paddingLeft - padding.paddingRight,
		thisWindowRect.bottom - thisWindowRect.top - 2 * borderWidth - padding.paddingTop - padding.paddingBottom,
		0
	);


	POINT mousePoint = { 0 };
	switch (mode)
	{
	case MyMagnifier::system:
	{
		GetCursorPos(&mousePoint);
	}
	break;
	case MyMagnifier::customize:
	{
		RECT thisWindowRect;
		GetWindowRect((HWND)this->winId(), &thisWindowRect);
		int sx = parentWindowRect.right - parentWindowRect.left;
		int sy = parentWindowRect.bottom - parentWindowRect.top;
		if (sx - thisWindowRect.right + thisWindowRect.left)
			mousePoint.x = parentWindowRect.left + ((thisWindowRect.left - parentWindowRect.left) * sx / (sx - thisWindowRect.right + thisWindowRect.left));
		if (sy - thisWindowRect.bottom + thisWindowRect.top)
			mousePoint.y = parentWindowRect.top + ((thisWindowRect.top - parentWindowRect.top) * sy / (sy - thisWindowRect.bottom + thisWindowRect.top));
	}
	break;
	default:
		GetCursorPos(&mousePoint);
		break;
	}

	//放大资源的矩形宽高
	int width = (int)((magWindowRect.right - magWindowRect.left) / MAGFACTOR);
	int height = (int)((magWindowRect.bottom - magWindowRect.top) / MAGFACTOR);
	RECT sourceRect;//放大资源的矩形
	sourceRect.left = mousePoint.x - width / 2;
	sourceRect.top = mousePoint.y - height / 2;

	// Don't scroll outside desktop area.
	// Don't scroll outside Client area.
	if (sourceRect.left < parentWindowRect.left)
		sourceRect.left = parentWindowRect.left;
	if (sourceRect.left > parentWindowRect.right - width)
		sourceRect.left = parentWindowRect.right - width;
	sourceRect.right = sourceRect.left + width;

	if (sourceRect.top < parentWindowRect.top)
		sourceRect.top = parentWindowRect.top;
	if (sourceRect.top > parentWindowRect.bottom - height)
		sourceRect.top = parentWindowRect.bottom - height;
	sourceRect.bottom = sourceRect.top + height;

	// Set the source rectangle for the magnifier control.
	MagSetWindowSource(hwndMag, sourceRect);

	// Reclaim topmost status, to prevent unmagnified menus from remaining in view. 
	SetWindowPos(hwndHost, HWND_TOPMOST, 0, 0, 0, 0,
		SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

	// Force redraw.
	InvalidateRect(hwndMag, NULL, TRUE);
	update();
}
