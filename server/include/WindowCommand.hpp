#pragma once
#include <iostream>
#include <cstdlib>
#include "RequestHandler.hpp"
#include <gdiplus.h>
#include <propidl.h>
#include "Response.hpp"
#include "WindowCommand.hpp"

#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <iostream> // For output

class WindowCommand: public FeatureHandler
{
public:
    //static void SaveBitmapToFile(HBITMAP HBitmap,const PacketHeader& header);
    static std::string GetDIBitsBMP(HBITMAP hBitmap, int width, int height);
    static void ScreenShot(SOCKET& client_socket,const PacketHeader& header);
    static void ShutDown(SOCKET& client_socket,const PacketHeader& header);
    static void Restart(SOCKET& client_socket,const PacketHeader& header);
    static void SetVolume(const float absolute, SOCKET& client_socket,const PacketHeader& header);
    void HandleRequest(SOCKET client_socket, const PacketHeader &header) override;
};