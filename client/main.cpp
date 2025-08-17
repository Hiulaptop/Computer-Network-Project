#include <winsock2.h>
#include <iostream>
#include <mfapi.h>
#include <ws2tcpip.h>

#include "HandleFeature.hpp"
#include "Mail.hpp"
#include "UICore.hpp"

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock!" << std::endl;
        return 1;
    }
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize COM library: " << std::hex << hr << std::endl;
        return 1;
    }
    hr = MFStartup(MF_VERSION);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize Media Foundation: " << std::hex << hr << std::endl;
        CoUninitialize();
        WSACleanup();
        return 1;
    }
    MailService::Init("c2c.server.mmt@gmail.com", "thsf taeu xfjw lnqq", "curl-ca-bundle.crt");
    Core core;
    core.Init();
    core.Start();
    core.Shutdown();
    WSACleanup();
    MFShutdown();
    CoUninitialize();
    return 0;
}
