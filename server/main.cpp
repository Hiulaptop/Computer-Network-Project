#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

int main(int agrc, char **agrv)
{
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;

    int wsaerr = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaerr != 0)
    {
        std::cerr << "Khoi tao Winsock that bai: " << wsaerr << '\n';
        return 1;
    }
    std::cout << "Winsock da duoc khoi tao thanh cong" << '\n';

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "Khong the tao socket" << '\n';
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Bind that bai!" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    listen(serverSocket, SOMAXCONN);
    std::cout << "Server dang lang nghe tren cong 8080..." << std::endl;

    while (true)
    {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);

        if (clientSocket != INVALID_SOCKET) {
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
            std::cout << "Ket noi tu: " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;
            char buffer[1024];
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0'; // Đảm bảo chuỗi kết thúc đúng
                std::cout << "Request tu client: " << buffer << std::endl;
            }
            closesocket(clientSocket);
            break;
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}