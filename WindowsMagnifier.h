#pragma once
#include <qwidget.h>
#include <qmessagebox.h>
#include <Windows.h>
class MyWindowsMagnifier :
    public QWidget
{
public:
	enum Mode {
		system,
		customize
	};

	MyWindowsMagnifier(QWidget* parent, QRect geometry = QRect());//Magnification API要求窗口样式必须包含WS_CHILD，所以这里parent不能为nullptr
	MyWindowsMagnifier(QWidget* parent, Mode mode, QRect geometry = QRect());//Magnification API要求窗口样式必须包含WS_CHILD，所以这里parent不能为nullptr
	~MyWindowsMagnifier();
	bool init(QRect geometry, Mode mode);
public slots:
	void UpdateMagWindow();
protected:
	void paintEvent(QPaintEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
	void mousePressEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
	bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result);
	bool eventFilter(QObject* watched, QEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;
	void showErrorMessageBox() {
		QMessageBox::critical(nullptr, "Error", "Magnification API requires that the window style must include WS_CHILD, so the parent cannot be nullptr.");
	}
private:
	QWidget* _parent = nullptr;
	//struct Ratio {
	//	double left = 0.0,
	//		top = 0.0,
	//		width = 0.0,
	//		height = 0.0;
	//	bool valid() {
	//		return left > 0 && top > 0 && width > 0 && height > 0;
	//	}
	//	operator bool() {
	//		return valid();
	//	}
	//} ratio;// 放大镜窗口相对于父窗口的位置和大小(相对坐标),大/小
	float MAGFACTOR = 2.0f;//放大倍数
	HWND hwndHost = (HWND)this->winId();//父窗口句柄(当前窗口句柄)
	HWND hwndMag = NULL;//放大镜窗口句柄
	int borderWidth = 10;//放大镜窗口边框宽度
	struct Padding {
		int paddingLeft, paddingTop, paddingRight, paddingBottom;
	} padding = { 0,0,0,0 };//放大镜窗口边框内边距

	enum ResizeDirection {
		None = 0x0000,
		Left = 0x0001,
		Right = 0x0010,
		Top = 0x0100,
		Bottom = 0x1000,
		TopLeft = Top | Left,
		TopRight = Top | Right,
		BottomLeft = Bottom | Left,
		BottomRight = Bottom | Right
	} resizeDirection = None;
	Mode mode = Mode::customize;
	QPoint mouseDownPos;
	QPoint mouseDownOffset;
	QRect originGeo;
};

