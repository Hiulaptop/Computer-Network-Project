#include "VideoRecording.hpp"

#include "File.hpp"
#include "Response.hpp"

HRESULT Camera::CreateCaptureDevice(int deviceIndex) {
    HRESULT hr = S_OK;
    UINT32 count = 0;
    IMFActivate **devices = nullptr;
    IMFAttributes *attributes = nullptr;
    EnterCriticalSection(&criticalSection);
    if (SUCCEEDED(hr))
        hr = MFStartup(MF_VERSION);
    if (SUCCEEDED(hr))
        hr = MFCreateAttributes(&attributes, 1);
    if (SUCCEEDED(hr))
        attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (SUCCEEDED(hr)) {
        hr = MFEnumDeviceSources(attributes, &devices, &count);
        this->deviceCount = count;
    }
    if (SUCCEEDED(hr) && count > 0) {
        for (UINT32 i = 0; i < count; i++) {
            if (i == deviceIndex) {
                hr = GetSourceReader(devices[i]);
                if (SUCCEEDED(hr)) {
                    WCHAR *nameString = nullptr;
                    UINT32 nameLength = 0;
                    hr = devices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &nameString, &nameLength);
                    if (SUCCEEDED(hr)) {
                        this->bytePerPixel = abs(stride)/width;
                        wcscpy_s(deviceName, nameString);
                    }
                    CoTaskMemFree(nameString);
                }
            }
            SafeRelease(&devices[i]);
        }
    } else {
        std::cerr << "No video capture devices found." << std::endl;
    }
    LeaveCriticalSection(&criticalSection);
    SafeRelease(&attributes);
    SafeRelease(devices);
    return hr;
}

HRESULT Camera::CreateMediaSink() {
    HRESULT hr = S_OK;
    IMFSinkWriter *writer = nullptr;
    IMFMediaType  *pOutputType = nullptr;
    IMFMediaType  *pInputType = nullptr;

    hr = MFCreateSinkWriterFromURL(L"Recording.mp4", nullptr, nullptr, &writer);

    if (SUCCEEDED(hr))
        hr = MFCreateMediaType(&pOutputType);
    if (SUCCEEDED(hr))
        hr = pOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    if (SUCCEEDED(hr))
        hr = pOutputType->SetGUID(MF_MT_SUBTYPE, VIDEO_ENCODING_FORMAT);
    if (SUCCEEDED(hr))
        hr = pOutputType->SetUINT32(MF_MT_AVG_BITRATE, VIDEO_BITRATE);
    if (SUCCEEDED(hr))
        hr = pOutputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    if (SUCCEEDED(hr))
        hr = pOutputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
    if (SUCCEEDED(hr))
        hr = MFSetAttributeSize(pOutputType, MF_MT_FRAME_SIZE, width, height);
    if (SUCCEEDED(hr))
        hr = MFSetAttributeRatio(pOutputType, MF_MT_FRAME_RATE, FRAME_RATE, 1);
    if (SUCCEEDED(hr))
        hr = MFSetAttributeRatio(pOutputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
    if (SUCCEEDED(hr))
        hr = writer->AddStream(pOutputType, &streamIndex);

    if (SUCCEEDED(hr))
        hr = MFCreateMediaType(&pInputType);
    if (SUCCEEDED(hr))
        hr = pInputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    if (SUCCEEDED(hr))
        hr = pInputType->SetGUID(MF_MT_SUBTYPE, this->m_MachineGuid);
    if (SUCCEEDED(hr))
        hr = pInputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    if (SUCCEEDED(hr))
        hr = MFSetAttributeSize(pInputType, MF_MT_FRAME_SIZE, width, height);
    if (SUCCEEDED(hr))
        hr = MFSetAttributeRatio(pInputType, MF_MT_FRAME_RATE, FRAME_RATE, 1);
    if (SUCCEEDED(hr))
        hr = MFSetAttributeRatio(pInputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
    if (SUCCEEDED(hr))
        hr = writer->SetInputMediaType(streamIndex, pInputType, nullptr);
    if (SUCCEEDED(hr))
        hr = writer->BeginWriting();
    if (SUCCEEDED(hr)) {
        sink = writer;
        sink->AddRef();
    }
    SafeRelease(&pOutputType);
    SafeRelease(&writer);

    return hr;
}

HRESULT Camera::GetSourceReader(IMFActivate *device) {
    HRESULT hr = S_OK;

    IMFMediaType *pType = nullptr;
    IMFMediaSource *pSource = nullptr;

    EnterCriticalSection(&criticalSection);
    hr = device->ActivateObject(__uuidof(IMFMediaSource), (void**)&pSource);
    if (SUCCEEDED(hr))
        hr = device->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &wSymbolicLink,
                                        &cchSymbolicLink);
    if (SUCCEEDED(hr))
        hr = MFCreateSourceReaderFromMediaSource(pSource, nullptr, &reader);
    if (SUCCEEDED(hr)) {
        for (DWORD i = 0;; i++) {
            hr = reader->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, i, &pType);
            if (FAILED(hr)) break;
            GetDefaultStride(pType);
            hr = IsMediaTypeSupported(pType);
            if (FAILED(hr)) break;
            MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, (UINT32 *) &width, (UINT32 *) &height);
            SafeRelease(&pType);
            if (hr == S_OK) break;
        }
    }
    if (SUCCEEDED(hr)) {
        IMFSample *pSample = nullptr;
        DWORD streamIndex = 0;
        DWORD flags = 0;
        hr = reader->ReadSample( MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &streamIndex, &flags, &timestamp, &pSample);
    }
    if (FAILED(hr)) {
        if (pSource)
            pSource->Shutdown();
        SafeRelease(&reader);
        CoTaskMemFree(wSymbolicLink);
        wSymbolicLink = nullptr;
        cchSymbolicLink = 0;
    }
    SafeRelease(&pSource);
    LeaveCriticalSection(&criticalSection);
    return hr;
}

