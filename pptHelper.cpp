#include "pptHelper.h"

using namespace PowerPoint;



QRect screenRect;
bool comInitialProc();
HWND thisApp;//这个软件的窗口HWND
HWND ColorDialog1 = NULL;
bool dbclick[2] = { false,false };//检测是否双击按钮，dbclick[0]有用，dbclick[1]无用
QColor PointerColor1;//设置的画笔颜色
QColor PointerColor2;

EA eaSink;
DWORD dw;
HRESULT hr;
int pptShowPosition=-1;
int pptShowTotalNum =-1;
QTimer* GetPptSlideShowStateTimer;
long pptHWND, pptAppHWND;

QPropertyAnimation* HideAni;
QPropertyAnimation* ShowAni;
#include "LicenseAndPolicyDialogClass.h"

pptHelper::pptHelper(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    thisApp = (HWND)this->winId();
    this->setWindowFlags(this->windowFlags()|Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint/*|Qt::Tool*/);
    this->setAttribute(Qt::WA_TranslucentBackground, true);//窗体背景全透明
    //setWindowOpacity(0.5);//主窗体及其所有的子控件整体半透明
    this->setWindowIcon(QIcon(":/pptHelper/pptHelper.ico"));
    QScreen* MyScreen = this->screen();
    screenRect = MyScreen->geometry();

    std::string InConfigTxtStr;
    QFileInfo InConfigTxtInfo("config.ini");
    if (InConfigTxtInfo.isFile())
    {
        std::ifstream InConfigTxt("config.ini");
        if (!InConfigTxt.is_open())
        {
            QMessageBox::critical(this, "错误", "ERROR!\n配置文件读取失败！");
        }
        std::getline(InConfigTxt, InConfigTxtStr);
        InConfigTxt.close();
    }
    if(InConfigTxtStr != "NotFirstRun")
    {
        //软件许可协议弹窗
        LicenseAndPolicyDialogClass* licenseBox = new LicenseAndPolicyDialogClass(this);
        licenseBox->resize(screenRect.width() * 3 / 5, screenRect.height() * 3 / 5);
        switch (licenseBox->exec())
        {
        case 1://Yes
        {
            std::ofstream configTxt;
            configTxt.open("config.ini", std::ofstream::out);
            if (!configTxt.is_open())
            {
                QMessageBox::critical(this, "错误", "ERROR!\n配置文件读取写入失败！");
                exit(1);

            }
            else {
                configTxt << "NotFirstRun";
            }
            configTxt.close();
        }
        break;
        case 0://No or Close
            exit(0);
        default:
            break;
        }
    }
    ShowAni = new QPropertyAnimation(this);
    ShowAni->setTargetObject(this);
    ShowAni->setPropertyName("windowOpacity");
    ShowAni->setEasingCurve(QEasingCurve::Linear);
    ShowAni->setStartValue(0);
    ShowAni->setEndValue(1);
    ShowAni->setDuration(1000);

    HideAni = new QPropertyAnimation(this);
    HideAni->setTargetObject(this);
    HideAni->setDuration(1000);
    HideAni->setPropertyName("windowOpacity");
    HideAni->setEasingCurve(QEasingCurve::Linear);
    HideAni->setStartValue(1);
    HideAni->setEndValue(0);
    connect(HideAni, SIGNAL(finished()), this, SLOT(hide()));
    std::thread PPTDialog(comInitialProc);
    PPTDialog.detach();
    QTimer* setPageNumTimer;
    setPageNumTimer = new QTimer(this);
    connect(setPageNumTimer, SIGNAL(timeout()), this, SLOT(setPageNumAndPointerColor()));
    setPageNumTimer->start(100);
    GetPptSlideShowStateTimer = new QTimer(this);
    connect(GetPptSlideShowStateTimer, SIGNAL(timeout()), this, SLOT(getPptSlideShowState()));
    GetPptSlideShowStateTimer->start(100);
    //this->showMinimized();
    //this->hide();//怎么这条没用？？？？？？
}

