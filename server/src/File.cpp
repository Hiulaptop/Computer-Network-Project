#include "File.hpp"
#include "winsock2.h"
#include <filesystem>

void File::HandleRequest(SOCKET clientSocket,const PacketHeader& header){
    int len = header.packet_size - sizeof(header);

    if (len < 0)
    {
        return;
    }
    char * buf = new char[len + 1];
    int recive = recv(clientSocket, buf, len, 0);

    if (recive <= 0){
        std::cerr << "Failed to receive header from client." << std::endl;
        closesocket(clientSocket);
        return;
    }
    buf[len] = '\0';
    if (header.request_type == 0)
    {
    SendFile(buf, clientSocket, header);
    }
    else if (header.request_type == 1)
    {
        ListCurrentDir(buf, clientSocket, header);
    }
}

void File::SendFile(const char *PathName, SOCKET clientSocket,const PacketHeader& header)
{
    FILE *file;
    char *Buffer;
    unsigned long Size;
    file = fopen(PathName, "rb");
    printf(("path = %s\n", PathName));
    if (!file)
    {
        printf("Error while reading the file\n");
        getchar();
        return;
    }

    fseek(file, 0, SEEK_END);
    Size = ftell(file);
    fseek(file, 0, SEEK_SET);
    Buffer = new char[Size + 1];
    fread(Buffer, Size, 1, file);

    char cSize[MAX_PATH];
    sprintf(cSize, "%i", Size);

    fclose(file);

    Response res(header.request_id + 1, 200);
    res.setMessage(Buffer);
    res.sendResponse(clientSocket);

    // int sent = send(clientSocket, Buffer, Size, 0);
    // if (sent == SOCKET_ERROR) {
    //     std::cerr << "Failed to send file to client." << std::endl;
    // }
    delete[] Buffer;
}

void File::ListCurrentDir(const char* PathName, SOCKET clientSocket, const PacketHeader& header)
{
    std::string s;
    for (const auto& entry : std::filesystem::directory_iterator(PathName)) {
        // std::cout << entry.path() << std::endl;
        s += entry.path().string();
        s += '\n';
    }

    Response res(header.request_id + 1, 200);
    res.setMessage(s.c_str());
    res.sendResponse(clientSocket);
}
