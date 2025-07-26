#pragma once
#include <cstdint>

class Response {
private:
    uint16_t statusCode;
    uint32_t dataSize;
    char *data;
public:
};
