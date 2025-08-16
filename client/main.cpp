#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdint>

#include "UICore.hpp"

struct PacketHeader {
    uint32_t packet_size;
    uint16_t request_id;
    uint8_t  request_type;
    uint8_t  request_key;
};
struct ResponseHeader {
    uint32_t packageSize;
    uint16_t responseID;
    uint16_t statusCode;
};

// void send_connect_request(const SOCKET clientSocket) {
//     PacketHeader header{};
//     header.request_id = 1;
//     header.request_type = 1;
//     header.request_key = 0;
//
//     char request[] = "C:\\Users\\Phat Truong\\Downloads";
//     header.packet_size = (sizeof(PacketHeader) + sizeof(request) - 1);
//     send(clientSocket, reinterpret_cast<const char *>(&header), sizeof(header), 0);
//     send(clientSocket, request, sizeof(request) - 1, 0);
//     ResponseHeader responseHeader{};
//     recv(clientSocket, reinterpret_cast<char *>(&responseHeader), sizeof(responseHeader), 0);
//     int len = responseHeader.packageSize - sizeof(responseHeader) + 1;
//     if (len > 0)
//     {
//         char *buf = new char[len];
//         recv(clientSocket, buf, len - 1, 0);
//         buf[len - 1] = '\0';
//         std::cout << "Received response: " << buf << std::endl;
//         delete[] buf;
//     }
// }

int main(int argc, char *argv[]) {
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
