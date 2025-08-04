#pragma once
#include <atomic>
#include <string>
#include <iostream>

#include "RequestHandler.hpp"

#include <mfapi.h>
#include <mfidl.h>
#include "Shlwapi.h"
#include "mfreadwrite.h"
#include <mfobjects.h>
#include <mferror.h>
#include <Dbt.h>
#include <ks.h>
#include <ksmedia.h>

class Camera {
public:
    int frameCount = 0;
    const int FRAME_RATE = 30;
    const LONGLONG FRAME_DURATION = 10000000 / FRAME_RATE;
    const UINT32 VIDEO_BITRATE = 8000000;
    const GUID VIDEO_ENCODING_FORMAT = MFVideoFormat_H264;
    IMFSourceReader* reader = nullptr;
    IMFSinkWriter* sink = nullptr;
    IMFMediaType* videoFormat = nullptr;
    BYTE* raw = nullptr;
    LONG stride = 0;
    LONG bytePerPixel = 0;
    WCHAR deviceName[2048] = {0};
    UINT32 deviceCount = 0;
    LONG width = 0;
    LONG height = 0;
    WCHAR* wSymbolicLink = nullptr;
    UINT32 cchSymbolicLink = 0;
    CRITICAL_SECTION criticalSection = {nullptr};
    DWORD streamIndex = 0;
    LONGLONG timestamp = 0;
    HRESULT CreateCaptureDevice(int deviceIndex);
    HRESULT CreateMediaSink();
    HRESULT GetSourceReader(IMFActivate *device);
    HRESULT IsMediaTypeSupported(IMFMediaType *mediaType);
    HRESULT GetDefaultStride(IMFMediaType *mediaType);
    HRESULT WriteFrame(IMFSample *sample);
public:
    Camera();
    void Capture(int second);
};

class VideoRecording : public FeatureHandler {
public:
    void HandleRequest(SOCKET client_socket, const PacketHeader &header) override;

private:
    constexpr static int REQUEST_KEY = 0x03;
    constexpr static std::string VIDEO_FILENAME = "recording.mp4";
    static HANDLE hThread;
    static DWORD dwThreadID;
    static void StartRecording(SOCKET client_socket,const PacketHeader &header, int seconds);
    static std::atomic_bool isRecording;
    static Camera *camera;
};

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}