#pragma once
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <winsock2.h>

struct ResponseHeader {
    uint32_t packageSize;
    uint16_t responseID;
    uint16_t statusCode;
};

class Response {
protected:
    ResponseHeader header;
    char *message;
    int messageLength = 0;

public:
    ~Response() {
        delete[] message;
        message = nullptr;
    }

    Response(const uint16_t responseID, const uint16_t statusCode): header(0,responseID,statusCode),
                                                                    message(nullptr) {
    }

    void setStatusCode(const uint16_t code) {
        header.statusCode = code;
    }

    uint16_t getStatusCode() const {
        return header.statusCode;
    }

    void setResponseID(const uint16_t id) {
        header.responseID = id;
    }

    uint16_t getResponseID() const {
        return header.responseID;
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

    void sendResponse(const SOCKET clientSocket) {
        if (clientSocket == INVALID_SOCKET) {
            return;
        }
        header.packageSize = sizeof(header) + messageLength;
        send(clientSocket, (char*)&header, sizeof(header), 0);
        if (messageLength == 0 || message == nullptr) {
            return;
        }
        int sentBytes = 0;
        sentBytes = send(clientSocket, message, messageLength, 0);
        if (sentBytes != messageLength) {
            std::cerr << "Failed to send complete response. Sent: " << sentBytes << ", Expected: " << messageLength << std::endl;
        }
    }
};
