#include "pptHelper.h"
#include <QtWidgets/QApplication>
#include <Windows.h>
#include <qtranslator.h>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HANDLE hMutex = CreateMutex(nullptr, TRUE, qApp->applicationName().toStdString().c_str());
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        //QMessageBox::warning(nullptr, "Error", "An instance of the application is already running.");
        if(hMutex != NULL)
            CloseHandle(hMutex);
        hMutex = NULL;
        MessageBoxW(0, L"请不要多开哦~", L"", 0);
        return ERROR_ALREADY_EXISTS;
    }

    //汉化
    //QTranslator AppTranslator;//自己的代码汉化
    QTranslator SysTranslator;//Qt自带汉化
    //if (AppTranslator.load(QString(":/pptHelper/res/qt_zh_CN.qm")))
    //    qApp->installTranslator(&AppTranslator);
    if(SysTranslator.load(QString(":/pptHelper/res/qt_zh_CN1.qm")))
        qApp->installTranslator(&SysTranslator);

    pptHelper w;
    w.show();
    return a.exec();
}
