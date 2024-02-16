#include "pptHelper.h"
#include <iostream>

using namespace PowerPoint;



QRect screenRect;
bool comInitialProc();
HWND thisApp;
HWND ColorDialog1 = NULL;
bool dbclick[2] = { false,false };//检测是否双击按钮，dbclick[0]有用，dbclick[1]无用
QColor PointerColor1 = Qt::red;//画笔颜色
bool setPenSuccess = false;//判断是否成功设置画笔及其颜色

EA eaSink;
DWORD dw;
HRESULT hr;
int pptShowPosition=-1;
int pptShowTotalNum =-1;
pptHelper::pptHelper(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    thisApp = (HWND)this->winId();
    this->setWindowFlags(this->windowFlags()|Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint|Qt::Tool);
    this->setAttribute(Qt::WA_TranslucentBackground, true);//窗体背景全透明
    //setWindowOpacity(0.5);//主窗体及其所有的子控件整体半透明

    QScreen* MyScreen = this->screen();
    screenRect = MyScreen->geometry();
    std::thread PPTDialog(comInitialProc);
    PPTDialog.detach();
    QTimer* setPageNumTimer;
    setPageNumTimer = new QTimer(this);
    connect(setPageNumTimer, SIGNAL(timeout()), this, SLOT(setPageNum()));
    setPageNumTimer->setInterval(100);
    setPageNumTimer->start();
}

pptHelper::~pptHelper()
{
    CoUninitialize();
}

void pptHelper::paintEvent(QPaintEvent* e)
{
    QPainter p(this);
    p.setBrush(QBrush(QColor(255,255,255, 0)));
    p.setPen(Qt::NoPen);
    p.drawRect(this->rect());
    //QSize btsize(screenRect.height() / 20, screenRect.height() / 20);//相对屏幕大小计算
    QSize WidgetSize(this->height() / 20, this->height() / 20);//相对窗口大小计算
    ui.toolBarBottom->resize(WidgetSize.height()*5, WidgetSize.height());
    ui.toolBarBottom->move((this->width() - ui.toolBarBottom->width())/2,this->height()-ui.toolBarBottom->height());

    ui.turnPageToolBarLeft->resize(WidgetSize.height(), WidgetSize.height() * 3);
    ui.turnPageToolBarRight->resize(WidgetSize.height(), WidgetSize.height() * 3);
    ui.turnPageToolBarLeftBottom->resize(WidgetSize.height() * 3, WidgetSize.height());
    ui.turnPageToolBarRightBottom->resize(WidgetSize.height() * 3, WidgetSize.height());

    ui.turnPageToolBarLeft->move(0, (this->height() - ui.turnPageToolBarLeft->height()) / 2);
    ui.turnPageToolBarRight->move(this->width()-ui.turnPageToolBarRight->width(), (this->height() - ui.turnPageToolBarRight->height()) / 2);
    ui.turnPageToolBarLeftBottom->move(0, this->height() - ui.turnPageToolBarLeftBottom->height());
    ui.turnPageToolBarRightBottom->move(this->width() - ui.turnPageToolBarRightBottom->width(), this->height() - ui.turnPageToolBarRightBottom->height());

    //ui.toolBarBottom->setStyleSheet(".QWidget{border: 1px solid grey;border - radius: 10px;background - color: rgba(244, 255, 220, 100);}QWidget > QPushButton{background - color:transparent;}QWidget > QPushButton:focus{outline: none;}");

    QList<QPushButton*> btns = findChildren<QPushButton*>();
    for (QPushButton* btn : btns)
    {
        if (btn->parent() == ui.turnPageToolBarLeft || btn->parent() == ui.turnPageToolBarRight)
        {
            QSize btnIconSize(btn->size().width() * 3 / 5, btn->size().width() * 3 / 5);//相对按钮的图标大小
            btn->setIconSize(btnIconSize);
        }
        else
        {
            QSize btnIconSize(btn->size().height() * 3 / 5, btn->size().height() * 3 / 5);//相对按钮的图标大小
            btn->setIconSize(btnIconSize);
        }
    }
    
    QMainWindow::paintEvent(e);
    //QWidget::paintEvent(e);
}


