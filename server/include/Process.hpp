#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <tchar.h>
#include <iostream>
#include <vector>
#include <string>
#include<fstream>

#pragma comment(lib, "Psapi.lib")

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

class Process{
public:
    static bool TerminateProcessByID(const int& PID);
    static bool OpenFileByPath(const std::string& FilePath);
    static std::vector<ProcessInfo> ListProcess();
    static void PrintProcessList(const std::vector<ProcessInfo>& ProcessList);
};