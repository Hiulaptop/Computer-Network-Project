#include "VideoRecording.hpp"

#include <iostream>
#include <opencv2/opencv.hpp>

HRESULT Media::CreateCaptureDevice() {
    HRESULT hr = S_OK;

    hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    UINT32 count = 0;
    IMFAttributes* attributes = nullptr;
    IMFActivate** devices = nullptr;
    if (FAILED(hr)) {
        CLEAN_ATTRIBUTES()
    }

    hr = MFCreateAttributes(&attributes, 1);

    if (FAILED(hr)) {
        CLEAN_ATTRIBUTES()
    }

    hr = attributes->SetGUID(
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
    );
    if (FAILED(hr)) {
        CLEAN_ATTRIBUTES()
    }
    hr = MFEnumDeviceSources(attributes, &devices, &count);
    deviceCount = count;
    if (FAILED(hr)) {
        CLEAN_ATTRIBUTES()
    }
    if (count > 0) {
        SetSourceReader(devices[deviceIndex]);
        WCHAR *nameString = nullptr;
        UINT32 cchName = 0;
        hr = devices[deviceIndex]->GetAllocatedString(
            MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
            &nameString,
            &cchName
        );
        if (SUCCEEDED(hr)) {
            bytePerPixel = abs(stride)/width;
            raw = new BYTE[width * height * bytePerPixel];
            wcscpy(deviceName, nameString);
        }
        CoTaskMemFree(nameString);
    }
    CLEAN_ATTRIBUTES()
}

HRESULT Media::SetSourceReader(IMFActivate *device) {
    HRESULT hr = S_OK;

    IMFMediaSource *source = nullptr;
    IMFAttributes* attributes = nullptr;
    IMFMediaType *mediaType = nullptr;

    EnterCriticalSection(&criticalSection);

    hr = device->ActivateObject(__uuidof(IMFMediaSource), reinterpret_cast<void **>(&source));

    if (SUCCEEDED(hr))
        hr = device->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_SYMBOLIC_LINK, &wSymbolicLink, &cchSymbolicLink);

    if (SUCCEEDED(hr))
        hr = MFCreateAttributes(&attributes, 2);

    if (SUCCEEDED(hr))
        hr = attributes->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, TRUE);

    if (SUCCEEDED(hr))
        hr = attributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, this);

    if (SUCCEEDED(hr))
        hr = MFCreateSourceReaderFromMediaSource(source, attributes, &reader);

    if (SUCCEEDED(hr)) {
        for (DWORD i = 0;; i++) {
            hr = reader->GetNativeMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), i, &mediaType);
            if (FAILED(hr)) {break;}
            GetDefaultStride(mediaType, &stride);
            hr = IsMediaTypeSupported(mediaType);
            if (FAILED(hr)) {break;}
            MFGetAttributeSize(mediaType, MF_MT_FRAME_RATE, &width, &height);
            SafeRelease(&mediaType);
            if (hr == S_OK)
                break;
        }
    }
    if (SUCCEEDED(hr))
        hr = reader->ReadSample(static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), 0 ,nullptr, nullptr, nullptr, nullptr);
    if (FAILED(hr)) {
        if (source) {
            source->Shutdown();
        }
        Close();
    }
    if (source){source->Release(); source = nullptr;}
    if (attributes) {attributes->Release(); attributes = nullptr;}
    if (mediaType) {mediaType->Release(); mediaType = nullptr;}
    LeaveCriticalSection(&criticalSection);
    return hr;
}

HRESULT Media::IsMediaTypeSupported(IMFMediaType *pType) {
    HRESULT hr = S_OK;
    GUID subtype = {0};
    hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);
    videoFormat = subtype;
    if (FAILED(hr)) return hr;
    if (subtype == MFVideoFormat_RGB24) {
        return S_OK;
    }
    return S_FALSE;
}

