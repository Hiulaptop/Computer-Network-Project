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
    delete[] buf;
}

void File::SendFile(const char *PathName, SOCKET clientSocket,const PacketHeader& header)
{
    FILE *file = fopen(PathName, "rb");
    printf("path = %s\n", PathName);
    if (!file)
    {
        printf("Error while reading the file\n");
        getchar();
        return;
    }

    fseek(file, 0, SEEK_END);
    unsigned long Size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *Buffer = new char[Size + 1];
    fread(Buffer, Size, 1, file);

    char cSize[MAX_PATH];
    sprintf(cSize, "%i", Size);

    fclose(file);

    Response res(header.request_id + 1, 0x00);
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
        if (entry.is_directory()) {
            s += "D";
            s += entry.path().filename().string();
            s += '\n';
            s += "0\n";
        } else if (entry.is_regular_file()) {
            s += "F";
            s += entry.path().filename().string();
            s += '\n';

            double size = entry.file_size();
            double tomb = size/1000.0f;
            if (tomb <= 0.0001f){
                s += std::to_string(size);
                s += "kb\n";
            }
            else{
                s += std::to_string(tomb);
                s += "mb\n";
            }
            
        } else {
            s += "N";
            s += entry.path().filename().string();
            s += '\n';
            s += "0\n";
        }
        
    }

    Response res(header.request_id + 1, 0x00);
    res.setMessage(s.c_str());
    res.sendResponse(clientSocket);
}
