#include "pptHelperUpdateChecker.h"
#include "stringProcess.h"
const char* filenameA = "pptHelper.exe";
const wchar_t* filenameW = L"pptHelper.exe";
int versionCmp(string ver1, string ver2)
{
    int res = 0;
    vector<string> v1 = split(ver1, ".");
    vector<string> v2 = split(ver2, ".");
    size_t lt = (v1.size() > v2.size() ? v2.size() : v1.size());
    for (size_t i = 0; i < lt; i++)
    {
        long long v1ll = stoll(v1[i]);
        long long v2ll = stoll(v2[i]);
        if (v1ll != v2ll)
            res = (v1ll > v2ll ? 1 : -1);
    }
    if (res == 0 && v1.size() != v2.size())
        res = (v1.size() > v2.size() ? 1 : -1);
    return res;
}

pptHelperUpdateChecker::pptHelperUpdateChecker(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    //CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    wstring exitingFileInfo = L"";
    exitingFileInfo += GetFileProductVersion(filenameW);
#ifdef _DEBUG
    MessageBox((HWND)winId(), exitingFileInfo.c_str(), L"当前版本号", 0);
#else
#endif // DEBUG

    SHELLEXECUTEINFOW a;
    ZeroMemory(&a, sizeof(SHELLEXECUTEINFO));//使用前最好清空
    a.cbSize = sizeof(SHELLEXECUTEINFO);
    a.hwnd = 0;
    a.fMask = SEE_MASK_NOCLOSEPROCESS;
    a.lpDirectory = L"";
    a.lpFile = L"aria2c.exe";
    a.lpParameters = L"-V --auto-file-renaming=false --allow-overwrite=true -o newestInfo.json https://api.github.com/repos/lyxyz5223/ppthelper/releases/latest";
#ifdef _DEBUG
    a.nShow = SW_NORMAL;
#else
    a.nShow = SW_HIDE;
#endif // _DEBUG
    BOOL result = ShellExecuteEx(&a);
    if (result == FALSE)
    {
        //MessageBox(0,to_wstring(GetLastError()).c_str(), 0, 0);
        RunAndExit(1);
    }
    HANDLE hdl = a.hProcess;
    WaitForSingleObject(a.hProcess, INFINITE);//等待执行完毕
    unsigned long exitCode = STILL_ACTIVE;
    int ecpResult = GetExitCodeProcess(hdl, &exitCode);
    if (ecpResult == 0)
        RunAndExit(2);
    if (exitCode == 0)
        ;
    //MessageBox((HWND)winId(), L"Success!", L"notice", 0);
    else
        RunAndExit(3);
    ifstream inJson("newestInfo.json");
    if (!inJson.is_open())
        RunAndExit(4);
    Json::CharReaderBuilder jsReaderBuilder;
    Json::Value root;
    string StrError;
    bool isOK = Json::parseFromStream(jsReaderBuilder, inJson, &root, &StrError);
    if (!isOK)
        RunAndExit(5);
    string tag_name = root["tag_name"].asString();//UTF8文本
    if(tag_name.size())
        tag_name.erase(tag_name.begin());
    /*
        string body = root["body"].asString();//UTF8文本
    string qtVersionTag = "QtVersion:";
    string qtVersionText;
    string::size_type st = body.find(qtVersionTag);
    if (st != string::npos)
        qtVersionText = body.substr(st + qtVersionTag.size());
    else
        RunAndExit(10);
    for (size_t i = 0; i < qtVersionText.size(); i++)
    {
        if (qtVersionText[i] != '.' && !isdigit(qtVersionText[i]))
        {
            qtVersionText = qtVersionText.substr(0, i);
            break;
        }
    }
    */
#ifdef _DEBUG
    MessageBox((HWND)winId(), str2wstr(tag_name).c_str(), L"最新版本号", 0);
#else
#endif // DEBUG
    inJson.close();
    if (versionCmp(wstr2str_2UTF8(exitingFileInfo), tag_name) >= 0)
        RunAndExit(9);
    string download_link = "https://github.com/lyxyz5223/pptHelper/releases/download/";
    download_link += tag_name;
    download_link += "/";
    download_link += filenameA;
    wstring download_link_wstr = L"-V -o newVersion.exe --auto-file-renaming=false --allow-overwrite=true " + str2wstr(download_link);
    ZeroMemory(&a, sizeof(SHELLEXECUTEINFO));//使用前最好清空
    a.cbSize = sizeof(SHELLEXECUTEINFO);
    a.hwnd = 0;
    a.fMask = SEE_MASK_NOCLOSEPROCESS;
    a.lpDirectory = L"";
    a.lpFile = L"aria2c.exe";
    a.lpParameters = download_link_wstr.c_str();
#ifdef _DEBUG
    a.nShow = SW_NORMAL;
#else
    a.nShow = SW_HIDE;
#endif // DEBUG

    result = ShellExecuteEx(&a);
    if (result == FALSE)
    {
        //MessageBox(0,to_wstring(GetLastError()).c_str(), 0, 0);
        RunAndExit(6);
    }
    hdl = a.hProcess;
    WaitForSingleObject(a.hProcess, INFINITE);//等待执行完毕
    exitCode = STILL_ACTIVE;
    ecpResult = GetExitCodeProcess(hdl, &exitCode);
    if (ecpResult == 0)
        RunAndExit(7);
    if (exitCode == 0)
#ifdef _DEBUG
        MessageBox((HWND)winId(), L"Success!", L"notice", 0);
#else
        ;
#endif // DEBUG
    else
        RunAndExit(8);
    MoveFileEx(L"newVersion.exe", filenameW, MOVEFILE_REPLACE_EXISTING);
    RunAndExit(0);
}