HRESULT Media::GetDefaultStride(IMFMediaType *pType, LONG *pStride) {
    LONG tStride = 0;

    HRESULT hr = pType->GetUINT32(MF_MT_DEFAULT_STRIDE, reinterpret_cast<UINT32 *>(&tStride));
    UINT32 width = 0, height = 0;
    if (FAILED(hr)) {
        GUID subtype = {0};
        hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);
        if (SUCCEEDED(hr)) hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height);
        if (SUCCEEDED(hr)) hr = MFGetStrideForBitmapInfoHeader(subtype.Data1, width, &tStride);
        if (SUCCEEDED(hr)) (void)pType->SetUINT32(MF_MT_DEFAULT_STRIDE, tStride);
    }
    if (SUCCEEDED(hr))
        stride = tStride;
    return hr;
}

HRESULT Media::Close() {
    EnterCriticalSection(&criticalSection);
    SafeRelease(&reader);

    CoTaskMemFree(wSymbolicLink);
    wSymbolicLink = nullptr;
    cchSymbolicLink = 0;
    LeaveCriticalSection(&criticalSection);
    return S_OK;
}

Media::Media(int index): stride(0), bytePerPixel(0), videoFormat(), deviceName{}, deviceCount(0) {
    InitializeCriticalSection(&criticalSection);
    refCount = 1;
    wSymbolicLink = nullptr;
    cchSymbolicLink = 0;
    width = 0;
    height = 0;
    reader = nullptr;
    raw = nullptr;
    deviceIndex = index;
}

Media::~Media() {
    if (wSymbolicLink) {
        delete wSymbolicLink;
        wSymbolicLink = nullptr;
    }
    EnterCriticalSection(&criticalSection);

    SafeRelease(&reader);

    if (raw) {
        delete raw;
        raw = nullptr;
    }

    CoTaskMemFree(wSymbolicLink);
    wSymbolicLink = nullptr;
    cchSymbolicLink = 0;

    LeaveCriticalSection(&criticalSection);
    DeleteCriticalSection(&criticalSection);
}

HRESULT Media::QueryInterface(const IID &riid, void **ppv) {
    static const QITAB qit[] = {QITABENT(Media, IMFSourceReaderCallback),{ nullptr },};
    return QISearch(this, qit, riid, ppv);
}

ULONG Media::Release() {
    ULONG count = InterlockedDecrement(&refCount);
    if (count == 0) {
        delete this;
    }
    return count;
}

STDMETHODIMP Media::OnReadSample(HRESULT status, DWORD streamIndex, DWORD streamFlags, LONGLONG timeStamp,
    IMFSample *sample) {
    HRESULT hr = S_OK;
    IMFMediaBuffer *buffer = nullptr;

    EnterCriticalSection(&criticalSection);
    if (FAILED(status)) hr = status;
    if (SUCCEEDED(hr)) {
        if (sample) {
            hr = sample->GetBufferByIndex(0, &buffer);
            if (SUCCEEDED(hr)) {
                BYTE *data = nullptr;
                buffer->Lock(&data, nullptr, nullptr);
                // Save data here
                CopyMemory(raw, data, width * height * bytePerPixel);
                buffer->Unlock();
            }
        }
    }
    if (SUCCEEDED(hr)) reader->ReadSample(
        static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM),
        0, nullptr, nullptr, nullptr, nullptr
    );
    if (FAILED(hr)) {
        std::cout << "Failed to read sample: " << std::hex << hr << std::endl;
    }
    SafeRelease(&buffer);
    LeaveCriticalSection(&criticalSection);
    return hr;
}

STDMETHODIMP Media::OnEvent(DWORD streamIndex, IMFMediaEvent *event) {
    return S_OK;
}

STDMETHODIMP Media::OnFlush(DWORD streamIndex) {
    return S_OK;
}

void VideoRecording::HandleRequest(SOCKET client_socket, const PacketHeader &header) {
}

Media *VideoRecording::media = nullptr;
std::atomic_bool VideoRecording::isRecording = false;

void VideoRecording::StartRecording() {
    media = new Media(1);
    HRESULT hr = media->CreateCaptureDevice();
    if (FAILED(hr)) {
        std::cerr << "Failed to create capture device: " << std::hex << hr << std::endl;
        delete media;
        media = nullptr;
        return;
    }
    isRecording = true;
}

void VideoRecording::StopRecording() {
}

void VideoRecording::SendVideoFile(SOCKET client_socket, const PacketHeader &header) {
}