pptHelper::~pptHelper()
{
    CoUninitialize();
}
bool pptHelper::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
    if (eventType == "windows_generic_MSG") {
        MSG* msg = static_cast<MSG*>(message);
        // 在这里处理Windows消息
        // ...
        switch (msg->message)
        {
        case 0x9876:
            //MessageBox(0, "", "", 0);
            if (msg->wParam == 9876)
            {
                ShowAni->stop();
                HideAni->setDuration(msg->lParam);
                HideAni->start();
                
                return true;
            }
            break;
        case 0x6789:
            //MessageBox(0, "", "", 0);
            if (msg->wParam == 6789)
            {
                show();
                HideAni->stop();
                ShowAni->setDuration(msg->lParam);
                ShowAni->start();
                return true;
            }
            break;
        default:
            break;
        }
    }
    return QWidget::nativeEvent(eventType,message,result);
}
PowerPoint::PpSlideShowPointerType NowUsingButtonDisplay = PowerPoint::PpSlideShowPointerType::ppSlideShowPointerAutoArrow;
PowerPoint::PpSlideShowPointerType NowUsingButtonRealTimeCheck = PowerPoint::PpSlideShowPointerType::ppSlideShowPointerNone;

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
    switch (NowUsingButtonRealTimeCheck)
    {
    case PowerPoint::PpSlideShowPointerType::ppSlideShowPointerNone:
        break;
    case PowerPoint::PpSlideShowPointerType::ppSlideShowPointerAutoArrow:
    case PowerPoint::PpSlideShowPointerType::ppSlideShowPointerArrow:
    case PowerPoint::PpSlideShowPointerType::ppSlideShowPointerAlwaysHidden:
    {
        QRect pointerRect(ui.pointer->pos().x() + ui.pointer->parentWidget()->pos().x(), ui.pointer->pos().y() + ui.pointer->parentWidget()->pos().y(), ui.pointer->size().width(), ui.pointer->size().height());
        p.fillRect(pointerRect, QColor(213, 194, 255, 255));
    }
        break;
    case PowerPoint::PpSlideShowPointerType::ppSlideShowPointerPen:
    {
        QRect penRect(ui.pen->pos().x() + ui.pen->parentWidget()->pos().x(), ui.pen->pos().y() + ui.pen->parentWidget()->pos().y(), ui.pen->size().width(), ui.pen->size().height());
        p.fillRect(penRect, QColor(213, 194, 255, 255));
    }
        break;
    case PowerPoint::PpSlideShowPointerType::ppSlideShowPointerEraser:
    {
        QRect eraserRect(ui.eraser->pos().x() + ui.eraser->parentWidget()->pos().x(), ui.eraser->pos().y() + ui.eraser->parentWidget()->pos().y(), ui.eraser->size().width(), ui.eraser->size().height());
        p.fillRect(eraserRect, QColor(213, 194, 255, 255));
    }
        break;
    default:
        break;
    }
    WCHAR pptAppTitle_wchar[256];
    std::wstring pptAppTitle_wstring;
    pptAppTitle_wstring = pptAppTitle_wstring;
    GetWindowTextW((HWND)pptAppHWND, pptAppTitle_wchar, 256);
    if (pptAppTitle_wstring.find(L" - WPS Office") == std::wstring::npos)
    {
        //PowerPoint
        if (NowUsingButtonRealTimeCheck == PowerPoint::ppSlideShowPointerPen || NowUsingButtonRealTimeCheck == PowerPoint::ppSlideShowPointerEraser)
        {
            ui.turnPageToolBarLeft->hide();
            ui.turnPageToolBarRight->hide();
            ui.turnPageToolBarLeftBottom->show();
            ui.turnPageToolBarRightBottom->show();
        }
        else
        {
            ui.turnPageToolBarLeftBottom->hide();
            ui.turnPageToolBarRightBottom->hide();
            ui.turnPageToolBarLeft->show();
            ui.turnPageToolBarRight->show();
        }
    }
    else
    {
        //WPS Office
        ui.turnPageToolBarLeft->hide();
        ui.turnPageToolBarRight->hide();
        ui.turnPageToolBarLeftBottom->show();
        ui.turnPageToolBarRightBottom->show();

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
    //MoveWindow(thisApp, 0, 0, 0, 0, 0);
    //ShowWindow(thisApp, SW_MINIMIZE);
    Sleep(500);
    PostMessage(thisApp, 0x9876, 9876, 1000);

    //ShowWindow(thisApp, SW_HIDE);
    bool initPointerColor = true;
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
            //long pptHWND;
            pPowerPointApp->ActivePresentation->SlideShowWindow->get_HWND(&pptHWND);
            pPowerPointApp->get_HWND(&pptAppHWND);

            CComPtr<IConnectionPointContainer> icpc;
            icpc = pPowerPointApp;
            CComPtr<IConnectionPoint> icp;
            icpc->FindConnectionPoint(__uuidof(EApplication), &icp);
            if (lastPptShowWindow != (HWND)pptHWND && pptHWND != NULL && icp != NULL)
            {
                icp->Advise(&eaSink, &dw);
                if (initPointerColor)
                {

                    int r = GetRValue(pPowerPointApp->ActivePresentation->SlideShowSettings->PointerColor->GetRGB());
                    int g = GetGValue(pPowerPointApp->ActivePresentation->SlideShowSettings->PointerColor->GetRGB());
                    int b = GetBValue(pPowerPointApp->ActivePresentation->SlideShowSettings->PointerColor->GetRGB());
                    PointerColor2.setRgb(r, g, b);
                    PointerColor1 = PointerColor2;
                    initPointerColor = false;
                }
            }
            pptShowPosition = pPowerPointApp->ActivePresentation->SlideShowWindow->View->GetCurrentShowPosition();
            pptShowTotalNum = pPowerPointApp->ActivePresentation->Slides->GetCount();// pPowerPointApp->ActivePresentation->SlideShowSettings->EndingSlide;
#define PPTPOINTERCOLOR pPowerPointApp->ActivePresentation->SlideShowWindow->View->PointerColor->GetRGB()
            if (pPowerPointApp->ActivePresentation->SlideShowWindow->View->GetPointerType() == ppSlideShowPointerPen)
            {
                int r = GetRValue(PPTPOINTERCOLOR);
                int g = GetGValue(PPTPOINTERCOLOR);
                int b = GetBValue(PPTPOINTERCOLOR);
                PointerColor2.setRgb(r,g,b);
                PointerColor2.setAlpha(255);
            }
            else
            {
                PointerColor2.setAlpha(0);
            }
#undef PPTPOINTERCOLOR
            //std::string versionName1;
            //versionName1 = pPowerPointApp->GetName();
            HWND ToCompareForegroundWindow = GetForegroundWindow();
            //if((HWND)pptHWND == GetForegroundWindow())
            if (pPowerPointApp->ActivePresentation->SlideShowWindow->Active == Office::msoTrue)
            {
                if (!IsWindowVisible(thisApp))//(IsIconic(thisApp))//检测软件窗口是否已经最小化，是则返回True
                {
                    //ShowWindow(thisApp, SW_MINIMIZE);
                    //ShowWindow(thisApp, SW_NORMAL);
                    PostMessage(thisApp, 0x6789, 6789, 1000);

                    if (pptHWND != NULL)
                        SetForegroundWindow((HWND)pptHWND);
                }
            }
            else if (thisApp != ToCompareForegroundWindow && ColorDialog1 != ToCompareForegroundWindow && (HWND)pptHWND != ToCompareForegroundWindow)
            {//判断是否PPT幻灯片放映或者颜色选择窗口是前端窗口
                if (IsWindowVisible(thisApp))//(!IsIconic(thisApp))//软件可见的情况下，最小化或者隐藏
                    //ShowWindow(thisApp, SW_MINIMIZE);
                    //ShowWindow(thisApp, SW_HIDE);
                    PostMessage(thisApp, 0x9876, 9876, 100);//隐藏窗口，带动画
            }
            if (IsWindowVisible(thisApp))//(!IsIconic(thisApp))//软件可见的情况下，调整软件窗口大小，使其适应ppt放映窗口
            {
                //OutputDebugString(L"Visible");
                RECT pptrect = { 0,0,0,0 };
                if (pptHWND != NULL)//句柄有效才获取ppt放映窗口Rect
                    GetWindowRect((HWND)pptHWND, &pptrect);//获取ppt放映窗口Rect
                MoveWindow(thisApp, pptrect.left, pptrect.top, pptrect.right - pptrect.left, pptrect.bottom - pptrect.top, 1);//调整软件窗口大小，使其适应ppt放映窗口
                //获取指针状态:箭头/画笔/橡皮
                NowUsingButtonRealTimeCheck = pPowerPointApp->ActivePresentation->SlideShowWindow->View->GetPointerType();
            }
            lastPptShowWindow = (HWND)pptHWND;//储存当前窗口句柄数据
            //break;
        }
        catch (...)
        {
            //QMessageBox::critical(0, "", "ERROR!!!!!!!!!!!!!!!!!!!!!!!!!");
            //ShowWindow(thisApp, SW_HIDE);
            //ShowWindow(thisApp, SW_MINIMIZE);
            if (IsWindowVisible(thisApp))
            {
                PostMessage(thisApp, 0x9876, 9876, 1000);
                //ShowWindow(thisApp, SW_HIDE);//没有ppt放映窗口?那就软件藏起来吧
            }
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
        //ShowWindow(thisApp, SW_MINIMIZE);
        //ShowWindow(thisApp, SW_HIDE);

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
    PointerColor1 = PointerColor2;
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
        ColorDialog1 = NULL;
    }
    std::thread clickBtnsThread(ClickBtnsProc, ClickBtnsMode::Pen);
    clickBtnsThread.detach();
}
void pptHelper::dcCheckProc()
{
    dbclick[0] = false;
    dbclick[1] = false;
}