pptHelperUpdateChecker::~pptHelperUpdateChecker()
{

}

string UTF8ToGBK(const char* str)
{
    string result;
    WCHAR* strSrc;
    LPSTR szRes;

    int i = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    strSrc = new WCHAR[i + 1];
    MultiByteToWideChar(CP_UTF8, 0, str, -1, strSrc, i);

    i = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL);
    szRes = new CHAR[i + 1];
    WideCharToMultiByte(CP_ACP, 0, strSrc, -1, szRes, i, NULL, NULL);

    result = szRes;
    delete[]strSrc;
    delete[]szRes;
    return result;
}
wstring str2wstr(string str)
{
    int cchWideChar = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, 0, 0);
    WCHAR* wchar1 = new WCHAR[cchWideChar];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wchar1, cchWideChar);
    wstring wstr = wchar1;
    return wstr;
}

std::wstring GetFileProductVersion(const wchar_t* fileAndpath)
{
    std::wstring description = L"";
    TCHAR* file_path = (WCHAR*)fileAndpath;
    //获取版本信息大小
    DWORD dwSize = GetFileVersionInfoSize(file_path, NULL);
    if (dwSize > 0) {
        TCHAR* pBuf = new TCHAR[dwSize + 1];
        memset(pBuf, 0, dwSize + 1);
        //获取版本信息
        GetFileVersionInfo(file_path, NULL, dwSize, pBuf);

        // Read the list of languages and code pages.
        LPVOID lpBuffer = NULL;
        UINT uLen = 0;

        UINT nQuerySize;
        DWORD* pTransTable;
        VerQueryValue(pBuf, L"\\VarFileInfo\\Translation", (void**)&pTransTable, &nQuerySize);
        DWORD m_dwLangCharset = MAKELONG(HIWORD(pTransTable[0]), LOWORD(pTransTable[0]));

        TCHAR SubBlock[50] = { 0 };
        wsprintf(SubBlock, L"\\StringFileInfo\\%08lx\\ProductVersion", m_dwLangCharset);

        VerQueryValue(pBuf, SubBlock, &lpBuffer, &uLen);
        if (uLen) 
            description = (TCHAR*)lpBuffer;
        delete[]pBuf;
    }

    return description;
}

void RunAndExit(int ExitCode)
{
    if(ClearFileReadOnlyAttribute(L"newestInfo.json"))
    remove("newestInfo.json");
    remove("newVersion.exe");
    remove("newVersion.exe.aria2");
#ifndef _DEBUG
    ShellExecute(0, L"open", L"pptHelper.exe", L"", L"", SW_NORMAL);

#endif // _DEBUG

    exit(ExitCode);
}

BOOL ClearFileReadOnlyAttribute(const wchar_t* fileNameAndPath)
{
    LPCTSTR ExistFile = fileNameAndPath;

    // 获取文件属性
    DWORD FileAttribute = GetFileAttributes(ExistFile);

    // 判断文件属性
    if (FileAttribute == INVALID_FILE_ATTRIBUTES)
    {
        // 隐藏属性 -- 直接返回
        return FALSE;
    }
    //else if (FileAttribute & FILE_ATTRIBUTE_READONLY)
    //{
    //    // 只读属性 -- 修改文件属性
    //    if(SetFileAttributes(ExistFile, FILE_ATTRIBUTE_NORMAL))
    //        return TRUE;
    //}
    if(!SetFileAttributes(ExistFile, FILE_ATTRIBUTE_NORMAL))
        return FALSE;
    // 文件的删除
    //BOOL ResDelete = DeleteFile(ExistFile);

    //if (!ResDelete)
    //{
    //    // 显示提示框
    //    //MessageBox(NULL, L"删除失败", L"提示窗口", 0);
    //}
    return TRUE;
}