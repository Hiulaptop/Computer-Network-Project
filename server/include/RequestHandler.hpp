#pragma once
#include <cstdint>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

constexpr int FEATURE_COUNT = 5;

struct PacketHeader {
    uint32_t packet_size;
    uint16_t request_id;
    uint8_t  request_type;
    uint8_t  request_key;
};

class FeatureHandler {
public:
    virtual ~FeatureHandler() = default;

    virtual void HandleRequest(SOCKET client_socket,const PacketHeader& header) = 0;
};

class RequestHandler {
public:
    RequestHandler();
    static DWORD WINAPI ProcessClient(LPVOID lpParam);
private:
    static FeatureHandler* featureHandlers[FEATURE_COUNT];
};
