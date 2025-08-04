#pragma once
#include <iostream>
#include <vector>
#include <string>

#include "RequestHandler.hpp"
#include "Response.hpp"
#include <windows.h>

struct ProcessInfo {
    DWORD PID; //ProcessID
    char Name[260];
    std::wstring FullPath;
    DWORD ParentPID;
    DWORD ThreadCount;
    SIZE_T MemoryUsage;
    ULONGLONG CPUTimeUser;
    ULONGLONG CPUTimeKernel;
    DWORD PriorityClass;
    DWORD SessionID;
};

class Process: public FeatureHandler{
    constexpr static int REQUEST_KEY = 0x01;
public:
    static bool TerminateProcessByID(const int& PID);
    static bool OpenFileByPath(const std::string& FilePath);
    static std::vector<ProcessInfo> ListProcess();
    static void PrintProcessList(const std::vector<ProcessInfo>& ProcessList);

    void HandleRequest(SOCKET client_socket, const PacketHeader &header) override;
};