HRESULT EA::SlideShowBegin(SlideShowWindow* Wn)
{
    return S_OK;
}
bool comInitialProc()
{
    //HRESULT hr = CoInitialize(NULL);
    hr = CoInitializeEx(0, COINIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
        
        std::thread getPptWindthread(getWind);
        getPptWindthread.detach();
        //initThread1->start();

        std::cout << "Listening for PowerPoint events..." << std::endl;
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    CoUninitialize();
    return true;

}

void getWind()
{
    //hr = pPowerPointApp.CoCreateInstance(__uuidof(Application));
    //std::string spath;// = "C:\\Users\\seewo\\Desktop\\数学\\早测\\1月11号.pptx";
    //QString qs = "C:\\Users\\seewo\\Desktop\\数学\\早测\\1月11号.pptx";
    //spath = qs.toLocal8Bit();
    //pPowerPointApp->Presentations->Open(_com_util::ConvertStringToBSTR(spath.c_str()), Office::msoTrue, Office::msoFalse, Office::msoTrue);

    HWND lastPptShowWindow = NULL;
    Sleep(500);
    //MoveWindow(thisApp, 0, 0, 0, 0, 0);
    ShowWindow(thisApp, SW_MINIMIZE);

    while (true)
    {
        CComPtr<_Application> pPowerPointApp;

        // 在这里可以执行其他操作
        Sleep(100);
        try
        {
            do
            {
                CComPtr<IUnknown> iunknown1;
                hr = GetActiveObject(__uuidof(PowerPoint::Application), NULL, &iunknown1);
                pPowerPointApp = iunknown1;
            } while (pPowerPointApp == NULL);
            pPowerPointApp->ActivePresentation;
            long pptHWND;
            pPowerPointApp->ActivePresentation->SlideShowWindow->get_HWND(&pptHWND);
            CComPtr<IConnectionPointContainer> icpc;
            icpc = pPowerPointApp;
            CComPtr<IConnectionPoint> icp;
            icpc->FindConnectionPoint(__uuidof(EApplication), &icp);
            if (lastPptShowWindow != (HWND)pptHWND && pptHWND != NULL && icp != NULL)
                icp->Advise(&eaSink, &dw);
            pptShowPosition = pPowerPointApp->ActivePresentation->SlideShowWindow->View->GetCurrentShowPosition();
            pptShowTotalNum = pPowerPointApp->ActivePresentation->Slides->GetCount();// pPowerPointApp->ActivePresentation->SlideShowSettings->EndingSlide;
            //std::string versionName1;
            //versionName1 = pPowerPointApp->GetName();
            //if((HWND)pptHWND == GetForegroundWindow())
if (pPowerPointApp->ActivePresentation->SlideShowWindow->Active == Office::msoTrue)
{
    if (IsIconic(thisApp))
    {
        ShowWindow(thisApp, SW_NORMAL);
        if (pptHWND != NULL)
            SetForegroundWindow((HWND)pptHWND);
    }
}
else if (thisApp != GetForegroundWindow() && ColorDialog1 != GetForegroundWindow())
{
    if (!IsIconic(thisApp))
        ShowWindow(thisApp, SW_MINIMIZE);
}
if (!IsIconic(thisApp))
{
    //OutputDebugString(L"Visible");
    RECT pptrect = { 0,0,0,0 };
    if (pptHWND != NULL)
        GetWindowRect((HWND)pptHWND, &pptrect);
    MoveWindow(thisApp, pptrect.left, pptrect.top, pptrect.right - pptrect.left, pptrect.bottom - pptrect.top, 1);
}
lastPptShowWindow = (HWND)pptHWND;
//break;
        }
        catch (...)
        {
            //QMessageBox::critical(0, "", "ERROR!!!!!!!!!!!!!!!!!!!!!!!!!");
            //ShowWindow(thisApp, SW_HIDE);
            ShowWindow(thisApp, SW_MINIMIZE);
            PointerColor1 = Qt::red;
        }

    }

}

HRESULT __stdcall EA::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
    std::cout << dispIdMember << std::endl;
    if (dispIdMember == 2011)//
    {
        std::cout << "SlideShowBegin!" << std::endl;
        //QMessageBox::information(0, "SlideShowBegin!!!", "SlideShowBegin!!!");
        //ShowWindow(thisApp, SW_NORMAL);

    }
    if (dispIdMember == 2014)//
    {
        //ShowWindow(thisApp, SW_HIDE);
        //MoveWindow(thisApp, 0, 0, 0, 0, 1);
        ShowWindow(thisApp, SW_MINIMIZE);
    }
    return S_OK;
}



