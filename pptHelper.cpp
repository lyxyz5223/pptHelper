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
int pptShowPosition = -1;
int pptShowTotalNum = -1;
QTimer* GetPptSlideShowStateTimer;
long pptHWND, pptAppHWND;

QPropertyAnimation* HideAni;
QPropertyAnimation* ShowAni;
#include "LicenseAndPolicyDialogClass.h"
bool RectEqual(RECT r1, RECT r2) {
	if (r1.left != r2.left || r1.right != r2.right || r1.top != r2.top || r1.bottom != r2.bottom)
		return false;
	return true;
}

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
	std::fstream fs("config.ini", std::ios::in);
	if (fs.is_open())
	{
		std::string tmpConf;
		std::vector<std::string> conf;
		while (getline(fs, tmpConf))
		{
			conf = split(tmpConf);
			Config.push_back(conf);
		}
		Config.erase(Config.begin());
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
	show();//手动显示窗口，getWind函数就不用先等待500ms再隐藏窗口
}

pptHelper::~pptHelper()
{
	CoUninitialize();
}
void pptHelper::moveBars()
{
	QSize ToolBtnSize = QSize(this->height() / 20, this->height() / 20);//相对窗口大小计算
	ui.ToolBarBottom->resize(ui.fold->isFolded() ? ToolBtnSize.width()/2 : ToolBtnSize.height() * 5 + 1 + ToolBtnSize.height() / 2, ToolBtnSize.height());
	ui.TurnPageToolBarLeft->resize(ToolBtnSize.height(), ToolBtnSize.height() * 3);
	ui.TurnPageToolBarRight->resize(ToolBtnSize.height(), ToolBtnSize.height() * 3);
	ui.TurnPageToolBarLeftBottom->resize(ToolBtnSize.height() * 3, ToolBtnSize.height());
	ui.TurnPageToolBarRightBottom->resize(ToolBtnSize.height() * 3, ToolBtnSize.height());

	QPoint ToolBarBottomPos = QPoint((this->width() - ui.ToolBarBottom->getAniEndSize().width()) / 2, this->height() - ui.ToolBarBottom->getAniEndSize().height()),//底部工具栏居中处理
		TurnPageToolBarLeftPos = QPoint(0, (this->height() - ui.TurnPageToolBarLeft->height()) / 2),
		TurnPageToolBarRightPos = QPoint(this->width() - ui.TurnPageToolBarRight->width(), (this->height() - ui.TurnPageToolBarRight->height()) / 2),
		TurnPageToolBarLeftBottomPos = QPoint(0, this->height() - ui.TurnPageToolBarLeftBottom->height()),
		TurnPageToolBarRightBottomPos = QPoint(this->width() - ui.TurnPageToolBarRightBottom->width(), this->height() - ui.TurnPageToolBarRightBottom->height());
#define JUDGECONFIG(str,iter) try{\
	if(iter->at(0) == _CRT_STRINGIZE(str##Pos)) {\
		if(iter->at(1) == "relative") {\
			str##Pos.setX((size().width() - ui.str->size().width()) * std::stod(iter->at(2)));\
			str##Pos.setY((size().height() - ui.str->size().height()) * std::stod(iter->at(3)));\
		}\
		else if(iter->at(1) == "absolute") {\
			str##Pos.setX(std::stold(iter->at(2)));\
			str##Pos.setY(std::stold(iter->at(3)));\
		}\
	}\
	}\
	catch(...){\
		\
	}

	for (auto iter = Config.begin(); iter != Config.end(); iter++)
	{
		JUDGECONFIG(ToolBarBottom, iter)
		JUDGECONFIG(TurnPageToolBarLeft, iter)
		JUDGECONFIG(TurnPageToolBarRight, iter)
		JUDGECONFIG(TurnPageToolBarLeftBottom, iter)
		JUDGECONFIG(TurnPageToolBarRightBottom, iter)
	}
	if(!ui.ToolBarBottom->isCustomized())
		ui.ToolBarBottom->move(ToolBarBottomPos);
	else {
		QRect geoBak = ui.ToolBarBottom->getBackUpWindowRect();
		QRect geo = ui.ToolBarBottom->geometry();
		geo.setTopLeft(QPoint(geo.x() * width() / geoBak.width(), geo.y() * height() / geoBak.height()));
		ui.ToolBarBottom->move(geo.topLeft());
	}
	//ui.ToolBarBottom->move((this->width() - ui.ToolBarBottom->width()) / 4, this->height() - ui.ToolBarBottom->height());//底部工具栏置于屏幕左侧1/4处
	ui.TurnPageToolBarLeft->move(TurnPageToolBarLeftPos);
	ui.TurnPageToolBarRight->move(TurnPageToolBarRightPos);
	ui.TurnPageToolBarLeftBottom->move(TurnPageToolBarLeftBottomPos);
	ui.TurnPageToolBarRightBottom->move(TurnPageToolBarRightBottomPos);
	//ui.ToolBarBottom->setStyleSheet(".QWidget{border: 1px solid grey;border - radius: 10px;background - color: rgba(244, 255, 220, 100);}QWidget > QPushButton{background - color:transparent;}QWidget > QPushButton:focus{outline: none;}");
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
			if (msg->wParam == 9876)//隐藏
			{
				ShowAni->stop();
				HideAni->setStartValue(ShowAni->currentValue());
				HideAni->setDuration(msg->lParam);
				HideAni->start();
				return true;
			}
			break;
		case 0x6789:
			//MessageBox(0, "", "", 0);
			if (msg->wParam == 6789)
			{
				HideAni->stop();
				ShowAni->setStartValue(HideAni->currentValue());
				show();
				ShowAni->setDuration(msg->lParam);
				ShowAni->start();
				return true;
			}
			break;
		case 0x1145://窗口移动时绘制
			//moveBars();
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

	moveBars();
	QList<QPushButton*> btns = findChildren<QPushButton*>();
	for (QPushButton* btn : btns)
	{
		if (btn->parent() == ui.TurnPageToolBarLeft || btn->parent() == ui.TurnPageToolBarRight)
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
#define PAINTANDBREAK(WidgetName) {\
	QColor usingBtnBgColor(213, 194, 255, 255);\
	QWidget* parentW = ui.WidgetName->parentWidget();\
	QRect WidgetName##Rect(\
		parentW->mapTo(parentW->parentWidget(),\
			QPoint(ui.WidgetName->pos().x() + (ui.WidgetName->width() - ui.WidgetName->iconSize().width())/2,ui.WidgetName->pos().y() + (ui.WidgetName->height() - ui.WidgetName->iconSize().height())/2)),\
			ui.WidgetName->iconSize());\
	p.fillRect(WidgetName##Rect, usingBtnBgColor);\
	}\
	break
//end define
	if(!ui.fold->isFolded())
		switch (NowUsingButtonRealTimeCheck)
		{
		case PowerPoint::PpSlideShowPointerType::ppSlideShowPointerAutoArrow:
		case PowerPoint::PpSlideShowPointerType::ppSlideShowPointerArrow:
		case PowerPoint::PpSlideShowPointerType::ppSlideShowPointerAlwaysHidden:
			PAINTANDBREAK(pointer);
		case PowerPoint::PpSlideShowPointerType::ppSlideShowPointerPen:
			PAINTANDBREAK(pen);
		case PowerPoint::PpSlideShowPointerType::ppSlideShowPointerEraser:
			PAINTANDBREAK(eraser);
		case PowerPoint::PpSlideShowPointerType::ppSlideShowPointerNone:
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
			ui.TurnPageToolBarLeft->hide();
			ui.TurnPageToolBarRight->hide();
			ui.TurnPageToolBarLeftBottom->show();
			ui.TurnPageToolBarRightBottom->show();
		}
		else
		{
			ui.TurnPageToolBarLeftBottom->hide();
			ui.TurnPageToolBarRightBottom->hide();
			ui.TurnPageToolBarLeft->show();
			ui.TurnPageToolBarRight->show();
		}
	}
	else
	{
		//WPS Office
		ui.TurnPageToolBarLeft->hide();
		ui.TurnPageToolBarRight->hide();
		ui.TurnPageToolBarLeftBottom->show();
		ui.TurnPageToolBarRightBottom->show();

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
	RECT lastPptShowWindowRect = { 0,0,0,0 };
	//MoveWindow(thisApp, 0, 0, 0, 0, 0);
	//ShowWindow(thisApp, SW_MINIMIZE);
	//Sleep(500);
	PostMessage(thisApp, 0x9876, 9876, 1000);

	//ShowWindow(thisApp, SW_HIDE);
	bool initPointerColor = true;
	while (true)
	{
		CComPtr<_Application> pPowerPointApp;

		// 在这里可以执行其他操作
		//Sleep(100);
		try
		{
			do
			{
				CComPtr<IUnknown> iunknown1;
				hr = GetActiveObject(__uuidof(PowerPoint::Application), NULL, &iunknown1);
				pPowerPointApp = iunknown1;
				Sleep(100);
			} while (pPowerPointApp == NULL);
			pPowerPointApp->ActivePresentation;
			//long pptHWND;
			pPowerPointApp->ActivePresentation->SlideShowWindow->get_HWND(&pptHWND);
			pPowerPointApp->get_HWND(&pptAppHWND);
			if (dw == NULL)
			{
				CComPtr<IConnectionPointContainer> icpc;
				icpc = pPowerPointApp;
				CComPtr<IConnectionPoint> icp;
				icpc->FindConnectionPoint(__uuidof(EApplication), &icp);
				if (icp != NULL)
					icp->Advise(&eaSink, &dw);
			}
			if (lastPptShowWindow != (HWND)pptHWND && pptHWND != NULL)
			{
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
			if (pPowerPointApp->ActivePresentation->SlideShowWindow->Active == Office::msoTrue && pPowerPointApp->WindowState != PowerPoint::ppWindowMinimized)
			{
				qDebug() << "Active";
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
				RECT pptRect = { 0,0,0,0 };
				if (pptHWND != NULL)//句柄有效才获取ppt放映窗口Rect
				{
					GetWindowRect((HWND)pptHWND, &pptRect);//获取ppt放映窗口Rect
					if (pPowerPointApp->WindowState != PowerPoint::ppWindowMinimized)
					{
						qDebug() << "WindowState-Normal";
						if (!RectEqual(lastPptShowWindowRect, pptRect))
						{
							lastPptShowWindowRect = pptRect;
							MoveWindow(thisApp, pptRect.left, pptRect.top, pptRect.right - pptRect.left, pptRect.bottom - pptRect.top, 1);//调整软件窗口大小，使其适应ppt放映窗口
							PostMessage(thisApp, 0x1145, 114514, 0);
						}
					}
					qDebug() << "Rect:(" << pptRect.left << "," << pptRect.top << "," << pptRect.right - pptRect.left << "," << pptRect.bottom - pptRect.top << ")";
				}
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

HRESULT __stdcall EA::Invoke(_In_ DISPID dispIdMember, _In_ REFIID riid, _In_ LCID lcid, _In_ WORD wFlags, _In_ DISPPARAMS* pDispParams, _Out_opt_ VARIANT* pVarResult, _Out_opt_ EXCEPINFO* pExcepInfo, _Out_opt_ UINT* puArgErr)
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
		if(selectColorBox->result() == QColorDialog::Accepted)
			PointerColor1 = selectColorBox->selectedColor();
		ColorDialog1 = NULL;
	}
	std::thread clickBtnsThread(ClickBtnsProc, ClickBtnsMode::Pen);
	clickBtnsThread.detach();
}
void pptHelper::dcCheckProc()//双击检测
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

void pptHelper::foldMenu()
{
	QList<QPushButton*> btns = ui.ToolBarBottom->findChildren<QPushButton*>();
	if (btns.isEmpty()) 
		return;
	if(btns[0]->isVisible())
		ui.ToolBarBottom->backupSize();
	for (QPushButton* btn : btns)
	{
		if (btn->isVisible())
		{
			QSize size = ui.ToolBarBottom->size();
			size.setWidth(size.width() - btn->size().width());
			btn->setVisible(false);
			ui.ToolBarBottom->resize(size);
			ui.fold->setFold(true);
		}
		else
		{
			btn->setVisible(true);
			ui.ToolBarBottom->restoreSize();
			ui.fold->setFold(false);
		}
	}
	QList<QFrame*> frames = ui.ToolBarBottom->findChildren<QFrame*>();
	for (QFrame* frame : frames)
	{
		if (typeid(*frame) != typeid(QFrame))
			continue;
		if (frame->isVisible())
		{
			int s = frame->width();
			QString str = frame->objectName();
			QSize size = ui.ToolBarBottom->size();
			size.setWidth(size.width() - frame->width());
			frame->setVisible(false);
			ui.ToolBarBottom->resize(size);
			ui.fold->setFold(true);
		}
		else
		{
			frame->setVisible(true);
			ui.ToolBarBottom->restoreSize();
			ui.fold->setFold(false);
		}
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
		int pptShowPosition = PPTAPP->ActivePresentation->SlideShowWindow->View->GetCurrentShowPosition();
		int pptShowClickIndex = PPTAPP->ActivePresentation->SlideShowWindow->View->GetClickIndex();
		PPTAPP->ActivePresentation->SlideShowWindow->View->Exit();
		//PPTAPP->ActivePresentation->SlideShowSettings->put_ShowType(ppShowTypeWindow);
		PPTAPP->ActivePresentation->SlideShowSettings->Run();
		PPTAPP->ActivePresentation->SlideShowWindow->View->GotoSlide(pptShowPosition, Office::msoTrue);
		PPTAPP->ActivePresentation->SlideShowWindow->View->GotoClick(pptShowClickIndex);
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