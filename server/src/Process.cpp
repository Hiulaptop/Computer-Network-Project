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
            memcpy(Process.Name, pe.szExeFile, sizeof(pe.szExeFile));
            Process.ParentPID = pe.th32ParentProcessID;
            Process.MemoryUsage = 0;
            Process.CPUTimeUser = 0;
            HANDLE HProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, Process.PID);
            if(HProcess){
                FILETIME createTime, exitTime, kernelTime, userTime;
                if (GetProcessTimes(HProcess, &createTime, &exitTime, &kernelTime, &userTime)) {

                    Process.CPUTimeUser = FileTimeToULL(userTime);
                }
                PROCESS_MEMORY_COUNTERS pmc;
                if(GetProcessMemoryInfo(HProcess,&pmc,sizeof(pmc)))
                    Process.MemoryUsage = pmc.WorkingSetSize;
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

std::wstring ProcessListToString(std::vector<ProcessInfo>& ProcessList)
{
    std::wstringstream ProcessInfo;
    for (const auto& Process : ProcessList) {
        ProcessInfo << L"=============================\n";
        ProcessInfo << L"Name       : " << Process.Name << L"\n";
        ProcessInfo << L"PID        : " << Process.PID << L"\n";
        ProcessInfo << L"Parent PID : " << Process.ParentPID<< L"\n";
        ProcessInfo << L"RAM Usage  : " << Process.MemoryUsage / 1024 << L" KB\n";
        ProcessInfo << L"CPU User   : " << Process.CPUTimeUser / 10000 << L" ms\n";
    }
    std::wcout << ProcessInfo.str();
}

std::string Process::ProcessListToMessage(std::vector<ProcessInfo>& ProcessList)
{
    std::string buffer;


    uint32_t count = static_cast<uint32_t>(ProcessList.size());
    buffer.append(reinterpret_cast<char*>(&count), sizeof(count));


    for (auto& p : ProcessList)
    {
        buffer.append(reinterpret_cast<char*>(&p.PID), sizeof(p.PID));

        buffer.append(p.Name, 260);

        buffer.append(reinterpret_cast<char*>(&p.ParentPID), sizeof(p.ParentPID));

        buffer.append(reinterpret_cast<char*>(&p.MemoryUsage), sizeof(p.MemoryUsage));


        buffer.append(reinterpret_cast<char*>(&p.CPUTimeUser), sizeof(p.CPUTimeUser));
    }

    return buffer;
}

void Process::HandleRequest(SOCKET client_socket, const PacketHeader &header) {
    if (header.request_key != REQUEST_KEY) {
        std::cerr << "Invalid request key." << std::endl;
        closesocket(client_socket);
        return;
    }
    if (header.request_type == 0x01) {
        auto ProcessList = ListProcess();
        std::string response = ProcessListToMessage(ProcessList);
        Response res(header.request_id + 1, 0x00);
        res.setMessage(response);
        res.sendResponse(client_socket);
    } else if (header.request_type == 0x02) {
        int PID;
        recv(client_socket, reinterpret_cast<char*>(&PID), sizeof(PID), 0);
        bool result = TerminateProcessByID(PID);
        Response res(header.request_id + 1, result ? 0x00 : 0x01);
        if (result) {
            res.setMessage("Process terminated successfully.");
        } else {
            res.setMessage("Failed to terminate process.");
        }
        res.sendResponse(client_socket);
    } else if (header.request_type == 0x03) {
        char filePath[260];
        recv(client_socket, filePath, sizeof(filePath), 0);
        bool result = OpenFileByPath(filePath);
        Response res(header.request_id + 1, result ? 0x00 : 0x02);
        if (result) {
            res.setMessage("File opened successfully.");
        } else {
            res.setMessage("Failed to open file.");
        }
        res.sendResponse(client_socket);
    } else {
        std::cerr << "Unknown request type." << std::endl;
        Response res(header.request_id + 1, 0x03);
        res.setMessage("Unknown request type.");
        res.sendResponse(client_socket);
        return;
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
