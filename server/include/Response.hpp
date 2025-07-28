#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <winsock2.h>

class Response {
protected:
    uint16_t responseID;
    uint16_t statusCode;
    uint32_t messageLength;
    char *message;

public:
    ~Response() {
        delete[] message;
        message = nullptr;
    }

    Response(const uint16_t responseID, const uint16_t statusCode): responseID(responseID), statusCode(statusCode),
                                                                    messageLength(0),
                                                                    message(nullptr) {
    }

    void setStatusCode(const uint16_t code) {
        statusCode = code;
    }

    uint16_t getStatusCode() const {
        return statusCode;
    }

    void setResponseID(const uint16_t id) {
        responseID = id;
    }

    uint16_t getResponseID() const {
        return responseID;
    }

    void setMessage(const std::string &msg) {
        messageLength = static_cast<uint32_t>(msg.size());
        message = new char[messageLength + 1];
        std::strcpy(message, msg.c_str());
    }

    const char *getMessage() const {
        return message;
    }

    uint32_t getMessageLength() const {
        return messageLength;
    }

    void sendResponse(const SOCKET clientSocket) const {
        if (clientSocket == INVALID_SOCKET || message == nullptr) {
            return;
        }
        const uint32_t totalSize = sizeof(responseID) + sizeof(statusCode) + sizeof(messageLength) + messageLength;
        const auto buffer = new char[totalSize];
        std::memcpy(buffer, &responseID, sizeof(responseID));
        std::memcpy(buffer + sizeof(responseID), &statusCode, sizeof(statusCode));
        std::memcpy(buffer + sizeof(responseID) + sizeof(statusCode), &messageLength, sizeof(messageLength));
        std::memcpy(buffer + sizeof(responseID) + sizeof(statusCode) + sizeof(messageLength), message, messageLength);
        send(clientSocket, buffer, totalSize, 0);
        delete[] buffer;
    }
};