HRESULT Camera::IsMediaTypeSupported(IMFMediaType *mediaType) {
    HRESULT hr = S_OK;

    GUID subtype = GUID_NULL;
    hr = mediaType->GetGUID(MF_MT_SUBTYPE, &subtype);
    if (FAILED(hr)) return hr;
    this->videoFormat = mediaType;
    if (subtype == MFVideoFormat_RGB32 || subtype == MFVideoFormat_RGB24 || subtype == MFVideoFormat_YUY2 || subtype == MFVideoFormat_NV12)
    {
        this->m_MachineGuid = subtype;
        return S_OK;
    }
    return S_FALSE;
}

HRESULT Camera::GetDefaultStride(IMFMediaType *mediaType) {
    LONG tStride = 0;
    HRESULT hr = S_OK;
    hr = mediaType->GetUINT32(MF_MT_DEFAULT_STRIDE, (UINT32*)&tStride);
    if (FAILED(hr)) {
        GUID subtype = GUID_NULL;
        UINT32 twidth = 0, theight = 0;
        hr = mediaType->GetGUID(MF_MT_SUBTYPE, &subtype);
        if (SUCCEEDED(hr))
            hr = MFGetAttributeSize(mediaType, MF_MT_FRAME_SIZE, &twidth, &theight);
        if (SUCCEEDED(hr))
            hr = MFGetStrideForBitmapInfoHeader(subtype.Data1, twidth, &tStride);
        if (SUCCEEDED(hr))
            hr = mediaType->SetUINT32(MF_MT_DEFAULT_STRIDE, tStride);
    }
    if (SUCCEEDED(hr)) {
        stride = tStride;
    }
    return hr;
}

HRESULT Camera::WriteFrame(IMFSample *sample) {
    HRESULT hr = sample->SetSampleTime(timestamp);
    if (SUCCEEDED(hr))
        hr = sample->SetSampleDuration(FRAME_DURATION);
    if (SUCCEEDED(hr)) {
        hr = sink->WriteSample(streamIndex, sample);
        frameCount++;
    }
    return hr;
}

Camera::Camera() {
    InitializeCriticalSection(&criticalSection);
}

