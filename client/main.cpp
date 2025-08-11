#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <curl/curl.h>

#include "Mail.hpp"
// #pragma comment(lib, "Ws2_32.lib")
//
// struct PacketHeader {
//     uint16_t request_id;
//     uint8_t  request_type;
//     uint8_t  request_key;
// };
// struct ResponseHeader {
//     uint32_t packageSize;
//     uint16_t responseID;
//     uint16_t statusCode;
// };
//
// void send_connect_request(const SOCKET clientSocket) {
//     PacketHeader header{};
//     header.request_id = 1;
//     header.request_type = 1;
//     header.request_key = 1;
//
//     char request[] = "Hello world";
//     uint32_t packetSize = (sizeof(PacketHeader) + sizeof(request) - 1);
//     send(clientSocket, reinterpret_cast<const char *>(&packetSize), sizeof(packetSize), 0);
//     send(clientSocket, reinterpret_cast<const char *>(&header), sizeof(header), 0);
//     send(clientSocket, request, sizeof(request) - 1, 0);
//     ResponseHeader responseHeader{};
//     recv(clientSocket, reinterpret_cast<char *>(&responseHeader), sizeof(responseHeader), 0);
//     char *buf = new char[responseHeader.packageSize - sizeof(responseHeader) + 1];
//     recv(clientSocket, buf, responseHeader.packageSize - sizeof(responseHeader), 0);
//     buf[responseHeader.packageSize - sizeof(responseHeader)] = '\0';
//     std::cout << "Received response: " << buf << std::endl;
//     delete[] buf;
// }

static size_t WriteCallback(void *contents, const size_t size, const size_t nmemb, void *userp)
{
    static_cast<std::string *>(userp)->append(static_cast<char *>(contents), size * nmemb);
    return size * nmemb;
}

int main(int argc, char *argv[]) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    // WSADATA wsaData;
    // if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    //     std::cerr << "Failed to initialize Winsock!" << std::endl;
    //     return 1;
    // }
    //
    // SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    // if (clientSocket == INVALID_SOCKET) {
    //     std::cerr << "Cannot create socket!" << std::endl;
    //     WSACleanup();
    //     return 1;
    // }
    //
    // sockaddr_in serverAddr{};
    // serverAddr.sin_family = AF_INET;
    // serverAddr.sin_port = htons(8080);
    // serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server address
    //
    // if (connect(clientSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
    //     std::cerr << "Failed to connect to server!" << std::endl;
    //     closesocket(clientSocket);
    //     WSACleanup();
    //     return 1;
    // }
    //
    // std::cout << "Connected to server successfully!" << std::endl;
    //
    // // Send connect request to server
    // send_connect_request(clientSocket);
    //
    // closesocket(clientSocket);
    // WSACleanup();
    MailService ms;
    ms.Init("c2c.server.mmt@gmail.com","thsf taeu xfjw lnqq","C:\\Users\\BBQPa\\OneDrive\\Documents\\HCMUS\\OOP\\Computer-Network-Project\\client\\external\\curl-8.15.0_4-win64-mingw\\bin\\curl-ca-bundle.crt");
    ms.CheckMail();
    ms.Response("thie199299@gmail.com", "Hello, this is a test email from the Management System client.", nullptr);
    ms.CleanUpCURL();

    curl_global_cleanup();
    return 0;
}