void pptHelper::setPointer1()
{
    std::thread clickBtnsThread(ClickBtnsProc, ClickBtnsMode::Pointer);
    clickBtnsThread.detach();
}

void pptHelper::setPen1()
{
    if (!dbclick[0])
    {
        dbclick[0] = true;
        QTimer* dcCheck = new QTimer(this);
        connect(dcCheck, SIGNAL(timeout()), this, SLOT(dcCheckProc()));
        dcCheck->setSingleShot(true);
        dcCheck->start(500);
    }
    else
    {
        QColorDialog* selectColorBox = new QColorDialog(this);
        ColorDialog1 = (HWND)selectColorBox->winId();
        selectColorBox->exec();
        
        PointerColor1 = selectColorBox->selectedColor();
        //ColorDialog1 = ui.pen->getColorDialogHWND();
        //PointerColor1 = ui.pen->getPointerColor();
        ColorDialog1 = NULL;
    }
    std::thread clickBtnsThread(ClickBtnsProc, ClickBtnsMode::Pen);
    clickBtnsThread.detach();
    while(true)
    if(setPenSuccess)
    {
        setPenSuccess = false;
        QFile penIconFile;
        penIconFile.setFileName(QString(":/svg/svgs/填充画笔.svg"));
        penIconFile.open(QIODevice::ReadOnly | QIODevice::Text);
        QString penIconStr;
        penIconStr = penIconFile.readAll();
        QRegularExpression QRE("fill=\".*?\"");
        QRegularExpressionMatch QREM;
        QREM = QRE.match(penIconStr);
        QString matchStr = QREM.captured();
        bool hasMatch = QREM.hasMatch();
        /*
        <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><g data-name="Layer 2"><g data-name="edit"><rect width="24" height="24" opacity="0"/>    <!--This is to be changed!--><circle cx="7" cy="17" r="2.5" fill="#00ff0000" /><path d="M19.4 7.34L16.66 4.6A2 2 0 0 0 14 4.53l-9 9a2 2 0 0 0-.57 1.21L4 18.91a1 1 0 0 0 .29.8A1 1 0 0 0 5 20h.09l4.17-.38a2 2 0 0 0 1.21-.57l9-9a1.92 1.92 0 0 0-.07-2.71zM9.08 17.62l-3 .28.27-3L12 9.32l2.7 2.7zM16 10.68L13.32 8l1.95-2L18 8.73z"/></g><g gata-name="backg"></g></g></svg>
        */
        if (hasMatch)
        {
            QString ColorHex = PointerColor1.name();
            penIconStr.replace(QRE, "fill=\""+ColorHex+"\"");
        QIcon penIcon;
        QPixmap qpm;
        qpm.loadFromData(penIconStr.toLocal8Bit());
        penIcon.addPixmap(qpm);
        ui.pen->setIcon(penIcon);
        break;
        }
    }
}
void pptHelper::dcCheckProc()
{
    dbclick[0] = false;
}