void Camera::Capture() {
    HRESULT hr = CreateCaptureDevice(0);
    DWORD streamFlags = 0, actualStreamIndex = 0;
    IMFSample *sample = nullptr;
    LONGLONG firstTimestamp = 0;
    bool isFirstFrame = true;

    if (FAILED(hr)) {
        std::cerr << "Failed to create capture device: " << std::hex << hr << std::endl;
        return;
    }
    std::wcout << "Using device: " << deviceName << std::endl;
    std::cout << "Height: " << height << ", Width: " << width << ", Stride: " << stride << std::endl;
    hr = CreateMediaSink();
    if (FAILED(hr)) {
        std::cerr << "Failed to create media sink: " << std::hex << hr << std::endl;
        return;
    }
    isCapturing = true;
    do {
        hr = reader->ReadSample((DWORD) MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &actualStreamIndex, &streamFlags, &timestamp, &sample);
        if (FAILED(hr)) {
            std::cerr << "Failed to read sample: " << std::hex << hr << std::endl;
            break;
        }
        if (streamFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
            std::cout << "Stream ended." << std::endl;
            break;
        }
        if (sample) {
            if (isFirstFrame) {
                firstTimestamp = timestamp;
                isFirstFrame = false;
            }
            timestamp -= firstTimestamp;
            hr = WriteFrame(sample);
            SafeRelease(&sample);
            if (FAILED(hr)) {
                std::cerr << "Failed to write sample: " << std::hex << hr << std::endl;
                break;
            }
        }
    }while (isCapturing);

    sink->Finalize();
    SafeRelease(&reader);
    SafeRelease(&sink);
    CoTaskMemFree(wSymbolicLink);
    wSymbolicLink = nullptr;
    cchSymbolicLink = 0;
    DeleteCriticalSection(&criticalSection);
    isStopping = true;
    std::cout << "Recording finished. Total frames: " << frameCount << std::endl;
}

void Camera::StopCapture() {
    isCapturing = false;
    while (!isStopping){}
}

void VideoRecording::HandleRequest(SOCKET client_socket, const PacketHeader &header) {
    if (header.request_key != REQUEST_KEY) {
        Response response(header.request_id, 0x01);
        response.sendResponse(client_socket);
        return;
    }
    if (header.request_type == 0x01) {
        if (isRecording) {
            Response response(header.request_id, 0x02);
            response.sendResponse(client_socket);
            return;
        }
        CreateThread(
            nullptr,
            0,
            LPTHREAD_START_ROUTINE(&VideoRecording::StartRecording),
            nullptr,
            0,
            nullptr
        );
        Response response(header.request_id, 0x00);
        response.sendResponse(client_socket);
    }
    else if (header.request_type == 0x02) {
        if (!isRecording || camera == nullptr) {
            Response response(header.request_id, 0x03);
            response.sendResponse(client_socket);
            return;
        }
        camera->isStopping = false;
        camera->StopCapture();
        delete camera;
        camera = nullptr;
        isRecording = false;
        FILE *file = fopen(VIDEO_FILENAME.c_str(), "rb");
        if (!file)
        {
            printf("Error while reading the file\n");
            getchar();
            return;
        }

        fseek(file, 0, SEEK_END);
        unsigned long Size = ftell(file);
        fseek(file, 0, SEEK_SET);
        char *Buffer = new char[Size + 1];
        fread(Buffer, Size, 1, file);

        fclose(file);
        ResponseHeader responseHeader( Size + sizeof(ResponseHeader), header.request_id + 1, 0x00);
        send(client_socket, (char*)&responseHeader, sizeof(responseHeader), 0);
        int sent = send(client_socket, Buffer, Size, 0);
        while (sent < Size) {
            int bytesSent = send(client_socket, Buffer + sent, Size - sent, 0);
            if (bytesSent <= 0) {
                std::cerr << "Failed to send video data." << std::endl;
                break;
            }
            sent += bytesSent;
        }
        delete[] Buffer;
    }
    else {
        Response response(header.request_id, 0x04);
        response.sendResponse(client_socket);
    }
}

std::atomic_bool VideoRecording::isRecording = false;
Camera *VideoRecording::camera = nullptr;

DWORD VideoRecording::StartRecording(LPVOID *lpParam) {
    camera = new Camera();
    isRecording = true;
    camera->Capture();
    std::cout << "Video recording completed." << std::endl;
    return 0;
}
