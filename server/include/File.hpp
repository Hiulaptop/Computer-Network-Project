#pragma once
#include <iostream>
#include <stdio.h>

#include "RequestHandler.hpp"
#include "Response.hpp"

class File : public FeatureHandler, public Response
{
public:
    constexpr static int REQUEST_KEY = 0x02;
    void HandleRequest(SOCKET clientSocket,const PacketHeader& header) override;
    static void SendFile(const char* PathName, SOCKET clientSocket,const PacketHeader& header);
};
