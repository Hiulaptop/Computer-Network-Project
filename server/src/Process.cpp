#include <fstream>
#include <Process.hpp>
#include <psapi.h>
#include <tlhelp32.h>

ULONGLONG FileTimeToULL(const FILETIME& FT) {
    ULARGE_INTEGER res;
    res.LowPart = FT.dwLowDateTime;
    res.HighPart = FT.dwHighDateTime;
    return res.QuadPart;
}

std::wstring ConvertTCHARToWString(TCHAR* tchar_str) {
    int len = MultiByteToWideChar(CP_ACP, 0, tchar_str, -1, nullptr, 0);
    std::wstring wstr(len, L'\0');
    MultiByteToWideChar(CP_ACP, 0, tchar_str, -1, &wstr[0], len);
    return wstr;
}


std::wstring StringToWString(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size_needed);
    return wstr;
}

std::vector<ProcessInfo> Process::ListProcess(){
    std::vector<ProcessInfo> ProcessList;
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
    if(hSnapShot == INVALID_HANDLE_VALUE) return ProcessList;
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);
    if(Process32First(hSnapShot,&pe)){
        do{
            ProcessInfo Process;
            Process.PID = pe.th32ProcessID;
            memcpy(Process.Name,pe.szExeFile,sizeof(pe.szExeFile));
            Process.ParentPID = pe.th32ParentProcessID;
            Process.ThreadCount = pe.cntThreads;
            Process.MemoryUsage = 0;
            Process.FullPath = ConvertTCHARToWString("Unknow");
            Process.CPUTimeUser = 0;
            Process.CPUTimeKernel = 0;
            Process.PriorityClass = 0;
            Process.SessionID = 0;

            HANDLE HProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, Process.PID);
            if(HProcess){
                TCHAR Path[MAX_PATH];
                if(GetModuleFileNameEx(HProcess,nullptr,Path,MAX_PATH))
                    Process.FullPath = ConvertTCHARToWString(Path);

                FILETIME createTime, exitTime, kernelTime, userTime;
                if (GetProcessTimes(HProcess, &createTime, &exitTime, &kernelTime, &userTime)) {
                    Process.CPUTimeKernel = FileTimeToULL(kernelTime);
                    Process.CPUTimeUser = FileTimeToULL(userTime);
                }

                PROCESS_MEMORY_COUNTERS pmc;
                if(GetProcessMemoryInfo(HProcess,&pmc,sizeof(pmc)))
                    Process.MemoryUsage = pmc.WorkingSetSize;

                Process.PriorityClass = GetPriorityClass(HProcess);

                ProcessIdToSessionId(Process.PID, &Process.SessionID);

                CloseHandle(HProcess);
            }

            ProcessList.push_back(Process);

        }while(Process32Next(hSnapShot,&pe));
    }
    return ProcessList;
}

bool Process::TerminateProcessByID(const int& PID){
    DWORD ProcessID = (DWORD)PID;
    HANDLE HProcess = OpenProcess(PROCESS_TERMINATE | PROCESS_VM_READ, FALSE, ProcessID);
    if(!HProcess || !TerminateProcess(HProcess,0))
        return false;
    return true;
}

void Process::PrintProcessList(const std::vector<ProcessInfo>& ProcessList) {
    std::wofstream outFile(L"processes.txt");
    for (const auto& Process : ProcessList) {
        outFile << L"=============================\n";
        outFile << L"Name       : " << Process.Name << L"\n";
        outFile << L"PID        : " << Process.PID << L"\n";
        outFile << L"Parent PID : " << Process.ParentPID<< L"\n";
        outFile << L"Threads    : " << Process.ThreadCount << L"\n";
        outFile << L"Full Path  : " << Process.FullPath << L"\n";
        outFile << L"RAM Usage  : " << Process.MemoryUsage / 1024 << L" KB\n";
        outFile << L"CPU User   : " << Process.CPUTimeUser / 10000 << L" ms\n";
        outFile << L"CPU Kernel : " << Process.CPUTimeKernel / 10000 << L" ms\n";
        outFile << L"Priority   : " << Process.PriorityClass << L"\n";
        outFile << L"Session ID : " << Process.SessionID << L"\n";
    }
}

void Process::HandleRequest(SOCKET client_socket, const PacketHeader &header) {
    if (header.request_key != REQUEST_KEY) {
        std::cerr << "Invalid request key." << std::endl;
        return;
    }
    if (header.request_type == 0x01) {
        auto ProcessList = ListProcess();
        PrintProcessList(ProcessList);
        std::string response = "Process list saved to processes.txt";
        send(client_socket, response.c_str(), response.size(), 0);
    } else if (header.request_type == 0x02) {
        int PID;
        recv(client_socket, reinterpret_cast<char*>(&PID), sizeof(PID), 0);
        bool result = TerminateProcessByID(PID);
        std::string response = result ? "Process terminated successfully." : "Failed to terminate process.";
        send(client_socket, response.c_str(), response.size(), 0);
    } else if (header.request_type == 0x03) {
        char filePath[260];
        recv(client_socket, filePath, sizeof(filePath), 0);
        bool result = OpenFileByPath(filePath);
        std::string response = result ? "File opened successfully." : "Failed to open file.";
        send(client_socket, response.c_str(), response.size(), 0);
    } else {
        std::cerr << "Unknown request type." << std::endl;
    }
    closesocket(client_socket);
}

bool Process::OpenFileByPath(const std::string& FilePath){
    HINSTANCE res = ShellExecuteA(
        nullptr,
        "open",
        FilePath.c_str(),
        nullptr,
        nullptr,
        SW_SHOWNORMAL
    );
    if((INT_PTR)res > 32)
        return true;
    return false;
}
