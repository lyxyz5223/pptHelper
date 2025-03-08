#include <windows.h>
#include <stdint.h>
#include <stdio.h>


void ShootScreen(const wchar_t* filename, HWND hWnd)
{
    HDC hdc = CreateDC(L"DISPLAY", NULL, NULL, NULL);
    int32_t ScrWidth = 0, ScrHeight = 0;
    RECT rect = { 0 };
    if (hWnd == NULL)
    {
        ScrWidth = GetDeviceCaps(hdc, HORZRES);
        ScrHeight = GetDeviceCaps(hdc, VERTRES);
        ScrWidth = GetSystemMetrics(SM_CXSCREEN);
        ScrHeight = GetSystemMetrics(SM_CYSCREEN);
    }
    else
    {
        GetWindowRect(hWnd, &rect);
        ScrWidth = rect.right - rect.left;
        ScrHeight = rect.bottom - rect.top;
    }
    HDC hmdc = CreateCompatibleDC(hdc);

    HBITMAP hBmpScreen = CreateCompatibleBitmap(hdc, ScrWidth, ScrHeight);
    HBITMAP holdbmp = (HBITMAP)SelectObject(hmdc, hBmpScreen);

    BITMAP bm;
    GetObject(hBmpScreen, sizeof(bm), &bm);

    BITMAPINFOHEADER bi = { 0 };
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bm.bmWidth;
    bi.biHeight = bm.bmHeight;
    bi.biPlanes = bm.bmPlanes;
    bi.biBitCount = bm.bmBitsPixel;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = bm.bmHeight * bm.bmWidthBytes; // Í¼Æ¬µÄÏñËØÊý¾Ý
    char* buf = new char[bi.biSizeImage];
    BitBlt(hmdc, 0, 0, ScrWidth, ScrHeight, hdc, rect.left, rect.top, SRCCOPY);
    GetDIBits(hmdc, hBmpScreen, 0L, (DWORD)ScrHeight, buf, (LPBITMAPINFO)&bi, (DWORD)DIB_RGB_COLORS);

    BITMAPFILEHEADER bfh = { 0 };
    bfh.bfType = ((WORD)('M' << 8) | 'B');
    bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bi.biSizeImage;
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    DWORD dwWrite;
    WriteFile(hFile, &bfh, sizeof(BITMAPFILEHEADER), &dwWrite, NULL);
    WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &dwWrite, NULL);
    WriteFile(hFile, buf, bi.biSizeImage, &dwWrite, NULL);
    CloseHandle(hFile);
    hBmpScreen = (HBITMAP)SelectObject(hmdc, holdbmp);
}

int32_t main()
{
    wchar_t name[256] = { 0 };
    for (int32_t i = 0; i < 20; ++i)
    {
        wsprintf(name, L"%d.bmp", i);
        wprintf(L"shooting %s\n", name);
        ShootScreen(name, NULL);
        Sleep(1000);
    }
    return 0;
}