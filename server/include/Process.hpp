#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#include "RequestHandler.hpp"
#include "Response.hpp"
#include <windows.h>

struct ProcessInfo {
    DWORD PID; //ProcessID
    char Name[260];
    DWORD ParentPID;
    SIZE_T MemoryUsage;
    ULONGLONG CPUTimeUser;
};

class Process: public FeatureHandler{
    constexpr static int REQUEST_KEY = 0x00;
public:
    static bool TerminateProcessByID(const int& PID);
    static bool OpenFileByPath(const std::string& FilePath);
    static std::vector<ProcessInfo> ListProcess();
    static std::string ProcessListToMessage(std::vector<ProcessInfo>& ProcessList);


    void HandleRequest(SOCKET client_socket, const PacketHeader &header) override;
};