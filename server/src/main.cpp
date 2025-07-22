#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
// #pragma comment(lib, "Ws2_32.lib")

// Add all process here

struct Message
{
    std::string sender;
    std::string receiver;
    std::string content;
    std::string timestamp;
    // Add more fields as needed
};

// Example: parse a message in the format: sender|receiver|timestamp|content\n
Message parse_tcp_message(const std::string &raw)
{
    Message msg;
    size_t pos1 = raw.find('|');
    size_t pos2 = raw.find('|', pos1 + 1);
    size_t pos3 = raw.find('|', pos2 + 1);
    if (pos1 == std::string::npos || pos2 == std::string::npos || pos3 == std::string::npos)
    {
        // Invalid format, return empty message
        return msg;
    }
    msg.sender = raw.substr(0, pos1);
    msg.receiver = raw.substr(pos1 + 1, pos2 - pos1 - 1);
    msg.timestamp = raw.substr(pos2 + 1, pos3 - pos2 - 1);
    msg.content = raw.substr(pos3 + 1);
    return msg;
}

// Thread function to handle communication with a connected client
DWORD WINAPI ProcessClient(LPVOID arg)
{
    SOCKET clientSocket = (SOCKET)arg;
    char buffer[1024];
    int bytesReceived;
    while (true)
    {
        // Receive the size of the incoming message (4 bytes, network byte order)
        int total = 0;
        int to_read = sizeof(uint32_t);
        while (total < to_read)
        {
            int n = recv(clientSocket, buffer + total, to_read - total, 0);
            if (n <= 0)
            {
                // Connection closed or error
                closesocket(clientSocket);
                return 0;
            }
            total += n;
        }
        uint32_t msg_size;
        memcpy(&msg_size, buffer, sizeof(uint32_t));
        msg_size = ntohl(msg_size);
        if (msg_size > 1000)
            msg_size = 1000; // Prevent buffer overflow

        // Receive the actual message data
        total = 0;
        while (total < (int)msg_size)
        {
            int n = recv(clientSocket, buffer + total, msg_size - total, 0);
            if (n <= 0)
            {
                // Connection closed or error
                closesocket(clientSocket);
                return 0;
            }
            total += n;
        }
        buffer[msg_size] = '\0';
        std::string raw_msg(buffer);
        // Parse the received message
        Message msg = parse_tcp_message(raw_msg);
        // Print the parsed message information
        printf("Received message:\n  From: %s\n  To: %s\n  At: %s\n  Content: %s\n",
               msg.sender.c_str(), msg.receiver.c_str(), msg.timestamp.c_str(), msg.content.c_str());
        // Optionally, send a response here using send(clientSocket, ...)
    }
    // Close the client socket when done
    closesocket(clientSocket);
    return 0;
}

int main(int argc, char *argv[])
{
    // Initialize Winsock
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;

    int wsaerr = WSAStartup(wVersionRequested, &wsaData);
    if (wsaerr != 0)
    {
        // Failed to initialize Winsock
        std::cerr << "Failed to initialize Winsock: " << wsaerr << '\n';
        return 1;
    }
    // Winsock initialized successfully
    std::cout << "Winsock initialized successfully" << '\n';

    // Create server socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "Cannot create socket" << '\n';
        WSACleanup();
        return 1;
    }

    // Set up server address structure
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080); // Listen on port 8080


    // Bind the socket to the address and port
    if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed!" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Start listening for incoming connections
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed!" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "Server is listening on port 8080..." << std::endl;

    // Vector to store client thread handles
    std::vector<HANDLE> clients;
    int idx = 0;
    while (true)
    {
        // Accept a new client connection
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr *)&clientAddr, &clientSize);

        if (clientSocket == INVALID_SOCKET)
        {
            continue;
        }

        std::cout << "to Here1\n";

        // Create a new thread to handle the client
        HANDLE client_thread = CreateThread(
            NULL,                 // Default security attributes
            0,                    // Default stack size
            ProcessClient,        // Thread function
            (LPVOID)clientSocket, // Pass client socket as parameter
            0,                    // Default creation flags
            NULL                  // No thread identifier needed
        );

        if (client_thread == NULL)
        {
            closesocket(clientSocket); // Clean up socket on thread creation failure
            continue;                  // Handle error
        }
        std::cout << "to Here2\n";

        clients.push_back(client_thread); // Store thread handle
        idx++;
    }

    // Close all client thread handles before exiting
    for (HANDLE h : clients)
    {
        CloseHandle(h);
    }
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}