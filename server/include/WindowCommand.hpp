#pragma once
#include <iostream>
#include <cstdlib>
#include "RequestHandler.hpp"
#include <gdiplus.h>
#include <propidl.h>
#include "Response.hpp"
class WindowCommand: public FeatureHandler
{
public:
    static void SaveBitmapToFile(HBITMAP HBitmap, const wchar_t* filename);
    static void ScreenShot(const wchar_t* filename);
    static void ShutDown();
    static void Restart();
    void HandleRequest(SOCKET client_socket, const PacketHeader &header) override;
};