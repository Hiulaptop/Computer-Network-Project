#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

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
    Core core;
    core.Init();
    core.Start();
    core.Shutdown();
    WSACleanup();
    CoUninitialize();
    return 0;
}
