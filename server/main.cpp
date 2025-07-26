#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <vector>
#include <string>
#include <cstdint>

struct PacketHeader {
    uint16_t request_id;
    uint8_t  request_type;
    uint8_t  request_key;
};

int main(int argc, char *argv[])
{

    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;

    int wsaerr = WSAStartup(wVersionRequested, &wsaData);
    if (wsaerr != 0)
    {

        std::cerr << "Failed to initialize Winsock: " << wsaerr << '\n';
        return 1;
    }

    std::cout << "Winsock initialized successfully" << '\n';


    const SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "Cannot create socket" << '\n';
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed!" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }


    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed!" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "Server is listening on port 8080..." << std::endl;


    std::vector<HANDLE> clients;
    int idx = 0;
    while (true)
    {
        sockaddr_in clientAddr{};
        int clientSize = sizeof(clientAddr);
        const SOCKET clientSocket = accept(serverSocket, reinterpret_cast<sockaddr *>(&clientAddr), &clientSize);
        if (clientSocket == INVALID_SOCKET)
        {
            continue;
        }
        HANDLE client_thread = CreateThread(
            nullptr,
            0,
            ProcessClient,
            reinterpret_cast<LPVOID>(clientSocket),
            0,
            nullptr
        );

        if (client_thread == nullptr)
        {
            closesocket(clientSocket);
            break;
        }

        clients.push_back(client_thread);
        idx++;
    }


    for (HANDLE h : clients)
    {
        CloseHandle(h);
    }
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}