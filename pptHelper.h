#pragma once

//Qt
#include <QtWidgets/QMainWindow>
#include "ui_pptHelper.h"
#include <qmessagebox.h>
#include <qpainter.h>
#include <qscreen.h>
#include <qthread.h>
//#include <QtConcurrent/qtconcurrent>
#include <qgraphicseffect.h>
#include <qtimer.h>
#include <qcolordialog.h>
#include <qtextstream.h>
#include <qfile.h>
#include <QRegularExpression>
#include <qsvgrenderer.h>
#include <qlabel.h>
#include <qpropertyanimation.h>
#include <qfileinfo.h>
#include <qwindow.h>
#include "WindowsMagnifier.h"
#include "ScreenShotMagnifier.h"

//Visual C++
#import "C:\\Program Files\\Microsoft Office\\Root\\VFS\\ProgramFilesCommonX64\\Microsoft Shared\\OFFICE16\\MSO.DLL"
#import "C:\\Program Files\\Microsoft Office\\Root\\VFS\\ProgramFilesCommonX86\\Microsoft Shared\\VBA\\VBA6\\VBE6EXT.OLB"
#import "C:\\Program Files\\Microsoft Office\\root\\Office16\\MSPPT.OLB"
#include <atlbase.h>
#include <atlcom.h>
#include <iostream>
#include <thread>
#include <string>
#include <fstream>
#include "stringProcess.h"

class EA : public IDispatch
{
public:
    HRESULT SlideShowBegin(PowerPoint::SlideShowWindow* Wn);
    //HRESULT sb(PowerPoint::SlideShowWindow* Wn);
    HRESULT STDMETHODCALLTYPE QueryInterface(
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) {
        if (riid == IID_IUnknown)
        {
            *ppvObject = static_cast<IUnknown*>(this);
        }
        else if (riid == IID_IDispatch)
        {
            *ppvObject = static_cast<IDispatch*>(this);
        }
        else
        {
            *ppvObject = NULL;
            return E_NOINTERFACE;
        }
        static_cast<IUnknown*>(*ppvObject)->AddRef();
        return S_OK;
    }

    ULONG STDMETHODCALLTYPE AddRef(void) { ref++; return S_OK; }

    ULONG STDMETHODCALLTYPE Release(void) { ref = 0; return S_OK; }
    HRESULT STDMETHODCALLTYPE GetIDsOfNames(
        /* [in] */ __RPC__in REFIID riid,
        /* [size_is][in] */ __RPC__in_ecount_full(cNames) LPOLESTR* rgszNames,
        /* [range][in] */ __RPC__in_range(0, 16384) UINT cNames,
        /* [in] */ LCID lcid,
        /* [size_is][out] */ __RPC__out_ecount_full(cNames) DISPID* rgDispId) {
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetTypeInfoCount(
        /* [out] */ __RPC__out UINT* pctinfo) {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE GetTypeInfo(
        /* [in] */ UINT iTInfo,
        /* [in] */ LCID lcid,
        /* [out] */ __RPC__deref_out_opt ITypeInfo** ppTInfo) {
        return S_OK;
    }

    /* [local] */ HRESULT STDMETHODCALLTYPE Invoke(
        /* [annotation][in] */
        _In_  DISPID dispIdMember,
        /* [annotation][in] */
        _In_  REFIID riid,
        /* [annotation][in] */
        _In_  LCID lcid,
        /* [annotation][in] */
        _In_  WORD wFlags,
        /* [annotation][out][in] */
        _In_  DISPPARAMS* pDispParams,
        /* [annotation][out] */
        _Out_opt_  VARIANT* pVarResult,
        /* [annotation][out] */
        _Out_opt_  EXCEPINFO* pExcepInfo,
        /* [annotation][out] */
        _Out_opt_  UINT* puArgErr);


private:
    unsigned long long ref = 0;
};

/*
    DISPID
2001 WindowSelectionChange
2002 WindowBeforeRightClick
2003 WindowBeforeDoubleClick
2004 PresentationClose
2005 PresentationSave
2006 PresentationOpen
2007 NewPresentation
2008 PresentationNewSlide
2009 WindowActivate
2010 WindowDeactivate
2011 SlideShowBegin
2012 SlideShowNextBuild
2013 SlideShowNextSlide
2014 SlideShowEnd
2015 PresentationPrint
2016 SlideSelectionChanged x
2017 ColorSchemeChanged x
2018 PresentationBeforeSave x
2019 SlideShowNextClick x
*/

//dispid
//WindowActivate

enum ClickBtnsMode
{
    Pointer,
    Pen,
    Eraser,
    EraseAllDrawing,
    Magnifier,
    QuitSlideShowWindow,
    Previous,
    Next,
    Navigation
};

class pptHelper : public QMainWindow
{
    Q_OBJECT

public:
    pptHelper(QWidget *parent = nullptr);
    ~pptHelper();
    auto getHideAni() const { return HideAni; }
    auto getShowAni() const { return ShowAni; }
    bool comInitialProc();
    void getWind();
    void ClickBtnsProc(ClickBtnsMode mode);

protected:
    void paintEvent(QPaintEvent* e);
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result);
private:
    Ui::pptHelperClass ui;
    std::vector<std::vector<std::string>> Config;
    void moveBars();
    MyWindowsMagnifier* windowsMagnifier = nullptr;
    ScreenShotMagnifier* screenShotMagnifier = nullptr;
    void updateMagnifierState(bool showState);
    QPropertyAnimation* HideAni;
    QPropertyAnimation* ShowAni;
    QTimer* GetPptSlideShowStateTimer;

    QRect screenRect;
    HWND thisApp;//当前应用程序窗口的实例句柄
    HWND ColorDialog1 = NULL;//画笔选择窗口的句柄
    bool dbclick[2] = { false,false };//目前仅使用dbclick[0]而dbclick[1]暂不使用
    QColor PointerColor1;//
    QColor PointerColor2;

    EA eaSink;
    DWORD dw;
    HRESULT hr;
    int pptShowPosition = -1;
    int pptShowTotalNum = -1;
    long pptHWND, pptAppHWND;

    PowerPoint::PpSlideShowPointerType NowUsingButtonDisplay = PowerPoint::PpSlideShowPointerType::ppSlideShowPointerAutoArrow;
    PowerPoint::PpSlideShowPointerType NowUsingButtonRealTimeCheck = PowerPoint::PpSlideShowPointerType::ppSlideShowPointerNone;

public slots:
    void setPointer1();
    void setPen1();
    void setEraser1();
    void setMagnifier();
    void quitSlideShowWindow1();
    void previousPage1();
    void nextPage1();
    void setNavigation1();
    void setPageNumAndPointerColor();
    void dcCheckProc();
    void getPptSlideShowState();
    void foldMenu();
};



