#pragma once
#include <qwidget.h>
#include <qwindow.h>
class ScreenShotMagnifier :
    public QWidget
{
public:
    ScreenShotMagnifier(QWidget* parent, QRect geometry, QWindow* magnificationWindow = nullptr);
    ScreenShotMagnifier(QWidget* parent, QRect geometry, WId winId = 0);
    ScreenShotMagnifier(QWidget* parent, QRect geometry, HWND hWnd = 0);
    ~ScreenShotMagnifier();
    void setMagnificationWindow(QWindow* window) {
        this->window = window;
    }
    void setMagnificationWindow(HWND hWnd) {
        window = QWindow::fromWinId((WId)hWnd);
    }
    void setMagnificationWindow(WId winId) {
        window = QWindow::fromWinId(winId);
    }
    void magUpdateProc();
protected:
    void paintEvent(QPaintEvent* e) override;
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void mouseMoveEvent(QMouseEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);

private:
    void init(QWidget* parent, QRect geometry, QWindow* magnificationWindow);
    QWidget* _parent = nullptr;
    int borderWidth = 10;//放大镜窗口边框宽度
    double MAGFACTOR = 2.0;//放大倍数
    //HWND screenShotWindowHWND = 0;
    QWindow* window = nullptr;
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
    QPoint mouseDownPos;
    QPoint mouseDownOffset;
    QRect originGeo;
    struct Padding {
        int paddingLeft, paddingTop, paddingRight, paddingBottom;
    } padding = { 0,0,0,0 };//放大镜窗口边框内边距
    QTimer* magnifierUpdateTimer = nullptr;
    QPixmap screenShot;
};

