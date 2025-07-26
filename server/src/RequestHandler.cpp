#include "RequestHandler.hpp"

#include <iostream>

RequestHandler::RequestHandler() {

}

DWORD RequestHandler::ProcessClient(LPVOID lpParam) const {
    auto clientSocket = reinterpret_cast<SOCKET>(lpParam);

    PacketHeader header{};
    int bytesReceived = recv(clientSocket, reinterpret_cast<char *>(&header), sizeof(header), 0);
    if (bytesReceived <= 0)
    {
        std::cerr << "Failed to receive header from client." << std::endl;
        closesocket(clientSocket);
        return 1;
    }
    this->featureHandlers[header.request_key]->HandleRequest(clientSocket,header);
    closesocket(clientSocket);
    return 0;
}
