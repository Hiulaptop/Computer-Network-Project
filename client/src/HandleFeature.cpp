#include "HandleFeature.hpp"

#include <fstream>
#include <iostream>
#include <random>
#include <unordered_map>
#include <ws2tcpip.h>

int Feature::sendRequest(SOCKET sock, int type, int size, char *data) {
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
    *data = new char[responseHeader.packageSize - sizeof(responseHeader)];
    bytesReceived = recv(sock, *data, size, 0);
    if (bytesReceived <= 0) {
        delete[] *data;
        *data = nullptr;
        std::cerr << "Failed to receive response data." << std::endl;
        return -1;
    }
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
    returnValue = -2;
    if (rParam.data == nullptr) {
        return returnValue = -1;
    }
    if (rParam.size <= 0) {
        return returnValue = -1;
    }
    SOCKET sock = init();
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to initialize socket for process feature." << std::endl;
        return returnValue = -1;
    }
    if (rParam.type == 0) {
        if (rParam.saveFilePath == nullptr || rParam.saveFileSize <= 0) {
            std::cerr << "Invalid save file path or size." << std::endl;
            return returnValue = -1;
        }
        sendRequest(sock, rParam.type, rParam.size, rParam.data);
        int responseSize = 0;
        char *responseData = nullptr;
        int res = receiveResponse(sock, responseSize, &responseData);
        if (res < 0) {
            std::cerr << "Failed to receive response for file request." << std::endl;
            return returnValue = -1;
        }
        std::ofstream outFile(rParam.saveFilePath, std::ios::binary);
        if (!outFile) {
            std::cerr << "Failed to open file for writing: " << rParam.saveFilePath << std::endl;
            delete[] responseData;
            return returnValue = -1;
        }
        outFile.write(responseData, responseSize);
        outFile.close();
        delete[] responseData;
        return returnValue = 0;
    }
    if (rParam.type == 1) {
        sendRequest(sock, rParam.type, rParam.size, rParam.data);
        int responseSize = 0;
        char *responseData = nullptr;
        int res = receiveResponse(sock, responseSize, &responseData);
        if (res < 0) {
            std::cerr << "Failed to receive response for file list request." << std::endl;
            return returnValue = -1;
        }
        std::cout << "Received file list: " << responseData << std::endl;
        delete[] responseData;
        return returnValue = 0;
    }
    std::cerr << "Invalid request type: " << rParam.type << std::endl;
    return returnValue = -1;
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
        if (responseData != nullptr) {
            delete[] responseData;
        }
        return returnValue = res;
    }
    if (res == 0x02) {
        std::cerr << "Failed to start keylogger." << std::endl;
        if (responseData != nullptr) {
            delete[] responseData;
        }
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
        if (responseData != nullptr) {
            delete[] responseData;
        }
        return returnValue = res;
    }
    if (res == 0x03) {
        std::cerr << "Keylogger is not running." << std::endl;
        if (responseData != nullptr) {
            delete[] responseData;
        }
        return returnValue = res;
    }
    if (res == 0x04) {
        std::cout << responseData << std::endl;
        if (responseData != nullptr) {
            delete[] responseData;
        }
        return returnValue = res;
    }
    if (responseData != nullptr) {
        delete[] responseData;
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
        if (responseData != nullptr) {
            delete[] responseData;
        }
        return returnValue = res;
    }
    if (res == 0x02) {
        std::cerr << "Video recording is already in progress." << std::endl;
        if (responseData != nullptr) {
            delete[] responseData;
        }
        return returnValue = res;
    }
    if (res == 0x00) {
        if (rParam.type == 1) {
            std::cout << "Video recording started successfully." << std::endl;
        } else if (rParam.type == 2) {
            std::cout << "Video recording stopped successfully." << std::endl;
        }
        if (responseData != nullptr) {
            delete[] responseData;
        }
        return returnValue = res;
    }
    if (res == 0x03) {
        std::cerr << "Video recording is not in progress." << std::endl;
        if (responseData != nullptr) {
            delete[] responseData;
        }
        return returnValue = res;
    }
    if (res == 0x04) {
        std::cout << "Invalid request type for video recording." << std::endl;
        if (responseData != nullptr) {
            delete[] responseData;
        }
        return returnValue = res;
    }
    if (responseData != nullptr) {
        delete[] responseData;
    }
    std::cerr << "Unknown response status code: " << res << std::endl;
    return -1;
}

int WindowFeature::requestingFeature(RequestParam rParam) {
    returnValue = -1;
    if (rParam.type <= 0) {
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
        if (responseData != nullptr) {
            delete[] responseData;
        }
        return returnValue = res;
    }
    if (res == 0x02) {
        std::cerr << "Unable to open process token for shutdown/restart." << std::endl;
        if (responseData != nullptr) {
            delete[] responseData;
        }
        return returnValue = res;
    }
    if (res == 0x03) {
        std::cerr << "Failed to set Master Volume." << std::endl;
        if (responseData != nullptr) {
            delete[] responseData;
        }
        return returnValue = res;
    }
    if (res == 0x00) {
        if (rParam.type == 1) {
            std::cout << "Shutdown command executed successfully." << std::endl;
        } else if (rParam.type == 2) {
            std::cout << "Restart command executed successfully." << std::endl;
        } else if (rParam.type == 3) {
            std::cout << "Master Volume set successfully." << std::endl;
        }
        if (responseData != nullptr) {
            delete[] responseData;
        }
        return returnValue = res;
    }
    std::cerr << "Unknown response status code: " << res << std::endl;
    if (responseData != nullptr) {
        delete[] responseData;
    }
    return -1;
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
    ProcessInfoNode **newChildren = new ProcessInfoNode *[childCount + 1];
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
