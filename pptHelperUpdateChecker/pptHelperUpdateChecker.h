#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_pptHelperUpdateChecker.h"
#include <fstream>
#include <sstream>
#include "jsoncpp/json/json.h"
#include <string>
#include <Windows.h>
#include <shellapi.h>
#pragma comment(lib,"Version.lib")
using namespace std;

string UTF8ToGBK(const char* str);
wstring str2wstr(string str);
std::wstring GetFileProductVersion(const wchar_t* fileAndpath);
void RunAndExit(int ExitCode);
BOOL ClearFileReadOnlyAttribute(const wchar_t* fileNameAndPath);

class pptHelperUpdateChecker : public QMainWindow
{
    Q_OBJECT

public:
    pptHelperUpdateChecker(QWidget *parent = nullptr);
    ~pptHelperUpdateChecker();

private:
    Ui::pptHelperUpdateCheckerClass ui;
};
