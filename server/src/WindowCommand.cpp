#include "WindowCommand.hpp"
void WindowCommand::SaveBitmapToFile(HBITMAP HBitmap,  const wchar_t* filename)
{
    using namespace Gdiplus;
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    Bitmap bitmap(HBitmap,NULL);
    CLSID pngClsid;
    CLSIDFromString(L"{557CF406-1A04-11D3-9A73-0000F81EF32E}", &pngClsid); // PNG
    bitmap.Save(filename, &pngClsid, NULL);

    GdiplusShutdown(gdiplusToken);
}

void WindowCommand::ScreenShot(const wchar_t* filename)
{
    HWND hDesktopWnd = GetDesktopWindow();
    HDC hDesktopDC = GetDC(hDesktopWnd);
    HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);
    RECT desktopParams;
    GetWindowRect(hDesktopWnd, &desktopParams);
    int width = desktopParams.right;
    int height = desktopParams.bottom;

    HBITMAP hBitmap = CreateCompatibleBitmap(hDesktopDC, width, height);
    SelectObject(hCaptureDC, hBitmap);
    BitBlt(hCaptureDC, 0, 0, width, height, hDesktopDC, 0, 0, SRCCOPY | CAPTUREBLT);

    SaveBitmapToFile(hBitmap, filename);

    DeleteObject(hBitmap);
    DeleteDC(hCaptureDC);
    ReleaseDC(hDesktopWnd, hDesktopDC);
}

void WindowCommand::ShutDown()
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;


    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return;

    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0);

    BOOL result = ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, SHTDN_REASON_MAJOR_OPERATINGSYSTEM);

}

void WindowCommand::Restart()
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;


    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return;

    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0);

    BOOL result = ExitWindowsEx(EWX_REBOOT | EWX_FORCE, SHTDN_REASON_MAJOR_OPERATINGSYSTEM);
}

void HandleRequest(SOCKET client_socket, const PacketHeader &header)
{

}


