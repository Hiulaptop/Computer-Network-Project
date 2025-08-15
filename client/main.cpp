#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "UICore.hpp"

int main() {
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    int wsaerr = WSAStartup(wVersionRequested, &wsaData);
    if (wsaerr != 0) {
        std::cerr << "Failed to initialize Winsock: " << wsaerr << '\n';
        return 1;
    }
    std::cout << "Winsock initialized successfully" << '\n';
    Core core;
    core.Init();
    core.Start();
    core.Shutdown();
    WSACleanup();
    return 0;
}
