#include "File.hpp"


void File::HandleRequest(SOCKET clientSocket,const PacketHeader& header){
    char * buf;
    int len = header.packet_size - sizeof(header);
    int recive = recv(clientSocket, buf, len, 0);

    if (recive <= 0){
        std::cerr << "Failed to receive header from client." << std::endl;
        closesocket(clientSocket);
        return;
    }

    SendFile(buf, clientSocket, header);
}

void File::SendFile(const char *PathName, SOCKET clientSocket,const PacketHeader& header)
{
    FILE *file;
    char *Buffer;
    unsigned long Size;
    file = fopen(PathName, "rb");

    if (!file)
    {
        printf("Error while readaing the file\n");
        getchar();
        return;
    }

    fseek(file, 0, SEEK_END);
    Size = ftell(file);
    fseek(file, 0, SEEK_SET);
    Buffer = new char[Size];
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
    // delete[] Buffer;
}