void pptHelper::setEraser1()
{
    std::thread clickBtnsThread(ClickBtnsProc, ClickBtnsMode::Eraser);
    clickBtnsThread.detach();
    if (!dbclick[1])
    {
        dbclick[1] = true;
        QTimer* dcCheck = new QTimer(this);
        connect(dcCheck, SIGNAL(timeout()), this, SLOT(dcCheckProc()));
        dcCheck->setSingleShot(true);
        dcCheck->start(500);
    }
    else
    {
        std::thread clickBtnsThread(ClickBtnsProc, ClickBtnsMode::EraseAllDrawing);
        clickBtnsThread.detach();
        return;
    }

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


void pptHelper::setPageNumAndPointerColor()
{
    ui.pen->setIconColor(PointerColor2);
    if (pptShowPosition > 0 && pptShowTotalNum > 0)
    {
        QString setstr;
        if(pptShowPosition <= pptShowTotalNum)
            setstr = QString::number(pptShowPosition) + "/" + QString::number(pptShowTotalNum);
#define setI(i,str) {ui.SelectPage_##i##->setIcon(QIcon());ui.SelectPage_##i##->setText(str);}
        setI(1, setstr)
        setI(2, setstr)
        setI(3, setstr)
        setI(4, setstr)
#undef setI
    }

}

void pptHelper::getPptSlideShowState()
{
    if (NowUsingButtonDisplay != NowUsingButtonRealTimeCheck)
    {
        update();
        NowUsingButtonDisplay = NowUsingButtonRealTimeCheck;
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
        PPTAPP->ActivePresentation->SlideShowWindow->View->PointerType = PowerPoint::PpSlideShowPointerType::ppSlideShowPointerAutoArrow;
        break;
    case ClickBtnsMode::Pen:
        PPTAPP->ActivePresentation->SlideShowWindow->View->PointerColor->put_RGB(RGB(PointerColor1.red(), PointerColor1.green(), PointerColor1.blue()));
        PPTAPP->ActivePresentation->SlideShowWindow->View->PointerType = PowerPoint::PpSlideShowPointerType::ppSlideShowPointerPen;
        break;
    case ClickBtnsMode::Eraser:
        PPTAPP->ActivePresentation->SlideShowWindow->View->PointerType = PowerPoint::PpSlideShowPointerType::ppSlideShowPointerEraser;
        break;
    case ClickBtnsMode::EraseAllDrawing:
    {
        PPTAPP->ActivePresentation->SlideShowWindow->View->EraseDrawing();
        //凑合着用吧，这里不会写了
        PPTAPP->ActivePresentation->SlideShowWindow->View->Exit();
        //PPTAPP->ActivePresentation->SlideShowSettings->put_ShowType(ppShowTypeWindow);
        PPTAPP->ActivePresentation->SlideShowSettings->Run();
        PPTAPP->ActivePresentation->SlideShowWindow->View->PointerType = PowerPoint::PpSlideShowPointerType::ppSlideShowPointerPen;

    }   
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
    case ClickBtnsMode::Magnifier:

    default:
        break;
    }
}