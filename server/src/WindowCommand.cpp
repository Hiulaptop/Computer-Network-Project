#include "WindowCommand.hpp"
// void WindowCommand::SaveBitmapToFile(HBITMAP HBitmap, const PacketHeader& header)
// {
//     using namespace Gdiplus;
//     GdiplusStartupInput gdiplusStartupInput;
//     ULONG_PTR gdiplusToken;
//     GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
//
//     Bitmap bitmap(HBitmap,NULL);
//     // CLSID pngClsid;
//     // CLSIDFromString(L"{557CF406-1A04-11D3-9A73-0000F81EF32E}", &pngClsid); // PNG
//
//     Response res(header.request_id + 1, 200);
//     res.setMessage(bitmap.);
//
//     //bitmap.Save(filename, &pngClsid, NULL);
//
//     GdiplusShutdown(gdiplusToken);
// }
std::string WindowCommand::GetDIBitsBMP(HBITMAP hBitmap, int width, int height)
{
    HDC hdc = GetDC(NULL);

    BITMAPINFO bi;
    ZeroMemory(&bi, sizeof(BITMAPINFO));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = width;
    bi.bmiHeader.biHeight = height; // bottom-up BMP
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24; // 24-bit RGB
    bi.bmiHeader.biCompression = BI_RGB;

    DWORD dwBmpSize = ((width * bi.bmiHeader.biBitCount + 31) / 32) * 4 * height;

    std::string pixelData;
    pixelData.resize(dwBmpSize);

    if (::GetDIBits(hdc, hBitmap, 0, (UINT)height, pixelData.data(), &bi, DIB_RGB_COLORS) == 0)
    {
        ReleaseDC(NULL, hdc);
        return {};
    }

    ReleaseDC(NULL, hdc);

    BITMAPFILEHEADER bmfHeader;
    bmfHeader.bfType = 0x4D42; // "BM"
    bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmfHeader.bfSize = bmfHeader.bfOffBits + dwBmpSize;
    bmfHeader.bfReserved1 = 0;
    bmfHeader.bfReserved2 = 0;

    std::string bmpFile;
    bmpFile.resize(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwBmpSize);
    memcpy(bmpFile.data(), &bmfHeader, sizeof(BITMAPFILEHEADER));
    memcpy(bmpFile.data() + sizeof(BITMAPFILEHEADER), &bi.bmiHeader, sizeof(BITMAPINFOHEADER));
    memcpy(bmpFile.data() + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), pixelData.data(), dwBmpSize);

    return bmpFile;
}

void WindowCommand::ScreenShot(SOCKET& client_socket,const PacketHeader& header)
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


    //SaveBitmapToFile(hBitmap,header);
    std::string message = GetDIBitsBMP(hBitmap,width,height);
    Response res(header.request_id + 1, 0x00);
    res.setMessage(message);
    res.sendResponse(client_socket);
    DeleteObject(hBitmap);
    DeleteDC(hCaptureDC);
    ReleaseDC(hDesktopWnd, hDesktopDC);
}

void WindowCommand::ShutDown(SOCKET& client_socket,const PacketHeader& header)
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;


    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        Response res(header.request_id, 0x02);
        res.setMessage("Failed to open process token for shutdown.");
        res.sendResponse(client_socket);
        return;
    }

    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0);

    Response res(header.request_id, 0x00);
    res.setMessage("System shutdown initiated successfully.");
    BOOL result = ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, SHTDN_REASON_MAJOR_OPERATINGSYSTEM);
    if (result == FALSE) {
        res.setStatusCode(0x01);
        res.setMessage("Failed to shutdown the system.");
        res.sendResponse(client_socket);
        CloseHandle(hToken);
        return;
    }
    res.sendResponse(client_socket);
    CloseHandle(hToken);
}

void WindowCommand::Restart(SOCKET& client_socket,const PacketHeader& header)
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;


    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        Response res(header.request_id, 0x02);
        res.setMessage("Failed to open process token for restart.");
        res.sendResponse(client_socket);
        return;
    }

    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0);
    Response res(header.request_id, 0x00);
    res.setMessage("System restart initiated successfully.");
    BOOL result = ExitWindowsEx(EWX_REBOOT | EWX_FORCE, SHTDN_REASON_MAJOR_OPERATINGSYSTEM);
    if (result == FALSE) {
        res.setStatusCode(0x01);
        res.setMessage("Failed to restart the system.");
        res.sendResponse(client_socket);
        CloseHandle(hToken);
        return;
    }
    res.sendResponse(client_socket);
    CloseHandle(hToken);
}

void WindowCommand::SetVolume(const float absolute, SOCKET& client_socket,const PacketHeader& header)
{
    CoInitialize(NULL);
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (SUCCEEDED(hr))
    {
        hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    }
    IAudioEndpointVolume* pEndpointVolume = NULL;
    if (SUCCEEDED(hr))
    {
        hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&pEndpointVolume);
    }

    float currentVolume = 0.0f;
    if (SUCCEEDED(hr))
    {
        hr = pEndpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
        if (SUCCEEDED(hr))
        {
            std::cout << "Current Master Volume: " << currentVolume * 100.0f << "%" << std::endl;
        }
    }

    float newVolume = currentVolume + absolute; // Set to 50%
    if (SUCCEEDED(hr))
    {
        hr = pEndpointVolume->SetMasterVolumeLevelScalar(newVolume, NULL); // NULL for no event context
        if (SUCCEEDED(hr))
        {
            Response res(header.request_id, 0x00);
            res.setMessage("Master Volume set successfully.");
            res.sendResponse(client_socket);
            std::cout << "Master Volume set to: " << newVolume * 100.0f << "%" << std::endl;
        }
    }
    if (FAILED(hr))
    {
        Response res(header.request_id, 0x03);
        res.setMessage("Failed to set Master Volume.");
        res.sendResponse(client_socket);
        std::cerr << "Failed to set Master Volume: " << std::hex << hr << std::endl;
    }
    if (pEndpointVolume) pEndpointVolume->Release();
    if (pDevice) pDevice->Release();
    if (pEnumerator) pEnumerator->Release();
    CoUninitialize();
}

void WindowCommand::HandleRequest(SOCKET client_socket, const PacketHeader &header)
{
    switch (header.request_type)
    {
    case 0:
        ScreenShot(client_socket,header);
        break;
    case 1:
        SetVolume(0.1f,client_socket,header);
        break;
    case 2:
        SetVolume(-0.1f,client_socket,header);
        break;
    case 3:
        ShutDown(client_socket,header);
        break;
    case 4:
        Restart(client_socket,header);
        break;
    default: {
        Response res(header.request_id, 0x01);
        res.setMessage("Invalid request type for WindowCommand.");
        res.sendResponse(client_socket);
        std::cerr << "Invalid request type: " << header.request_type << std::endl;
        break;
    }
    }
}