void pptHelper::setEraser1()
{
    std::thread clickBtnsThread(ClickBtnsProc, ClickBtnsMode::Eraser);
    clickBtnsThread.detach();
}

void pptHelper::setMagnifier()
{
    exit(0);
}

void pptHelper::quitSlideShowWindow1()
{
    std::thread clickBtnsThread(ClickBtnsProc,ClickBtnsMode::QuitSlideShowWindow);
    clickBtnsThread.detach();
}

void pptHelper::previousPage1()
{
    std::thread clickBtnsThread(ClickBtnsProc, ClickBtnsMode::Previous);
    clickBtnsThread.detach();
}

void pptHelper::nextPage1()
{
    std::thread clickBtnsThread(ClickBtnsProc, ClickBtnsMode::Next);
    clickBtnsThread.detach();
}

void pptHelper::setNavigation1()
{
    std::thread clickBtnsThread(ClickBtnsProc, ClickBtnsMode::Navigation);
    clickBtnsThread.detach();

}

void pptHelper::setPageNum()
{
    if (pptShowPosition > 0 && pptShowTotalNum > 0)
    {
        QString setstr;
        setstr = QString::number(pptShowPosition) + "/" + QString::number(pptShowTotalNum);
#define setI(i,str) {ui.SelectPage_##i##->setIcon(QIcon());ui.SelectPage_##i##->setText(str);}
        setI(1, setstr)
        setI(2, setstr)
        setI(3, setstr)
        setI(4, setstr)
#undef setI
    }
}




void ClickBtnsProc(int mode)
{
    CComPtr<_Application> PPTAPP;
    CComPtr<IUnknown> iunknown1;
    hr = GetActiveObject(__uuidof(PowerPoint::Application), NULL, &iunknown1);
    PPTAPP = iunknown1;
    if (PPTAPP == NULL)
        return;
    try
    {

        PPTAPP->ActivePresentation;
        long appHWND,pptHWND;
        PPTAPP->get_HWND(&appHWND);
        PPTAPP->ActivePresentation->SlideShowWindow->get_HWND(&pptHWND);
        SetForegroundWindow((HWND)pptHWND);
    }
    catch (...)
    {
        //QMessageBox::critical(0, "", "ERROR!!!!!!!!!!!!!!!!!!!!!!!!!");
        return;
    }
    switch (mode)
    {
    case ClickBtnsMode::QuitSlideShowWindow:
        PPTAPP->ActivePresentation->SlideShowWindow->View->Exit();
        break;
    case ClickBtnsMode::Pointer:
        PPTAPP->ActivePresentation->SlideShowWindow->View->PointerType = PowerPoint::PpSlideShowPointerType::ppSlideShowPointerArrow;
        break;
    case ClickBtnsMode::Pen:
    {
        PPTAPP->ActivePresentation->SlideShowWindow->View->PointerColor->put_RGB(RGB(PointerColor1.red(), PointerColor1.green(), PointerColor1.blue()));
        PPTAPP->ActivePresentation->SlideShowWindow->View->PointerType = PowerPoint::PpSlideShowPointerType::ppSlideShowPointerPen;
        setPenSuccess = true;
    }
    break;
    case ClickBtnsMode::Eraser:
        PPTAPP->ActivePresentation->SlideShowWindow->View->PointerType = PowerPoint::PpSlideShowPointerType::ppSlideShowPointerEraser;
        break;
    case ClickBtnsMode::Previous:
        PPTAPP->ActivePresentation->SlideShowWindow->View->Previous();
        break;
    case ClickBtnsMode::Next:
        PPTAPP->ActivePresentation->SlideShowWindow->View->Next();
        break;
    case ClickBtnsMode::Navigation:
        PPTAPP->ActivePresentation->SlideShowWindow->SlideNavigation->put_Visible(true);
        break;
    default:
        break;
    }
}